/*
 * PeerManagerServerAux.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: eliseu
 */

#include "PeerManagerServerAux.hpp"
#include "PeerManager.hpp"

/*constructor*/
PeerManagerServerAux::PeerManagerServerAux(): PeerManager (){this->peerManagerState = NO_SERVER_AUX; mixAux_KillEnd = false;}

/*destructor*/
PeerManagerServerAux::~PeerManagerServerAux() {}

MesclarModeServer PeerManagerServerAux::Get_MixType(){return this->MixType;}
uint8_t PeerManagerServerAux::Get_QT_PeerMixType(){return this->QT_PeerMixType;}
uint8_t PeerManagerServerAux::Get_TimeDescPeerMix(){return this->TimeDescPeerMix;}

void PeerManagerServerAux::Set_MixType(MesclarModeServer MixType){
	if (MixType == 03){
		this->MixType = (MesclarModeServer) 01; mixAux_KillEnd = true; return;
	}
	else
		if (MixType == 04){
			this->MixType = (MesclarModeServer) 02; mixAux_KillEnd = true; return;
		}
	this->MixType = MixType;
}
void PeerManagerServerAux::Set_QT_PeerMixType(uint8_t QT_PeerMixType){this->QT_PeerMixType = QT_PeerMixType;}
void PeerManagerServerAux::Set_TimeDescPeerMix(uint8_t TimeDescPeerMix){this->TimeDescPeerMix = TimeDescPeerMix;}



set<string> PeerManagerServerAux::Get_Random_Mix_Peers(set<string>* peers, uint8_t QT){

	boost::mutex::scoped_lock peerListLock(peerListMutex);
	boost::mutex::scoped_lock peerActiveLock(peerActiveMutexOut);

	set<string> selectedPeers, allMixPeer;

	cout<<"Remove possible peer list [";
	for (set<string>::iterator mix = peers->begin(); mix != peers->end(); mix++)
		if (peerList[*mix].GetChannelId_Sub() < 0){
			allMixPeer.insert(*mix);
			cout<<*mix<<" ";
		}
	cout<<"]"<<endl;

	if (allMixPeer.size() < QT){
		return allMixPeer;
	}
	else{
		for (unsigned int qt=0;  qt<QT; qt++){
			set<string>::iterator it = allMixPeer.begin();
			srand (time (NULL));
			unsigned int sorteado;
			sorteado = rand () % allMixPeer.size();
			for (unsigned pos=0; pos<sorteado; pos++)
				it++;
			selectedPeers.insert(*it);
			allMixPeer.erase(it);
			}
		}
	peerActiveLock.unlock();
	peerListLock.unlock();

	return selectedPeers;
}

