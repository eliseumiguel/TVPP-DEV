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

        string logFilenamePerf_Master = logFilename + "mas-perf.txt";
        string logFilenameOverlay_Master = logFilename + "mas-overlay.txt";

        string logFilenamePerf_Total = logFilename + "all-perf.txt";
        string logFilenameOverlay_Total = logFilename + "all-overlay.txt";

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
			if (this->GetPeerListSizeChannel_Sub(i->second.GetchannelId_Sub()) < (this->maxPeerInSubChannel) && (!i->second.GetMesclando()))
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

/*ECM 17-01-2015
 * Novo código de criação de canal com cluster de servidores auxiliares em redes paralelas
 * Agora, mais de um servidor auxiliar terá o mesmo subCannel_ID. Com isso, contribuirão na mesma rede paralela
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
    cout <<"finalizou o vetor de servidores deste canal"<<endl;
	channelSubCandidatesLock.unlock();

	if (peerServerAuxNew.size()==sizeCluster && channel_Sub_List.size() < (maxSubChannel * sizeCluster))  //cria sub-channel
	{
		unsigned int channelID_New = 0;
        //ECM create a new subChannelID
		for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
			if (channelID_New < i->second.GetchannelId_Sub())
				channelID_New = i->second.GetchannelId_Sub();
		channelID_New ++;
        cout << "criou o novo ID "<<channelID_New<<endl;
		for (unsigned int pos=0; pos < sizeCluster; pos++){
		    channel_Sub_List[*peerServerAuxNew.at(pos)] = SubChannelData(channelId, channelID_New, peerList[*peerServerAuxNew.at(pos)].GetPeer() );//,0,0);
		}
		cout<< "configurou os servidores auxiliares"<<endl;
		channelSubListLock.unlock();
		for (unsigned int pos=0; pos <peerServerAuxNew.size(); pos++)
		   delete peerServerAuxNew.at(pos); //Atualizacao 18-11-15
		cout<<"deletou o vetor temporário"<<endl;
		return true;
	}
	channelSubListLock.unlock();
	for (unsigned int pos=0; pos <peerServerAuxNew.size(); pos++)
	   delete peerServerAuxNew.at(pos); //Atualizacao 18-11-15
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
	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
	{
		int totalPeerInSubChannel=0;
		for (map<string, PeerData>::iterator j = peerList.begin(); j != peerList.end(); j++)
	    	if (j->second.GetChannelId_Sub() == (int)i->second.GetchannelId_Sub())
	    		totalPeerInSubChannel++;
			cout<<"["<<i->first<<"] scID ["<< i->second.GetchannelId_Sub()<<"] TPeerSub ["<<totalPeerInSubChannel<<"] Life ["<<i->second.GetChannelLife()<<"] Mesc ["<<i->second.GetMesclando()<<"] ReNeW ["<<i->second.GetReNewServerSub()<<"]"<<endl;
		    //cout<<"scID ["<< i->second.GetchannelId_Sub()<<"] TPeerSub ["<<totalPeerInSubChannel<<"] TLife ["<<i->second.GetChannelLife()<<"] Mesc ["<<i->second.GetMesclando()<<"] ReNeW ["<<i->second.GetReNewServerSub()<<"]"<<endl;
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

void Channel::SetChannelMode(ChannelModes New_channelMode)
{
	boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
	map<string,SubChannelCandidateData> subCandidatosTempList;

	cout<<"estado atual do channel "<<this->channelMode<<endl;
    cout<<"tentando mudar o estado do channel para "<<New_channelMode<<endl;

    switch (New_channelMode)
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
       					cout<<"configurando "<<i->first<<" no-server "<<endl; //teste
       					i->second.SetState(NO_SERVER_AUX);
       					i->second.SetPeerWaitInform(true);
       					cout<<"esperando informação para "<<i->first<<" "<<i->second.GetPeerWaitInform()<<endl; //teste
       				}
       			this->Remove_AllChannelSub();
        	}

        	this->channelMode = New_channelMode;
        	break;
        }

    	case MODE_FLASH_CROWD_MESCLAR:
    	{
    		//codigo antigo
    		this->mesclarRedes = true;
    	   	//New_channelMode = MODE_FLASH_CROWD;
    	   	/* NÃO TEM BREAK
    	   	 *  Continua o código do flash crowd no case seguinte
    	   	 */
   			for (map<string,SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++){
   				server_Sub_Candidates[i->first].SetState(SERVER_AUX_MESCLAR);
                server_Sub_Candidates[i->first].SetPeerWaitInform(true);
				this->Remove_ChannelSub(&(i->first), true);
				i->second.SetMesclando(true);
				cout<<"esperando informação para "<<i->first<<" "<< server_Sub_Candidates[i->first].GetPeerWaitInform()<<endl; //teste
   			}
   			break;
    	}

    	case MODE_FLASH_CROWD:
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

            this->channelMode = New_channelMode;
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
    			i->second.SetChannelId_Sub(CHANNEL_ID_MESCLANDO);
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

