#include "Channel.hpp"

Channel::Channel(unsigned int channelId, Peer* serverPeer,
				 unsigned int maxPeerInSubChannel,
				 unsigned int maxSubChannel,
				 unsigned int maxSubServerAux,
				 bool mesclar)
: channel_Sub_List_Mutex (new boost::mutex()),
  channel_Sub_Candidates_Mutex (new boost::mutex())
{
    if (channelId != 0 || serverPeer != NULL) //Avoid creation by map[]
    {
    	this->channelMode = MODE_NORMAL;
        this->channelId = channelId;
        this->serverPeer = serverPeer;
        if(serverPeer)
            AddPeer(serverPeer);
        serverEstimatedStreamRate      = 0;
        this->maxPeer_ChannelSub       = maxPeerInSubChannel;
        this->max_channelSub           = maxSubChannel;
        this->max_ServerSubCandidate   = maxSubServerAux;
        this->mesclarRedes             = mesclar;
        cout<<"Mesclar Redes = ["<<mesclarRedes<<"]"<<endl;

        //Logging
        struct tm * timeinfo;
        char timestr[20];
        time(&creationTime);
        timeinfo = localtime(&creationTime);
        strftime (timestr,20,"%Y%m%d%H%M",timeinfo);
        string logFilename = "log-";
        logFilename += boost::lexical_cast<string>(channelId) + "-";
        logFilename += serverPeer->GetIP() + "_" + serverPeer->GetPort() + "-";
        logFilename += timestr;
        logFilename += "-";
        string logFilenamePerf = logFilename + "perf.txt";
        string logFilenameOverlay = logFilename + "overlay.txt";
        performanceFile = fopen(logFilenamePerf.c_str(),"w");
        overlayFile = fopen(logFilenameOverlay.c_str(),"w");
    } 
}

void Channel::SetServerNewestChunkID(ChunkUniqueID serverNewestChunkID)
{
    this->serverNewestChunkID = serverNewestChunkID;
}

ChunkUniqueID Channel::GetServerNewestChunkID()
{
    return serverNewestChunkID;
}

void Channel::SetServerEstimatedStreamRate(int serverEstimatedStreamRate)
{
    this->serverEstimatedStreamRate = serverEstimatedStreamRate;
}

int Channel::GetServerEstimatedStreamRate()
{
    return serverEstimatedStreamRate;
}

Peer* Channel::GetServer()
{
    return serverPeer;
}

Peer* Channel::GetPeer(Peer* peer)
{
    if (peerList.count(peer->GetID()) > 0)
        return peerList[peer->GetID()].GetPeer();
    return NULL;
}

bool Channel::HasPeer(Peer* peer)
{
    if (GetPeer(peer))
        return true;
    else
        return false;
}

bool Channel::GetServerSubWaitInform(Peer* peer)
{
	bool waiting = false;
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
	if (server_Sub_Candidates.count(peer->GetID()) > 0)
		waiting = server_Sub_Candidates[peer->GetID()].GetPeerWaitInform();
	channelSubCandidatesLock.unlock();
	return waiting;
}

void Channel::SetServerSubWaitInform(Peer* peer, bool waiting)
{
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
	if (server_Sub_Candidates.count(peer->GetID()) > 0)
		this->server_Sub_Candidates[peer->GetID()].SetPeerWaitInfor(waiting);
	channelSubCandidatesLock.unlock();
}

ServerAuxTypes Channel::GetServerSubNewMode (Peer* peer)
{
	ServerAuxTypes serverAuxMode;
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
	if (server_Sub_Candidates.count(peer->GetID()) > 0)
		serverAuxMode = this->server_Sub_Candidates[peer->GetID()].GetState();
	channelSubCandidatesLock.unlock();
	return serverAuxMode;
}

/* ECM ****
 * Insere o peer no canal principal se MODE == NORMAL ou em um sub-canal se MODE == FLASH CROWD
 * Em MODE FLASH CROWD: caso todos sub-canais estejam cheios, tenta criar novo sub canal e inserir
 *                      caso não consiga criar novo canal, insere na rede principal.
 */
void Channel::AddPeer(Peer* peer)
{
	if (!this->AddPeerChannel(peer))                                      // tenta inserir enquanto sub canais permitire
	{
        if(!Create_New_ChannelSub())
        {
           	cout<<"Subcanais Esgotados."<<endl;
           	cout<<"Inserindo ["<<peer->GetID()<<"] no canal principal"<<endl;
           	peerList[peer->GetID()] = PeerData(peer);                     // insere no canal prinipal
           	peerList[peer->GetID()].SetChannelId_Sub(this->channelId);
        }
        else	       	                                                  // tenta criar novo sub canal
        	if (!this->AddPeerChannel(peer))
        	{
        		peerList[peer->GetID()] = PeerData(peer);                 // insere no canal prinipal;
        		peerList[peer->GetID()].SetChannelId_Sub(this->channelId);
        	}
	}
}

