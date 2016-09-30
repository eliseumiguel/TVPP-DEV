/* Alterado: Eliseu César Miguel
 * 13/01/2015
 * Várias alterações para comportar a separação entre In e Out
 */

#include "PeerManager.hpp"

PeerManager::PeerManager(){}


ServerAuxTypes PeerManager::GetPeerManagerState(){return this->peerManagerState;}

unsigned int PeerManager::GetMaxActivePeers(set<string>* peerActive)
{
	if (peerActive == &peerActiveIn) return maxActivePeersIn;
	if (peerActive == &peerActiveOut) return maxActivePeersOut;
	if (peerActive == &peerActiveOutFREE) return maxActivePeersOutFREE; //ECM terceira lista
	return 0;
}

void PeerManager::SetMaxActivePeersIn(unsigned int maxActivePeers){this->maxActivePeersIn = maxActivePeers;}
void PeerManager::SetMaxActivePeersOut(unsigned int maxActivePeers){this->maxActivePeersOut = maxActivePeers;}
void PeerManager::SetMaxActivePeersOutFREE(unsigned int maxActivePeers){this->maxActivePeersOutFREE = maxActivePeers;}

//adiciona peer na lista de vizinhos
bool PeerManager::AddPeer(Peer* newPeer, int sizePeerListOut, int sizePeerListOut_FREE)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	if (peerList.find(newPeer->GetID()) == peerList.end())
	{
		peerList[newPeer->GetID()] = PeerData(newPeer);
		peerList[newPeer->GetID()].SetSizePeerListOutInformed(sizePeerListOut);
		peerList[newPeer->GetID()].SetSizePeerListOutInformed_FREE(sizePeerListOut_FREE);
		if (peerManagerState == SERVER_AUX_ACTIVE)
			peerList[newPeer->GetID()].SetChannelId_Sub(SERVER_AUX_SUB_CHANNEL_ID);
		peerListLock.unlock();
		cout<<"Peer "<<newPeer->GetID()<<" added to PeerList"<<endl;
		return true;
	}
	//atualiza sizePeerListOut caso peer já esteja na lista
	if (sizePeerListOut >= 0)
		peerList[newPeer->GetID()].SetSizePeerListOutInformed(sizePeerListOut);
	    peerList[newPeer->GetID()].SetSizePeerListOutInformed_FREE(sizePeerListOut_FREE);
	peerListLock.unlock();
	return false;
}

void PeerManager::SetMaxOutFreeToBeSeparated(unsigned int outLimitToSeparateFree){this->outLimitToSeparateFree = outLimitToSeparateFree;}

set<string>* PeerManager::GetPeerActiveIn(){return &peerActiveIn;}
//set<string>* PeerManager::GetPeerActiveOut(){return &peerActiveOut;}
//set<string>* PeerManager::GetPeerActiveOutFREE(){return &peerActiveOutFREE;} //ECM terceira lista
set<string>* PeerManager::GetPeerActiveOut(bool separatedFree, uint16_t peerOut){

	if ((separatedFree) && (this->outLimitToSeparateFree >= peerOut))
		return  &peerActiveOutFREE;
	return &peerActiveOut;
}


map<string, unsigned int>* PeerManager::GetPeerActiveCooldown(set<string>* peerActive)
{
	if (peerActive == &peerActiveIn) return &peerActiveCooldownIn;
	if (peerActive == &peerActiveOut) return &peerActiveCooldownOut;
	if (peerActive == &peerActiveOutFREE) return &peerActiveCooldownOutFREE;
	return NULL;
}

