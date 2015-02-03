#include "FIFOMessageScheduler.hpp"

FIFOMessageScheduler::FIFOMessageScheduler()
{
}

void FIFOMessageScheduler::Push(AddressedMessage* aMessage)
{
    boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
    fifoQueue.push(aMessage);
    queueAccessLock.unlock();
}

AddressedMessage* FIFOMessageScheduler::Pop()
{
    if (!fifoQueue.empty())
    {
        AddressedMessage* aMessage;
        boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
        aMessage = fifoQueue.front();
        fifoQueue.pop();
        queueAccessLock.unlock();
        return aMessage;
    } else
        return NULL;
    
}

uint32_t FIFOMessageScheduler::GetSize()
{
    int size = 0;
    boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
    size += fifoQueue.size();
    queueAccessLock.unlock();
    return size;
}