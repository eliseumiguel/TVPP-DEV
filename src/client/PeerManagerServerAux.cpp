/*
 * PeerManagerServerAux.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: eliseu
 */

#include "PeerManagerServerAux.hpp"
#include "PeerManager.hpp"

/*constructor*/
PeerManagerServerAux::PeerManagerServerAux(): PeerManager (){this->peerManagerState = NO_SERVER_AUX; mergeAux_KillEnd = false;}

/*destructor*/
PeerManagerServerAux::~PeerManagerServerAux() {}

MesclarModeServer PeerManagerServerAux::Get_MergeType(){return this->MergeType;}
uint8_t PeerManagerServerAux::Get_QT_PeerMergeType(){return this->QT_PeerMergeType;}
uint8_t PeerManagerServerAux::Get_TimeDescPeerMerge(){return this->TimeDescPeerMerge;}

void PeerManagerServerAux::Set_MergeType(MesclarModeServer MergeType){
	if (MergeType == 03){
		this->MergeType = (MesclarModeServer) 01; mergeAux_KillEnd = true; return;
	}
	else
		if (MergeType == 04){
			this->MergeType = (MesclarModeServer) 02; mergeAux_KillEnd = true; return;
		}
	this->MergeType = MergeType;
}
void PeerManagerServerAux::Set_QT_PeerMergeType(uint8_t QT_PeerMergeType){this->QT_PeerMergeType = QT_PeerMergeType;}
void PeerManagerServerAux::Set_TimeDescPeerMerge(uint8_t TimeDescPeerMerge){this->TimeDescPeerMerge = TimeDescPeerMerge;}



set<string> PeerManagerServerAux::Get_Random_Merge_Peers(set<string>* peers, uint8_t QT){

	boost::mutex::scoped_lock peerListLock(peerListMutex);
	boost::mutex::scoped_lock peerActiveLock(peerActiveMutexOut);

	set<string> selectedPeers, allMergePeer;

	cout<<"Remove possible peer list [";
	for (set<string>::iterator merge = peers->begin(); merge != peers->end(); merge++)
		if (peerList[*merge].GetChannelId_Sub() < 0){
			allMergePeer.insert(*merge);
			cout<<*merge<<" ";
		}
	cout<<"]"<<endl;

	if (allMergePeer.size() < QT){
		return allMergePeer;
	}
	else{
		for (unsigned int qt=0;  qt<QT; qt++){
			set<string>::iterator it = allMergePeer.begin();
			srand (time (NULL));
			unsigned int sorteado;
			sorteado = rand () % allMergePeer.size();
			for (unsigned pos=0; pos<sorteado; pos++)
				it++;
			selectedPeers.insert(*it);
			allMergePeer.erase(it);
			}
		}
	peerActiveLock.unlock();
	peerListLock.unlock();

	return selectedPeers;
}

uint8_t PeerManagerServerAux::ExecMesc (uint8_t timeMerge){
	timeMerge--;
	if (timeMerge == 0){

		switch (this->Get_MergeType())
		{
		case AUTO_KILL_SERVER: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			{
				cout<<" Time Out "<<(int)this->Get_TimeDescPeerMerge()<<": Server shutting down merge strategy ..."<<endl;
				exit (1);
			}
			break;

		case (AUTO_LIVE_AVOID_CONNECTION): //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			set<string> deletedPeer = this->Get_Random_Merge_Peers( &(this->peerActiveOut), (unsigned int)this->Get_QT_PeerMergeType());

			// AUTO_LIVE_AVOID_CONNECTION_KILLEND
			if ((deletedPeer.size() == 0) && (mergeAux_KillEnd)) {
				this->Set_MergeType(AUTO_KILL_SERVER);
				return timeMerge;
			}

			if ((deletedPeer.size() == 0)) {
				this->Set_MergeType(NONE_MERGE);
				return timeMerge;
			}

			boost::mutex::scoped_lock peerListRejectedLock(peerListRejectedMutexOut);
			for (set<string>::iterator it = deletedPeer.begin(); it != deletedPeer.end(); it++){
			     this->peerList_Rejected.insert(*it);
			     this->DisconnectPeer(*it, &peerActiveOut);
			}

			cout <<"Rejected reconnection list [";
			for(set<string>::iterator i = peerList_Rejected.begin(); i!=peerList_Rejected.end(); i++)
				cout<<*i<<" "; cout<<"]"<<endl;

			peerListRejectedLock.unlock();
		}
			break;

		case (AUTO_LIVE_PERMIT_CONNECTION): //++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			set<string> deletedPeer = this->Get_Random_Merge_Peers(&(this->peerActiveOut), (unsigned int) this->Get_QT_PeerMergeType());

			// AUTO_LIVE_PERMIT_CONNECTION_KillEND
			if ((deletedPeer.size() == 0) && (mergeAux_KillEnd)) {
				this->Set_MergeType(AUTO_KILL_SERVER);
				return timeMerge;
			}
			if ((deletedPeer.size() == 0)) {
				this->Set_MergeType(NONE_MERGE);
				return timeMerge;
			}
			boost::mutex::scoped_lock peerListLock(peerListMutex);
			for (set<string>::iterator it = deletedPeer.begin(); it != deletedPeer.end(); it++){
				this->peerList[*it].SetChannelId_Sub(0);
				this->DisconnectPeer(*it, &peerActiveOut);
			}
			peerListLock.unlock();
		}
			break;

		case (NONE_MERGE): //++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			cout<<"Noting to do in merge stage"<<endl;
		}
			break;
	    default:
	        cout<<"Invalid Merge Server option ["<<this->Get_MergeType()<<"]"<<endl;
	        break;
		}
	}
	return timeMerge;
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
			 * Isso pode ser implementado com o bootstrap informando para cada peer o subchannel
			 * dele em uma FlagSubChannel a cada conexão. Além disso, os peers devem trocar nos pings
			 * a FlagSubChannel para que cada um saiba se pode ou não atendê-lo. Inicialmente, pensei
			 * em deixar isso como controle apenas do bootstrap.
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
		    cout<<"Setting server merge stage ["<<(int)this->Get_MergeType()<<"]"<<endl;

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