vector<PeerData*> Channel::SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity, bool virtualPeer)
{
    vector<PeerData*> allPeers, selectedPeers;
    int peerChannelId_Sub = peerList[srcPeer->GetID()].GetChannelId_Sub();

    /* Garante fazer a vizinhança apenas com os pares que estão no mesmo subcanal
     * Atende ao caso de só haver o canal principal
     */
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
        if (srcPeer->GetID() != i->second.GetPeer()->GetID() &&                                         // (se não é ele mesmo) e
        		(peerChannelId_Sub == i->second.GetChannelId_Sub() ||                                   // (se o sub = sub) ou
        	     (peerChannelId_Sub == CHANNEL_ID_MESCLANDO &&  i->second.GetChannelId_Sub() == (int)this->channelId)))   // ((se esta em mesclagem) e
        																								//  (outro rede principal))
        {
        	// teste para não permitir que clientes virtuais em uma mesma máquina sejam vizinhos e parceiros
        	if (!virtualPeer){
        	    	allPeers.push_back(&(i->second));
        	}
            else
            	if  (srcPeer->GetIP() != i->second.GetPeer()->GetIP())
            	    allPeers.push_back(&(i->second));
        }
    /* Se src é servidor Auxiliar
     * Nao enviar um par em estado de mesclagem a um servidor auxiliar
     * Isso evita que ele entre em um subCanal
     */
	boost::mutex::scoped_lock channelSubCandidatesLock(*channel_Sub_Candidates_Mutex);
    if (server_Sub_Candidates.count(srcPeer->GetID())> 0)
    {
        for (vector<PeerData*>::iterator i = allPeers.begin(); i != allPeers.end(); i++)
        	if((*i)->GetChannelId_Sub() == (CHANNEL_ID_MESCLANDO))
        		allPeers.erase(i);
    }
    else
		//como os servidores não são executados por demanda, todos da lista de candidatos
		//são configurados como servidor. Por isso, para evitar mandar um par em mesclagem
		// ao servidor, procuro pela lista de candidatos e não de servidores com subcanais.
		// não deve mandar um servidor auxiliar para um peer que está em mesclagem
		if (peerChannelId_Sub == CHANNEL_ID_MESCLANDO)
			for (vector<PeerData*>::iterator i = allPeers.begin(); i != allPeers.end(); i++)
			{
				//como os servidores não são executados por demanda, todos da lista de candidatos
				//são configurados como servidor. Por isso, para evitar mandar um par em mesclagem
				// ao servidor, procuro pela lista de candidatos e não de servidores com subcanais.

				if (server_Sub_Candidates.count((*i)->GetPeer()->GetID()) > 0)
					allPeers.erase(i);
			}
    channelSubCandidatesLock.unlock();


	/* Se o par está em sub canal, incluir o servidor auxiliar na lista de parceiros dele
	* isso é importante visto que o serverAux->idChannel_Sub é igual ao idChannel do canal principal
	* Os servidores auxiliares têm o idChannel_sub na estrutura SubChannelData
	* No caso de CHANNEL_ID_MESCLANDO -1, ele estará em fase de mesclagem e pertence à rede principal
	*/
    if (peerChannelId_Sub != (int)this->channelId && peerChannelId_Sub != CHANNEL_ID_MESCLANDO)
    {
        boost::mutex::scoped_lock channelSubListLock(*channel_Sub_List_Mutex);
    	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
    		if (peerChannelId_Sub == (int)i->second.GetchannelId_Sub())
    			//ver isso
    			//para cluster, informar todos os IDs dos servidores auxiliares
    			allPeers.push_back(&(peerList[i->second.GetServer_Sub()->GetID()]));
      	channelSubListLock.unlock();
    }


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
/*
void Channel::CheckAllSubChannel()  //ECM Função chamada apenas em CheckActivePeers
{
	for (map<string, SubChannelData>::iterator i = channel_Sub_List.begin(); i != channel_Sub_List.end(); i++)
	{
		//garente o processo após o servidor auxiliar ser informado pelo bootstrap
		if (!server_Sub_Candidates[i->first].GetPeerWaitInform())
		{
			i->second.DecChannelLif();  //ECM aparece apenas aqui
			i->second.DecReNewServerSub(); //ECM aparece apenas aqui

			//No tempo 1 o bootstrap é preparado
			// para informar ao servidor o novo estado
			//
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
			if (i->second.GetChannelLife() == 1) //ECM aparece apenas aqui e abaixo (dois lugares)
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
*/
//não é necessário saber em qual subcanal o peer está
//considerar testar se um servidorAuxiliar entrou para remoção
void Channel::CheckActivePeers() //ECM Chamada em bootstrap
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
    cout <<"mesclar redes é "<<mesclarRedes<<endl;
    //if (mesclarRedes)
    //	this->CheckAllSubChannel();
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
