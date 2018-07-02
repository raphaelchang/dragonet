#include "dragonet.h"

#include "FreeRTOS.h"
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

#include <vector>
#include <tuple>

namespace dragonet {

class IntraImpl
{
}

class InterImpl
{
public:
    InterImpl()
    {
        rpmsg_ = rpmsg_lite_remote_init(rpmsg_lite_base_, RL_PLATFORM_IMX6SX_M4_LINK_ID, RL_NO_FLAGS);
        while(!rpmsg_lite_is_link_up(rpmsg_))
        {
            vTaskDelay(100);
        }
    }

    template<class MessageType>
    void RegisterSubscription(const char *channel, void (*callback)(const MessageType *msg))
    {
        rpmsg_queue_handle q = rpmsg_queue_create(rpmsg_);
        rpmsg_lite_endpoint *ept = rpmsg_lite_create_ept(rpmsg_, RL_ADDR_ANY, rpmsg_queue_rx_cb, q);
        rpmsg_ns_announce(my_rpmsg, ept, channel, RL_NS_CREATE);
        subscribers.push_back(std::make_tuple(q, [](char *data, int size) {
                        MessageType msg; 
                        msg.decode(data, 0, size);
                        callback(&msg);
                        }));
    }

    void DispatchCallbacks()
    {
        for (auto &sub : subscribers)
        {
            unsigned long src;
            char buf[256];
            int size;
            if (rpmsg_queue_recv(rpmsg_, std::get<0>(sub), &src, (char *)buf, 256, &size, 0) == RL_SUCCESS)
                std::get<1>(sub)(buf, size);
        }
    }

private:
    const void *rpmsg_lite_base_{BOARD_SHARED_MEMORY_BASE};
    struct rpmsg_lite_instance *rpmsg_;
    std::vector<std::tuple<rpmsg_queue_handle, std::function<void(char*, int)>> subscribers;
}

Dragonet::Dragonet()
    : intra_(new IntraImpl()),
      inter_(new InterImpl())
{
}

Dragonet::~Dragonet()
{
}

template<class MessageType>
int Dragonet::Publish(const char *channel, const MessageType *msg)
{
}

template<class MessageType>
void Dragonet::Subscribe(const char *channel, void (*callback)(const MessageType *msg), int queue_size=10)
{
}
int Dragonet::Spin()
{
    while(true)
    {
    }
}

}
