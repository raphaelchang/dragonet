#include "dragonet.h"

#include <map>
#include <function>
#include <vector>

namespace dragonet {

class DragonetImpl
{
public:
    DragonetImpl()
    {
    }

    template<class MessageType>
    void RegisterSubscription(const char *channel, void (*callback)(const MessageType *msg), int queue_size)
    {
    }

    template<class MessageType>
    void PublishMessage(const char *channel, const MessageType *msg)
    {
    }

    void DispatchCallbacks()
    {
    }

    static void RegisterPublishEndpoint(const char *channel, unsigned long dst)
    {
    }

private:
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
