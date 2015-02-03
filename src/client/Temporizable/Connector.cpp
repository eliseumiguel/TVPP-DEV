#include "Connector.hpp"

Connector::Connector(Strategy *connectorStrategy, PeerManager* peerManager, uint64_t timerPeriod) : Temporizable(timerPeriod)
{
	this->strategy = connectorStrategy;
	this->peerManager = peerManager;
}

void Connector::Connect()
{
	vector<PeerData*> peers;
	boost::mutex::scoped_lock peerListLock(*peerManager->GetPeerListMutex());
	for (map<string, PeerData>::iterator i = peerManager->GetPeerList()->begin(); i != peerManager->GetPeerList()->end(); i++)
	{
		if (!peerManager->IsPeerActive(i->first))
			peers.push_back(&i->second);
	}
	strategy->Execute(&peers, NULL, peerManager->GetMaxActivePeers());

	unsigned int vacancies = peerManager->GetMaxActivePeers() - peerManager->GetPeerActiveSize();
	if (vacancies > peers.size()) vacancies = peers.size();

	if (!peers.empty())
	{
		for (unsigned int i = 0; i < vacancies; i++)
		{
			peerManager->ConnectPeer(peers[i]->GetPeer()->GetID());
		}
	}
	peerListLock.unlock();
}

void Connector::TimerAlarm(uint64_t timerPeriod, string timerName)
{
	Connect();
}
