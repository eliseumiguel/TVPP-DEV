#ifndef I_MESSAGE_SCHEDULER_H
#define I_MESSAGE_SCHEDULER_H

#include "../AddressedMessage.hpp"

using namespace std;

class IMessageScheduler
{
    public:    
        virtual ~IMessageScheduler() {}
        virtual void Push(AddressedMessage* aMessage) = 0;
        virtual AddressedMessage* Pop() = 0;
        virtual uint32_t GetSize() = 0;
};

#endif