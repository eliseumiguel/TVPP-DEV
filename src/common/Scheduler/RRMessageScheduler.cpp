#include "RRMessageScheduler.hpp"

RRMessageScheduler::RRMessageScheduler()
{
    keyPointer = "invalid";
}

void RRMessageScheduler::Push(AddressedMessage* aMessage)
{
    string address = aMessage->GetAddress();
    
    boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
    keySet.insert(address);
    rrQueue[address].push(aMessage);
    queueAccessLock.unlock();

    //cout << "SCHEDULER  IN [" << aMessage->GetAddress() << "] " <<endl;
    //aMessage->GetMessage()->PrintHeader();
}

AddressedMessage* RRMessageScheduler::Pop()
{
    if (!keySet.empty())
    {
        if (keyPointer == "invalid")
            keyPointer = *keySet.begin();

        /** DEBUG
        cout << "DEBUG keyPointer started: " << keyPointer << endl;
        cout << "KeyList : ";
        for (set<string>::iterator it=keySet.begin(); it!=keySet.end(); ++it)
            cout << *it << " ";
        cout << endl;
        cout << "KeyMap : ";
        for (map<string, queue<AddressedMessage*> >::iterator it=rrQueue.begin(); it!=rrQueue.end(); ++it)
            cout << it->first << "["<<it->second.size()<<"] ";
        cout << endl;
        **/

        //Pop one
        AddressedMessage* aMessage;
        boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
        aMessage = rrQueue.at(keyPointer).front();
        rrQueue.at(keyPointer).pop();

        string deletedKey = "invalid";
        if (rrQueue.at(keyPointer).empty())
            deletedKey = keyPointer;
        
        //Get next address
        uint32_t count = 0;
        do 
        {
            set<string>::iterator nextKey = keySet.find(keyPointer);
            ++nextKey;
            if (nextKey == keySet.end())
                keyPointer = *keySet.begin();
            else
                keyPointer = *nextKey;
            count++;
        } while (count < rrQueue.size() && rrQueue.at(keyPointer).empty());
        if (count >= rrQueue.size())
            keyPointer = "invalid";

        //Deletes the unuseful key
        if (deletedKey != "invalid")
        {
            //cout << "deleted " << deletedKey <<endl;
            keySet.erase(deletedKey);
            rrQueue.erase(deletedKey);
        }
        queueAccessLock.unlock();

        //cout << "KeyPointer ended: " << keyPointer << endl;

        //cout << "SCHEDULER OUT [" << aMessage->GetAddress() << "] " << endl;
        //aMessage->GetMessage()->PrintHeader();
        return aMessage;
    } 
    //queueAccessLock.unlock();
    return NULL;
}

uint32_t RRMessageScheduler::GetSize()
{
    int size = 0;
    boost::mutex::scoped_lock queueAccessLock(queueAccessMutex);
    for (std::map<string,queue<AddressedMessage*> >::iterator it=rrQueue.begin(); it!=rrQueue.end(); ++it)
        size += it->second.size();
    queueAccessLock.unlock();
    return size;
}