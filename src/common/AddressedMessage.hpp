#ifndef ADDRESSED_MESSAGE_H
#define ADDRESSED_MESSAGE_H

#include <string>
#include <sys/time.h>
#include "Messages/Message.hpp"

using namespace std;

class AddressedMessage
{
    private:
        string address;
        struct timeval creationTime;
        Message* message;
    public:
        AddressedMessage(string address, Message* message);
        string GetAddress();
        struct timeval GetCreationTime();
        float GetAge();
        Message* GetMessage();
};


#endif // ADDRESSED_MESSAGE_H

