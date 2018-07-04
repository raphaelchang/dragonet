#pragma once

#include <memory>
#include <functional>

namespace dragonet
{

class DragonetImpl;

class Dragonet
{
public:
    Dragonet();
    ~Dragonet();

    template<class MessageType>
    int Publish(const char *channel, const MessageType *msg)
    {
        char buf[256];
        msg->encode((char*) buf, 0, msg->getEncodedSize());
        return serializeAndPublish(channel, (char*) buf, msg->getEncodedSize());
    }

    template<class MessageType>
    void Subscribe(const char *channel, void (*callback)(const MessageType *msg), int queue_size=10)
    {
        std::function<void(char*)> callback_raw = [&](char *data) {
            MessageType msg;
            msg.decode(data, 0, msg.getEncodedSize());
            callback(&msg);
        };
        MessageType msg;
        subscribeSerialized(channel, callback_raw, msg.getEncodedSize(), queue_size);
    }

    void Spin();

private:
    int serializeAndPublish(const char *channel, const char *msg, int size);
    void subscribeSerialized(const char *channel, std::function<void(char*)> callback, int msg_size, int queue_size);

    std::unique_ptr<DragonetImpl> impl_;
};

}
