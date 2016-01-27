#include "Channel.hpp"

Channel::Channel(unsigned int channelId, Peer* serverPeer,
				 unsigned int maxSubChannel,
				 unsigned int maxServerAuxCandidate,
				 unsigned int maxPeerInSubChannel,
				 unsigned int sizeCluster,
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
        this->mesclarRedes             = mesclar;
        this->GenerateAllLogs = false; //ECM Falso, gera apenas o log perf-all e overaly-all
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

/* ECM ****
 * Insere o peer no canal principal se MODE == NORMAL ou em um sub-canal se MODE == FLASH CROWD
 * Em MODE FLASH CROWD: caso todos sub-canais estejam cheios, tenta criar novo sub canal e inserir
 *                      caso não consiga criar novo canal, insere na rede principal.
 * Este método é chamado no bootstrap. O mutex já fechado...
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
//ECM - Mutex fechado ... pode-se pegar o idSubChannelServer in peerlist
bool Channel::AddPeerChannel(Peer* peer)
{
	if (channelMode != MODE_FLASH_CROWD)
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

		for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (this->GetPeerListSizeChannel_Sub(i->second.Get_ServerAuxChannelId_Sub()) < (this->maxPeerInSubChannel))
			{
				peerList[peer->GetID()] = PeerData(peer);
				peerList[peer->GetID()].SetChannelId_Sub(i->second.Get_ServerAuxChannelId_Sub());

		       	cout<<"Inserindo ["<<peer->GetID()<<"] SUBCHANNEL ["<<i->second.Get_ServerAuxChannelId_Sub()<<"]"<<endl;
				channelSubListLock.unlock();
				return true;
	        }
		channelSubListLock.unlock();
	}
	return false;
}

/*ECM 17-01-2015
 * Novo código de criação de canal com cluster de servidores auxiliares em redes paralelas
 * Agora, mais de um servidor auxiliar terá o mesmo subCannel_ID. Com isso, contribuirão na mesma rede paralela
 * OBS: Mutex fechado no bootstrap
*/
bool Channel::Create_New_ChannelSub()
{
	vector<string*> peerServerAuxNew;
	unsigned int size = sizeCluster;
    cout<<" entrou em criando novo sub canal"<<endl;
	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);

	for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
		if (channel_Sub_List.count(i->first) == 0 )
		{
			size--;
			peerServerAuxNew.push_back(new string(i->first));
			if (size==0) break; //modified at 17-01-16
		}
    cout <<"All server ["<<peerServerAuxNew.size()<<"] candidate are able to create new subchannel"<<endl;
	channelSubCandidatesLock.unlock();

	//create sizeCluster sub-channel with same ID_SubChannel
	if (peerServerAuxNew.size()==sizeCluster && channel_Sub_List.size() < (maxSubChannel * sizeCluster))
	{
		//Get a new subChannel ID
		unsigned int channelID_New = 0;
		for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (channelID_New < i->second.Get_ServerAuxChannelId_Sub())
				channelID_New = i->second.Get_ServerAuxChannelId_Sub();
		channelID_New ++;
        cout << "Got the new subChannel ID "<<channelID_New<<endl;
		for (unsigned int pos=0; pos < sizeCluster; pos++){
		    channel_Sub_List[*peerServerAuxNew.at(pos)] = SubChannelServerAuxData(channelId, channelID_New, peerList[*peerServerAuxNew.at(pos)].GetPeer(),GenerateAllLogs);
		}

		for (unsigned int pos=0; pos <peerServerAuxNew.size(); pos++)
		   delete peerServerAuxNew.at(pos);

		channelSubListLock.unlock();
		return true;
	}

	channelSubListLock.unlock();
	for (unsigned int pos=0; pos <peerServerAuxNew.size(); pos++)
	   delete peerServerAuxNew.at(pos);
	return false;
}

/* Atualmente, este método seleciona para servidor auxiliar os primeiros peers que entram no canal
 * Assim, são escolhidos os peers mais próximos do servidor principal.
 * Outras estratégias podem ser idealizadas considerando a qualidade dos pares (contracts article)
 */
