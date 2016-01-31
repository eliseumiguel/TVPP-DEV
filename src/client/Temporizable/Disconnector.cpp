#include "Disconnector.hpp"

Disconnector::Disconnector(Strategy *disconnectorStrategy, PeerManager* peerManager, uint64_t timerPeriod, set<string>* peerActive, unsigned int quantity, bool disabled_ServerAuxActive) : Temporizable(timerPeriod)
{
	this->strategy = disconnectorStrategy;
	this->peerManager = peerManager;
	this->peerActive = peerActive;
	this->quantity = quantity;
	this->disabled_ServerAuxActive = disabled_ServerAuxActive;  //disable disconnector whem peer is auxiliar server active or mixing server
}

void Disconnector::Disconnect()
{

	if (!(this->disabled_ServerAuxActive) || peerManager->GetPeerManagerState() == NO_SERVER_AUX)
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
		      if (!peerManager->GetPeerData(peers.at(i-1)->GetPeer()->GetID())->GetSpecialPeer()){
			      peerManager->DisconnectPeer(peers.at(i-1)->GetPeer()->GetID(),peerActive);
			      cout <<"Disconnector removing "<<peers.at(i-1)->GetPeer()->GetID()<<" to flexibly relationships" <<endl;
		      }
		      else
		      	 cout <<"peer "<<peers.at(i-1)->GetPeer()->GetID()<<" not disconnected because it is a Auxiliary Server"<<endl;
	       }
	   }
	   else {cout << "Disconnector able but not peer delected" <<std::endl;}
	   peerListLock.unlock();
	}
	else
		cout<<"Auto disconnector disabled because client state is "<<peerManager->GetPeerManagerState()<<endl;
}

void Disconnector::TimerAlarm(uint64_t timerPeriod, string timerName)
{
	Disconnect();
}