//ECM - efetivamente, insere o par em uma das lista In ou Out
//neste método, é certo que o par pertence a peerList...
//Deve ser chamado com o peerListMutex fechado....
bool PeerManager::ConnectPeer(string peer, set<string>* peerActive)
{

	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	map<string, unsigned int>* peerActiveCooldown = this->GetPeerActiveCooldown(peerActive);
	map<string, unsigned int>::iterator i = peerActiveCooldown->find(peer);

	if (i != (*peerActiveCooldown).end())
		cout<<"Peer "<<peer<<" must wait "<<i->second<<" secunds to the next connection "<<endl;
	else
	{
		boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);
		if (peerActive->size() < this->GetMaxActivePeers(peerActive) || (this->removeWorsePartner))
		{

			boost::mutex::scoped_lock peerListRejectedLock(peerListRejectedMutexOut);
			if ((peerList_Rejected.find(peer) != peerList_Rejected.end()) && (peerActive != &peerActiveIn)){
				cout<<"Peer "<<peer<<" is in peerList_Rejected. Could not be connected... "<<endl;
				peerListRejectedLock.unlock();
				return false;
			}
			peerListRejectedLock.unlock();

			/* controle de SERVER AUX ACTIVE.
			 * Para OUT, aceita somente pares da rede paralela
			 */
			if (peerManagerState == SERVER_AUX_ACTIVE && peerActive != &peerActiveIn)
			{
				if (peerList[peer].GetChannelId_Sub() != SERVER_AUX_SUB_CHANNEL_ID)
				{
					if (peerActive == &peerActiveOut)
					cout<<"Peer "<<peer<<" request to be OUT, but its ID channel is "<<peerList[peer].GetChannelId_Sub()<<" and channel sub is "<<SERVER_AUX_SUB_CHANNEL_ID<<endl;
					return false;

					if (peerActive == &peerActiveOutFREE)
					cout<<"Peer "<<peer<<" request to be OUT-FREE, but its ID channel is "<<peerList[peer].GetChannelId_Sub()<<" and channel sub is "<<SERVER_AUX_SUB_CHANNEL_ID<<endl;
					return false;

				}
			}
			bool inserted = false;

			if (peerActive->size() < this->GetMaxActivePeers(peerActive))
			   inserted = peerActive->insert(peer).second;
			else if (peerActive != &peerActiveIn)
                   inserted = this->ConnectSpecial(peer,peerActive);

			if (inserted)
	    	{
		    	string list;
			    if (*(peerActive) == peerActiveIn){
				   this->peerList[peer].SetTTLIn(TTLIn);
				   list = "In";
			    }
			    else{
			    	this->peerList[peer].SetTTLOut(TTLOut);
			    	if (*(peerActive) == peerActiveOut){list = "Out";}
	    		    else {list = "Out-FREE";}
			    }

			    cout<<"Peer "<<peer<<" connected to PeerActive_"<<list<<" TTLIn ["<<this->peerList[peer].GetTTLIn()<<"] TTLOut ["<<this->peerList[peer].GetTTLOut()<<"]"<<endl;
			    peerActiveLock.unlock();
			    return true;
	    	}

		}
		peerActiveLock.unlock();
	}
	return false;
}

bool PeerManager::ConnectSpecial(string peer, set<string>* peerActive){

	if (peerActive->size() == 0 ) return false;

    set<string>::iterator smaller;
    smaller = peerActive->begin();
	for (set<string>::iterator count = peerActive->begin(); count != peerActive->end(); count++)
		if (peerList[*count].GetSizePeerListOutInformed() < peerList[*smaller].GetSizePeerListOutInformed())
			smaller = count;
	if (peerList[*smaller].GetSizePeerListOutInformed() < peerList[peer].GetSizePeerListOutInformed())
	{
		string text="";
		if (*(peerActive) == peerActiveOut){text = "Out";}
		else {text = "Out-FREE";}
		cout <<"removing "<<*smaller<<" "<<text<<" list["<<peerList[*smaller].GetSizePeerListOutInformed()<<"] to insert "<<peer<<" out list["<<peerList[peer].GetSizePeerListOutInformed()<<"]"<<endl;
		peerActive->erase(smaller);
		peerActive->insert(peer);
		this->SetRemoveWorsePartner(false);
		return true;
	}
	return false;
}


