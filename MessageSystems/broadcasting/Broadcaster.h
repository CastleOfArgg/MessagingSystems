#pragma once
#include <functional>
#include <queue>
#include <memory>
#include <vector>
#include <iostream>

//broadcasts sent messages to all connected receivers
template<typename T> //MyData
class Broadcaster {
    using MessageType = T;
    using callback_func = std::function<void(MessageType)>;
    using wp_callback = std::weak_ptr<callback_func>;
    using sp_callback = std::shared_ptr<callback_func>;

    //helper to convert function to shared_ptr<function>
    static sp_callback wrap_callback(callback_func func) {
        return std::make_shared<callback_func>(std::move(func));
    }
public:
    //add a message to the message queue
    void addMessage(MessageType msg) {
        messages.push(msg);
    }

    //registers a callback function with which messages will be sent to on notify()
    sp_callback registerFunction(callback_func func) {
        auto wrappedCallback = wrap_callback(func);
        callbacks.push_back(wrappedCallback);
        return wrappedCallback;
    }

    //send all messages in message queue to all callback functions and then delete message
    void notify() {
        if (messages.empty())
            return;

        //send out first msg and mark dead weak_ptrs in callbacks
        std::vector<wp_callback> new_callbacks;
        for (wp_callback callback : callbacks) {
            sp_callback callback_ptr = callback.lock();
            if (!callback.expired()) {
                (*callback_ptr)(messages.front());
                new_callbacks.push_back(callback_ptr); //mark as good
            }
        }
        messages.pop();
        
        //send aout all messages to all callbacks
        while (!messages.empty()) {
            for (wp_callback callback : callbacks) {
                sp_callback callback_ptr = callback.lock();
                if (!callback.expired()) {
                    (*callback_ptr)(messages.front());
                }
            }
            messages.pop();
        }

        //update callbacks
        callbacks = new_callbacks;
    }

    //for debugging. print out all data
    void printAll() {
        std::cout << "\n========MEM==============\ncalls\n";
        for (auto calls : callbacks)
            std::cout << calls.lock().get() << ", ";
        std::cout << "\nMSG\n";
        auto tempMsgQueue = messages;
        while (!tempMsgQueue.empty()) {
            std::cout << tempMsgQueue.front() << "\n";
            tempMsgQueue.pop();
        }
        std::cout << "\n=========================\n";
    }

private:
    std::vector<wp_callback> callbacks;
    std::queue<MessageType> messages;
};