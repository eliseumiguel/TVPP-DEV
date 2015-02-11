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

        string logFilenamePerf_Master = logFilename + "perf_Master.txt";
        string logFilenameOverlay_Master = logFilename + "overlay_Master.txt";

        string logFilenamePerf_Total = logFilename + "perf_Total.txt";
        string logFilenameOverlay_Total = logFilename + "overlay_Total.txt";

        performanceFile_Master = fopen(logFilenamePerf_Master.c_str(),"w");
        poverlayFile_Master = fopen(logFilenameOverlay_Master.c_str(),"w");

        performanceFile_Total = fopen(logFilenamePerf_Total.c_str(),"w");
        poverlayFile_Total = fopen(logFilenameOverlay_Total.c_str(),"w");

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
		this->server_Sub_Candidates[peer->GetID()].SetPeerWaitInform(waiting);
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

	for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
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
			server_Sub_Candidates[source->GetID()] = SubChannelCandidateData();
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
	for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
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
	    	if (j->second.GetChannelId_Sub() == (int)i->second.GetchannelId_Sub())
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

    switch (channelMode)
    {
    	case MODE_NORMAL:
        {
        	if (this->channelMode == MODE_FLASH_CROWD)
            /* Esta mudança é radical e não considera se o flash crowd é com mesclagem ou não.
             * Simplesmente, muda-se o estado finalizando todos os subcanais.
             * Não é de se esperar que este estado seja realizado sem que os subcanais terminem a execução.
             * Detalhe. Este modo foi implementado para finalizer um flash crowd que não é com tempo de vida.
             * Assim, pode-se permitir que o ambiente fique em flash crowd por um tempo e finalize as subredes
             * sem preocupar com a mesclagem.
             */
        	{
        		//faz com que todos os pares passem a pertencentes ao canal principal. Mutex de peerList fechado no bootstrap,
       			for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
       				if (i->second.GetState()!= NO_SERVER_AUX)
       				{
       					i->second.SetState(NO_SERVER_AUX);
       					i->second.SetPeerWaitInform(true);
       				}
       			this->Remove_AllChannelSub();
        	}

        	this->channelMode = channelMode;
        	break;
        }

    	case MODE_FLASH_CROWD_MESCLAR:
    	{
    	   	this->mesclarRedes = true;

    	   	channelMode = MODE_FLASH_CROWD;
    	   	/* NÃO TEM BREAK
    	   	 *  Continua o código do flash crowd no case seguinte
    	   	 */
    	}

    	case MODE_FLASH_CROWD:
        {
        	if (this->channelMode == MODE_NORMAL)
        	/*faz todos os cadidatos a servidor auxilar tornarem-se disponíveis para o flash crwod
        	 * seria melhor por demanda, porém isso exige fazer um sistema de contenção dos recém chegados
        	 */
        	{
        		for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
        		{
        			i->second.SetState(SERVER_AUX_ACTIVE);
        			i->second.SetPeerWaitInform(true);
        		}
            this->channelMode = channelMode;
        	}
        	break;

        }
        default:
            cout <<"Não houve mudança no estado do Channel"<<endl;
            break;

    	channelSubListLock.unlock();
    	channelSubCandidatesLock.unlock();
    }
}

