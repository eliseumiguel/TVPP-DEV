#include "Channel.hpp"

Channel::Channel(unsigned int channelId, Peer* serverPeer,
				 unsigned int maxSubChannel,
				 unsigned int maxServerAuxCandidate,
				 unsigned int maxPeerInSubChannel,
				 unsigned int sizeCluster,
				 bool modeFlasCrowdSemSubChannel,
				 bool mesclar)

: channel_Sub_List_Mutex (new boost::mutex()),
  channel_Sub_Candidates_Mutex (new boost::mutex())
{
    if (channelId != 0 || serverPeer != NULL) //Avoid creation by map[]
    {
    	this->channelMode = MODE_NORMAL;
        this->channelId = channelId;
        this->serverPeer = serverPeer;
        this->sizeCluster = sizeCluster;
        if(serverPeer)
            AddPeer(serverPeer);
        serverEstimatedStreamRate      = 0;
        this->maxPeerInSubChannel      = maxPeerInSubChannel;
        this->maxSubChannel            = maxSubChannel;
        this->maxServerAuxCandidate    = maxServerAuxCandidate;
        this->modeFlasCrowdSemSubChannel = modeFlasCrowdSemSubChannel;
        this->mesclarRedes             = mesclar;
        this->GenerateAllLogs          = false; //ECM Falso, gera apenas o log perf-all e overaly-all

        cout<<endl;
        cout<<" ---------------CRIANDO CANAL --------------------"<<endl;
        cout<< "maxPeerInSubChannel      = "<< maxPeerInSubChannel<<endl;
        cout<< "maxSubChannel            = "<< maxSubChannel<<endl;
        cout<< "maxServerAuxCandidate    = "<< maxServerAuxCandidate<<endl;
        cout<<"Mesclar Redes = ["<<mesclarRedes<<"]"<<endl;
        cout<<" -------------------------------------------------"<<endl;

        //Logging
        struct tm * timeinfo;
        char timestr[20];
        time(&creationTime);
        timeinfo = localtime(&creationTime);
        strftime (timestr,20,"%Y%m%d%H%M",timeinfo);
        string logFilename = "log-";
        logFilename += boost::lexical_cast<string>(channelId) + "-";

        string ip = serverPeer->GetIP();
        int tamanho = ip.size();
        for (int j = 0; j < 20 - tamanho ; j++)
        {
             ip = "0" + ip;
        }
        logFilename += ip + "_" ;

        logFilename += serverPeer->GetPort() + "-";
        logFilename += timestr;
        logFilename += "-";

        string logFilenamePerf_Total = logFilename + "all-perf.txt";
        string logFilenameOverlay_Total = logFilename + "all-overlay.txt";

        performanceFile_Total = fopen(logFilenamePerf_Total.c_str(),"w");
        poverlayFile_Total = fopen(logFilenameOverlay_Total.c_str(),"w");

        if(GenerateAllLogs){

        	string logFilenamePerf_Master = logFilename + "mas-perf.txt";
        	string logFilenameOverlay_Master = logFilename + "mas-overlay.txt";

        	performanceFile_Master = fopen(logFilenamePerf_Master.c_str(),"w");
        	poverlayFile_Master = fopen(logFilenameOverlay_Master.c_str(),"w");
        }

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

bool Channel::GetWaitServerList(Peer* peer)
{
    if (peerList.count(peer->GetID()) > 0)
        return peerList[peer->GetID()].GetPeerWaitListServer();
    return false;
}

void Channel::SetWaitServerList(Peer* peer, bool waitServerList)
{
    if (peerList.count(peer->GetID()) > 0)
        return peerList[peer->GetID()].SetPeerWaitListServer(waitServerList);
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

bool Channel::allsubChannelFull(){
	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
	unsigned int channelsubID_temp = 0;
	for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin() ; i != channel_Sub_List.end(); i++){
		unsigned int countPeer = 0;
		if (i->second.Get_ServerAuxChannelId_Sub() != channelsubID_temp){
			for (map<string, PeerData>::iterator master = peerList.begin(); master != peerList.end(); master++){
				if(unsigned(master->second.GetChannelId_Sub()) == i->second.Get_ServerAuxChannelId_Sub())
					countPeer++;

			}
			if (countPeer < this->maxPeerInSubChannel){
				posInsertSubChannel = i;
				channelSubListLock.unlock();
				return false;
			}
		}
		channelsubID_temp = i->second.Get_ServerAuxChannelId_Sub();
	}
	channelSubListLock.unlock();
	return true;
}


void Channel::AddPeerMasterChannel(Peer* peer){
	cout<<"Adding ["<<peer->GetID()<<"] on master channel"<<endl;
	peerList[peer->GetID()] = PeerData(peer);
   	peerList[peer->GetID()].SetChannelId_Sub(this->channelId);
}

void Channel::AddPeer(Peer* peer){
	if (!this->AddPeerChannel(peer))
	{
		if (!this->allsubChannelFull()){                // se não cheio, seleciona subchannell com vaga...
			if (!this->AddPeerChannel(peer)){           // tenta inserir novamente...
				cout<<"All subChannel full. "<<endl;
				this->AddPeerMasterChannel(peer);
		     }
		}
    	else
		{
			cout<<"All subChannel full. "<<endl;
			this->AddPeerMasterChannel(peer);
		}
     }
}

bool Channel::AddPeerChannel(Peer* peer){
	if (channelMode != MODE_FLASH_CROWD)
	{
		this->AddPeerMasterChannel(peer);
		return true;
	}
	else //FLASH CROWD
	{
		boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
		if (posInsertSubChannel == channel_Sub_List.end())
			posInsertSubChannel == channel_Sub_List.begin();

		if (this->GetPeerListSizeChannel_Sub(this->posInsertSubChannel->second.Get_ServerAuxChannelId_Sub()) < (this->maxPeerInSubChannel))
		{
			peerList[peer->GetID()] = PeerData(peer);
			peerList[peer->GetID()].SetChannelId_Sub(posInsertSubChannel->second.Get_ServerAuxChannelId_Sub());

	       	//new position for adding next peer
			for (map<string, SubChannelServerAuxData>::iterator i = posInsertSubChannel ; i != channel_Sub_List.end(); i++)
			{
				if (i->second.Get_ServerAuxChannelId_Sub() != posInsertSubChannel->second.Get_ServerAuxChannelId_Sub())
				{

					posInsertSubChannel = i;
					channelSubListLock.unlock();
					return true;
				}
			}
			posInsertSubChannel = channel_Sub_List.begin();
			channelSubListLock.unlock();
     		return true;
		}

	}
	return false;
}

/* ECM Considera que nenhum subchannel foi criado e que a
 * lista de candidatos tem a quantidade necessária para
 * criar os subchannel com os servidores em cluster.
 */
void Channel::Create_All_ChannelSub(){

	vector<string> peerServerAuxNew;
	unsigned int size = sizeCluster;
	unsigned int channelID_New = this->channelId;

	for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)

		if (channel_Sub_List.count(i->first) == 0 )
		{
			size--;
			peerServerAuxNew.push_back(i->first);
			if (size==0)
			{
				channelID_New++;
				size = sizeCluster;
				if ((peerServerAuxNew.size()==sizeCluster) && (channel_Sub_List.size() < (maxSubChannel * sizeCluster)))
				{
					for (unsigned int pos=0; pos < sizeCluster; pos++){
					    channel_Sub_List[peerServerAuxNew.at(pos)] = SubChannelServerAuxData(channelId, channelID_New, peerList[peerServerAuxNew.at(pos)].GetPeer(),GenerateAllLogs);
					}
                    while (peerServerAuxNew.size() > 0){
					    peerServerAuxNew.erase(peerServerAuxNew.end());
					}
			    }
		    }

        }
    while (peerServerAuxNew.size() > 0){
	    peerServerAuxNew.erase(peerServerAuxNew.end());
	}
	this->posInsertSubChannel= this->channel_Sub_List.begin();
}


/* Choose a random auxiliar server peer */
void Channel::analizePeerToBeServerAux(Peer* source)
{
	if (this->channelMode == MODE_NORMAL)
	{
		boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
		if ((this->HasPeer(source)) && (server_Sub_Candidates.size() < this->maxServerAuxCandidate) && (source->GetID() != this->GetServer()->GetID()))
		{
			if(server_Sub_Candidates.count(source->GetID()) == 0)
				server_Sub_Candidates[source->GetID()] = SubChannelCandidateData();
		}
		channelSubCandidatesLock.unlock();
	}
}

void Channel::printChannelProfile()
{
	cout<<"###-------"<<endl;
	cout<<"Channel ["<<channelId<<"] profile:"<<endl;
	cout<<"T Peers ["<<peerList.size()<<"] T SChannel ["<<channel_Sub_List.size()<<"] ";

	int auxiliarServerDisponiveis = 0;
	for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
		auxiliarServerDisponiveis++;
	cout<<"EnableSS: ["<<auxiliarServerDisponiveis<<"]"<<endl;

    cout<<"#"<<endl;
	for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
	{
		int totalPeerInSubChannel=0;
		for (map<string, PeerData>::iterator j = peerList.begin(); j != peerList.end(); j++)
	    	if (j->second.GetChannelId_Sub() == (int)i->second.Get_ServerAuxChannelId_Sub())
	    		totalPeerInSubChannel++;
			cout<<"["<<i->first<<"] scID ["<< i->second.Get_ServerAuxChannelId_Sub()<<"] TPeerSub ["<<totalPeerInSubChannel<<"]"<<endl;
	}
	cout<<"###-------"<<endl;
}

void Channel::RemovePeer(Peer* peer)
{
    peerList.erase(peer->GetID());
}

void Channel::RemovePeer(string peerID)
{
    peerList.erase(peerID);
}

PeerData& Channel::GetPeerData(Peer* peer)
{
    return peerList[peer->GetID()];
}

/*ECM IMPORTANTE: não colocar sleep neste código. Isso faz com que todos que tentam abrir os mutex ficam esperando o mesmo sleep.
 * O bootstrap tem de tratar outras mensagens e o mutex que prende a lista de canais (channelListLock) está fechado lá quando a execução
 * passa nesta função.
 *
 * Mutex peerList fechado em bootstrap
 */
void Channel::SetChannelMode(ChannelModes New_channelMode, bool TIMEMERGE)
{
	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
	map<string,SubChannelCandidateData> subCandidatosTempList;

	cout<<"Channel current State "<<this->channelMode<<endl;
    cout<<"Configuring channel new state "<<New_channelMode<<" ..."<<endl;

    switch (New_channelMode)
    {
    	case MODE_NORMAL: //*************************************************************************
        {
        	if ((this->channelMode == MODE_FLASH_CROWD) || (this->channelMode == MODE_FLASH_CROWD_MESCLAR))
        	{
       			for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
       				if (i->second.GetState()!= NO_SERVER_AUX)
       				{
       					cout<<"setting "<<i->first<<" no-server "<<endl;
       					i->second.SetState(NO_SERVER_AUX);
       					i->second.SetPeerWaitInform(true);
       				}
       			this->Remove_AllChannelSub();
        	}
        	this->mesclarRedes = false;
        	this->channelMode = New_channelMode;
        	break;
        }
    	case MODE_FLASH_CROWD_MESCLAR: //***********************************************************
    	{
    		if ((this->channelMode == MODE_FLASH_CROWD) || (this->channelMode == MODE_FLASH_CROWD_MESCLAR))
    		{
    			this->mesclarRedes = true;
    			for (map<string,SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++){
    				if (server_Sub_Candidates[i->first].GetState() != SERVER_AUX_MESCLAR){
      				    server_Sub_Candidates[i->first].SetState(SERVER_AUX_MESCLAR);
    				    server_Sub_Candidates[i->first].SetPeerWaitInform(true);
    				    this->Remove_ChannelSub(&(i->first), this->mesclarRedes);
   					    cout<<"setting "<<i->first<<" server merging networks"<<endl;
   					    if (TIMEMERGE) break;
    				}

    			}
    			this->channelMode = New_channelMode;
    		}
    		else cout <<"Is not permitted change the state channel "<<this->channelMode<<" to "<<New_channelMode<<endl;

   			break;
    	}

    	case MODE_FLASH_CROWD: //*********************************************************************
        {
        	if (this->channelMode == MODE_NORMAL)
        	{
        		/* Gera lista de candidatos disponíveis de forma aleatória
        		 * Quantidade permitida de candidatos    : maxServerAuxCandidate
        		 * Quantidade de candidatos selecionados : (maxSubChannel * sizeCluster)
        		 */
        		if (server_Sub_Candidates.size() > maxSubChannel * sizeCluster)
        		{
        			for (unsigned int i=0; i < (maxSubChannel * sizeCluster) ;i++)
        		    {
        		       //sorteia número aleatório para seleção do servidor
        		       srand (time (NULL)); // Gera uma 'random seed' baseada no retorno da funcao time()
        		       int sorteado;
        		       sorteado = rand () % server_Sub_Candidates.size(); // Retorna um numero aleatorio entre 0 e tamanho do mapa
        		       map<string,SubChannelCandidateData>::iterator j = server_Sub_Candidates.begin();
        		       for(int mapItIndex = 0; mapItIndex < sorteado; mapItIndex++) j++;
        		       subCandidatosTempList.insert(subCandidatosTempList.end(), std::pair<string,SubChannelCandidateData>((*j).first, (*j).second));
        		       server_Sub_Candidates.erase((*j).first);
        		     }

        		     server_Sub_Candidates.clear();
        		     server_Sub_Candidates = subCandidatosTempList;
        		}

        		for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
        		{
  					cout<<"setting "<<i->first<<" auxiliary active server "<<endl;
        			i->second.SetState(SERVER_AUX_ACTIVE);
        			i->second.SetPeerWaitInform(true);
        		}
        		//ECM Codigo inserido para preencher subchannel por demanda///////
        		this->Create_All_ChannelSub();

        		/* ECM prepara estado dos participantes da rede principal para receber a lista de servidores auxiliares selecionados.
            	 * Ao receber a lista, os participantes da rede principal não irão desconectar um servidor auxliar durante o flash crowd
            	*/
           		for (map<string,PeerData>::iterator peerRedePrincipal = this->peerList.begin(); peerRedePrincipal != peerList.end(); peerRedePrincipal++)
        			if (server_Sub_Candidates.find(peerRedePrincipal->first) == server_Sub_Candidates.end())
        				peerList[peerRedePrincipal->first].SetPeerWaitListServer(true);

        	this->mesclarRedes = false;
            this->channelMode = New_channelMode;

        	}
        	else
        		if (this->channelMode == MODE_FLASH_CROWD_MESCLAR)
        			cout <<"Is not permitted change the state channel "<<this->channelMode<<" to "<<New_channelMode<<endl;

        	break;

        }
        default:
        	cout <<"Mode "<<New_channelMode<<" is not permitted or unknown"<<endl;
            break;

    	channelSubListLock.unlock();
    	channelSubCandidatesLock.unlock();
    }
}
/*
 * Testa se existe outro servidor auxiliar para um determinado sub_channel
 */
bool Channel::Finishing_Server (string* server){
	for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++){
		if ((channel_Sub_List[*(server)].Get_ServerAuxChannelId_Sub() == i->second.Get_ServerAuxChannelId_Sub()) && (*(server) != i->first))
			return false;
	}
	return true;
}

