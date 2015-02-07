#include "Channel.hpp"

Channel::Channel(unsigned int channelId, Peer* serverPeer)
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
        serverEstimatedStreamRate = 0;
        this->maxPeer_ChannelSub = MAXPEER_CHANNELSUB;
        this->max_channelSub =     MAX_CHANNELSUB;

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

/* ECM ****
 * Insere o peer no canal principal se MODE == NORMAL ou em um sub-canal se MODE == FLASH CROWD
 * Em MODE FLASH CROWD: caso todos sub-canais estejam cheios, tenta criar novo sub canal e inserir
 *                      caso não consiga criar novo canal, insere na rede principal.
 */
void Channel::AddPeer(Peer* peer)
{
	if (!this->AddPeerChannel(peer))                                      // tenta inserir enquanto sub canais permitire
	{
        if(!Creat_New_ChannelSub())
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
			if (this->GetPeerListSizeChannel_Sub(i->second.GetchannelId_Sub()) < (this->maxPeer_ChannelSub))
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

bool Channel::Creat_New_ChannelSub()
{
	string* peerServerAuxNew = NULL;

	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);

	for (set<string>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
		if (channel_Sub_List.count(*i) == 0)
			peerServerAuxNew =  new string(*i);

	channelSubCandidatesLock.unlock();
	if (peerServerAuxNew && channel_Sub_List.size() < max_channelSub)  //cria sub-channel
	{
		unsigned int channelID_New = 0;

		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (channelID_New < i->second.GetchannelId_Sub())
				channelID_New = i->second.GetchannelId_Sub();
		channelID_New ++;
		channel_Sub_List[*peerServerAuxNew] = SubChannelData(channelId, channelID_New, peerList[*peerServerAuxNew].GetPeer());
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
		if (this->HasPeer(source) && server_Sub_Candidates.size() < MAXPEER_CHANNEL_SUB_CANDIDATE && source->GetID() != this->GetServer()->GetID())
			server_Sub_Candidates.insert(source->GetID());
		channelSubCandidatesLock.unlock();
		//printPossibleServerAux();
	}
}

// ECM usada para teste em implementação
void Channel::printChannelProfile()
{
	cout<<"###----------------------------------------###"<<endl;
	cout<<"Channel ["<<channelId<<"] profile:"<<endl;
	cout<<"Total Peers ["<<peerList.size()<<"]"<<endl;
	cout<<"Total SubChannel ["<<channel_Sub_List.size()<<"]"<<endl;
	cout<<"Servidores Auxiliares disponíveis"<<endl;
	int auxiliarServerDisponiveis = 0;
	for (set<string>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
	{
		cout<<" ["<<*i<<"] ";
		auxiliarServerDisponiveis++;
	}
	cout<<"Total ["<<auxiliarServerDisponiveis<<"]"<<endl;

    cout<<"#"<<endl;
	cout<<"List of subServer and subChannel **"<<endl;

	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
	{
		int totalPeerInSubChannel=0;
		for (map<string, PeerData>::iterator j = peerList.begin(); j != peerList.end(); j++)
	    	if (j->second.GetChannelId_Sub() == i->second.GetchannelId_Sub())
	    		totalPeerInSubChannel++;
			cout<<"PServer ["<<i->first<<"] SChannel ["<< i->second.GetchannelId_Sub()<<"] TPeerSub ["<<totalPeerInSubChannel<<"]"<<endl;
	}
	cout<<"###----------------------------------------###"<<endl;
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
	if (this->channelMode == MODE_FLASH_CROWD  && channelMode == MODE_NORMAL)
	{
		boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);

		/*faz com que todos os pares passem a pertencentes ao canal principal
		 * Neste ponto deverá ser tratada a mesclagem das redes
		 * o mutex da lista peerList é fechado no bootstrap,
		 * onde a mensagem para mudar o estado é tratada.
		 */

		this->Remove_AllChannelSub();
		channelSubListLock.unlock();

		this->channelMode = channelMode;
	}
	else; // por enquanto, seria alterar de normal para flash crowd. (ou não alterar)
		this->channelMode = channelMode;
}

void Channel::Remove_AllChannelSub()
{
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
		i->second.SetChannelId_Sub(this->channelId);
   	channel_Sub_List.clear();
}

/* TODO ECM conferir o mutex que está em bootstrap
 * sempre na chamada em caso de remoção de apenas um canal,
 * o que irá acontecer ao mesclar redes.
 */
void Channel::Remove_ChannelSub(const string* source)
{
	cout<<"Churn no servidor auxiliar: "<<*source<<" Removendo o Subcanal"<<endl;
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
    	if (i->second.GetChannelId_Sub() == channel_Sub_List[*source].GetchannelId_Sub())
    		i->second.SetChannelId_Sub(this->channelId);
    channel_Sub_List.erase(*source);
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

    if (peerChannelId_Sub != this->channelId) //se o par está em sub canal, inclui o servidor auxiliar na lista de parceiros
    {
    	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
    	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
    		if (peerChannelId_Sub == i->second.GetchannelId_Sub())
    			allPeers.push_back(&(peerList[i->second.GetServer_Sub()->GetID()]));
    	channelSubListLock.unlock();
    }

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

    for (vector<string>::iterator peerId = deletedPeer.begin(); peerId < deletedPeer.end(); peerId++)
    {
    	/*TODO ECM IMPORTANTE: reavaliar essa remoção.
    	*	Aqui, os pares da rede paralela serão automaticamente enviados à rede principal
    	*	em caso de churn do servidor auxiliar
    	*/
    	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
		boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
    	if (channel_Sub_List.find(*peerId) != channel_Sub_List.end())
    		this->Remove_ChannelSub(&(*peerId));
    	if (server_Sub_Candidates.find(*peerId) != server_Sub_Candidates.end())
    		server_Sub_Candidates.erase(*peerId);
		channelSubListLock.unlock();
		channelSubCandidatesLock.unlock();

    	RemovePeer(*peerId);
    }
    //usado para testes de implementação...
   	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
    this->printChannelProfile();
	channelSubListLock.unlock();
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