bool Channel::AddPeerChannel(Peer* peer)
{
	if (channelMode == MODE_NORMAL)
	{
		peerList[peer->GetID()] = PeerData(peer);
		peerList[peer->GetID()].SetChannelId_Sub(this->channelId);
       	cout<<"Inserindo ["<<peer->GetID()<<"] no canal principal"<<endl;
		return true;
	}
	else //FLASH CROWD
	{
		//tenta inserir em sub canal
		boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);

		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (this->GetPeerListSizeChannel_Sub(i->second.GetchannelId_Sub()) < (this->maxPeer_ChannelSub) && (!i->second.GetMesclando()))
			{
				peerList[peer->GetID()] = PeerData(peer);
				peerList[peer->GetID()].SetChannelId_Sub(i->second.GetchannelId_Sub());

		       	cout<<"Inserindo ["<<peer->GetID()<<"] SUBCHANNEL ["<<i->second.GetchannelId_Sub()<<"]"<<endl;
				channelSubListLock.unlock();
				return true;
	        }
		channelSubListLock.unlock();
	}
	return false;
}

bool Channel::Create_New_ChannelSub()
{
	string* peerServerAuxNew = NULL;

	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);

	for (map<string,SubChannelCandidateDate>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
		if (channel_Sub_List.count(i->first) == 0 )
			peerServerAuxNew =  new string(i->first);

	channelSubCandidatesLock.unlock();
	if (peerServerAuxNew && channel_Sub_List.size() < max_channelSub)  //cria sub-channel
	{
		unsigned int channelID_New = 0;

		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (channelID_New < i->second.GetchannelId_Sub())
				channelID_New = i->second.GetchannelId_Sub();
		channelID_New ++;
		channel_Sub_List[*peerServerAuxNew] = SubChannelData(channelId, channelID_New, peerList[*peerServerAuxNew].GetPeer() );//,0,0);
		channelSubListLock.unlock();
		return true;
	}
	channelSubListLock.unlock();
	return false;
}

/* Atualmente, este método seleciona para servidor auxiliar os primeiros peers que entram no canal
 * Assim, são escolhidos os peers mais próximos do servidor principal.
 * Outras estratégias podem ser idealizadas considerando a qualidade dos pares (contracts article)
 */
void Channel::analizePeerToBeServerAux(Peer* source)
{
	if (this->channelMode == MODE_NORMAL)
	{
		boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
		if (this->HasPeer(source) && server_Sub_Candidates.size() < this->max_ServerSubCandidate && source->GetID() != this->GetServer()->GetID())
		{
			server_Sub_Candidates[source->GetID()] = SubChannelCandidateDate();
		}
		channelSubCandidatesLock.unlock();
		//printPossibleServerAux();
	}
}