void Channel::Remove_AllChannelSub()
{
	for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
		i->second.SetChannelId_Sub(this->channelId);
    channel_Sub_List.clear();
}

void Channel::Remove_ChannelSub(const string* source, bool mesclar)
{
    if (mesclar)
    {
    	for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
    		if (i->second.GetChannelId_Sub() == (int)channel_Sub_List[*source].Get_ServerAuxChannelId_Sub())
    		//	i->second.SetChannelId_Sub( i->second.GetChannelId_Sub() * (-1));
    			i->second.SetChannelId_Sub(-1); // isso foi mudado para permitir que haja um único canal mesclando.
    }
    else
    {
        for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
        	if (i->second.GetChannelId_Sub() == (int)channel_Sub_List[*source].Get_ServerAuxChannelId_Sub())
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
	this->maxPeerInSubChannel = maxpeerChannelSub;
}

/* ECM Gera a lista de servidores auxiliares para enviar aos participantes da rede principal.
 * Método chamado no bootstrap. Mutex do peerList já fechado.
 */
vector<PeerData*> Channel::MakeServerAuxList()
{
    vector<PeerData*> ServerAuxList;
    for (map<string, SubChannelCandidateData>::iterator c = server_Sub_Candidates.begin(); c != server_Sub_Candidates.end(); c++)
		ServerAuxList.push_back(new PeerData(new Peer(c->first)));
    return ServerAuxList;
}


 /* É necessário filtrar parceiros neste método apenas quando o channel está em ChannelMode MODE_FLASH_CROWD.
 * Isso porque em qualquer outro estado, os pares podem se conhecer mutuamente. O filtro no caso MODE_FLASH_CROWD
 * garante que as redes (paralelas e principal) serão isoladas.
 * Método chamado pelo Bootstrap. Mutex de peerList já fechado.
 */
vector<PeerData*> Channel::SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity,
		                                  bool virtualPeer, uint8_t minimumBandwidth, uint8_t minimumBandwidth_FREE,
										  bool separatedFreeOutList)
{
    vector<PeerData*> allPeers, selectedPeers;
    int srcPeerChannelId_Sub = peerList[srcPeer->GetID()].GetChannelId_Sub();

    if (((this->GetChannelMode() != MODE_FLASH_CROWD) && (this->GetChannelMode() != MODE_FLASH_CROWD_MESCLAR)) || (modeFlasCrowdSemSubChannel)){
        for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
            if (srcPeer->GetID() != i->second.GetPeer()->GetID()){
            	if (!virtualPeer)
            		allPeers.push_back(&(i->second));
                else
                	if (srcPeer->GetIP() != i->second.GetPeer()->GetIP())
                		allPeers.push_back(&(i->second));
            }
    }
    else  //Modo flash crowd com subcanais isolados ou mesclando
    {
    	/* Garante fazer a vizinhança apenas com os pares que estão no mesmo subcanal
    	 * Atende ao caso de só haver o canal principal
    	 * Se for flash crowd, preserva na íntegra os subcanais.
    	 * Se for um subcanal mesclando:
    	 *          a) ele protege os outros subcanais não mesclando isolados da rede
    	 *          b) ele não permite mesclar os free rider do canal que está mesclado
    	 */

		boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
    	for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
    	{
    		if (  (srcPeer->GetID() != i->second.GetPeer()->GetID())                                 && (         // (se ele não é o mesmo escolhido)  E
    			    (srcPeerChannelId_Sub == i->second.GetChannelId_Sub())  ||                                  // (   (se o subChannel_ID = subChannel_ID) ou
    		        ( (srcPeerChannelId_Sub <0) && (srcPeer->GetSizePeerListOutInformed() > 0) &&                //     ((está mesclando) e (ele não é free) e
    		    		       (i->second.GetChannelId_Sub()==(int)this->channelId )) ||                         //     (escolhido é da rede principal)) ou
							   ((srcPeerChannelId_Sub == (int)this->channelId) &&                                //     (ele é da rede principal e escolhido está mesclando) ) e
									   (i->second.GetChannelId_Sub() < 0)) )                         && (
       	          channel_Sub_List.find(i->first) == channel_Sub_List.end())                                    // (não fornece server aux para par da rede principal)
			   )
    			{
    				if (!virtualPeer)                                              // permit relationship between same IP
    					allPeers.push_back(&(i->second));
    				else
    					if  (srcPeer->GetIP() != i->second.GetPeer()->GetIP())
    						allPeers.push_back(&(i->second));
    			}
    	}
		channelSubListLock.unlock();

    	/* Se peer está em sub canal em estado Flash Crowd,
    	 * inserir todos os servidores auxiliares na lista do peer
    	 */
    	if ((srcPeerChannelId_Sub != (int)this->channelId) && (srcPeerChannelId_Sub >0))
    	{
    		boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
    		for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
    			if (srcPeerChannelId_Sub == (int)i->second.Get_ServerAuxChannelId_Sub())
    				allPeers.push_back(&(peerList[i->second.GetServer_Sub()->GetID()]));
    		channelSubListLock.unlock();
    	}
	}


    vector<PeerData*> selectedPeersAUX;
    if (!(separatedFreeOutList) || (srcPeer->GetSizePeerListOutInformed() > 0))
    {
         /* elimina pares com banda inferior à definida
          * Se as listas não forem separadas, todos recebem parceiros com OUT >= minimumBandwidth
          * Se as listas forem separadas e o par for contribuidor, ele recebe parceiros com OUT >= minimumBandwidth
          * */
         for (uint16_t i = 0; i < allPeers.size(); i++) {
    	     if (allPeers[i]->GetSizePeerListOutInformed() >= minimumBandwidth) {
    		    selectedPeersAUX.push_back(allPeers[i]);
    	     }
          }
    }
    else{
    	  /* as listas aqui são separaddas e o par não é contribuidor.
    	   *  Assim recebe quem tem OUT_FREE
    	   */
          for (uint16_t i = 0; i < allPeers.size(); i++) {
             	if (allPeers[i]->GetSizePeerListOutInformed_FREE() >= minimumBandwidth_FREE) {
           		    selectedPeersAUX.push_back(allPeers[i]);
            	}
          }


    }

    allPeers = selectedPeersAUX;

    if (peerQuantity > allPeers.size())
    	peerQuantity = allPeers.size();

    strategy->Execute(&allPeers, srcPeer, peerQuantity);
    selectedPeers.insert(selectedPeers.begin(),allPeers.begin(),allPeers.begin()+peerQuantity);

    /*ECM Tratar a tecnica que mistura os modelos: Sem subcanais e Subcanais isolados.
     * nesta técnica, tratamos o FC sem subcanais totalmente isolados, contudo selecionamos servidores
     * para expor recursos aos novos participantes.
     * Assim, caso o peer estaja em subcanal, enviar a ele possíveis servidores auxiliares.
    */
  	if ((modeFlasCrowdSemSubChannel) && (this->GetChannelMode() == MODE_FLASH_CROWD)){

     	/* Se peer está em sub canal em estado Flash Crowd,
      	 * inserir todos os servidores auxiliares na lista do peer
       	 */
       	if ((srcPeerChannelId_Sub != (int)this->channelId) && (srcPeerChannelId_Sub >0))
       	{
       		boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
       		for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
       			if (srcPeerChannelId_Sub == (int)i->second.Get_ServerAuxChannelId_Sub())
       				selectedPeers.push_back(&(peerList[i->second.GetServer_Sub()->GetID()]));
       		channelSubListLock.unlock();
       	}
    }
  	cout<<"sending "<<selectedPeers.size()<<" peer to "<<srcPeer->GetID()<<"'s neighbor"<<endl;
  	/*
  	for (std::vector<PeerData*>::iterator it = selectedPeers.begin() ; it != selectedPeers.end(); ++it)
  	    std::cout << ' ' << (*it)->GetPeer()->GetID();
     cout<<endl;
     */
    return selectedPeers;
}

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

