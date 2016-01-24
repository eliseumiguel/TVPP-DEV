#include "Disconnector.hpp"

Disconnector::Disconnector(Strategy *disconnectorStrategy, PeerManager* peerManager, uint64_t timerPeriod, set<string>* peerActive, unsigned int quantity) : Temporizable(timerPeriod)
{
	this->strategy = disconnectorStrategy;
	this->peerManager = peerManager;
	this->peerActive = peerActive;
	this->quantity = quantity;
}

void Disconnector::Disconnect()
{
	// ECM Evitar que um servidor auxiliar remova parceiros Out na rede paralela
	if (peerManager->GetPeerManagerState() == NO_SERVER_AUX)
	{
	   vector<PeerData*> peers;
	   boost::mutex::scoped_lock peerListLock(*peerManager->GetPeerListMutex());
	   for (set<string>::iterator i = peerActive->begin(); i != peerActive->end(); i++)
	   {
	     peers.push_back(peerManager->GetPeerData(*i));
	   }
	   strategy->Execute(&peers, NULL, this->quantity);

	   /* ECM Código alterado para desconectar peers de acordo com a quantidade informada
	   * remove sempre no início do vetor, de acordo com a estratégia
	   * Com essa estratégia, as parcerias tornam-se mais flexíveis durante os experimentos */
	   if ((this->quantity <= peers.size()) && (this->quantity > 0))
	   {
	       for (unsigned int i = this->quantity; i > 0; i--){
	    	  cout<<"o peer "<<peers.at(i-1)->GetPeer()->GetID() <<" tem estado especial "<<peerManager->GetPeerData(peers.at(i-1)->GetPeer()->GetID())->GetSpecialPeer()<<endl;
		      if (!peerManager->GetPeerData(peers.at(i-1)->GetPeer()->GetID())->GetSpecialPeer()){
			      peerManager->DisconnectPeer(peers.at(i-1)->GetPeer()->GetID(),peerActive);
			      cout <<"Disconnector removing "<<peers.at(i-1)->GetPeer()->GetID()<<" to flexibly relationships" <<endl;
		      }
		      else
		      	 cout <<"peer "<<peers.at(i-1)->GetPeer()->GetID()<<" not disconnected because it is a auxiliary server"<<endl;
	       }
	   }
	   else {cout << "Disconnector able but not peer delected" <<std::endl;}
	   peerListLock.unlock();
	}
	else
		cout<<"Disconnector unable because client's state is "<<peerManager->GetPeerManagerState()<<endl;
}

void Disconnector::TimerAlarm(uint64_t timerPeriod, string timerName)
{
	Disconnect();
}
