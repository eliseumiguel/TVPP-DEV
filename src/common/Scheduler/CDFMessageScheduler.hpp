#ifndef CDF_MESSAGE_SCHEDULER_H
#define CDF_MESSAGE_SCHEDULER_H

#include "IMessageScheduler.hpp"
#include <map>
#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "../Messages/MessageRequest.hpp"

class CDFMessageScheduler : public IMessageScheduler
{
    private:
        multimap<ChunkUniqueID,AddressedMessage*> requestQueue;
        queue<AddressedMessage*> fifoQueue;
        boost::mutex requestQueueAccessMutex, queueAccessMutex;
    public:    
        CDFMessageScheduler();
        virtual void Push(AddressedMessage* aMessage);
        virtual AddressedMessage* Pop();
        virtual uint32_t GetSize();
};

#endif