/*
 * Método chamado pelo Bootstrap
 * Mutex de peerList já fechado
 */
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
    	/*TODO ECM IMPORTANTE:
    	 * Caso o peer seja servidor auxiliar e único no sub canal, Remove_ChannelSub.
    	 * Caso haja outro servidor em cluster, remove o peer e mantém o canal.
    	*/

		if (posInsertSubChannel == channel_Sub_List.find(*peerId))
			posInsertSubChannel ++;

    	if (channel_Sub_List.find(*peerId) != channel_Sub_List.end()){
    		if (this->Finishing_Server(&(*peerId))){
    			this->Remove_ChannelSub(&(*peerId));}
    		else{
    			channel_Sub_List.erase(*peerId);}
    	}
    	if (server_Sub_Candidates.find(*peerId) != server_Sub_Candidates.end())
    		server_Sub_Candidates.erase(*peerId);

    	RemovePeer(*peerId);
    }
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

	if(GenerateAllLogs)
	{
		// se for o servidor, retorna todos os arquivos de log
		if (srcPeer->GetID() == this->GetServer()->GetID())
		{
			vPerformanceFile.push_back(performanceFile_Master);
			for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
				vPerformanceFile.push_back(i->second.GetPerformanceFile());
			return vPerformanceFile;
		}


		if (channel_Sub_List.find(srcPeer->GetID()) != channel_Sub_List.end())
			vPerformanceFile.push_back((channel_Sub_List.find(srcPeer->GetID()))->second.GetPerformanceFile());
		for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)

			if (peerList[srcPeer->GetID()].GetChannelId_Sub() != (int)this->channelId)
				for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
					if (peerList[srcPeer->GetID()].GetChannelId_Sub() == (int)i->second.Get_ServerAuxChannelId_Sub())
					{
						vPerformanceFile.push_back(i->second.GetPerformanceFile());
						return vPerformanceFile;
					}
		vPerformanceFile.push_back(performanceFile_Master);
	}
	return vPerformanceFile;
}

vector<FILE*> Channel::GetOverlayFile(Peer* srcPeer)
{
	vPoverlayFile.clear();
	vPoverlayFile.push_back(poverlayFile_Total);

	if(GenerateAllLogs)
	{

		// se for o servidor, retorna todos os arquivos de log
		if (srcPeer->GetID() == this->GetServer()->GetID())
		{
			vPoverlayFile.push_back(poverlayFile_Master);
			for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
				vPoverlayFile.push_back(i->second.GetOverlayFile());
			return vPoverlayFile;
		}


		if (channel_Sub_List.find(srcPeer->GetID()) != channel_Sub_List.end())
			vPoverlayFile.push_back((channel_Sub_List.find(srcPeer->GetID()))->second.GetOverlayFile());
		for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)

			if (peerList[srcPeer->GetID()].GetChannelId_Sub() != (int)this->channelId)
				for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
					if (peerList[srcPeer->GetID()].GetChannelId_Sub() == (int)i->second.Get_ServerAuxChannelId_Sub())
					{
						vPoverlayFile.push_back(i->second.GetOverlayFile());
						return vPoverlayFile;
					}
		vPoverlayFile.push_back(poverlayFile_Master);
	}
	return vPoverlayFile;
}