/*este método não permite mesclar
 * deve ser usado para situações radicais em que todos os
 * subcanais deixarão de existir. Por exemplo, em experimentos
 * sem temporização ou mesclagem
 */

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
void Channel::Remove_ChannelSub(const string* source, bool mesclar)
{
    if (mesclar)
    {
    	for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
    		if (i->second.GetChannelId_Sub() == (int)channel_Sub_List[*source].GetchannelId_Sub())
    			i->second.SetChannelId_Sub(-1);
    }
    else
    {
        for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
        	if (i->second.GetChannelId_Sub() == (int)channel_Sub_List[*source].GetchannelId_Sub())
        		i->second.SetChannelId_Sub(this->channelId);
    	channel_Sub_List.erase(*source);
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
    int peerChannelId_Sub = peerList[srcPeer->GetID()].GetChannelId_Sub();  //descobre o subcanal do par requisitante

    boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);

   	if (peerChannelId_Sub != (int)this->channelId && peerChannelId_Sub != -1)
    	/* Se o par está em sub canal, incluir o servidor auxiliar na lista de parceiros dele
    	* isso é importante visto que o serverAux->idChannel_Sub é igual ao idChannle do canal principal
    	* Os servidores auxiliares têm o idChannel_sub na estrutura SubChannelData
    	* No caso de -1, ele estará em fase de mesclagem e pertence à rede principal
    	*/
    {
    	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
    		if (peerChannelId_Sub == (int)i->second.GetchannelId_Sub())
    			allPeers.push_back(&(peerList[i->second.GetServer_Sub()->GetID()]));

    }

    /* Garante fazer a vizinhança apenas com os pares que estão no mesmo subcanal
     * Desta forma, a estratégia de selação já atua na lista allpeers selecionanda
     * Também, o próprio par não é sugerido a ele mesmo como vizinho
     */
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
        if (srcPeer->GetID() != i->second.GetPeer()->GetID() &&                                         // (se não é ele mesmo) e
        		(peerChannelId_Sub == i->second.GetChannelId_Sub() ||                                   // (se o sub = sub) ou
        	     (peerChannelId_Sub == -1 &&  i->second.GetChannelId_Sub() == (int)this->channelId)))   // ((se esta em mesclagem) e (outro rede principal))
            allPeers.push_back(&(i->second));


    /* -----------------------------------------------------------------------------------------
     *  nao enviar um par em estado de mesclagem a um servidor auxiliar
     * O par em estado de mesclagem deve encontrar outro par na rede e não entrar em um sub canal
     */
    //if (channel_Sub_List.count(srcPeer->GetID()) > 0)
    if (server_Sub_Candidates.count(srcPeer->GetID())> 0)
    {
        for (vector<PeerData*>::iterator i = allPeers.begin(); i != allPeers.end(); i++)
        {
        	if((*i)->GetChannelId_Sub() == (-1))
        		allPeers.erase(i);
        }
    }
    //como os servidores não são executados por demanda, todos da lista de candidatos
    //são configurados como servidor. Por isso, para evitar mandar um par em mesclagem
    // ao servidor, procuro pela lista de candidatos e não de servidores com subcanais.

    // não deve mandar um servidor auxiliar para um peer que está em mesclagem
    if (peerChannelId_Sub == -1)
        for (vector<PeerData*>::iterator i = allPeers.begin(); i != allPeers.end(); i++)
        {
        	//como os servidores não são executados por demanda, todos da lista de candidatos
        	//são configurados como servidor. Por isso, para evitar mandar um par em mesclagem
        	// ao servidor, procuro pela lista de candidatos e não de servidores com subcanais.

            //if (channel_Sub_List.count((*i)->GetPeer()->GetID()) > 0)
        	if (server_Sub_Candidates.count(srcPeer->GetID())> 0)
            {
            	allPeers.erase(i);
      		}

        }
    //---------------------------------------------------------------------------------------------

  	channelSubListLock.unlock();

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
unsigned int Channel::GetPeerListSizeChannel_Sub(int channelId_Sub)
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
		//garente o processo após o servidor auxiliar ser informado pelo bootstrap
		if (!server_Sub_Candidates[i->first].GetPeerWaitInform())
		{
			i->second.DecChannelLif();
			i->second.DecReNewServerSub();

			/*No tempo 1 o bootstrap é preparado
			 * para informar ao servidor o novo estado
			 */
			if (i->second.GetReNewServerSub() == 1)
			{
				server_Sub_Candidates[i->first].SetState(NO_SERVER_AUX);
				server_Sub_Candidates[i->first].SetPeerWaitInform(true);
			}
			else
				if (i->second.GetReNewServerSub() == 0) //no tempo 0 o canal é atualizado
				{
					this->Remove_ChannelSub(&(i->first), false);
					i->second.SetMesclando(false);
					cout<<"&&&&&& REMOVENDO &&&&& subChannel ["<<i->second.GetchannelId_Sub()<<"]"<<endl;
				}

			//servidor permanece em estado de mesclagem. Canal atua quando peer for informado (permite sincronizar)
			if (i->second.GetChannelLife() == 1)
			{
				server_Sub_Candidates[i->first].SetState(SERVER_AUX_MESCLAR);
				server_Sub_Candidates[i->first].SetPeerWaitInform(true);
			}
			else
				if (i->second.GetChannelLife() == 0)
				{
					this->Remove_ChannelSub(&(i->first), true);
					i->second.SetMesclando(true);
					cout<<"&&&&& MESCLANDO &&&&& subChannel ["<<i->second.GetchannelId_Sub()<<"]"<<endl;
				}

		} //primeiro if...
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

vector<FILE*> Channel::GetPerformanceFile(Peer* srcPeer)
{
	vPerformanceFile.clear();
	vPerformanceFile.push_back(performanceFile_Total);

	// se for o servidor, retorna todos os arquivos de log
	if (srcPeer->GetID() == this->GetServer()->GetID())
	{
		vPerformanceFile.push_back(performanceFile_Master);
		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			vPerformanceFile.push_back(i->second.GetPerformanceFile());
		return vPerformanceFile;
	}


	if (channel_Sub_List.find(srcPeer->GetID()) != channel_Sub_List.end())
		vPerformanceFile.push_back((channel_Sub_List.find(srcPeer->GetID()))->second.GetPerformanceFile());
	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)

	if (peerList[srcPeer->GetID()].GetChannelId_Sub() != (int)this->channelId)
		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (peerList[srcPeer->GetID()].GetChannelId_Sub() == (int)i->second.GetchannelId_Sub())
			{
				vPerformanceFile.push_back(i->second.GetPerformanceFile());
				return vPerformanceFile;
			}
	vPerformanceFile.push_back(performanceFile_Master);
	return vPerformanceFile;
}

vector<FILE*> Channel::GetOverlayFile(Peer* srcPeer)
{
	vPoverlayFile.clear();
	vPoverlayFile.push_back(poverlayFile_Total);

	// se for o servidor, retorna todos os arquivos de log
	if (srcPeer->GetID() == this->GetServer()->GetID())
	{
		vPoverlayFile.push_back(poverlayFile_Master);
		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			vPoverlayFile.push_back(i->second.GetOverlayFile());
		return vPoverlayFile;
	}


	if (channel_Sub_List.find(srcPeer->GetID()) != channel_Sub_List.end())
		vPoverlayFile.push_back((channel_Sub_List.find(srcPeer->GetID()))->second.GetOverlayFile());
	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)

	if (peerList[srcPeer->GetID()].GetChannelId_Sub() != (int)this->channelId)
		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (peerList[srcPeer->GetID()].GetChannelId_Sub() == (int)i->second.GetchannelId_Sub())
			{
				vPoverlayFile.push_back(i->second.GetOverlayFile());
				return vPoverlayFile;
			}
	vPoverlayFile.push_back(poverlayFile_Master);
	return vPoverlayFile;
}