void PeerManager::DisconnectPeer(string peer, set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	map<string, unsigned int>* peerActiveCooldown = this->GetPeerActiveCooldown(peerActive);
	boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);
	peerActive->erase(peer);
	peerActiveLock.unlock();
	(*peerActiveCooldown)[peer] = PEER_ACTIVE_COOLDOWN;

	string list;
	if (*(peerActive) == peerActiveIn)	list = "In";
	else{
		if (*(peerActive) == peerActiveOut)	list = "Out";
		else list = "Out-Free";
	}
    cout<<"Peer "<<peer<<" disconnected from PeerActive_"<<list<<endl;
}


void PeerManager::RemovePeer(string peer)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	cout<<"Peer "<<peer<<" removed from PeerList: TTLIn["<<peerList[peer].GetTTLIn()<<"] TTLOut["<<peerList[peer].GetTTLOut()<<"]"<<endl;
	peerList.erase(peer);
	peerListLock.unlock();
}

unsigned int PeerManager::GetPeerActiveSize(set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);
	unsigned int size = peerActive->size();
	peerActiveLock.unlock();
	return size;
}

// Gera o total de parceiros somando In e Out sem repeticoes
unsigned int PeerManager::GetPeerActiveSizeTotal()
{
	unsigned int size = this->GetPeerActiveSize(&peerActiveIn);
	boost::mutex::scoped_lock peerActiveInLock(peerActiveMutexIn);
	boost::mutex::scoped_lock peerActiveOutLock(peerActiveMutexOut);

	for (set<string>::iterator i = peerActiveOut.begin(); i != peerActiveOut.end(); i++)
	{
		if (peerActiveIn.find(*i) == peerActiveIn.end())
			size++;
	}
	peerActiveOutLock.unlock();
	boost::mutex::scoped_lock peerActiveOutFREELock(peerActiveMutexOutFREE);

	for (set<string>::iterator i = peerActiveOutFREE.begin(); i != peerActiveOutFREE.end(); i++)
	{
		if (peerActiveIn.find(*i) == peerActiveIn.end())
			size++;
	}

	peerActiveInLock.unlock();
	peerActiveOutFREELock.unlock();
    return size;
}

bool PeerManager::IsPeerInPeerList(string peer)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	   if (this->peerList.find (peer) != peerList.end())
		   return true;
	   peerListLock.unlock();
	return false;
}

bool PeerManager::IsPeerActive(string peer,set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);
	if (peerActive->find(peer) != peerActive->end())
	{
		peerActiveLock.unlock();
		return true;
	}
	peerActiveLock.unlock();
	return false;
}

PeerData* PeerManager::GetPeerData(string peer){return &peerList[peer];}

map<string, PeerData>* PeerManager::GetPeerList(){return &peerList;}

boost::mutex* PeerManager::GetPeerListMutex(){return &peerListMutex;}

boost::mutex* PeerManager::GetPeerActiveMutex(set<string>* peerActive)
{
	if (peerActive == &peerActiveIn) return &peerActiveMutexIn;
	if (peerActive == &peerActiveOut) return &peerActiveMutexOut;
	if (peerActive == &peerActiveOutFREE) return &peerActiveMutexOutFREE; //ECM terceira lista
	return NULL;
}

//ECM metodo privado criado para ser chamado duas vezes (In e Out e Out-FREE) em CheckPeerList()
void PeerManager::CheckpeerActiveCooldown(map<string, unsigned int>* peerActiveCooldown)
{
	set<string> deletedPeer;
	for (map<string, unsigned int>::iterator i = peerActiveCooldown->begin(); i != peerActiveCooldown->end(); i++)
	   {
		i->second--;
		if (i->second == 0)
			deletedPeer.insert(i->first);
	}
	for (set<string>::iterator i = deletedPeer.begin(); i != deletedPeer.end(); i++) {peerActiveCooldown->erase(*i);}
	deletedPeer.clear();
}

