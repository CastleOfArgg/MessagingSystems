#pragma once

#include "IReceiver.h"
#include <memory>

//generic receiver class. subscribe to a broadcast.
//Broadcaster's messages will be sent to onNotify() which needs to be implemented
template<typename T>
class Basic_Reveiver : IReveiver<T> {
public:
    //initization
    Basic_Reveiver() : callbackFunc(nullptr) {}
    
    //receive messages from broadcaster
    //setup a shared_ptr/weak_ptr link from callbackFunc to broadcaster's callback storage
    void subscribe(std::weak_ptr<Broadcaster<T>> broadcaster) override {
        auto sp_broadcaster = broadcaster.lock();
        callbackFunc = sp_broadcaster->registerFunction([=](T msg) { onNotify(msg); });
    }

    //stop receiving messages from broadcaster
    //break the shared_ptr/weak_ptr link from callbackFunc to broadcaster's weak_ptr
    void unsubscribe() override {
        //callbackFunc.reset();
        callbackFunc = std::shared_ptr<std::function<void(T)>>();
    }

    //must be implemented in subclass. receives all messages from broadcaster
    virtual void onNotify(T msg) = 0;

protected:
    std::shared_ptr<std::function<void(T)>> callbackFunc;
};