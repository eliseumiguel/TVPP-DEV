/*
 * Modificado: Eliseu César miguel
 * Data:       2015-01-19
 * Com a separação da lista de PeerManage::peerActive em listas de In e Out,
 * não é necessário executar o connector para a lista Out, visto que os pares
 * que solicitam dados tornam-se, automaticamente Out. Contudo, os parâmetros
 * permanecem pois, pode-se implementar um bom super nó que decida oferecer
 * dados procurando por alguns Out.
 */

#include "Connector.hpp"

Connector::Connector(Strategy *connectorStrategy, PeerManager* peerManager, uint64_t timerPeriod, set<string>* peerActive) : Temporizable(timerPeriod)
{
	this->strategy = connectorStrategy;
	this->peerManager = peerManager;
	this->peerActive = peerActive;
}

void Connector::Connect()
{
	vector<PeerData*> peers;
	boost::mutex::scoped_lock peerListLock(*peerManager->GetPeerListMutex());
	for (map<string, PeerData>::iterator i = peerManager->GetPeerList()->begin(); i != peerManager->GetPeerList()->end(); i++)
	{
		if (!peerManager->IsPeerActive(i->first,peerActive))
			peers.push_back(&i->second);
	}
	strategy->Execute(&peers, NULL, peerManager->GetMaxActivePeers(peerActive));

	unsigned int vacancies = peerManager->GetMaxActivePeers(peerActive) - peerManager->GetPeerActiveSize(peerActive);
	if (vacancies > peers.size()) vacancies = peers.size();

	if (!peers.empty())
	{
		for (unsigned int i = 0; i < vacancies; i++)
		{
			peerManager->ConnectPeer(peers[i]->GetPeer()->GetID(),peerActive);
		}
	}
	peerListLock.unlock();
}

void Connector::TimerAlarm(uint64_t timerPeriod, string timerName)
{
	Connect();
}
