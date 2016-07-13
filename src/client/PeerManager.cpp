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
	return 0;
}

void PeerManager::SetMaxActivePeersIn(unsigned int maxActivePeers){this->maxActivePeersIn = maxActivePeers;}
void PeerManager::SetMaxActivePeersOut(unsigned int maxActivePeers){this->maxActivePeersOut = maxActivePeers;}

bool PeerManager::AddPeer(Peer* newPeer)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	if (peerList.find(newPeer->GetID()) == peerList.end())
	{
		//ECM Inserting peer....
		peerList[newPeer->GetID()] = PeerData(newPeer);
		if (peerManagerState == SERVER_AUX_ACTIVE)
			peerList[newPeer->GetID()].SetChannelId_Sub(SERVER_AUX_SUB_CHANNEL_ID);
		peerListLock.unlock();
		cout<<"Peer "<<newPeer->GetID()<<" added to PeerList"<<endl;
		return true;
	}
	peerListLock.unlock();
	return false;
}

set<string>* PeerManager::GetPeerActiveIn(){return &peerActiveIn;}
set<string>* PeerManager::GetPeerActiveOut(){return &peerActiveOut;}

map<string, unsigned int>* PeerManager::GetPeerActiveCooldown(set<string>* peerActive)
{
	if (peerActive == &peerActiveIn) return &peerActiveCooldownIn;
	if (peerActive == &peerActiveOut) return &peerActiveCooldownOut;
	return NULL;
}

//ECM - efetivamente, insere o par em uma das lista In ou Out
//neste método, é certo que o par pertence a peerList...
//Deve ser chamado com o peerListMutex fechado....
bool PeerManager::ConnectPeer(string peer, set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	map<string, unsigned int>* peerActiveCooldown = this->GetPeerActiveCooldown(peerActive);

	for (map<string, unsigned int>::iterator i = peerActiveCooldown->begin(); i != peerActiveCooldown->end(); i++)
		if ((i->first == peer) && (i != peerActiveCooldown->end()))
			cout<<"Peer"<<peer<<" must wait "<<i->second<<" secunds to the next connection "<<endl;

	if (peerActiveCooldown->find(peer) == (*peerActiveCooldown).end())
	{
		boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);
		//ECM Inserir aqui condição para inserir caso escolhida a estratégia de desconectar o com menor banda ...
		if (peerActive->size() < this->GetMaxActivePeers(peerActive))// || (desconectarMenorBanda)
		{

			boost::mutex::scoped_lock peerListRejectedLock(peerListRejectedMutexOut);
			if ((peerList_Rejected.find(peer) != peerList_Rejected.end()) && (peerActive == &peerActiveOut)){
				cout<<"Peer "<<peer<<" is in peerList_Rejected. Could not be connected... "<<endl;
				peerListRejectedLock.unlock();
				return false;
			}

			/* controle de SERVER AUX ACTIVE.
			 * Para OUT, aceita somente pares da rede paralela
			 */
			if (peerManagerState == SERVER_AUX_ACTIVE && peerActive == &peerActiveOut){
				if (peerList[peer].GetChannelId_Sub() != SERVER_AUX_SUB_CHANNEL_ID)
				{
					cout<<"Peer "<<peer<<" request to be OUT, but its ID channel is "<<peerList[peer].GetChannelId_Sub()<<" and channel sub is "<<SERVER_AUX_SUB_CHANNEL_ID<<endl;
					return false;
				}
			}

			peerListRejectedLock.unlock();

			if (peerActive->insert(peer).second)
			{
				string list;
				if (*(peerActive) == peerActiveIn){
					this->peerList[peer].SetTTLIn(TTLIn);
					list = "In";
				}
				else {
					this->peerList[peer].SetTTLOut(TTLOut);
					list = "Out";
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
	else list = "Out";
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
	peerActiveInLock.unlock();
	peerActiveOutLock.unlock();
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
	return NULL;
}

//ECM metodo privado criado para ser chamado duas vezes (In e Out) em CheckPeerList()
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
	//|----------------------------------------------------------------------------------------------------------|
	//| ttlIn ttlOut PeerActiveIn    PeerActiveOut |  Desconectar In | Desconectar Out | Remover PeerList | caso |
	//|----------------------------------------------------------------------------------------------------------|
	//|   0    <>0     pertence        pertence    |       X         |                 |                  |   1  |
	//|  <>0    0      pertence        pertence    |                 |        X        |                  |   2  |
	//|   0     0      pertence        pertence    |       X         |        X        |     X            |   3  |
	//|   0    <>0     pertence      nao pertence  |       X         |                 |     X            |   4  |
	//|  <>0    0    nao pertence      pertence    |                 |        X        |     X            |   5  |
    //|----------------------------------------------------------------------------------------------------------|

    set<string> desconectaPeerIn;  //DesconectarIn
    set<string> desconectaPeerOut; //DesconectarOut
    set<string> deletaPeer;        //Remover

    bool isPeerActiveIn = false;
    bool isPeerActiveOut  = false;

    boost::mutex::scoped_lock peerActiveInLock(peerActiveMutexIn);
    boost::mutex::scoped_lock peerActiveOUTLock(peerActiveMutexOut);

    //gera lista com todos os pares ativos
    set<string> temp_allActivePeer (peerActiveIn);
    for (set<string>::iterator i = peerActiveOut.begin(); i != peerActiveOut.end(); i++)
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
    	if ((!isPeerActiveIn) && (!isPeerActiveOut))
     		deletaPeer.insert(*i);
    }
    peerActiveInLock.unlock();
	peerActiveOUTLock.unlock();

    for (set<string>::iterator i = desconectaPeerIn.begin(); i != desconectaPeerIn.end(); i++)
    {
    	DisconnectPeer(*i, &peerActiveIn);
    }
    for (set<string>::iterator i = desconectaPeerOut.begin(); i != desconectaPeerOut.end(); i++)
        {
        	DisconnectPeer(*i, &peerActiveOut);
        }

    for (set<string>::iterator i = deletaPeer.begin(); i != deletaPeer.end(); i++)
    		RemovePeer(*i);

    this->CheckpeerActiveCooldown(&peerActiveCooldownIn);
    this->CheckpeerActiveCooldown(&peerActiveCooldownOut);
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
    cout<<endl<<"- Peer List Active -"<<endl;
    k = showPeerActive(&peerActiveIn);
    j = showPeerActive(&peerActiveOut);
    cout<<"Total In ["<<k<<"]  Total Out ["<<j<<"]"<<endl;
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

