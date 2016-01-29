/*
 * PeerManagerServerAux.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: eliseu
 */

#include "PeerManagerServerAux.hpp"
#include "PeerManager.hpp"

/*constructor*/
PeerManagerServerAux::PeerManagerServerAux(): PeerManager (){this->peerManagerState = NO_SERVER_AUX;}

/*destructor*/
PeerManagerServerAux::~PeerManagerServerAux() {}

MesclarModeServer PeerManagerServerAux::Get_MixType(){return this->MixType;}
uint8_t PeerManagerServerAux::Get_QT_PeerMixType(){return this->QT_PeerMixType;}
uint8_t PeerManagerServerAux::Get_TimeDescPeerMix(){return this->TimeDescPeerMix;}

void PeerManagerServerAux::Set_MixType(MesclarModeServer MixType){this->MixType = MixType;}
void PeerManagerServerAux::Set_QT_PeerMixType(uint8_t QT_PeerMixType){this->QT_PeerMixType = QT_PeerMixType;}
void PeerManagerServerAux::Set_TimeDescPeerMix(uint8_t TimeDescPeerMix){this->TimeDescPeerMix = TimeDescPeerMix;}


set<string> PeerManagerServerAux::Get_Random_Mix_Peers(set<string>* peers, uint8_t QT){
	set<string> selectedPeers, allMixPeer;

	for (set<string>::iterator mix = peers->begin(); mix != peers->end(); mix++)
		if (peerList[*mix].GetChannelId_Sub() < 0){
			allMixPeer.insert(*mix);
			cout<<" Possível removido em breve..."<<*mix<<endl;
		}

	cout<< "lista de possíveis removidos tem "<<allMixPeer.size()<<" e quantidade escolhida é "<<(int)QT<<endl;

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
	return selectedPeers;
}

uint8_t PeerManagerServerAux::ExecMesc (uint8_t timeMix){
	timeMix--;
	if (timeMix == 0){
		boost::mutex::scoped_lock peerListLock(peerListMutex);
		boost::mutex::scoped_lock peerActiveLock(peerActiveMutexOut);

		switch (this->Get_MixType())
		{
		case AUTO_KILL_SERVER: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			if (true)
			{
				cout<<" Time Out "<<(int)this->Get_TimeDescPeerMix()<<": Server shutting down Mix strategy ..."<<endl;
				cout<<" done 01"<<endl;
				exit (1);
			}
			break;

		case AUTO_LIVE_AVOID_CONNECTION: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			set<string> deletedPeer = this->Get_Random_Mix_Peers( &(this->peerActiveOut), (unsigned int)this->Get_QT_PeerMixType());

			cout<<"Foram selecionados "<<deletedPeer.size()<<" para remoção na mesclagem"<<endl;

			boost::mutex::scoped_lock peerListRejectedLock(peerListRejectedMutexOut);
			for (set<string>::iterator it = deletedPeer.begin(); it != deletedPeer.end(); it++){
			     this->peerList_Rejected.insert(*it);
			     this->peerActiveOut.erase(*it);
				 cout<<"Removendo o peer "<<*it<<" na mesclagem"<<endl;
			}

			cout <<"rejeitados para reconexão:"<<endl;
			for(set<string>::iterator i = peerList_Rejected.begin(); i!=peerList_Rejected.end(); i++)
				cout<<*i<<" "<<endl;

			peerListRejectedLock.unlock();
			cout<<"done 01"<<endl;

		}
			break;

		case AUTO_LIVE_PERMIT_CONNECTION: //++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			set<string> deletedPeer = this->Get_Random_Mix_Peers(&(this->peerActiveOut), (unsigned int) this->Get_QT_PeerMixType());

			cout<<"Foram selecionados "<<deletedPeer.size()<<" para remoção na mesclagem"<<endl;

			for (set<string>::iterator it = deletedPeer.begin(); it != deletedPeer.end(); it++){
				map<string, unsigned int>* peerActiveCooldown = this->GetPeerActiveCooldown(&peerActiveOut);
				(*peerActiveCooldown)[*it] = PEER_ACTIVE_COOLDOWN;
			    this->peerActiveOut.erase(*it);
			    cout<<"Removendo o peer "<<*it<<" na mesclagem"<<endl;

			}
			cout<<"done 02"<<endl;
		}
			break;

	    default:
	        cout<<"Invalid Mix Server option ["<<this->Get_MixType()<<"]"<<endl;
	        break;
		}
		peerActiveLock.unlock();
		peerListLock.unlock();

		cout<<"executou mesclagem tempo "<<(int)timeMix<<endl;
	}
	return timeMix;
}

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
//		    for (map<string,PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
//				peerList_Rejected.insert(i->first);
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
        cout<<"Invalid peer state option ["<<newPeerManagerState<<"]"<<endl;
        break;
	}
	peerActiveLock.unlock();
	peerListLock.unlock();
}

