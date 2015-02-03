#include "CDFMessageScheduler.hpp"

CDFMessageScheduler::CDFMessageScheduler()
{
}

void CDFMessageScheduler::Push(AddressedMessage* aMessage)
{
    Message* internGenericMessage = aMessage->GetMessage();
    if (internGenericMessage->GetOpcode() == OPCODE_REQUEST)
    {
        MessageRequest* internMessage = new MessageRequest(internGenericMessage);
        vector<int> requestHeader = internMessage->GetHeaderValues();
        ChunkUniqueID requestedChunk(requestHeader[0], (uint16_t)requestHeader[1]);
        ChunkUniqueID playbackTipChunk(requestHeader[2], (uint16_t)requestHeader[3]);
        ChunkUniqueID diff = requestedChunk - playbackTipChunk;

        boost::mutex::scoped_lock requestQueueLock(requestQueueAccessMutex);
        requestQueue.insert(pair<ChunkUniqueID,AddressedMessage*>(diff,aMessage));
        requestQueueLock.unlock();

        //If i delete it here, the original message will be deleted, if i dont... memleak
        //delete internMessage;

        cout << "SCHEDULER  IN REQUEST [" << diff << "][" << aMessage->GetAddress() << "] "<<endl;
        //aMessage->GetMessage()->PrintHeader();
    }
    else
    {
        boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
        fifoQueue.push(aMessage);
        queueAccessLock.unlock();
        cout << "SCHEDULER  IN [" << aMessage->GetAddress() << "] "<<endl;
        //aMessage->GetMessage()->PrintHeader();
    }
}

AddressedMessage* CDFMessageScheduler::Pop()
{
    AddressedMessage* aMessage;
    if (!fifoQueue.empty())
    {
        boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
        aMessage = fifoQueue.front();
        fifoQueue.pop();
        queueAccessLock.unlock();
        cout << "SCHEDULER OUT [" << aMessage->GetAddress() << "] "<<endl;
        //aMessage->GetMessage()->PrintHeader();
        return aMessage;
    } 
    else if (!requestQueue.empty())
    {
        boost::mutex::scoped_lock requestQueueLock(requestQueueAccessMutex);
        multimap<ChunkUniqueID,AddressedMessage*>::iterator it = requestQueue.begin();
        ChunkUniqueID diff = (*it).first;
        aMessage = (*it).second;
        requestQueue.erase(it);
        requestQueueLock.unlock();
        cout << "SCHEDULER OUT REQUEST[" << diff << "][" << aMessage->GetAddress() << "] " <<endl;
        //aMessage->GetMessage()->PrintHeader();
        return aMessage;
    } 
    else
        return NULL;
    
}

uint32_t CDFMessageScheduler::GetSize()
{
    int size = 0;
    boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
    size += fifoQueue.size();
    queueAccessLock.unlock();
    boost::mutex::scoped_lock requestQueueAccessLock(requestQueueAccessMutex);
    size += requestQueue.size();
    requestQueueAccessLock.unlock();
    return size;
}