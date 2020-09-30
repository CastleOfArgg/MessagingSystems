#pragma once

#include "ISender.h"
#include <memory>

//generic sender. connect to a broadcaster and send messages
template<typename T>
class Basic_Sender : ISender<T> {
public:
    //initialize
    Basic_Sender() : broadcaster() {}

    //save address of broadcaster to connect to 
    void connect(std::shared_ptr<Broadcaster<T>> broadcaster) override {
        this->broadcaster = broadcaster;
    }

    //discard address of broadcaster. can no longer send messages to it without reconnection
    void disconnect() override {
        broadcaster.reset();
    }

    //send a message to the broadcaster
    //TODO weak_ptr
    void send(T msg) override {
        broadcaster->addMessage(msg);
    }
protected:
    std::shared_ptr<Broadcaster<T>> broadcaster;
};