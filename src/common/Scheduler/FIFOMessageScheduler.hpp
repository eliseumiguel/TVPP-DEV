#ifndef FIFO_MESSAGE_SCHEDULER_H
#define FIFO_MESSAGE_SCHEDULER_H

#include "IMessageScheduler.hpp"
#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

class FIFOMessageScheduler : public IMessageScheduler
{
    private:
        queue<AddressedMessage*> fifoQueue;
        boost::mutex queueAccessMutex;
    public:    
        FIFOMessageScheduler();
        virtual void Push(AddressedMessage* aMessage);
        virtual AddressedMessage* Pop();
        virtual uint32_t GetSize();
};

#endif