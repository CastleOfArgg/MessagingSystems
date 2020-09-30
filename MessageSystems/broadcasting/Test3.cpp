#include "Broadcaster.h"
#include "Basic_Reveiver.h"
#include "Basic_Sender.h"
#include <ostream>
#include <iostream>
#include <string>

//custom message struct
typedef struct {
    std::string sender;
    std::string payload;
} data;

//overloaded output stream << operator for use with custom message struct named 'data'
std::ostream& operator<<(std::ostream& os, const data& data) {
    return os << "sender: " << data.sender << ", payload: " << data.payload;
}


//example Receiver class, takes in a string name, prints msg on onNotify()
template<typename T>
class Receiver : public Basic_Reveiver<T> {
public:
    //this example requries a string name
    Receiver(std::string name) : m_name(name) {}

    //override onNotify for custom message filtering and reactions
    virtual void onNotify(T msg) override {
        //filter messages here
        std::cout << getName() << ": received: " << msg << "\n";
    }

    //get the receiver's name
    std::string getName() const {
        return m_name;
    }
private:
    const std::string m_name;
};

//example class that is both a sender and a receiver
template<typename T>
class Basic_Sender_Receiver : public Receiver<T>, public Basic_Sender<T> {
public:
    Basic_Sender_Receiver(std::string name) : Receiver<T>(name), Basic_Sender<T>() {}
};

/*
Integer data:
RECEIVER: received: 6
BOLTHER: received: 6
RECEIVER: received: 10
BOLTHER: received: 10
RECEIVER unsubscribed
BOLTHER: received: 11
*/
void test_int(){
    std::cout << "Integer data:\n";
    auto broadcast = new Broadcaster<int>;
    auto sp_broadcast = std::shared_ptr<Broadcaster<int>>(broadcast);
    Receiver<int> r("RECEIVER");
    Basic_Sender<int> s;
    Basic_Sender_Receiver<int> sr("BOLTHER");
    s.connect(sp_broadcast);
    r.subscribe(sp_broadcast);
    sr.connect(sp_broadcast);
    sr.subscribe(sp_broadcast);
    s.send(6);
    sr.send(10);
    sp_broadcast->notify();
    std::cout << "RECEIVER unsubscribed\n";
    r.unsubscribe();
    sr.send(11);
    sp_broadcast->notify();
    std::cout << "\n";
}

/*
Custom data:
RECEIVER: received: sender: SENDER, payload: SENDING
BOLTHER: received: sender: SENDER, payload: SENDING
RECEIVER: received: sender: BOLTHER, payload: BOLTHER
BOLTHER: received: sender: BOLTHER, payload: BOLTHER
RECEIVER unsubscribed
BOLTHER: received: sender: BOLTHER, payload: BOLTHER2
*/
void test_data() {
    std::cout << "Struct data:\n";
    auto broadcast = new Broadcaster<data>;
    auto sp_broadcast = std::shared_ptr<Broadcaster<data>>(broadcast);
    Receiver<data> r("RECEIVER");
    Basic_Sender<data> s;
    Basic_Sender_Receiver<data> sr("BOLTHER");
    s.connect(sp_broadcast);
    r.subscribe(sp_broadcast);
    sr.connect(sp_broadcast);
    sr.subscribe(sp_broadcast);
    s.send({ "SENDER", "SENDING" });
    sr.send({ sr.getName(), "BOLTHER" });
    sp_broadcast->notify();
    std::cout << "RECEIVER unsubscribed\n";
    r.unsubscribe();
    sr.send({ sr.getName(), "BOLTHER2" });
    sp_broadcast->notify();
    std::cout << "\n";
}

/*
break sender:
RECEIVER: received: sender: BOLTHER, payload: BOLTHER
BOLTHER: received: sender: BOLTHER, payload: BOLTHER
RECEIVER: received: sender: SENDER, payload: SENDING
BOLTHER: received: sender: SENDER, payload: SENDING
SENDER dead
RECEIVER: received: sender: BOLTHER, payload: BOLTHER2
BOLTHER: received: sender: BOLTHER, payload: BOLTHER2
*/
void test_break_sender() {
    std::cout << "break sender:\n";
    auto broadcast = new Broadcaster<data>;
    auto sp_broadcast = std::shared_ptr<Broadcaster<data>>(broadcast);
    Receiver<data> r("RECEIVER");
    r.subscribe(sp_broadcast);
    Basic_Sender_Receiver<data> sr("BOLTHER");
    sr.connect(sp_broadcast);
    sr.subscribe(sp_broadcast);
    sr.send({ sr.getName(), "BOLTHER" });
    {
        Basic_Sender<data> s;
        s.connect(sp_broadcast);
        s.send({ "SENDER", "SENDING" });
        sp_broadcast->notify();
    }
    std::cout << "SENDER dead\n";
    sr.send({ sr.getName(), "BOLTHER2" });
    sp_broadcast->notify();
    std::cout << "\n";
}

