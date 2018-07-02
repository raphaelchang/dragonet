#pragma once

#include <memory>

namespace dragonet
{

class IntraImpl;
class InterImpl;

class Dragonet
{
public:
    Dragonet();
    ~Dragonet();
    template<class MessageType>
    int Publish(const char *channel, const MessageType *msg);
    template<class MessageType>
    void Subscribe(const char *channel, void (*callback)(const MessageType *msg), int queue_size=10);
    int Spin();
private:
    std::unique_ptr<IntraImpl> intra_;
    std::unique_ptr<InterImpl> inter_;
};

}
