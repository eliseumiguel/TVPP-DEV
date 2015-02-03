#ifndef RR_MESSAGE_SCHEDULER_H
#define RR_MESSAGE_SCHEDULER_H

#include "IMessageScheduler.hpp"
#include <map>
#include <queue>
#include <set>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

class RRMessageScheduler : public IMessageScheduler
{
    private:
        string keyPointer;
        set<string> keySet;
        map<string, queue<AddressedMessage*> > rrQueue;
        boost::mutex queueAccessMutex;
    public:    
        RRMessageScheduler();
        virtual void Push(AddressedMessage* aMessage);
        virtual AddressedMessage* Pop();
        virtual uint32_t GetSize();
};

#endif