/*
break receiver:
BOLTHER: received: sender: SENDER, payload: SENDING
RECEIVER: received: sender: SENDER, payload: SENDING
BOLTHER: received: sender: BOLTHER, payload: BOLTHER
RECEIVER: received: sender: BOLTHER, payload: BOLTHER
RECEIVER dead
BOLTHER: received: sender: BOLTHER, payload: BOLTHER2
*/
void test_break_receiver() {
    std::cout << "break receiver:\n";
    auto broadcast = new Broadcaster<data>;
    auto sp_broadcast = std::shared_ptr<Broadcaster<data>>(broadcast);
    Basic_Sender<data> s;
    s.connect(sp_broadcast);
    s.send({ "SENDER", "SENDING" });
    Basic_Sender_Receiver<data> sr("BOLTHER");
    sr.connect(sp_broadcast);
    sr.subscribe(sp_broadcast);
    sr.send({ sr.getName(), "BOLTHER" });
    {
        Receiver<data> r("RECEIVER");
        r.subscribe(sp_broadcast);
        sp_broadcast->notify();
    }
    std::cout << "RECEIVER dead\n";
    sr.send({ sr.getName(), "BOLTHER2" });
    sp_broadcast->notify();
    std::cout << "\n";
}

/*
break broadcaster:
RECEIVER: received: sender: BOLTHER, payload: BOLTHER1
BOLTHER: received: sender: BOLTHER, payload: BOLTHER1
Broadcast dead (but still alive)
*/
void test_break_broadcaster() {
    std::cout << "break broadcaster:\n";
    Basic_Sender<data> s;
    Receiver<data> r("RECEIVER");
    Basic_Sender_Receiver<data> sr("BOLTHER");
    {
        auto broadcast = new Broadcaster<data>;
        auto sp_broadcast = std::shared_ptr<Broadcaster<data>>(broadcast);
        s.connect(sp_broadcast);
        r.subscribe(sp_broadcast);
        sr.connect(sp_broadcast);
        sr.subscribe(sp_broadcast);
        sr.send({ sr.getName(), "BOLTHER1" });
        sp_broadcast->notify();
    }
    std::cout << "Broadcast dead (but still alive)\n";
    s.send({ sr.getName(), "SENT1" });
    std::cout << "\n";
}

/*
memory:
subscribe:
subscribe:
send:
send:

========MEM==============
calls
00C06A08, 00C06D30,
MSG
sender: SENDER, payload: SENDING
sender: BOLTHER, payload: BOLTHER1

=========================
notify:

========MEM==============
calls
00C06A08, 00C06D30,
MSG

=========================
RECEIVER unsubscribed
unsubscribe:
send:

========MEM==============
calls
00000000, 00C06D30,
MSG
sender: BOLTHER, payload: BOLTHER2

=========================
subscribe:

========MEM==============
calls
00000000, 00C06D30, 00C06790,
MSG

=========================
BOLTHER: received: sender: , payload: sdfghj
RECEIVER: received: sender: , payload: sdfghj
notify:

========MEM==============
calls
00C06D30, 00C06790,
MSG

=========================
*/
void test_mem() {
    std::cout << "memory:\n";
    auto broadcast = new Broadcaster<data>;
    auto sp_broadcast = std::shared_ptr<Broadcaster<data>>(broadcast);
    Receiver<data> r("RECEIVER");
    Basic_Sender<data> s;
    Basic_Sender_Receiver<data> sr("BOLTHER");
    s.connect(sp_broadcast);
    r.subscribe(sp_broadcast);
    std::cout << "subscribe:\n";/////////
    sr.connect(sp_broadcast);
    sr.subscribe(sp_broadcast);
    std::cout << "subscribe:\n";//////////
    s.send({ "SENDER", "SENDING" });
    std::cout << "send:\n";//////////
    sr.send({ sr.getName(), "BOLTHER1" });
    std::cout << "send:\n";//////////
    sp_broadcast->printAll();

    sp_broadcast->notify();
    std::cout << "notify:\n";//////////
    sp_broadcast->printAll();

    std::cout << "RECEIVER unsubscribed\n";
    r.unsubscribe();
    std::cout << "unsubscribe:\n";//////////
    sr.send({ sr.getName(), "BOLTHER2" });
    std::cout << "send:\n";//////////
    sp_broadcast->printAll();

    r.subscribe(sp_broadcast);
    std::cout << "subscribe:\n";//////////
    sp_broadcast->printAll();

    sp_broadcast->notify();
    std::cout << "notify:\n";//////////
    sp_broadcast->printAll();

    std::cout << "\n";
}

//example program using the broadcasting messaging system
int main() {
    test_int();
    test_data();
    test_break_sender();
    test_break_receiver();
    test_break_broadcaster();
    test_mem();

    std::cout << "DONE\n";
    return 0;
}