// ECM usada para teste em implementação
void Channel::printChannelProfile()
{
	cout<<"###-------"<<endl;
	cout<<"Channel ["<<channelId<<"] profile:"<<endl;
	cout<<"T Peers ["<<peerList.size()<<"] T SChannel ["<<channel_Sub_List.size()<<"] ";

	//cout<<"Servidor Auxiliar Disponívl"<<endl;
	int auxiliarServerDisponiveis = 0;
	for (map<string,SubChannelCandidateDate>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
	{
		//cout<<" ["<<i->first<<"] ";
		auxiliarServerDisponiveis++;
	}
	cout<<"EnableSS: ["<<auxiliarServerDisponiveis<<"]"<<endl;

    cout<<"#"<<endl;
	//cout<<"List of subServer and subChannel **"<<endl;

	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
	{
		int totalPeerInSubChannel=0;
		for (map<string, PeerData>::iterator j = peerList.begin(); j != peerList.end(); j++)
	    	if (j->second.GetChannelId_Sub() == i->second.GetchannelId_Sub())
	    		totalPeerInSubChannel++;
			cout<<"["<<i->first<<"] scID ["<< i->second.GetchannelId_Sub()<<"] TPeerSub ["<<totalPeerInSubChannel<<"] Life ["<<i->second.GetChannelLife()<<"] Mesc ["<<i->second.GetMesclando()<<"] ReNeW ["<<i->second.GetReNewServerSub()<<"]"<<endl;
		    //cout<<"scID ["<< i->second.GetchannelId_Sub()<<"] TPeerSub ["<<totalPeerInSubChannel<<"] TLife ["<<i->second.GetChannelLife()<<"] Mesclando? ["<<i->second.GetMesclando()<<"] ReNeW ["<<i->second.GetReNewServerSub()<<"]"<<endl;
	}
	cout<<"###-------"<<endl;
}

void Channel::RemovePeer(Peer* peer)
{
    peerList.erase(peer->GetID());
}

void Channel::RemovePeer(string peerId)
{
    peerList.erase(peerId);
}

PeerData& Channel::GetPeerData(Peer* peer)
{
    return peerList[peer->GetID()];
}

void Channel::SetChannelMode(ChannelModes channelMode)
{
	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);

	if (this->channelMode == MODE_FLASH_CROWD  && channelMode == MODE_NORMAL)
	{

		/*faz com que todos os pares passem a pertencentes ao canal principal
		 * Neste ponto deverá ser tratada a mesclagem das redes
		 * o mutex da lista peerList é fechado no bootstrap,
		 * onde a mensagem para mudar o estado é tratada.
		 */
		if (!mesclarRedes)
		{
			for (map<string,SubChannelCandidateDate>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
				if (i->second.GetState()!= NO_SERVER_AUX)
				{
					i->second.SetState(NO_SERVER_AUX);
					i->second.SetPeerWaitInfor(true);
				}
			this->Remove_AllChannelSub(mesclarRedes);
		}
	}
	else // por enquanto, seria alterar de normal para flash crowd. (ou não alterar)

		for (map<string,SubChannelCandidateDate>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
		{
			i->second.SetState(SERVER_AUX_ACTIVE);
			i->second.SetPeerWaitInfor(true);
		}

	this->channelMode = channelMode;

	channelSubListLock.unlock();
	channelSubCandidatesLock.unlock();
}

void Channel::Remove_AllChannelSub(bool mesclar)
{
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
		i->second.SetChannelId_Sub(this->channelId);

    if (!mesclar)
    	channel_Sub_List.clear();
}

/* TODO ECM conferir o mutex que está em bootstrap
 * sempre na chamada em caso de remoção de apenas um canal,
 * o que irá acontecer ao mesclar redes.
 */
void Channel::Remove_ChannelSub(const string* source, bool mesclar)
{
	cout<<"Mesclando do pares de: ["<<*source<<"] na rede principal *****"<<endl;
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
    	if (i->second.GetChannelId_Sub() == channel_Sub_List[*source].GetchannelId_Sub())
    		i->second.SetChannelId_Sub(this->channelId);
    if (!mesclar)
    {
    	channel_Sub_List.erase(*source);
    	cout<<"Removendo o sub canal definitivamente"<<endl;
    }
}
ChannelModes Channel::GetChannelMode()
{
	return channelMode;
}

void Channel::SetmaxPeer_ChannelSub(int unsigned maxpeerChannelSub)
{
	this->maxPeer_ChannelSub = maxpeerChannelSub;
}

/* ECM ****
 * Faz a busca considerando qual subcanal o peer está.
 * Esse procedimento já trata, por si só, o flash crowd. Isso porque,
 * caso não esteja em flash crowd, todos os peers têm channelId_Sub = 0
 * OBS: Não tem mutex pois é chamado e travado no bootstrap
 */
vector<PeerData*> Channel::SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity)
{
    vector<PeerData*> allPeers, selectedPeers;
    int unsigned peerChannelId_Sub = peerList[srcPeer->GetID()].GetChannelId_Sub();  //descobre o subcanal do par requisitante

    if (peerChannelId_Sub != this->channelId)
    	/* Se o par está em sub canal, incluir o servidor auxiliar na lista de parceiros dele
    	* isso é importante visto que o serverAux->idChannel_Sub é igual ao idChannle do canal principal
    	* Os servidores auxiliares têm o idChannel_sub na estrutura SubChannelData
    	*/
    {
    	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
    	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
    		if (peerChannelId_Sub == i->second.GetchannelId_Sub())
    			allPeers.push_back(&(peerList[i->second.GetServer_Sub()->GetID()]));
    	channelSubListLock.unlock();
    }

    /* Garante fazer a vizinhança apenas com os pares que estão no mesmo subcanal
     * Desta forma, a estratégia de selação já atua na lista allpeers selecionanda
     * Também, o próprio par não é sugerido a ele mesmo como vizinho
     */
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
        if (srcPeer->GetID() != i->second.GetPeer()->GetID() && peerChannelId_Sub == i->second.GetChannelId_Sub())
            allPeers.push_back(&(i->second));

    if (this->GetPeerListSizeChannel_Sub(peerChannelId_Sub) <= peerQuantity)
        return allPeers;
    else
    {
        strategy->Execute(&allPeers, srcPeer, peerQuantity);
        selectedPeers.insert(selectedPeers.begin(),allPeers.begin(),allPeers.begin()+peerQuantity);
        return selectedPeers;
    }
}

/* ECM ***
  * retorna o tamanho da lista do peer (subcanal ou canal principal)
  * caso não esteja em flash crowd, retorna peerList.size (todos peers em canal principal)
  */
unsigned int Channel::GetPeerListSizeChannel_Sub(unsigned int channelId_Sub)
{
	unsigned int count = 0;
	for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
	{
		if (channelId_Sub == i->second.GetChannelId_Sub())
			count++;
	}
	return count;
}

//***************************************************************************

time_t Channel::GetCreationTime()
{
    return creationTime;
}
unsigned int Channel::GetPeerListSize()
{
    return peerList.size();
}

//Mutex fechados em CheckActiveList
//Entra neste método se o mesclarRedes == true
void Channel::CheckAllSubChannel()
{
	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
	{
		i->second.DecChannelLif();
		i->second.DecReNewServerSub();

		//servidor auxiliar pode ser usado em outro subcanal
		if (i->second.GetReNewServerSub() == 0)
		{
			this->Remove_ChannelSub(&(i->first), false);
			i->second.SetMesclando(false);

			server_Sub_Candidates[i->first].SetState(NO_SERVER_AUX);
			server_Sub_Candidates[i->first].SetPeerWaitInfor(true);
			cout<<"&&&&&& REMOVENDO &&&&& subChannel ["<<i->second.GetchannelId_Sub()<<"]"<<endl;
		}
		else
			//servidor permanece em estado de mesclagem
			if (i->second.GetChannelLife() == 0)
			{
				/* ECM Neste deve enviar uma mensagem ao cliente
				 * definindo o estado de fim de exclusividade e
				 * o indicando para ele executar a mesclagem
				 */
				this->Remove_ChannelSub(&(i->first), true);
				i->second.SetMesclando(true);

				server_Sub_Candidates[i->first].SetState(SERVER_AUX_MESCLAR);
				server_Sub_Candidates[i->first].SetPeerWaitInfor(true);
				cout<<"&&&&& MESCLANDO &&&&& subChannel ["<<i->second.GetchannelId_Sub()<<"]"<<endl;
			}
	}
}

//não é necessário saber em qual subcanal o peer está
//considerar testar se um servidorAuxiliar entrou para remoção
void Channel::CheckActivePeers()
{
    vector<string> deletedPeer;
    for (map<string,PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++) 
    {
        i->second.DecTTLChannel();
        if (i->second.GetTTLChannel() <= 0)
            deletedPeer.push_back(i->first);
    }

	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);

    for (vector<string>::iterator peerId = deletedPeer.begin(); peerId < deletedPeer.end(); peerId++)
    {
    	/*TODO ECM IMPORTANTE: reavaliar essa remoção.
    	*	Aqui, os pares da rede paralela serão automaticamente enviados à rede principal
    	*	em caso de churn do servidor auxiliar
    	*/
    	if (channel_Sub_List.find(*peerId) != channel_Sub_List.end())
    		this->Remove_ChannelSub(&(*peerId));
    	if (server_Sub_Candidates.find(*peerId) != server_Sub_Candidates.end())
    		server_Sub_Candidates.erase(*peerId);

    	RemovePeer(*peerId);
    }
    cout<<"Mesclar redes é "<<mesclarRedes<<endl;
    if (mesclarRedes)
    	this->CheckAllSubChannel();
    /*usado para testes de implementação...
     * pode ser removida em outro momento*/
     this->printChannelProfile();

    channelSubListLock.unlock();
    channelSubCandidatesLock.unlock();
}

void Channel::PrintPeerList()
{
    cout<<"Channel ["<<channelId<<"] Tip["<<serverNewestChunkID<<"] Rate["<<serverEstimatedStreamRate<<"] Peer List:"<<endl;
    for (map<string,PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
        cout<<"PeerID: "<<i->first<<" Mode: "<<(int)i->second.GetMode()<<" TTL: "<<i->second.GetTTLChannel()<<endl;
}

FILE* Channel::GetPerformanceFile(Peer* srcPeer)
{
	if (peerList[srcPeer->GetID()].GetChannelId_Sub() != this->channelId)
		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (peerList[srcPeer->GetID()].GetChannelId_Sub() == i->second.GetchannelId_Sub())
					return i->second.GetPerformanceFile();
	return performanceFile;
}

FILE* Channel::GetOverlayFile(Peer* srcPeer)
{
	if (peerList[srcPeer->GetID()].GetChannelId_Sub() != this->channelId)
		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (peerList[srcPeer->GetID()].GetChannelId_Sub() == i->second.GetchannelId_Sub())
					return i->second.GetOverlayFile();

    return overlayFile;
}
