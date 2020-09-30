#pragma once
#include "Broadcaster.h"
#include <memory>

//interface to connect to a broadcaster to send messages to
template<typename T>
class ISender {
public:
    virtual void connect(std::shared_ptr<Broadcaster<T>> broadcaster) = 0;
    virtual void disconnect() = 0;
    virtual void send(T msg) = 0;
};