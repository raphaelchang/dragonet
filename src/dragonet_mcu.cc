#include "dragonet.h"

#include "FreeRTOS.h"
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

#include <map>
#include <function>
#include <vector>

namespace dragonet {

static void publisher_ept_rx_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
}

static void subscriber_ept_rx_cb(void *payload, int payload_len, unsigned long src, void *queues)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    for (std::tuple<QueueHandle_t, TaskHandle_t> &qt : *(std::vector<std::tuple<QueueHandle_t, TaskHandle_t>>*) queues)
    {
        BaseType_t higherPriorityTaskWokenCur = pdFALSE;
        xQueueSendFromISR(std::get<0>(qt), payload, higherPriorityTaskWokenCur);
        if (higherPriorityTaskWokenCur == pdTRUE)
        {
            higherPriorityTaskWoken = pdTRUE;
        }
    }
    if (higherPriorityTaskWoken == pdTRUE)
    {
        taskYIELD();
    }
}

static void publisher_ns_new_ept_cb(unsigned int new_ept, const char *new_ept_name, unsigned long flags)
{
    if (flags == RL_NS_CREATE)
    {
        DragonetImpl::RegisterPublishEndpoint(new_ept_name, new_ept);
    }
}

class DragonetImpl
{
public:
    DragonetImpl()
    {
        subscribers_mutex_ = xSemaphoreCreateMutexStatic(&subscribers_mutex_buffer_);
        queue_set_map_mutex_ = xSemaphoreCreateMutexStatic(&queue_set_map_mutex_buffer_);
        subscriber_queues_mutex_ = xSemaphoreCreateMutexStatic(&subscriber_queues_mutex_buffer_);
        publish_epts_mutex_ = xSemaphoreCreateMutexStatic(&publish_epts_mutex_buffer_);
        rpmsg_ = rpmsg_lite_remote_init(rpmsg_lite_base_, RL_PLATFORM_IMX6SX_M4_LINK_ID, RL_NO_FLAGS);
        rpmsg_ns_bind(rpmsg_, publisher_ns_new_ept_cb);
        while(!rpmsg_lite_is_link_up(rpmsg_))
        {
            vTaskDelay(100);
        }
    }

    template<class MessageType>
    void RegisterSubscription(const char *channel, void (*callback)(const MessageType *msg), int queue_size)
    {
        TaskHandle_t t = xTaskGetCurrentTaskHandle();
        if (xSemaphoreTake(queue_set_map_mutex_, (TickType_t) 1000) == pdTRUE)
        {
            if (queue_set_map_.find(t) == queue_set_map_.end())
            {
                QueueSetHandle_t qs = xQueueCreateSet(1024);
                queue_set_map_[t] = qs;
            }
        }
        else
        {
            return;
        }
        xSemaphoreGive(queue_set_map_mutex_);
        MessageType msg;
        QueueHandle_t q = xQueueCreate(queue_size, msg.getEncodedSize());
        xQueueAddToSet(q, queue_set_map_[t]);
        if (xSemaphoreTake(subscriber_queues_mutex_, (TickType_t) 1000) == pdTRUE)
        {
            if (subscriber_queues_.find(std::string(channel)) == subscriber_queues_.end())
            {
                subscriber_queues_[std::string(channel)] = std::vector<std::tuple<QueueHandle_t, TaskHandle_t>>();
                rpmsg_lite_endpoint *ept = rpmsg_lite_create_ept(rpmsg_, RL_ADDR_ANY, subscriber_ept_rx_cb, &subscriber_queues_[std::string(channel)]);
                rpmsg_ns_announce(rpmsg_, ept, channel, RL_NS_CREATE);
            }
            subscriber_queues_[std::string(channel)].push_back(std::make_tuple(q, t))
        }
        else
        {
            return;
        }
        xSemaphoreGive(subscriber_queues_mutex_);
        if (xSemaphoreTake(subscribers_mutex_, (TickType_t) 1000) == pdTRUE)
        {
            subscribers_[q] = [](char *data) {
                MessageType msg;
                msg.decode(data, 0, msg.getEncodedSize());
                callback(&msg);
            };
        }
        else
        {
            return;
        }
        xSemaphoreGive(subscribers_mutex_);
    }

