#include "RandomMessageScheduler.hpp"

RandomMessageScheduler::RandomMessageScheduler()
{
}

void RandomMessageScheduler::Push(AddressedMessage* aMessage)
{
    list<AddressedMessage*>::iterator it; 
    
    boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
    it = randomList.begin(); 
    uint32_t randomPos = (uint32_t)drand48() % ((uint32_t)randomList.size()+1);
    for(uint32_t i=0; i<randomPos; i++) it++;
    randomList.insert(it,aMessage);
    queueAccessLock.unlock();
    //cout << "SCHEDULER  IN [" << aMessage->GetAddress() << "] ";
    //aMessage->GetMessage()->PrintHeader();
}

AddressedMessage* RandomMessageScheduler::Pop()
{
    if (!randomList.empty())
    {
        AddressedMessage* aMessage;
        boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
        aMessage = randomList.front();
        randomList.pop_front();
        queueAccessLock.unlock();
        //cout << "SCHEDULER OUT [" << aMessage->GetAddress() << "] ";
        //aMessage->GetMessage()->PrintHeader();
        return aMessage;
    } else
        return NULL;    
}

uint32_t RandomMessageScheduler::GetSize()
{
    int size = 0;
    boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
    size += randomList.size();
    queueAccessLock.unlock();
    return size;
}