void PeerManager::CheckPeerList()
{
	//ECM Tabela de decisao para remover peer.

	//|  ttlOut      PeerActiveOUtFREE                                Desconectar OutFREE                   caso
	//|      0           pertence                                             x                               0

	//|----------------------------------------------------------------------------------------------------------|
	//| ttlIn ttlOut PeerActiveIn    PeerActiveOut |  Desconectar In | Desconectar Out | Remover PeerList | caso |
	//|----------------------------------------------------------------------------------------------------------|
	//|   0    <>0     pertence        pertence    |       X         |                 |                  |   1  |
	//|  <>0    0      pertence        pertence    |                 |        X        |                  |   2  |
	//|   0     0      pertence        pertence    |       X         |        X        |     X            |   3  |
	//|   0    <>0     pertence      nao pertence  |       X         |                 |     X            |   4  |
	//|  <>0    0    nao pertence      pertence    |                 |        X        |     X            |   5  |
    //|----------------------------------------------------------------------------------------------------------|


    set<string> desconectaPeerIn;      //DesconectarIn
    set<string> desconectaPeerOut;     //DesconectarOut
    set<string> desconectaPeerOutFREE; //DesconectarOutFREE
    set<string> deletaPeer;            //Remover

    bool isPeerActiveIn = false;
    bool isPeerActiveOut  = false;
    bool isPeerActiveOutFREE  = false;

    boost::mutex::scoped_lock peerActiveInLock(peerActiveMutexIn);
    boost::mutex::scoped_lock peerActiveOUTLock(peerActiveMutexOut);
    boost::mutex::scoped_lock peerActiveOUTFREELock(peerActiveMutexOutFREE);

    //gera lista com todos os pares ativos
    set<string> temp_allActivePeer (peerActiveIn);
    for (set<string>::iterator i = peerActiveOut.begin(); i != peerActiveOut.end(); i++)
    	temp_allActivePeer.insert(*i);
    for (set<string>::iterator i = peerActiveOutFREE.begin(); i != peerActiveOutFREE.end(); i++)
    	temp_allActivePeer.insert(*i);


    for (set<string>::iterator i = temp_allActivePeer.begin(); i != temp_allActivePeer.end(); i++)
    {
    	isPeerActiveIn = peerActiveIn.find(*i) != peerActiveIn.end();
    	if (isPeerActiveIn)
    	{
    		peerList[*i].DecTTLIn();
    		if (peerList[*i].GetTTLIn() <= 0)
    		{
    			desconectaPeerIn.insert(*i);
    			isPeerActiveIn = false;
    			/*
    			 * problema descoberto.... quando o servidor auxiliar Sa elimina os parceiros da rede principal em peerListOut,
    			 * todos esses parceiros vão decrementar o TTLIn de Sa até chegar em 0. Caso Sa seja parceiro Out destes mesmos
    			 * parceiros, o TTLOut de Sa não chegará a 0 enquanto ele for peerActiveOut e, com isso, ele nunca irá recuperar
    			 * seu TTLIn em cada um de seus parceiros. Assim, ao retornar do processo de servidor auxiliar, ele não será
    			 * visto pelos antigos parceiros como uma possível fonte de dados.
    			 * Uma solução é ele ser free rider bom por algum tempo. Ao mesclar, isso se resolve...(já  implementado no client)
    			 * Esta solução faz com que ele mantenha ping para os antigos contatos durante as atividades de servidor auxiliar e,
    			 * com isso, ele poderá ser uma fonte de dados ao finalizar a rede paralela.
    			*/
    		}
    	}
    	isPeerActiveOut = peerActiveOut.find(*i) != peerActiveOut.end();
    	if (isPeerActiveOut)
    	{
    		peerList[*i].DecTTLOut();
    		if (peerList[*i].GetTTLOut() <= 0)
    	    {
    			desconectaPeerOut.insert(*i);
    			isPeerActiveOut = false;
    		}

    	}
    	isPeerActiveOutFREE = peerActiveOutFREE.find(*i) != peerActiveOutFREE.end();
    	    	if (isPeerActiveOutFREE)
    	    	{
    	    		peerList[*i].DecTTLOut();
    	    		if (peerList[*i].GetTTLOut() <= 0)
    	    	    {
    	    			desconectaPeerOutFREE.insert(*i);
    	    			isPeerActiveOutFREE = false;
    	    		}

    	    	}


    	if ((!isPeerActiveIn) && (!isPeerActiveOut) && (!isPeerActiveOutFREE))
     		deletaPeer.insert(*i);
    }
    peerActiveInLock.unlock();
	peerActiveOUTLock.unlock();
	peerActiveOUTFREELock.unlock();

    for (set<string>::iterator i = desconectaPeerIn.begin(); i != desconectaPeerIn.end(); i++)
    {
    	DisconnectPeer(*i, &peerActiveIn);
    }
    for (set<string>::iterator i = desconectaPeerOut.begin(); i != desconectaPeerOut.end(); i++)
        {
        	DisconnectPeer(*i, &peerActiveOut);
        }

    for (set<string>::iterator i = desconectaPeerOutFREE.begin(); i != desconectaPeerOutFREE.end(); i++)
        {
        	DisconnectPeer(*i, &peerActiveOutFREE);
        }

    for (set<string>::iterator i = deletaPeer.begin(); i != deletaPeer.end(); i++)
    		RemovePeer(*i);

    this->CheckpeerActiveCooldown(&peerActiveCooldownIn);
    this->CheckpeerActiveCooldown(&peerActiveCooldownOut);
    this->CheckpeerActiveCooldown(&peerActiveCooldownOutFREE);
}

