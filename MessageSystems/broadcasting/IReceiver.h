#pragma once

#include "Broadcaster.h"
#include <memory>

//interface for receivers to connect with a broadcaster to receive messages from
template<typename T>
class IReveiver {
public:
    virtual void subscribe(std::weak_ptr<Broadcaster<T>> broadcaster) = 0;
    virtual void unsubscribe() = 0;
    virtual void onNotify(T msg) = 0;
};