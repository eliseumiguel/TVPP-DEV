#include "PeerManager.hpp"

PeerManager::PeerManager()
{
}

unsigned int PeerManager::GetMaxActivePeers()
{
	return maxActivePeers;
}

void PeerManager::SetMaxActivePeers(unsigned int maxActivePeers)
{
	this->maxActivePeers = maxActivePeers;
}

bool PeerManager::AddPeer(Peer* newPeer)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	if (peerList.find(newPeer->GetID()) == peerList.end())
	{
		peerList[newPeer->GetID()] = PeerData(newPeer);
		peerListLock.unlock();
		cout<<"Peer "<<newPeer->GetID()<<" added to PeerList"<<endl;
		return true;
	}
	peerListLock.unlock();
	return false;
}

bool PeerManager::ConnectPeer(string peer)
{
	if (peerActiveCooldown.find(peer) == peerActiveCooldown.end())
	{
		boost::mutex::scoped_lock peerActiveLock(peerActiveMutex);
		if (peerActive.size() < maxActivePeers)
		{
			if (peerActive.insert(peer).second)
			{
				peerActiveLock.unlock();
				cout<<"Peer "<<peer<<" connected to PeerActive"<<endl;
				return true;
			}
		}
		peerActiveLock.unlock();
	}
	return false;
}

void PeerManager::DisconnectPeer(string peer)
{
	boost::mutex::scoped_lock peerActiveLock(peerActiveMutex);
	peerActive.erase(peer);
	peerActiveLock.unlock();
	peerActiveCooldown[peer] = PEER_ACTIVE_COOLDOWN;
    cout<<"Peer "<<peer<<" disconnected from PeerActive"<<endl;
}

void PeerManager::RemovePeer(string peer)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	DisconnectPeer(peer);
	peerList.erase(peer);
	peerListLock.unlock();
	cout<<"Peer "<<peer<<" removed from PeerList"<<endl;
}

unsigned int PeerManager::GetPeerActiveSize()
{
	boost::mutex::scoped_lock peerActiveLock(peerActiveMutex);
	unsigned int size = peerActive.size();
	peerActiveLock.unlock();
	return size;
}

bool PeerManager::IsPeerActive(string peer)
{
	boost::mutex::scoped_lock peerActiveLock(peerActiveMutex);
	if (peerActive.find(peer) != peerActive.end())
	{
		peerActiveLock.unlock();
		return true;
	}
	peerActiveLock.unlock();
	return false;
}

PeerData* PeerManager::GetPeerData(string peer)
{
	return &peerList[peer];
}

map<string, PeerData>* PeerManager::GetPeerList()
{
	return &peerList;
}

set<string>* PeerManager::GetPeerActive()
{
	return &peerActive;
}

boost::mutex* PeerManager::GetPeerListMutex()
{
	return &peerListMutex;
}

boost::mutex* PeerManager::GetPeerActiveMutex()
{
	return &peerActiveMutex;
}

void PeerManager::CheckPeerList()
{
    set<string> deletedPeer;

	boost::mutex::scoped_lock peerActiveLock(peerActiveMutex);
    for (set<string>::iterator i = peerActive.begin(); i != peerActive.end(); i++)
    {
        peerList[*i].DecTTL();
        if (peerList[*i].GetTTL() <= 0)
        {
            deletedPeer.insert(*i);
        }
    }
	peerActiveLock.unlock();

    for (set<string>::iterator i = deletedPeer.begin(); i != deletedPeer.end(); i++)
    {
        RemovePeer(*i);
    }

	deletedPeer.clear();
	//Cooldown
	for (map<string, unsigned int>::iterator i = peerActiveCooldown.begin(); i != peerActiveCooldown.end(); i++)
    {
		i->second--;
		if (i->second == 0)
			deletedPeer.insert(i->first);
	}
	for (set<string>::iterator i = deletedPeer.begin(); i != deletedPeer.end(); i++)
    {
        peerActiveCooldown.erase(*i);
    }
}

void PeerManager::ShowPeerList()
{
    int j = 0;
    
    cout<<endl<<"- Peer List Active -"<<endl;
	boost::mutex::scoped_lock peerActiveLock(peerActiveMutex);
    for (set<string>::iterator i = peerActive.begin(); i != peerActive.end(); i++, j++)
	{
		cout<<"Key: "<<*i<<" ID: "<<peerList[*i].GetPeer()->GetID()<<" Mode: "<<(int)peerList[*i].GetMode()<<" TTL: "<<peerList[*i].GetTTL() << " PR: "<<peerList[*i].GetPendingRequests() << endl;
	}
	peerActiveLock.unlock();

    cout<<"Total: "<<j<<" Peers"<<endl<<endl;

	j = 0;
	cout<<endl<<"- Peer List Total -"<<endl;
	boost::mutex::scoped_lock peerListLock(peerListMutex);
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++, j++)
	{
		cout<<"Key: "<<i->first<< endl;
		cout<<"ID: "<<i->second.GetPeer()->GetID()<<" Mode: "<<(int)i->second.GetMode()<<" TTL: "<<i->second.GetTTL() << " RTT(delay): " <<i->second.GetDelay()<< "s PR: "<<i->second.GetPendingRequests() << endl;
	}
	peerListLock.unlock();
	cout<<"Total: "<<j<<" Peers"<<endl<<endl;
}