uint8_t PeerManagerServerAux::ExecMesc (uint8_t timeMix){
	timeMix--;
	if (timeMix == 0){

		switch (this->Get_MixType())
		{
		case AUTO_KILL_SERVER: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			{
				cout<<" Time Out "<<(int)this->Get_TimeDescPeerMix()<<": Server shutting down mix strategy ..."<<endl;
				exit (1);
			}
			break;

		case (AUTO_LIVE_AVOID_CONNECTION): //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			set<string> deletedPeer = this->Get_Random_Mix_Peers( &(this->peerActiveOut), (unsigned int)this->Get_QT_PeerMixType());

			// AUTO_LIVE_AVOID_CONNECTION_KILLEND
			if ((deletedPeer.size() == 0) && (mixAux_KillEnd)) {
				this->Set_MixType(AUTO_KILL_SERVER);
				return timeMix;
			}

			if ((deletedPeer.size() == 0)) {
				this->Set_MixType(NONE_MIX);
				return timeMix;
			}

			boost::mutex::scoped_lock peerListRejectedLock(peerListRejectedMutexOut);
			for (set<string>::iterator it = deletedPeer.begin(); it != deletedPeer.end(); it++){
			     this->peerList_Rejected.insert(*it);
			     this->DisconnectPeer(*it, &peerActiveOut);
			}

			cout <<" Rejected list for reconnection "<<endl;
			for(set<string>::iterator i = peerList_Rejected.begin(); i!=peerList_Rejected.end(); i++)
				cout<<*i<<" "<<endl;

			peerListRejectedLock.unlock();
		}
			break;

		case (AUTO_LIVE_PERMIT_CONNECTION): //++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			set<string> deletedPeer = this->Get_Random_Mix_Peers(&(this->peerActiveOut), (unsigned int) this->Get_QT_PeerMixType());

			// case AUTO_LIVE_PERMIT_CONNECTION_KillEND
			if ((deletedPeer.size() == 0) && (mixAux_KillEnd)) {
				this->Set_MixType(AUTO_KILL_SERVER);
				return timeMix;
			}
			if ((deletedPeer.size() == 0)) {
				this->Set_MixType(NONE_MIX);
				return timeMix;
			}
			boost::mutex::scoped_lock peerListLock(peerListMutex);
			for (set<string>::iterator it = deletedPeer.begin(); it != deletedPeer.end(); it++){
				this->peerList[*it].SetChannelId_Sub(0);
				this->DisconnectPeer(*it, &peerActiveOut);
			}
			peerListLock.unlock();
		}
			break;

		case (NONE_MIX): //++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			cout<<"Noting to do in merge stage"<<endl;
		}
			break;
	    default:
	        cout<<"Invalid Mix Server option ["<<this->Get_MixType()<<"]"<<endl;
	        break;
		}
	}
	return timeMix;
}


//Configure only when server got a new stage sub channel
void PeerManagerServerAux::SetPeerManagerState(ServerAuxTypes newPeerManagerState)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	boost::mutex::scoped_lock peerActiveLock(peerActiveMutexOut);

	switch (newPeerManagerState)
	{
	case SERVER_AUX_ACTIVE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		if (this->peerManagerState == NO_SERVER_AUX)
		{
			peerActiveOut.clear();

			/*ECM peerList_Rejected começa com peerList na criação da rede paralela  para evitar
			 * que um par da rede principal seja conectado como OUT do servidor na rede paraleal.
			 * O FATO É: Caso um peer da rede principal que é IN do servidor seja desconectado,
			 * ele é removido de peerList. Assim, se ele tenta conexão com o servidor aux ativo,
			 * ele é visto como um membro provenente do flash crowd, e não da rede principal.
			 * Para evitar isso, é necessário peerList_Reject = peerList no início da rede paralela
			 * Quando entrar na fase Mesclar, a lista deve voltar a vazia, permitindo conexão
			 * entre o servidor auxliar e os peers da rede principal.
			 */
		    for (map<string,PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
				peerList_Rejected.insert(i->first);
		    this->peerManagerState = newPeerManagerState;
		}
		break;

	case SERVER_AUX_MESCLAR: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		if (this->peerManagerState == SERVER_AUX_ACTIVE)
		{
			peerList_Rejected.clear();
		    for (set<string>::iterator i = peerActiveOut.begin(); i != peerActiveOut.end(); i++){
		    	peerList[*i].SetChannelId_Sub(peerList[*i].GetChannelId_Sub() * (-1));
		    }
		    cout<<"Setting server mix stage ["<<(int)this->Get_MixType()<<"]"<<endl;

			this->peerManagerState = newPeerManagerState;
		}
		break;

	case NO_SERVER_AUX:      //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	    for (set<string>::iterator i = peerActiveOut.begin(); i != peerActiveOut.end(); i++)
	    	peerList[*i].SetChannelId_Sub(0);
	    this->peerList_Rejected.clear();
		this->peerManagerState = newPeerManagerState;
		break;

    default:
        cout<<"Invalid peer state option changing ["<<this->peerManagerState<<"] to ["<<newPeerManagerState<<"]"<<endl;
        break;
	}
	peerActiveLock.unlock();
	peerListLock.unlock();
}

