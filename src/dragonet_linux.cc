#include "dragonet.h"

#include <lcm/lcm-cpp.hpp>

#include <sys/ioctl.h>
#include <linux/rpmsg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#include <string>
#include <map>
#include <functional>
#include <vector>

namespace dragonet {

#define MAX_EVENTS 100
#define RPMSG_ADDR_ANY 0xFFFFFFFF

class DragonetImpl
{
public:
    DragonetImpl()
    {
        epoll_fd_ = epoll_create1(0);
        if (!lcm_.good())
        {
            // TODO: panic
        }
        fd_callbacks_[lcm_.getFileno()] = std::bind(&lcm::LCM::handle, lcm_);
        addFileDescriptorToEpoll(lcm_.getFileno());
    }

    void RegisterSubscription(const char *channel, std::function<void(char*)> callback, int msg_size, int queue_size)
    {
        lcm_.subscribeFunction(std::string(channel), lcmCallback, &callback);

        struct rpmsg_endpoint_info info;
        strcpy(info.name, channel);
        info.src = RPMSG_ADDR_ANY;
        info.dst = 0;
        int fd = open("/dev/rpmsg_ctrl0", O_RDWR);
        if (fd == -1)
        {
            return;
        }
        ioctl(fd, RPMSG_CREATE_EPT_IOCTL, &info);
        close(fd);
        // TODO: get device file from sysfs
        // ...
        //open("", O_RDONLY);
        fd_callbacks_[fd] = [&]() {
            char buf[256];
            int len = read(fd, (char*) buf, msg_size);
            if (len == msg_size)
            {
                callback((char*) buf);
            }
        };
        addFileDescriptorToEpoll(fd);
    }

    void PublishMessage(const char *channel, const char *msg, int size)
    {
        // Intra-core subscribers
        lcm_.publish(std::string(channel), msg, size);

        // Inter-core subscribers
        // TODO: search through device files for matching channel
        // ...
        //open(fd, O_WRONLY);
        //write(fd, msg, size);
        //close(fd);
    }

    void DispatchCallbacks()
    {
        struct epoll_event events[MAX_EVENTS];
        int event_count = epoll_wait(epoll_fd_, events, MAX_EVENTS, 30000);
        for(int i = 0; i < event_count; i++)
        {
            fd_callbacks_[events[i].data.fd]();
        }
    }

private:
    void addFileDescriptorToEpoll(int fd)
    {
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = fd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, 0, &event);
    }

    static void lcmCallback(const lcm::ReceiveBuffer* rbuf, const std::string& channel, std::function<void(char*)> *callback)
    {
        (*callback)((char*) rbuf->data);
    };

    lcm::LCM lcm_;
    std::map<int, std::function<void(void)>> fd_callbacks_;
    int epoll_fd_;
};

Dragonet::Dragonet()
    : impl_(new DragonetImpl())
{
}

Dragonet::~Dragonet() = default;

void Dragonet::Spin()
{
    while(true)
    {
        impl_->DispatchCallbacks();
    }
}

int Dragonet::serializeAndPublish(const char *channel, const char *msg, int size)
{
    impl_->PublishMessage(channel, (char*) msg, size);
    return 0;
}

void Dragonet::subscribeSerialized(const char *channel, std::function<void(char*)> callback, int msg_size, int queue_size)
{
    impl_->RegisterSubscription(channel, callback, msg_size, queue_size);
}

}