int PeerManager::showPeerActive(set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	int j = 0; int ttl = 0; string ttlRotulo("");
	boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);

    for (set<string>::iterator i = (*peerActive).begin(); i != (*peerActive).end(); i++, j++)
	{
    	if (peerActive == &peerActiveIn)
        {
        	ttl = peerList[*i].GetTTLIn();
        	ttlRotulo == "TTLIn";
        }
        else
        {
        	ttl = peerList[*i].GetTTLOut();
        	ttlRotulo == "TTLOut";
        }

	    cout<<"Key: "<<*i<<" ID: "<<peerList[*i].GetPeer()->GetID()<<" Mode: "<<(int)peerList[*i].GetMode()<<ttlRotulo<<": "<<ttl << " PR: "<<peerList[*i].GetPendingRequests() << endl;
	}
	peerActiveLock.unlock();
	return j;

}

void PeerManager::ShowPeerList()
{
 	int k = 0;
    int j = 0;
    int m = 0;
    cout<<endl<<"- Peer List Active -"<<endl;
    k = showPeerActive(&peerActiveIn);
    j = showPeerActive(&peerActiveOut);
    m = showPeerActive(&peerActiveOutFREE);

    cout<<"Total In ["<<k<<"]  Total Out ["<<j<<"] Total Out-FREE ["<<m<<"]"<<endl;
    cout<<"Total different Peers Active: ["<<this->GetPeerActiveSizeTotal()<<"] "<<endl<<endl;
	j = 0;
	cout<<endl<<"- Peer List Total -"<<endl;
	boost::mutex::scoped_lock peerListLock(peerListMutex);

    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++, j++)
	{
		cout<<"Key: "<<i->first<< endl;
		cout<<"ID: "<<i->second.GetPeer()->GetID()<<" Mode: "<<(int)i->second.GetMode()<<" TTLIn: "<<i->second.GetTTLIn() <<" TTLOut: "<<i->second.GetTTLOut() << " RTT(delay): " <<i->second.GetDelay()<< "s PR: "<<i->second.GetPendingRequests() << endl;
	}
	peerListLock.unlock();
	cout<<"Total PeerList: "<<j<<endl<<endl;

}

void PeerManager::SetRemoveWorsePartner (bool removeWorsePartner)
{
	this->removeWorsePartner = removeWorsePartner;
}

bool PeerManager::GetRemoveWorsePartner (){
	return this->removeWorsePartner;
}
