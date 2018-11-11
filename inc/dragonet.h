#pragma once

#include <memory>
#include <cstring>
#include <functional>
#include <cstdio>

#define MAX_MESSAGE_SIZE 256

namespace dragonet
{

class DragonetImpl;

class Dragonet
{
public:
    Dragonet();
    ~Dragonet();

    void Init();

    template<class MessageType>
    int Publish(std::string channel, const MessageType *msg)
    {
        truncateChannelName(channel);
        char buf[MAX_MESSAGE_SIZE];
        std::memcpy(buf, msg, sizeof(MessageType));
        return serializeAndPublish(channel.c_str(), (char*) buf, sizeof(MessageType));
    }

    template<class MessageType>
    void Subscribe(std::string channel, void(*callback)(const MessageType*), int queue_size=10)
    {
        truncateChannelName(channel);
        std::function<void(char*)> callback_raw = [=](char *data) {
            MessageType msg;
            std::memcpy(&msg, data, sizeof(MessageType));
            callback(&msg);
        };
        subscribeSerialized(channel.c_str(), callback_raw, sizeof(MessageType), queue_size);
    }

    template<class MessageType, class T>
    void Subscribe(std::string channel, void(T::*callback)(const MessageType*), T *const obj, int queue_size=10)
    {
        truncateChannelName(channel);
        std::function<void(char*)> callback_raw = [=](char *data) {
            MessageType msg;
            std::memcpy(&msg, data, sizeof(MessageType));
            (obj->*callback)(&msg);
        };
        subscribeSerialized(channel.c_str(), callback_raw, sizeof(MessageType), queue_size);
    }

    void Spin();

private:
    int serializeAndPublish(const char *channel, const char *msg, int size);
    void subscribeSerialized(const char *channel, std::function<void(char*)> callback, int msg_size, int queue_size);
    inline void truncateChannelName(std::string &channel)
    {
        if (channel.length() > 28)
        {
            channel.resize(28);
        }
    }

    std::unique_ptr<DragonetImpl> impl_;
    bool initialized_{false};
};

}
