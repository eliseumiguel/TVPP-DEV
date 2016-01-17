#include "Disconnector.hpp"

Disconnector::Disconnector(Strategy *disconnectorStrategy, PeerManager* peerManager, uint64_t timerPeriod, set<string>* peerActive, int quantity) : Temporizable(timerPeriod)
{
	this->strategy = disconnectorStrategy;
	this->peerManager = peerManager;
	this->peerActive = peerActive;
	this->quantity = quantity;
}

void Disconnector::Disconnect()
{
	vector<PeerData*> peers;
	boost::mutex::scoped_lock peerListLock(*peerManager->GetPeerListMutex());
	for (set<string>::iterator i = peerActive->begin(); i != peerActive->end(); i++)
	{
		peers.push_back(peerManager->GetPeerData(*i));
	}
	strategy->Execute(&peers, NULL, this->quantity);

	/* Código alterado para desconectar peers de acordo com a quantidade informada
	 * remove sempre no início do vetor, de acordo com a estratégia
	 */

    // ECM Remove a quatidade de pares na lista para dar abertura a novas conexões à rede
	if ((this->quantity <= peers.size()) && (this->quantity > 0))
	{
	    for (unsigned int i = this->quantity; i > 0; i--){
		   peerManager->DisconnectPeer(peers.at(i-1)->GetPeer()->GetID(),peerActive);
		    cout <<"Desconnector removendo "<<peers.at(i-1)->GetPeer()->GetID()<<" para flexibiliar as parcerias" <<endl;
	    }
	}
	else {cout << "Desconnector chamado mas não houve par removido da lista de Out" <<std::endl;}
	peerListLock.unlock();
}

void Disconnector::TimerAlarm(uint64_t timerPeriod, string timerName)
{
	Disconnect();
}