void Channel::analizePeerToBeServerAux(Peer* source)
{
	//ECM O peer só pode ser candidato se o modo é normal.
	//No início do experimento, tudo certo. Contudo, caso o estado tenha se tornado MODE_NORMAL recentemente, caso o peer esteja
	//aguardando notificação pelo waitInformation, ele já é servidor e não pode ser reinserido.
	if (this->channelMode == MODE_NORMAL)
	{
		boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
		if ((this->HasPeer(source)) && (server_Sub_Candidates.size() < this->maxServerAuxCandidate) && (source->GetID() != this->GetServer()->GetID()))
		{
			//Assegura que um servidor em estado de mesclagem ou flash crowd
			//passe para o estado normal sem ser reinserido na lista de candidatos
			if(server_Sub_Candidates.count(source->GetID()) == 0)
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
 */
void Channel::SetChannelMode(ChannelModes New_channelMode)
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
            /* Mudança é radical se MODE_FLASH_CROWD ...
             * Simplesmente, muda-se o estado finalizando todos os subcanais.
             * Não é de se esperar que este estado seja realizado sem que os subcanais passem pela mesclagem.
             */
        	{
        		//faz com que todos os pares passem a pertencentes ao canal principal. Mutex de peerList fechado no bootstrap,
       			for (map<string,SubChannelCandidateData>::iterator i = server_Sub_Candidates.begin(); i != server_Sub_Candidates.end(); i++)
       				if (i->second.GetState()!= NO_SERVER_AUX)
       				{
       					cout<<"configurando "<<i->first<<" no-server "<<endl; //teste
       					i->second.SetState(NO_SERVER_AUX);
       					i->second.SetPeerWaitInform(true);
       					cout<<"esperando informação para "<<i->first<<" "<<i->second.GetPeerWaitInform()<<endl; //teste
       				}
       			this->Remove_AllChannelSub();
        	}

        	this->mesclarRedes = false;
        	this->channelMode = New_channelMode;
        	break;
        }

    	case MODE_FLASH_CROWD_MESCLAR: //***********************************************************
    	{
    		if (this->channelMode == MODE_FLASH_CROWD)
    		{
    			this->mesclarRedes = true;
    			for (map<string,SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++){
    				server_Sub_Candidates[i->first].SetState(SERVER_AUX_MESCLAR);
    				server_Sub_Candidates[i->first].SetPeerWaitInform(true);
    				this->Remove_ChannelSub(&(i->first), this->mesclarRedes);
    				cout<<"Server "<<i->first<<" waiting state change to ["<<New_channelMode<<"]"<<endl;
    			}
    			this->channelMode = New_channelMode;
    		}
    		else cout <<"Is not permitted change the state channel "<<this->channelMode<<" to "<<New_channelMode<<endl;

   			break;
    	}

    	case MODE_FLASH_CROWD: //*********************************************************************
        {
        	if (this->channelMode == MODE_NORMAL)
        	/*faz todos os cadidatos a servidor auxilar tornarem-se disponíveis para o flash crwod
        	 * seria melhor por demanda, porém isso exige fazer um sistema de contenção dos recém chegados
        	 * Com a atual mudança (18-11-2015) a lista de candidatos pode ser muito grande. Assim, todos da rede
        	 * principal podem ser candidatos. Contudo, ao isolar os candidatos, isso é feito no limite da quantidade de
        	 * subcanais a serem criados x tamanho do cluster. Com isso, a rede principal não sofre perdas com o isolamento
        	 * de pares que não serão utilizados como servidores auxiliares. Mesmo assim, seria melhor fazer por demanda.
        	 */
        	{
        		// início do código para gerar nova lista de candidatos aleatória limitada pela quantidade de subcanais x cluster.
        		// aqui são selecionados aleatoriamente candidatos a servidor auxiliar para os subcanais. Contudo,
        		// para fazer cluster em subcanais, deve-se somar ao limite maxSubChannel + tamanho do cluster
        		// ECM Outra solução é aplicar a estratégia randômica na lista e remover o excesso.
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
        			cout<<"configurando "<<i->first<<" server aux active "<<endl;
        			i->second.SetState(SERVER_AUX_ACTIVE);
        			i->second.SetPeerWaitInform(true);
        		}
        		// ECM Configura o estado de cada participante da rede principal para esperar a lista de servidores auxiliares gerada
            	// Com isso, o bootstrap informa a todos os clientes da rede principal quais são os servidores auxiliares. Assim eles podem ter
        		// privilégios na rede principal, com não serem removidos da lista de ativos nas parcerias durante o flash cowd.
        		for (map<string,PeerData>::iterator peerRedePrincipal = this->peerList.begin(); peerRedePrincipal != peerList.end(); peerRedePrincipal++)
        			if (server_Sub_Candidates.find(peerRedePrincipal->first) == server_Sub_Candidates.end())
        				peerList[peerRedePrincipal->first].SetPeerWaitListServer(true);

        	this->mesclarRedes = false;
            this->channelMode = New_channelMode;
        	}
        	break;

        }
        default:
        	cout <<"Mode "<<New_channelMode<<" is not permitted or unknown"<<endl;
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

/* TODO ECM conferir o mutex que está em bootstrap
 * sempre na chamada em caso de remoção de apenas um canal,
 * o que irá acontecer ao mesclar redes.
 */
void Channel::Remove_ChannelSub(const string* source, bool mesclar)
{
    if (mesclar)
    {
    	for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
    		if (i->second.GetChannelId_Sub() == (int)channel_Sub_List[*source].Get_ServerAuxChannelId_Sub())
    			i->second.SetChannelId_Sub( i->second.GetChannelId_Sub() * (-1));
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

/* Método chamado no bootstrap.
 * Mutex do peerList já fechado.
 */

//ECM Gera a lista de servidores auxiliares para enviar aos participantes da rede principal
vector<PeerData*> Channel::MakeServerAuxList()
{
    vector<PeerData*> ServerAuxList;
    for (map<string, SubChannelCandidateData>::iterator c = server_Sub_Candidates.begin(); c != server_Sub_Candidates.end(); c++)
		ServerAuxList.push_back(new PeerData(new Peer(c->first)));
    return ServerAuxList;
}

/*
 * Método chamado pelo Bootstrap
 * Mutex de peerList já fechado
 * É necessário filtrar parceiros neste método apenas quando o channel está em ChannelMode MODE_FLASH_CROWD.
 * Isso porque em qualquer outro estado, os pares podem se conhecer mutuamente. O filtro no caso MODE_FLASH_CROWD
 * garante que as redes (paralelas e principal) serão isoladas.
 */
vector<PeerData*> Channel::SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity, bool virtualPeer)
{
    vector<PeerData*> allPeers, selectedPeers;
    int peerChannelId_Sub = peerList[srcPeer->GetID()].GetChannelId_Sub();

    if (this->GetChannelMode() != MODE_FLASH_CROWD){
        for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
            if (srcPeer->GetID() != i->second.GetPeer()->GetID()){
            	if (!virtualPeer)
            		allPeers.push_back(&(i->second));
                else
                	if  (srcPeer->GetIP() != i->second.GetPeer()->GetIP())
                		allPeers.push_back(&(i->second));
            }
    }
    else
    {
    	/* Garante fazer a vizinhança apenas com os pares que estão no mesmo subcanal
    	 * Atende ao caso de só haver o canal principal
    	 */
    	for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)

    		if ((srcPeer->GetID() != i->second.GetPeer()->GetID()) &&      		// (se não é ele mesmo)  e
    				(peerChannelId_Sub == i->second.GetChannelId_Sub()))        // (se o subChannel_ID = subChannel_ID)
    		{
    			if (!virtualPeer)                                              // permit relationship between same IP
    				allPeers.push_back(&(i->second));
    			else
    				if  (srcPeer->GetIP() != i->second.GetPeer()->GetIP())
    					allPeers.push_back(&(i->second));
    		}

    	/* Se o par está em sub canal, incluir TODOS os servidores auxiliares na lista de parceiros dele
    	 * isso é importante visto que o serverAux->idChannel_Sub é igual ao idChannel do canal principal
    	 * Os servidores auxiliares têm o idChannel_sub na estrutura SubChannelServerAuxData
    	 * No caso de CHANNEL_ID_MESCLANDO -1, ele estará em fase de mesclagem e pertence à rede principal
    	 */
    	if (peerChannelId_Sub != (int)this->channelId)
    	{
    		boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
    		for (map<string, SubChannelServerAuxData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
    			if (peerChannelId_Sub == (int)i->second.Get_ServerAuxChannelId_Sub())
    				allPeers.push_back(&(peerList[i->second.GetServer_Sub()->GetID()]));
    		channelSubListLock.unlock();
    	}

	}
    //cout <<"Lista de vizinhos a ser enviado para "<<srcPeer->GetID()<<" tem tamanho "<<allPeers.size()<<endl;
    if (allPeers.size() <= peerQuantity)
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
    	 * Caso o peer seja servidor auxiliar e único no sub canal, Remove_ChannelSub
    	 * Caso haja outro servidor em cluster no mesmo cananal, apenas remove o peer da lista.
    	*/
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
