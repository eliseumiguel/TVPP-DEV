#ifndef RANDOM_MESSAGE_SCHEDULER_H
#define RANDOM_MESSAGE_SCHEDULER_H

#include "IMessageScheduler.hpp"
#include <list>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

class RandomMessageScheduler : public IMessageScheduler
{
    private:
        list<AddressedMessage*> randomList;
        boost::mutex queueAccessMutex;
    public:    
        RandomMessageScheduler();
        virtual void Push(AddressedMessage* aMessage);
        virtual AddressedMessage* Pop();
        virtual uint32_t GetSize();
};

#endif