    template<class MessageType>
    void PublishMessage(const char *channel, const MessageType *msg)
    {
        TaskHandle_t t = xTaskGetCurrentTaskHandle();
        char buf[256];
        msg.encode((char*) buf, 0, msg.getEncodedSize());
        // Intra-core subscribers
        for (std::tuple<QueueHandle_t, TaskHandle_t> &qt : subscriber_queues_)
        {
            // Intra-task subscribers
            if (std::get<1>(qt) == t)
            {
                subscribers_[std::get<0>(qt)]((char*) buf);
            }
            // Inter-task subscribers
            else
            {
                xQueueSend(std::get<0>(qt), (char*) buf, 0);
            }
        }
        // Inter-core subscribers
        if (publish_epts_.find(std::string(channel)) != publish_epts_.end())
        {
            for (std::tuple<rpmsg_lite_endpoint*, unsigned long> &ept : publish_epts_[std::string(channel)])
            {
                rpmsg_lite_send(rpmsg_, std::get<0>(ept), std::get<1>(ept), (char*) buf, msg.getEncodedSize(), 0);
            }
        }
    }

    void DispatchCallbacks()
    {
        TaskHandle_t t = xTaskGetCurrentTaskHandle();
        QueueHandle_t q = xQueueSelectFromSet(queue_set_map_[t], 0xFFFF);
        if (q != NULL)
        {
            char buf[256];
            xQueueReceive(q, (char*) buf, 0);
            subscribers_[q]((char*) buf);
        }
    }

    static void RegisterPublishEndpoint(const char *channel, unsigned long dst)
    {
        // TODO: make thread safe
        rpmsg_lite_endpoint *ept = rpmsg_lite_create_ept(rpmsg_, RL_ADDR_ANY, publisher_ept_rx_cb, NULL);
        if (publish_epts_.find(std::string(channel)) == publish_epts_.end())
        {
            publish_epts_[std::string(channel)] = std::vector<std::tuple<rpmsg_lite_endpoint*, unsigned long>>();
        }
        publish_epts_[std::string(channel)].push_back(std::make_tuple(ept, dst));
    }

private:
    const void *rpmsg_lite_base_{BOARD_SHARED_MEMORY_BASE};
    static struct rpmsg_lite_instance *rpmsg_;
    std::map<QueueHandle_t, std::function<void(char*)> subscribers_;
    std::map<TaskHandle_t, QueueSetHandle_t> queue_set_map_;
    std::map<std::string, std::vector<std::tuple<QueueHandle_t, TaskHandle_t>>> subscriber_queues_;
    static std::map<std::string, std::vector<std::tuple<rpmsg_lite_endpoint*, unsigned long>>> publish_epts_;

    StaticSemaphore_t subscribers_mutex_buffer_;
    SemaphoreHandle_t subscribers_mutex_;
    StaticSemaphore_t queue_set_map_mutex_buffer_;
    SemaphoreHandle_t queue_set_map_mutex_;
    StaticSemaphore_t subscriber_queues_mutex_buffer_;
    SemaphoreHandle_t subscriber_queues_mutex_;
    static StaticSemaphore_t publish_epts_mutex_buffer_;
    static SemaphoreHandle_t publish_epts_mutex_;
}

Dragonet::Dragonet()
    : impl_(new DragonetImpl())
{
}

Dragonet::~Dragonet()
{
}

template<class MessageType>
int Dragonet::Publish(const char *channel, const MessageType *msg)
{
    impl_->PublishMessage(channel, msg);
    return 0;
}

template<class MessageType>
void Dragonet::Subscribe(const char *channel, void (*callback)(const MessageType *msg), int queue_size=10)
{
    impl_->RegisterSubscription(channel, callback, queue_size);
}

void Dragonet::Spin()
{
    while(true)
    {
        impl_->DispatchCallbacks();
    }
}

}
