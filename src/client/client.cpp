/*
 * Modificado: Eliseu César miguel
 * Data:       2015-01-19
 * Client deixa de ter um connectorOut. Isso porque os peers ocupam a lista de out quando realizam pedidos.
 * Enquanto isso, o connector tornou-se connectorIn para diferenciar.
 * Em relação às mensagens de Ping, um Client envia o buffermap para seus Out e uma mensagem de ping sem o chunkMap para seus In.
 * Assim, ele informa sua presença na rede.
 * Como agora um peer pode estar na lista de In e Out concomitantemente, podemos avaliar a possibilidade de este peer ter um "modo"
 * como In e outro "modo" como Out.
 */

#include "client.hpp"

using namespace std;

Client::Client(){}

void Client::ClientInit(char *host_ip, string TCP_server_port, string udp_port, uint32_t idChannel, 
            string peers_udp_port, string streamingPort, PeerModes mode, uint32_t buffer, 
            int maxPeersIn, int maxPeersOut, int janela, int num, int ttlIn, int ttlOut, int maxRequestAttempt, int tipOffsetTime, int limitDownload, int limitUpload,
            string disconnectorStrategyIn, string disconnectorStrategyOut, int quantityDisconnect,
			string connectorStrategy, unsigned int minimalBandwidthToBeMyIN,
			int timeToRemovePeerOutWorseBand, string chunkSchedulerStrategy,
            string messageSendScheduler, string messageReceiveScheduler,
			int maxPartnersOutFREE, unsigned int outLimitToSeparateFree)
{
    cout <<"Starting Client Version["<<VERSION<<"]" <<endl;
    Bootstrap_IP = host_ip;
    TCP_server_PORT = TCP_server_port;
    UDP_server_PORT = udp_port;
    peers_UDP_PORT = peers_udp_port;
    externalIp = "";
    externalPort = boost::lexical_cast<uint16_t>(peers_udp_port);
    channelCreationTime = 0;
    bootstrapTimeShift = 0;
    this->idChannel = idChannel;
    this->streamingPort = streamingPort;
    this->peerMode = mode;
    BUFFER_SIZE = buffer;
    peerManager.SetMaxActivePeersIn(maxPeersIn);
    peerManager.SetMaxActivePeersOut(maxPeersOut);
    peerManager.SetMaxActivePeersOutFREE(maxPartnersOutFREE);
    peerManager.SetRemoveWorsePartner (XPConfig::Instance()->GetBool("removeWorsePartner"));
    peerManager.SetMaxOutFreeToBeSeparated(outLimitToSeparateFree);
    JANELA = janela;
    NUM_PEDIDOS = num;
    TTL_MAX_In = ttlIn;
    TTL_MAX_Out = ttlOut;
    this->maxRequestAttempt = maxRequestAttempt;
    this->tipOffsetTime = tipOffsetTime;
    this->configurarBootID = true;
    this->timeToRemovePeerOutWorseBand = timeToRemovePeerOutWorseBand;

    if (limitDownload >= 0)
        this->leakyBucketDownload = new LeakyBucket(limitDownload);
    else
        this->leakyBucketDownload = NULL;
    if (leakyBucketDownload) temporizableList.push_back(leakyBucketDownload);

    if (limitUpload >= 0)
        this->leakyBucketUpload = new LeakyBucket(limitUpload);
    else
        this->leakyBucketUpload = NULL;
    if (leakyBucketUpload) temporizableList.push_back(leakyBucketUpload);

    chunksGen = 0;
    chunksSent = 0;
    chunksRecv = 0;
    chunksOverload = 0;
    requestsSent = 0;
    requestsRecv = 0;
    chunksMissed = -1;
    chunksPlayed = 0;
    downloadPerSecDone = 0;
    uploadPerSecDone = 0;
    
    if (chunkSchedulerStrategy == "MinimumFaultStrategy")
        this->chunkSchedulerStrategy = new MinimumFaultStrategy();
    else if (chunkSchedulerStrategy == "NullStrategy")
        this->chunkSchedulerStrategy = new NullStrategy();
    else
        this->chunkSchedulerStrategy = new RandomStrategy();

    uint64_t updatePeerListPeriod = 30; 
    updatePeerListPeriod = updatePeerListPeriod*SECONDS;

    if (disconnectorStrategyIn == "None")
        this->disconnectorIn = NULL;
    else
        this->disconnectorIn = new Disconnector(new RandomStrategy(), &peerManager, updatePeerListPeriod, peerManager.GetPeerActiveIn(), quantityDisconnect);


    if (disconnectorStrategyOut == "None")
        this->disconnectorOut = NULL;
    else {
    	bool disableServerAuxActive = (disconnectorStrategyOut == "RandomOnlyNoServerActive");
   		this->disconnectorOut = new Disconnector(new RandomStrategy(), &peerManager, updatePeerListPeriod,
    				peerManager.GetPeerActiveOut(), quantityDisconnect, disableServerAuxActive);
    }

    if (disconnectorIn) temporizableList.push_back(disconnectorIn);
    if (disconnectorOut) temporizableList.push_back(disconnectorOut);

    uint64_t updatePeerRequesterPeriod = 20; 
    updatePeerRequesterPeriod = updatePeerRequesterPeriod*SECONDS;
    this->requester = new PeerRequester(this, updatePeerRequesterPeriod);
    if (peerMode == MODE_SERVER)
    {
        this->connectorIn = NULL;
        delete requester;
        this->requester = NULL;
    }
    else
    {
    	if (connectorStrategy == "RandomWhitoutPoor")
            this->connectorIn = new Connector(new RandomStrategyWhitoutPoorBand(),
            		&peerManager, updatePeerListPeriod, peerManager.GetPeerActiveIn(),minimalBandwidthToBeMyIN);
    	else
    		if (connectorStrategy == "RandomWhitoutPoorFREE")
    			this->connectorIn = new Connector(new RandomStrategyWhitoutPoorBand_FREE(),
    			            		&peerManager, updatePeerListPeriod, peerManager.GetPeerActiveIn(),minimalBandwidthToBeMyIN);
    		else
    		          this->connectorIn = new Connector(new RandomStrategy(),
    				                          &peerManager, updatePeerListPeriod, peerManager.GetPeerActiveIn(), 0);
    }
    //TODO More connector options
    if (connectorIn) temporizableList.push_back(connectorIn);
    if (requester) temporizableList.push_back(requester);

    receiveRequestPosition = true;
    perform_udp_punch = true;
    serverActive = false;      //ECM usado apenas para o servidor principal
    quit = false;

    IMessageScheduler* aMessageSendScheduler = NULL; 
    if (messageSendScheduler == "RR")
        aMessageSendScheduler = new RRMessageScheduler();
    else if (messageSendScheduler == "Random")
        aMessageSendScheduler = new RandomMessageScheduler();
    else if (messageSendScheduler == "CDF")
        aMessageSendScheduler = new CDFMessageScheduler();
    else if (messageSendScheduler == "FIFO")
        aMessageSendScheduler = new FIFOMessageScheduler();

    IMessageScheduler* aMessageReceiveScheduler = new FIFOMessageScheduler(); 
    if (messageReceiveScheduler == "RR")
        aMessageReceiveScheduler = new RRMessageScheduler();
    else if (messageReceiveScheduler == "Random")
        aMessageReceiveScheduler = new RandomMessageScheduler();
    else if (messageReceiveScheduler == "CDF")
        aMessageReceiveScheduler = new CDFMessageScheduler();

    udp = new UDPServer(boost::lexical_cast<uint32_t>(peers_UDP_PORT),1,aMessageSendScheduler,aMessageReceiveScheduler);
    sourceVideoReceiver = new UDPServer(boost::lexical_cast<uint32_t>(streamingPort),0,NULL,new FIFOMessageScheduler());  
    peers_UDP_PORT = boost::lexical_cast<string>(udp->Get_binded_port());
    if (peers_UDP_PORT == "0")
      exit(1);

    playerTCPServer = new ClientTCPServer(boost::lexical_cast<uint32_t>(streamingPort),this);
    cbSession = new ClientBootstrapSession(cbSession_io_service,Bootstrap_IP,TCP_server_port,peers_UDP_PORT,this);
    mediaBuffer = new MediaBuffer(BUFFER_SIZE);

    requestPosition.SetBufferSize(BUFFER_SIZE); 
    latestReceivedPosition.SetBufferSize(BUFFER_SIZE); 
    playerDeliveryPosition.SetBufferSize(BUFFER_SIZE);
    lastMediaID = NULL;

    if(!ConnectToBootstrap())
      exit(1);

    if (XPConfig::Instance()->GetBool("clientLogsEnabled") && peerMode != MODE_SERVER)
        CreateLogFiles();
  
    srand ( time(NULL) );
} //Client

//ECM Informes bootstrap if the peer is or not parallel auxiliary candidate
bool Client::isServerCandidate(){return  XPConfig::Instance()->GetBool("serverCandidate");}

bool Client::ConnectToBootstrap()
{
    Message *message;
    time_t nowtime;
    time(&nowtime);
    if (peerMode == MODE_SERVER && !serverActive)
        message = new MessageChannel(CHANNEL_CREATE, perform_udp_punch, externalPort, idChannel, nowtime,
        		XPConfig::Instance()->GetBool("serverCandidate"), NULL_MODE,
				peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut()),
				peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut(true,0))); //servidor (poderei usar a lista outFree para conectar pares especiais ao servidor
    else
        message = new MessageChannel(CHANNEL_CONNECT, perform_udp_punch, externalPort, idChannel, nowtime,
        		XPConfig::Instance()->GetBool("serverCandidate"), NULL_MODE,
				peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut()),
				peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut(true,0))); //soma maxActiveOut com maxActiveOutFree

    message->SetIntegrity();
    int32_t peers_port;
    if(perform_udp_punch)    //perform_udp_punch é usado para informar (uma vez) a porta udp atrás do NAT
    {
        peers_port = cbSession->Punch_connect();
        if(peers_port < 0)   ///Failed
            return false;
    }
    else///!perform_udp_punch
    {
        if(!cbSession->Connect())
            return false;
    }

    if (cbSession->Sync_write(message->GetFirstByte(),message->GetSize()) < 0)//Failed to write
        return false;

    delete message;
    message = (Message *)cbSession->Receive();
    if (message == NULL)///Failed to receive
        return false;
    string bootstrapAddress = Bootstrap_IP;
    bootstrapAddress += ":";
    bootstrapAddress += TCP_server_PORT;
    HandleTCPMessage(message,bootstrapAddress,-1);
    cbSession->Stop();
    return true;
}

void Client::CyclicTimers()
{
    boost::xtime xt;
    uint16_t cycle = 0;
    uint32_t step = 100000000; //100mS++
    uint8_t  mergeCSA_Temp;
    uint8_t removeWorsePartnerTemp;
    mergeCSA_Temp = 0;                   //ECM
    removeWorsePartnerTemp = this->timeToRemovePeerOutWorseBand;          //ECM

    while (!quit)
    {
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.nsec += step;
        
        //Each 100mS
        for (list<Temporizable*>::iterator it = temporizableList.begin(); it != temporizableList.end(); it++)
        {
            ((Temporizable*)*it)->UpdateTimer(step);
        }

        if (playerBufferDuration > step/1000000)
            playerBufferDuration -= step/1000000;
        else
            playerBufferDuration = 0;
        
        if (cycle % 10 == 0)     //Each 1s
        {
            downloadPerSecDone = 0;
            uploadPerSecDone = 0;
            peerManager.CheckPeerList();

            if ((XPConfig::Instance()->GetBool("removeWorsePartner")))
            	if (!this->peerManager.GetRemoveWorsePartner()){
            		if (removeWorsePartnerTemp == 0){
            			this->peerManager.SetRemoveWorsePartner(true);
            			removeWorsePartnerTemp = this->timeToRemovePeerOutWorseBand;
            		}
            		else
            			removeWorsePartnerTemp--;
            	}

            //ECM time to Merge subnetworks
            if (this->peerManager.GetPeerManagerState() == SERVER_AUX_MESCLAR){
            	cout << "Merge Interval... "<<(int) mergeCSA_Temp<<endl;
            	if (mergeCSA_Temp == 0)
            		mergeCSA_Temp = this->peerManager.Get_TimeDescPeerMerge();
            	mergeCSA_Temp = this->peerManager.ExecMesc(mergeCSA_Temp);
            }
        }

        if (cycle % (10 * 30) == 0)     //Each 30s
        {
        	peerManager.ShowPeerList();
        }
        
        cycle++;
        if (cycle >= 10000)    //Reset @1000s
            cycle = 0;
        boost::thread::sleep(xt);
    }
}

/**
*Organiza os dados
*Separa todo streaming que foi recebido na mensagem sobre situação do client no servidor 
*e a lista de peers candidatos no formato padrão stlmap
*/
Message *Client::HandleTCPMessage(Message* message, string sourceAddress, uint32_t socket)
{
    uint8_t opcode = message->GetOpcode();

    switch (opcode)
    {
        case OPCODE_ERROR:
        {
            HandleErrorMessage(new MessageError(message), sourceAddress, socket);
            return NULL;
            break;
        }
        case OPCODE_PEERLIST:
        {
            HandlePeerlistMessage(new MessagePeerlist(message), sourceAddress, socket);
            return NULL;
            break;
        }
        case OPCODE_SERVERAUX:
        {
        	HandleMessageServerSub(new MessageServerSub(message), sourceAddress, socket);
            return NULL;
            break;
        }
        case OPCODE_SERVERAUXLIST:
        {
        	HandleServerAuxListMessage(new MessagePeerlist(message), sourceAddress, socket);
            return NULL;
            break;
        }
        default:
            return NULL;
            break;
    }
}
 
void Client::HandleUDPMessage(Message* message, string sourceAddress)
{
    uint8_t opcode = message->GetOpcode();
    switch (opcode)
    {
        case OPCODE_REQUEST:
        {
            HandleRequestMessage(new MessageRequest(message), sourceAddress);
            break;
        }
        case OPCODE_DATA:
        {
            chunksRecv++;
            downloadPerSecDone += message->GetBodySize();
            HandleDataMessage(new MessageData(message), sourceAddress);
            break;
        }
        case OPCODE_ERROR:
        {
            HandleErrorMessage(new MessageError(message), sourceAddress);
            break;
        }
        case OPCODE_PING:
        {
            HandlePingMessage(new MessagePing(message), sourceAddress);
            break;
        }
        default:
            cout<<"UNKNOWN OPCODE[" << opcode << "] from [" << sourceAddress << "]" <<endl;
            break;
    }
}


/* PEERLIST PACKET:    | OPCODE | HEADERSIZE | BODYSIZE | EXTPORT | CHUNKGUID | QTDPEERS | PEERLIST |  **************************************
** Sizes(bytes):       |   1    |     1      |     2    |    2    |  4  |  2  |    2     |    6     |  TOTAL: 14 Bytes + 6*QTDPEERS *********/
//ECM - Apenas informa os servidores auxiliares aos membros da rede principal
void Client::HandleServerAuxListMessage(MessagePeerlist* message, string sourceAddress, uint32_t socket)
{
    vector<int> peerlistHeader = message->GetHeaderValues();
    uint16_t peersReceived = peerlistHeader[1];

    cout<<"Recebi servidores auxiliares :::";
    for (uint16_t i = 0; i < peersReceived; i++)
         if (Peer* newPeertemp = message->GetPeer(i))
    	    cout<<newPeertemp->GetID()<<" ** ";
    cout <<endl;

    for (uint16_t i = 0; i < peersReceived; i++)
    {
        if (Peer* newPeer = message->GetPeer(i)){
            cout<<"procurando "<<message->GetPeer(i)->GetID()<<"na lista de pares"<<endl;
        	if (peerManager.IsPeerInPeerList(newPeer->GetID())){
        		cout<<"encontrou e está configurando "<<peerManager.GetPeerData(newPeer->GetID())->GetSpecialPeer() <<" para "<<true<<endl;
        		peerManager.GetPeerData(newPeer->GetID())->SetSpecialPeer(true);
        	}
        }
    }


}

/* PEERLIST PACKET:    | OPCODE | HEADERSIZE | BODYSIZE | EXTPORT | CHUNKGUID | QTDPEERS | PEERLIST |  **************************************
** Sizes(bytes):       |   1    |     1      |     2    |    2    |  4  |  2  |    2     |    6     |  TOTAL: 14 Bytes + 6*QTDPEERS *********/
//ECM - após receber lista de novos pares, os inclui na lista de In para fazer requisições (mudança apenas na última linha do código.
void Client::HandlePeerlistMessage(MessagePeerlist* message, string sourceAddress, uint32_t socket)
{
    vector<int> peerlistHeader = message->GetHeaderValues();
    PeerlistTypes messageType = (PeerlistTypes)peerlistHeader[0];
    uint16_t peersReceived = peerlistHeader[1];

    if (messageType == PEERLIST_SHARE)
    {
        MessagePeerlistShare* messageWrapper = new MessagePeerlistShare(message);
        peerlistHeader = messageWrapper->GetHeaderValues();

        uint32_t bootID = peerlistHeader[13];
        if (configurarBootID){
        	cout<<"IDBootConfig = "<<bootID<<endl;
        	this->SetAutentication(bootID);
        	configurarBootID = false;
        }
		else
			cout<<"Valor Autenticado = "<<this->GetAutentication()<<endl;
            cout<<"Valor recebido = "<<bootID<<endl;
		    if (this->GetAutentication() != bootID){
		    	cout<<"Bootstrap is not my. Shutting down ..."<<endl;
		    	exit (1);
		    }

        if (externalIp == "")
        {
            externalIp = boost::lexical_cast<string>((uint32_t)peerlistHeader[2]) + "." 
            + boost::lexical_cast<string>((uint32_t)peerlistHeader[3]) + "." 
            + boost::lexical_cast<string>((uint32_t)peerlistHeader[4]) + "." 
            + boost::lexical_cast<string>((uint32_t)peerlistHeader[5]);
        }
        // Servidor principal
        if (peerMode == MODE_SERVER)
        {
            serverActive = true;
        }
        else
        {
            if (perform_udp_punch)
            {
                externalPort = peerlistHeader[6];
                perform_udp_punch = false;
            }

            ChunkUniqueID serverTip(peerlistHeader[7],(uint16_t)peerlistHeader[8]);
            uint32_t estimatedServerStreamRate = peerlistHeader[9];

            if (channelCreationTime == 0) 
                channelCreationTime = peerlistHeader[10];

            //BootstrapTimeShift calc
            time_t nowtime;
            time(&nowtime);
            uint32_t bootTime = peerlistHeader[11];
            uint32_t sendTime = peerlistHeader[12];
            uint32_t rcvTime = (uint32_t)nowtime;
            bootstrapTimeShift = bootTime - sendTime - (rcvTime - sendTime)/2;
            
            /* Reseta a minha posição de chunk request para a posição do último chunk 
             * produzido pelo servidor recebida do bootstrap
             */
            if (receiveRequestPosition)
            {
                cout<<"EstimatedServerStreamRate: "<<estimatedServerStreamRate<<endl;
                Statistics::Instance()->SetEstimatedChunkRate(estimatedServerStreamRate);
                uint32_t initialOffset = (estimatedServerStreamRate*tipOffsetTime); //Request "tipOffsetTime" seconds prior to the tip
                if (serverTip.GetCycle() == 0 && serverTip.GetPosition() < initialOffset)
                {	
                    requestPosition = ChunkUniqueID();
                    playerDeliveryPosition = ChunkUniqueID();
                } 
                else
                {
                    requestPosition = serverTip - initialOffset;
                    playerDeliveryPosition = serverTip - initialOffset;
                }
                receiveRequestPosition = false;
            }
            cout<<"Server Tip Position: "<<serverTip<<endl;
            cout<<"Client Request Position: "<<requestPosition<<endl;
            cout<<"Client Latest Received Position: "<<latestReceivedPosition<<endl;
            cout<<"Client Player Delivery Position: "<<playerDeliveryPosition<<endl;
            cout<<"Estimated Bootstrap Time Shift: "<<bootstrapTimeShift<<endl;
            cout<<"Estimated Client Latency: "<<Statistics::Instance()->GetLatency()<<endl;

            //PEER LIST
            for (uint16_t i = 0; i < peersReceived; i++)
            {
                if (Peer* newPeer = message->GetPeer(i))
                    peerManager.AddPeer(newPeer);

            }
             //ECM
            this->connectorIn->Connect();
        }
    }
}

/* PING PACKET:        | OPCODE | HEADERSIZE | BODYSIZE | PINGCODE | X | CHUNKGUID |    BITMAP    | **************************************
** Sizes(bytes):       |   1    |     1      |     2    |    1     | 1 |  4  |  2  | BUFFERSIZE/8 | TOTAL: 6 || 12 + (BUFFERSIZE/8) Bytes */
/*ECM - função exclusiva para peerActiveIn
 * Neste caso, a mensagem foi enviada por quem é meu In para informar o buffermap
 */
void Client::HandlePingMessageIn(vector<int>* pingHeader, MessagePing* message, string sourceAddress, uint32_t socket)
{
    uint8_t pingType = (*pingHeader)[0];
    PeerModes otherPeerMode = (PeerModes)(*pingHeader)[1];
    ChunkUniqueID otherPeerTipChunk = ChunkUniqueID((*pingHeader)[2],(uint16_t)(*pingHeader)[3]);
    uint16_t sizePeerListOut = (*pingHeader)[4];
    uint16_t sizePeerListOut_FREE = (*pingHeader)[5];

    //I get to know that peer now if i didn't
    Peer* newPeer = new Peer(sourceAddress, sizePeerListOut, sizePeerListOut_FREE);
    if (!peerManager.AddPeer(newPeer, sizePeerListOut, sizePeerListOut_FREE))
        delete newPeer;

    boost::mutex::scoped_lock peerListLock(*peerManager.GetPeerListMutex());
    if (!peerManager.IsPeerActive(sourceAddress, peerManager.GetPeerActiveIn())) //I received a ping from someone that is not on my active peer list
    {
        if (!peerManager.ConnectPeer(sourceAddress, peerManager.GetPeerActiveIn()))
        {
        	cout<<"Ping by "<<sourceAddress<<" tried to connect to me to be a In but failed. Neighborhood ["<<peerManager.GetPeerActiveSize(peerManager.GetPeerActiveIn())<<"/"<<peerManager.GetMaxActivePeers(peerManager.GetPeerActiveIn())<<"]"<<endl;
            return;
        }
    }
    peerManager.GetPeerData(sourceAddress)->SetTTLIn(TTL_MAX_In);

    peerManager.GetPeerData(sourceAddress)->SetMode(otherPeerMode);
    peerListLock.unlock();

    //Trata a mensagem de ping
    switch (pingType)
    {
        case PING_PART_CHUNKMAP:
            peerListLock.lock();
            if (peerManager.IsPeerActive(sourceAddress, peerManager.GetPeerActiveIn()))
            {
                peerManager.GetPeerData(sourceAddress)->SetChunkMap(otherPeerTipChunk,
                    BytesToBitset(message->GetFirstByte()+message->GetHeaderSize(),(BUFFER_SIZE/8)));
            }
            peerListLock.unlock();
            break;
        default:
            break;
    }
}

// ECM PING LIVE OUT MESSAGE
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  PINGCODE | PEERMODE | CHUNKGUID |
//|   1    |     1      |     2    |     2    |     1     |     1    |  4  |  2  | TOTAL: 14
/*ECM - função exclusiva para peerActiveOut
 * Neste caso, a mensagem foi envida por quem é meu Out para informar que está vivo ou para entrar na lista de Out.
 */
void Client::HandlePingMessageOut(vector<int>* pingHeader, MessagePing* message, string sourceAddress, uint32_t socket)
{
    PeerModes otherPeerMode = (PeerModes)(*pingHeader)[1];
    uint16_t sizePeerListOut = (*pingHeader)[4];
    uint16_t sizePeerListOut_FREE = (*pingHeader)[5];
    
    //I get to know that peer now if i didnt
    Peer* newPeer = new Peer(sourceAddress, sizePeerListOut,sizePeerListOut_FREE );
    if (!peerManager.AddPeer(newPeer, sizePeerListOut, sizePeerListOut_FREE))
        delete newPeer;
    bool outFree= XPConfig::Instance()->GetBool("separatedFreeOutList");

    boost::mutex::scoped_lock peerListLock(*peerManager.GetPeerListMutex());
    if (!peerManager.IsPeerActive(sourceAddress, peerManager.GetPeerActiveOut(outFree,sizePeerListOut))) //I received a ping from someone that is not on my active peer list
    {
        if (!peerManager.ConnectPeer(sourceAddress, peerManager.GetPeerActiveOut(outFree,sizePeerListOut)))
        {
        	cout<<"Ping by "<<sourceAddress<<" tried to connect to me to be Out but failed. Neighborhood "
        			"                          ["<<peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut(outFree,sizePeerListOut))
					                           <<"/"<<peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut(outFree,sizePeerListOut))<<"]"<<endl;
            return;
        }
    }

    peerManager.GetPeerData(sourceAddress)->SetTTLOut(TTL_MAX_Out);
    peerManager.GetPeerData(sourceAddress)->SetMode(otherPeerMode);
    peerListLock.unlock();
}

//ECM Função que recebe um Ping de outro peer e decide se é Ping de In ou de Out
void Client::HandlePingMessage(MessagePing* message, string sourceAddress, uint32_t socket)
{
    vector<int> pingHeader = message->GetHeaderValues();
    uint8_t pingType = pingHeader[0];
    //Trata a mensagem de ping
    switch (pingType)
    {
        case PING_LIVE_OUT:
        	this->HandlePingMessageOut(&pingHeader, message, sourceAddress, socket);
            break;
        default:
        	this->HandlePingMessageIn(&pingHeader, message, sourceAddress, socket);
            break;
    }

}

/* ERRO PACKET:        | OPCODE | HEADERSIZE | BODYSIZE | ERRORCODE | X | **************************************
** Sizes(bytes):       |   1    |     1      |     2    |     1     | 1 | TOTAL: 6 Bytes ***********************/ 
void Client::HandleErrorMessage(MessageError* message, string sourceAddress, uint32_t socket)
{
    vector<int> errorHeader = message->GetHeaderValues();
    switch (errorHeader[0])
    {
        case ERROR_UPLOAD_LIMIT_EXCEEDED:
            //if (peerAtivo.find(sourceAddress) == peerAtivo.end())
            //peerAtivo[sourceAddress].GetPeer()->SetQuality(-1);
            //    return;
            break;
        case ERROR_INVALID_CLIENT_VERSION:
            cout<<"Invalid Client Version"<<endl;
            exit(1);
            break;
        case ERROR_CHANNEL_UNAVAILABLE:
            serverActive = false;
            cout<<"Channel Unavailable"<<endl;
            exit(1);
            break;
        case ERROR_CHANNEL_CANT_BE_CREATED:
            cout<<"Channel cant be created"<<endl;
            exit(1);
            break;
        default:
            break;
    }
    //delete message;
}

// SERVER SUB MESSAGE
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  STATECODE | X |
//|   1    |     1      |     2    |     2    |      1     | 1 | TOTAL: 8 Bytes
void Client::HandleMessageServerSub(MessageServerSub* message, string sourceAddress, uint32_t socket)
{
    vector<int> serverHeader = message->GetHeaderValues();

    peerManager.Set_MergeType((MesclarModeServer)serverHeader[1]);
    peerManager.Set_QT_PeerMergeType(serverHeader[2]);
    peerManager.Set_TimeDescPeerMerge(serverHeader[3]);

    switch ((ServerAuxTypes) serverHeader[0])
    {
        case NO_SERVER_AUX:
        	cout<<"*************************************"<<endl;
            cout<<" ****** NÃO SOU SERVIDOR **********  "<<endl;
            cout<<"*************************************"<<endl;

        	break;
        case  SERVER_AUX_ACTIVE:
        	cout<<"*************************************"<<endl;
            cout<<" ****** SOU SERVIDOR ATIVO ********  "<<endl;
            cout<<"*************************************"<<endl;

        	break;

        case SERVER_AUX_MESCLAR:
        	cout<<"*************************************"<<endl;
            cout<<" ****** SOU SERVIDOR MESCLANDO****   "<<endl;
            cout<<"*************************************"<<endl;

        	break;

        default:
        	 	 cout<<"Trying change server mode but invalid option for server type ["<<(ServerAuxTypes) serverHeader[0]<<"]"<<endl;
            break;
    }

    peerManager.SetPeerManagerState((ServerAuxTypes) serverHeader[0]);
}

/*
/ REQUEST PACKET:    | OPCODE | HEADERSIZE | BODYSIZE | CHUNKGUID |  **************************************
** Sizes(bytes):     |    1   |     1      |     2    |  4  |  2  |  TOTAL: 10 Bytes  **********************/
//ECM - Only for Out List
void Client::HandleRequestMessage(MessageRequest* message, string sourceAddress, uint32_t socket)
{
    requestsRecv++;
    Message* messageReply = NULL;
    vector<int> requestHeader = message->GetHeaderValues();
    ChunkUniqueID requestedChunk(requestHeader[0], (uint16_t)requestHeader[1]);

    if (peerManager.IsPeerActive(sourceAddress, peerManager.GetPeerActiveOut()) ||
    		peerManager.IsPeerActive(sourceAddress, peerManager.GetPeerActiveOut(true,0)))
    {
        if ((mediaBuffer->Available(requestedChunk.GetPosition()))             //If i have the chunk
            && (latestReceivedPosition >= requestedChunk)                      //The request is for past chunks
            && ((latestReceivedPosition - requestedChunk).GetCycle() == 0))    //1 cycle max diff
        {
            uploadPerSecDone += mediaBuffer->GetChunkSize(requestedChunk.GetPosition());
            //ECM. Para acertar o controle de banda, a identação de chunksSent passou a ser realizada após o envio da mensagem, no método UDPSend()
            //chunksSent++;
            messageReply = (Message*)(*mediaBuffer)[requestedChunk.GetPosition()];
        }
        else
        {
            cout<<"Request Error: Peer "<<sourceAddress<<" requested chunk ["<<requestedChunk<<"] that is not available"<<endl;
            messageReply = new MessageError(ERROR_CHUNK_UNAVAILABLE);
            messageReply->SetIntegrity();
        }
    }
    else
    {
        cout<<"Request Error: Peer "<<sourceAddress<<" requested chunk ["<<requestedChunk<<"], but it is not on PeerActiveOut"<<endl;
        messageReply = new MessageError(ERROR_NO_PARTNERSHIP);
        messageReply->SetIntegrity();
    }
    if (messageReply)
    {
        udp->EnqueueSend(sourceAddress, messageReply);
    }
}

/* DATA PACKET:        | OPCODE | HEADERSIZE | BODYSIZE | HOP | X | CHUNKGUID | TIME_STAMP | DATA |  **************************************
** Sizes(bytes):       |   1    |     1      |     2    |  1  | 1 |  4  |  2  |     4      |   x  |  TOTAL: 14 Bytes + DATA  **************/ 
void Client::HandleDataMessage(MessageData* message, string sourceAddress, uint32_t socket)
{
    Message* messageReply = NULL;
    boost::mutex::scoped_lock peerListLock(*peerManager.GetPeerListMutex());

    vector<int> dataHeader = message->GetHeaderValues();
    ChunkUniqueID receivedChunk(dataHeader[2], dataHeader[3]);
    
    if (peerManager.IsPeerActive(sourceAddress, peerManager.GetPeerActiveIn()))
    {
        peerManager.GetPeerData(sourceAddress)->DecPendingRequests();
        peerListLock.unlock();
        
        if ((mediaBuffer->Available(receivedChunk.GetPosition()) && receivedChunk.GetCycle() == mediaBuffer->GetID(receivedChunk.GetPosition()))
            || receivedChunk.GetCycle() < mediaBuffer->GetID(receivedChunk.GetPosition()))
        {
            //Received an copy of a chunk already received (old or not)
            chunksOverload++;

            //Logging
            if (chunkMissFile != NULL)
            {
                time_t rawtime;
                time(&rawtime);
                string chunkMissLog = "";
                if (receivedChunk.GetCycle() < mediaBuffer->GetID(receivedChunk.GetPosition()))
                    chunkMissLog += "DUPE ";
                else
                    chunkMissLog += "LATE ";
                chunkMissLog += boost::lexical_cast<string>(rawtime + bootstrapTimeShift) + " ";
                chunkMissLog += boost::lexical_cast<string>(receivedChunk.GetCycle()) + ":" + boost::lexical_cast<string>(receivedChunk.GetPosition()) + " ";
                chunkMissLog += boost::lexical_cast<string>(playerDeliveryPosition.GetCycle()) + ":" + boost::lexical_cast<string>(playerDeliveryPosition.GetPosition()) + " ";
                chunkMissLog += boost::lexical_cast<string>(latestReceivedPosition.GetCycle()) + ":" + boost::lexical_cast<string>(latestReceivedPosition.GetPosition()) + " ";
                boost::mutex::scoped_lock oldRequestListLock(oldRequestListMutex);
                Request* requestData = oldRequestList[playerDeliveryPosition];
                if (requestData)
                {
                    list<RequestAttempt*> requestAttempts = requestData->GetAttempts();
                    chunkMissLog += boost::lexical_cast<string>(requestAttempts.size()) + " ";
                    for (list<RequestAttempt*>::iterator attemptIt = requestAttempts.begin(); attemptIt != requestAttempts.end();attemptIt++)
                    {
                        RequestAttempt* attempt = *attemptIt;
                        chunkMissLog += boost::lexical_cast<string>(attempt->GetChosenPeerID())  + " ";
                        chunkMissLog += boost::lexical_cast<string>(attempt->GetLastRegisteredDelay()) + " ";
                        chunkMissLog += boost::lexical_cast<string>(attempt->GetCandidates()) + " ";
                    }
                } else
                    chunkMissLog += "0 ";
                oldRequestListLock.unlock();
                chunkMissLog += "\n";
                
            
                fwrite(chunkMissLog.c_str(), 1, chunkMissLog.size(), chunkMissFile);
                fflush(chunkMissFile);
            }
        } 
        else
        {
            Request* correspondentRequest = NULL;
            boost::mutex::scoped_lock requestListLock(requestListMutex);
            list<Request*>::iterator requestIt;
            for (requestIt = requestList.begin(); requestIt != requestList.end(); requestIt++)
            {
                if ((*requestIt)->GetChunkID() == receivedChunk) 
                {
                    correspondentRequest = (*requestIt);
                    break;
                }
            }
            //If the received data has a correspondent request
            if (correspondentRequest != NULL)
            {
                RequestAttempt* correspondentRequestAttempt = NULL;
                list<RequestAttempt*> requestAttempts = correspondentRequest->GetAttempts();
                list<RequestAttempt*>::iterator attemptIt;
                for (attemptIt = requestAttempts.begin(); attemptIt != requestAttempts.end();attemptIt++)
                {
                    if (sourceAddress.compare((*attemptIt)->GetChosenPeerID()) == 0) 
                    {
                        correspondentRequestAttempt = *attemptIt;
                        break;
                    }
                }
                if (correspondentRequestAttempt != NULL)
                {
                    //Clean request
                    boost::mutex::scoped_lock oldRequestListLock(oldRequestListMutex);
                    oldRequestList[correspondentRequest->GetChunkID()] = correspondentRequest;
                    requestList.erase(requestIt);
                    requestListLock.unlock();
                
                    //Tries update
                    uint32_t tries = dataHeader[1];
                    tries += correspondentRequest->GetAttempts().size();
                    message->SetTries(tries);

                    //Hop update
                    uint32_t hop = dataHeader[0];
                    hop++;                                                                 //It represents the hop accounting at this node!
                    message->SetHop(hop);

                    message->SetIntegrity();

                    //Estimate delay to that source
                    struct timeval now;
                    gettimeofday(&now, NULL);
                    boost::mutex::scoped_lock peerListLock(*peerManager.GetPeerListMutex());
                    PeerData* pdSource = peerManager.GetPeerData(sourceAddress);
                    float delay = (now.tv_sec - correspondentRequest->GetAttempts().back()->GetTimestamp().tv_sec);
                    delay += (now.tv_usec - correspondentRequest->GetAttempts().back()->GetTimestamp().tv_usec) / 1000000;
                    pdSource->SetDelay(delay);
                    peerListLock.unlock();

                    //Estimate latency
                    uint32_t estimatedLatency = ((uint32_t)now.tv_sec + bootstrapTimeShift) - dataHeader[4];
                    Statistics::Instance()->AddLatencySample(estimatedLatency);

                
                    mediaBuffer->Insert(message);
                    if (receivedChunk > latestReceivedPosition)
                    {
                        latestReceivedPosition = receivedChunk;
                    }

                    //Logging 
                    if (chunkRcvFile != NULL)
                    {
                        //Chunk sampling
                        //TODO: Make sampling a initialization parameter
                        //if (receivedChunk.GetPosition() % 500 == 0)
                        {
                            time_t rawtime;
                            time(&rawtime);
                            string chunkRcvLog = "";
                            chunkRcvLog += boost::lexical_cast<string>(receivedChunk.GetCycle()) + ":" + boost::lexical_cast<string>(receivedChunk.GetPosition()) + " ";
                            chunkRcvLog += boost::lexical_cast<string>(estimatedLatency) + " ";
                            chunkRcvLog += boost::lexical_cast<string>(hop) + " ";
                            chunkRcvLog += boost::lexical_cast<string>(sourceAddress) + " ";
                            chunkRcvLog += boost::lexical_cast<string>(requestAttempts.size()) + " ";
                            int count = 0;
                            for (attemptIt = requestAttempts.begin(); attemptIt != requestAttempts.end();attemptIt++)
                            {
                                count++;
                                if ((*attemptIt) == correspondentRequestAttempt) break;
                            }
                            chunkRcvLog += boost::lexical_cast<string>(count) + " ";
                            attemptIt = requestAttempts.begin();
                            for (int i = 0; i < count; i++)
                            {
                                RequestAttempt* requestAttempt = *attemptIt;
                                chunkRcvLog += boost::lexical_cast<string>(requestAttempt->GetChosenPeerID())  + " ";
                                chunkRcvLog += boost::lexical_cast<string>(requestAttempt->GetTimestamp().tv_sec + bootstrapTimeShift) + " ";
                                chunkRcvLog += boost::lexical_cast<string>(requestAttempt->GetCandidates()) + " ";
                                attemptIt++;
                            }
                            chunkRcvLog += boost::lexical_cast<string>(rawtime + bootstrapTimeShift) + " ";
                            chunkRcvLog += "\n";
                            oldRequestListLock.unlock();
 
                            fwrite(chunkRcvLog.c_str(), 1, chunkRcvLog.size(), chunkRcvFile);
                            fflush(chunkRcvFile);
                        }
                    } else
                        oldRequestListLock.unlock();
                }
                else
                    requestListLock.unlock();
            }
            else 
                requestListLock.unlock();
        }
    }
    else
    {
        peerListLock.unlock();
        cout<<"Data Error: Peer "<<sourceAddress<<" sent chunk ["<<receivedChunk<<"], but it is not on PeerActiveIn"<<endl;
        messageReply = new MessageError(ERROR_NO_PARTNERSHIP);
        messageReply->SetIntegrity();
    }
    if (messageReply)
    {
        //udp->Send(sourceAddress, messageReply->GetFirstByte(), messageReply->GetSize());
        udp->EnqueueSend(sourceAddress, messageReply);
    }    
}

void Client::Ping()
{
    Message* pingMessage;
    MessagePeerlistLog* peerlistMessage;  //ECM usado para gerar o log de overlay
                                          //Recebe os IP parceiros Out + 000.000.0.00 + parceiros In
    uint32_t pingsSend = 1;
    uint32_t step = 10;
    boost::xtime xt;
    time_t nowtime;
    Peer* bootstrap = new Peer(Bootstrap_IP, UDP_server_PORT);
    Peer peerNulo("0.0.0.0","0"); //ECM - Usado para cirar um separador entre Out e In no log do Overlay
    /** 
     * Loop principal
     * Envia uma mensagem ao servidor de bootstrap dizendo que este cliente está vivo
     * a cada xt.sec segundos
     */    
    
    while (!quit)
    {
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 1;
        boost::thread::sleep(xt);
        pingMessage = NULL;
        peerlistMessage = NULL;

        if (pingsSend % step != 0)    //Each 10s
        {
            pingMessage = new MessagePingBoot(peerMode, latestReceivedPosition,
            		                          //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveIn()),
											  //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut()),
											  //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut(true,0)),

            		                          peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut()),
            		                          peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut(true,0)),
            		                          Statistics::Instance()->GetEstimatedChunkRate(), idChannel);
        }
        else
        {
            int chunksGeneratedPerSecond = chunksGen/step;
            int chunksSentPerSecond = chunksSent/step;
            int chunksReceivedPerSecond = chunksRecv/step;
            int chunksOverloadPerSecond = chunksOverload; //It is missing 'div step' on porpose
            int requestsSentPerSecond = requestsSent/step;
            int requestsRecvPerSecond = requestsRecv/step;
            int requestRetriesPerSecond = requestRetries/step;

            float meanHop = 0;
            float meanTries = 0;
            int availableCount = 0;
            if (mediaBuffer->GetSize())
            {
                for (int i = 0; i < mediaBuffer->GetSize(); i++)
                {   
                    if (mediaBuffer->Available(i))
                    {
                        meanHop += mediaBuffer->GetHop(i);
                        meanTries += mediaBuffer->GetTries(i);
                        availableCount++;
                    }
                }
                meanHop /= availableCount;
                meanTries /= availableCount;
            }
            float meanTriesPerRequest = 0;
            if (!triesPerRequest.empty())
            {
                for (list<int>::iterator it = triesPerRequest.begin(); it != triesPerRequest.end(); it++)
                {   
                    meanTriesPerRequest += *it;
                }
                meanTriesPerRequest /= triesPerRequest.size();
            }

            int lastMediaHopCount = -1;
            int lastMediaTriesCount = -1;
            if (lastMediaID)
            {
                lastMediaHopCount = mediaBuffer->GetHop(lastMediaID->GetPosition());
                lastMediaTriesCount = mediaBuffer->GetTries(lastMediaID->GetPosition());
            }
        
            time(&nowtime);

            int chunksExpected = chunksMissed + chunksPlayed;

            pingMessage = new MessagePingBootPerf(
            		                peerMode,
									latestReceivedPosition,
            		                Statistics::Instance()->GetEstimatedChunkRate(),
									idChannel,
                                    chunksGeneratedPerSecond,
									chunksSentPerSecond,
									chunksReceivedPerSecond,
									chunksOverloadPerSecond,
                                    requestsSentPerSecond,
									requestsRecvPerSecond,
									requestRetriesPerSecond,
                                    chunksMissed,
									chunksExpected,
                                    meanHop,
									meanTries,
									meanTriesPerRequest,
                                    lastMediaID,
									lastMediaHopCount,
									lastMediaTriesCount,
									lastMediaTime + bootstrapTimeShift,
                                    nowtime + bootstrapTimeShift,
									peerManager.GetPeerActiveSize(peerManager.GetPeerActiveIn()),
									peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut()),
									peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut(true,0)),
			                        peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut()),
			                        peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut(true,0)));

            peerlistMessage = new MessagePeerlistLog( 1 + peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut())
            		                                    + peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut(true,0))
            		                                    + peerManager.GetPeerActiveSize(peerManager.GetPeerActiveIn()),
            		                                    idChannel, nowtime + bootstrapTimeShift);


            //Calculate an estimated chunk rate
            if (peerMode == MODE_SERVER)
            {
                estimatedChunkRate = chunksGeneratedPerSecond;
            } 
            else
            {
                if (Statistics::Instance()->GetEstimatedChunkRate() == 0)
                {
                    estimatedChunkRate = chunksReceivedPerSecond;
                }
                else
                {
                    float alpha = 0.125, beta = 0.25;
                    estimatedChunkRate = (1-alpha)*Statistics::Instance()->GetEstimatedChunkRate() + alpha*chunksReceivedPerSecond;
                    devEstimatedChunkRate = (1-beta)*devEstimatedChunkRate + beta*abs(chunksReceivedPerSecond-Statistics::Instance()->GetEstimatedChunkRate());
                }
            }
            Statistics::Instance()->SetEstimatedChunkRate(estimatedChunkRate);

            //Reset statistical metrics
            chunksGen = 0;
            chunksSent = 0;
            chunksRecv = 0;
            chunksOverload = 0;
            requestsSent = 0;
            requestsRecv = 0;
            requestRetries = 0;
            if (chunksMissed >= 0)
                chunksMissed = 0;
            chunksPlayed = 0;
            triesPerRequest.clear();
        }
      
        if (pingMessage)
        {
            pingMessage->SetIntegrity();
            udp->EnqueueSend(bootstrap->GetID(), pingMessage);


            /* ECM - código 100% incluído
             * ping to Active Peer List In
             * aqui, será enviada mensagem simples para informar aos peerActiveIn que this está vivo
             * informa ao in dele qual é o Out total que ele oferece */

            if(peerManager.GetPeerActiveSize(peerManager.GetPeerActiveIn()) > 0)
            {
                pingMessage = new MessagePing(PING_LIVE_OUT, BUFFER_SIZE/8, peerMode, latestReceivedPosition,
                                              //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveIn()),
						                      //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut()),
						                      //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut(true,0)),
                		                      peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut()),
                		                      peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut(true,0)));
                pingMessage->SetIntegrity();

                boost::mutex::scoped_lock peerListLock(*peerManager.GetPeerListMutex());
                boost::mutex::scoped_lock peerActiveInLock(*peerManager.GetPeerActiveMutex(peerManager.GetPeerActiveIn()));
                for (set<string>::iterator i = peerManager.GetPeerActiveIn()->begin(); i != peerManager.GetPeerActiveIn()->end(); i++)
                {
                    Peer* peer = peerManager.GetPeerData(*i)->GetPeer();
                    if (peerlistMessage)
                        peerlistMessage->AddPeer(peer); //ECM insere pares In no log de overlay do participante
                    if (pingMessage && peer)
                    {
                        udp->EnqueueSend(peer->GetID(), pingMessage);
                    }
                }
                peerActiveInLock.unlock();
                peerListLock.unlock();

            }

            //ECM Insere separador da lista de Out e In no log de overlay
            if (peerlistMessage)
                peerlistMessage->AddPeer(&peerNulo); //ECM insere separador entre pares Out e In no log de Overlay


            /* ping to Active Peer List Out
             * ECM. Aqui, será enviada mensagem com buffermap para os peerActiveOut
             * a variável i significa true e false. Com ela, o código é repetido duas vezes. Uma para os OUT e outra para OUTFREE
             * O parâmetro 0 permite pegar o tamanho da lista OUTFree*/

          for (int out=0;out<=1;out++){
            if (peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut(out,0)) > 0)
            {
                pingMessage = new MessagePing(PING_PART_CHUNKMAP, BUFFER_SIZE/8, peerMode, latestReceivedPosition,
                          //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveIn()),
						  //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut()),
						  //peerManager.GetPeerActiveSize(peerManager.GetPeerActiveOut(true,0)),
                		                      peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut(false,0)),
											  peerManager.GetMaxActivePeers(peerManager.GetPeerActiveOut(true,0)));
                pingMessage->SetIntegrity();
                uint8_t headerSize = pingMessage->GetHeaderSize();
                uint8_t *chunkMap = new uint8_t[BUFFER_SIZE/8];
                for (uint32_t i = 0; i < BUFFER_SIZE/8; i++)
                {
                    chunkMap[i] = 0;
                    if (peerMode == MODE_FULLCHUNKMAP)
                        chunkMap[i] = 0xFF;
                }

                if ((peerMode != MODE_FREERIDER_GOOD) && (peerMode != MODE_FULLCHUNKMAP))
                {
                    boost::mutex::scoped_lock bufferMapLock(bufferMapMutex);
                    chunkMap = BitsetToBytes(mediaBuffer->GetMap());
                    bufferMapLock.unlock();
                }

                for (uint32_t i = 0; i < BUFFER_SIZE/8; i++)
                {
                    pingMessage->GetFirstByte()[headerSize + i] = chunkMap[i];
                }

                boost::mutex::scoped_lock peerListLock(*peerManager.GetPeerListMutex());
                boost::mutex::scoped_lock peerActiveOutLock(*peerManager.GetPeerActiveMutex(peerManager.GetPeerActiveOut(out,0)));
                for (set<string>::iterator i = peerManager.GetPeerActiveOut(out,0)->begin(); i != peerManager.GetPeerActiveOut(out,0)->end(); i++)
                {
                    Peer* peer = peerManager.GetPeerData(*i)->GetPeer();
                    if (peerlistMessage)
                        peerlistMessage->AddPeer(peer); //insere pares out no log de ovrelay do participante
                    if (pingMessage && peer)
                    {
                        udp->EnqueueSend(peer->GetID(), pingMessage);
                    }
                }
                peerActiveOutLock.unlock();
                peerListLock.unlock();
            }
          }

        //---------------------------------------------------------------------------------------------------------
            pingsSend++;
        }

        if (peerlistMessage)
        {
            peerlistMessage->SetIntegrity();
            udp->EnqueueSend(bootstrap->GetID(), peerlistMessage);
        }
    }//while (true)
}

/*Converte um inteiro, para o seu equivalente em binário na estrutura de dados
dynamic_bitset*/
boost::dynamic_bitset<> Client::ByteToBitset(uint8_t a)
{
    boost::dynamic_bitset<> x;
    
    while (a >= 1)
    {
        x.push_back(a%2);
        a = a/2;                
    }

    if (a == 1)
        x.push_back(1);
    
    return x;
}

/*Retorna um vetor de unsigned chars  contendo o número binário do parametro "a"
Cada caracter no vetor representa 8 bits */
uint8_t* Client::BitsetToBytes(boost::dynamic_bitset<> a)
{
    unsigned char *c;
    c = new unsigned char [a.size()/8 + a.size()%8];
    unsigned int i;
    int j;
    boost::dynamic_bitset<> x(8);
    j = 0;

    for (i = 0; i < a.size(); i++)
    {
        x[i%8] = a[i];
        if (i%8 == 7)
        {
            c[j] = x.to_ulong();
            j++;
        }
    }
    
    while (i%8 != 0)
    { 
        //Preenche com zero os bits que sobram no ultimo caracter
        x[i%8] = 0;
        i++;
        if (i%8 == 7)
        {
            c[j] = x.to_ulong();
            j++;
        }
    }
    return c;
}
        
/*Retorna em binário o conteudo de um vetor de unsigned char */    
boost::dynamic_bitset<> Client::BytesToBitset(uint8_t* byteVector, int size)
{
    int i,j,k;
    uint8_t integer;
    boost::dynamic_bitset<> bitvector(size*8);
    boost::dynamic_bitset<> bitvector8;
    j = 0;
    
    for (i = 0; i<size; i++)
    {
        integer = byteVector[i];
        bitvector8 = ByteToBitset(integer);
        while (bitvector8.size() < 8)
            bitvector8.push_back(0);

        for (k = 0; k < 8; k++)
        {
            bitvector[j] = bitvector8[k];
            j++;
        }
    }
    return bitvector;
}

void Client::MontarListaPedidos()
{
    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    xt.sec += 1;
    boost::thread::sleep(xt);
    int failureCount = 0;
    int sleepStep = 5000000; //5ms
    int sleepStepInMs = sleepStep/1000000;
    int maxTimeWithoutMediaInSeconds = 10;
    int maxTimeWithoutMediaInMs = maxTimeWithoutMediaInSeconds*1000;

    while (!quit)
    {
        if (!receiveRequestPosition)
        {
            if (!ColocarNaListaDePedidos())
            {
                failureCount+=sleepStepInMs;
                if (failureCount%1000==0)
                    cout << "ERROR: Request Failed ("<<failureCount/1000<<"s / "<<maxTimeWithoutMediaInSeconds<<"s)"<<endl;
            }
            else
            {
                failureCount = 0;
            }
            FazPedidos(sleepStepInMs);

            if (failureCount > maxTimeWithoutMediaInMs)
            {
                failureCount = 0;
                cout << "RESET(MontarListaPedidos) -> The request position will be reseted on the next connection to the Bootstrap"<<endl;
                //Logging
                if (chunkMissFile != NULL)
                {
                    time_t rawtime;
                    time(&rawtime);
                    string chunkMissLog = "RESET REQUESTER ";
                    chunkMissLog += boost::lexical_cast<string>(rawtime + bootstrapTimeShift) + " ";
                    chunkMissLog += boost::lexical_cast<string>(requestPosition.GetCycle()) + ":" + boost::lexical_cast<string>(requestPosition.GetPosition()) + " ";
                    chunkMissLog += boost::lexical_cast<string>(playerDeliveryPosition.GetCycle()) + ":" + boost::lexical_cast<string>(playerDeliveryPosition.GetPosition()) + " ";
                    chunkMissLog += boost::lexical_cast<string>(latestReceivedPosition.GetCycle()) + ":" + boost::lexical_cast<string>(latestReceivedPosition.GetPosition()) + " ";
                    chunkMissLog += "\n";
                
                    fwrite(chunkMissLog.c_str(), 1, chunkMissLog.size(), chunkMissFile);
                    fflush(chunkMissFile);
                }

                receiveRequestPosition = true;
                requestList.clear();
                oldRequestList.clear();
            }
        }

        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.nsec += sleepStep;
        boost::thread::sleep(xt);
    }
}     

//Coloca os requests na lista de pedidos
bool Client::ColocarNaListaDePedidos()
{
    Request* request = CriaRequest();

    if (request->GetSize())
    {
        boost::mutex::scoped_lock requestListLock(requestListMutex);
        requestList.push_back(request);
        requestListLock.unlock();
        requestPosition++;
        boost::mutex::scoped_lock bufferMapLock(bufferMapMutex);
        mediaBuffer->UnsetAvailable(requestPosition.GetPosition());
        bufferMapLock.unlock();
        return true;
    }
    else
        return false;
}

// Verifica quais peers da lista de peers ativos possuem o chunk procurado. Retorna o request contendo os peers.
Request* Client::CriaRequest()
{    
    Request* newRequest = new Request(requestPosition);
   
    boost::mutex::scoped_lock peerListLock(*peerManager.GetPeerListMutex());
    boost::mutex::scoped_lock peerActiveInLock(*peerManager.GetPeerActiveMutex(peerManager.GetPeerActiveIn()));
    map<string, PeerData*> peerActiveWithData;
    for (set<string>::iterator i = peerManager.GetPeerActiveIn()->begin(); i != peerManager.GetPeerActiveIn()->end(); i++)
    {
        peerActiveWithData[*i] = peerManager.GetPeerData(*i);
    }
    peerActiveInLock.unlock();
    peerListLock.unlock();
    newRequest->SearchPeers(&peerActiveWithData);
    return newRequest;
}
// codigo original***
void Client::FazPedidos(int stepInMs)
{
    list<Request*>::iterator it;

    //Clean up request list
    boost::mutex::scoped_lock requestListLock(requestListMutex);
    it = requestList.begin();
    while (it != requestList.end())
    {
        bool eraseIt = false;
        if ((*it)->GetTTL() <= 0)
        {
            if (((maxRequestAttempt == -1) && ((playerBufferDuration < 2000) && (*it)->GetAttempts().size() > 1)) //playerBufferDuration -> something is WRONG, it doesnt accumulate ok
              || ((int)(*it)->GetAttempts().size() >= maxRequestAttempt))
            {
                eraseIt = true;
                cout << "Apagando pedido " << (*it)->GetChunkID() << " expirado depois de " << (*it)->GetAttempts().size() << " tentativas" << endl;
            }
            (*it)->SetTTL(500);
        }

        if (eraseIt)
        {
            triesPerRequest.push_back((*it)->GetAttempts().size());
            boost::mutex::scoped_lock oldRequestListLock(oldRequestListMutex);
            oldRequestList[(*it)->GetChunkID()] = (*it);
            oldRequestListLock.unlock();
            it = requestList.erase(it);
        } else it++;
    }
    requestListLock.unlock();

    //Do requests
    requestListLock.lock();
    for (it = requestList.begin(); it != requestList.end(); it++)
    {
        //Faz os pedidos
        if (((*it)->GetTTL() == 500))
        {
            requestsSent++;
            if ((*it)->GetAttempts().size() != 0)
            {
                requestRetries++;
            }
            //playerDeliveryPosition dá uma noção para o vizinho de quão longe do playback estamos requesting
            MessageRequest* message = new MessageRequest((*it)->GetChunkID(), playerDeliveryPosition);
            message->SetIntegrity();

            boost::mutex::scoped_lock peerListLock(*peerManager.GetPeerListMutex());
            boost::mutex::scoped_lock peerActiveInLock(*peerManager.GetPeerActiveMutex(peerManager.GetPeerActiveIn()));
            map<string, PeerData*> peerActiveWithData;
            for (set<string>::iterator i = peerManager.GetPeerActiveIn()->begin(); i != peerManager.GetPeerActiveIn()->end(); i++)
            {
                peerActiveWithData[*i] = peerManager.GetPeerData(*i);
            }

            (*it)->SearchPeers(&peerActiveWithData);
            //ECM com essa estratégia pode se usar o SURE
            Peer* chosenPeer = (*it)->GetPeer(chunkSchedulerStrategy);
            if (chosenPeer)
            {
                PeerData* chosenPeerData = peerManager.GetPeerData(chosenPeer->GetID());
                chosenPeerData->IncPendingRequests();
                (*it)->CreateAttempt(chosenPeerData);
                udp->EnqueueSend(chosenPeer->GetID(), message);
            }
            peerActiveInLock.unlock();
            peerListLock.unlock();
        }
        (*it)->DecTTL(stepInMs);
    }
    requestListLock.unlock();
}


void Client::GerarDados()
{
    int numbytes;
    clock_t dataArrivalTime;
    clock_t lastDataArrivalTime;
    double dataInterArrivalTime, devDataInterArrivalTime;
    uint8_t buf[MTU];
       
    while (!quit)
    {
        numbytes = sourceVideoReceiver->Sync_recv(buf, MTU, 0);
        
        dataArrivalTime = clock()/(CLOCKS_PER_SEC/1000); // In milliseconds
        double sampleInterArrivalTime = 0;
        if(lastDataArrivalTime != 0)
        {
            float alpha = 0.125, beta = 0.25;
            sampleInterArrivalTime = difftime(dataArrivalTime,lastDataArrivalTime);
            dataInterArrivalTime = (1-alpha)*dataInterArrivalTime + alpha*sampleInterArrivalTime;
            devDataInterArrivalTime = (1-beta)*devDataInterArrivalTime + beta*abs(sampleInterArrivalTime-dataInterArrivalTime);
            //O timeout estimado do pacote recem-recebido é = dataInterArrivalTime + 4*devDataInterArrivalTime
            //O timeout estimado do pacote (recem-recebido - 1) é = sampleInterArrivalTime
        }
        lastDataArrivalTime = dataArrivalTime;
        
        time_t nowtime;
        time(&nowtime);
        MessageData* chunk = new MessageData(numbytes, 0, 0, latestReceivedPosition, nowtime + bootstrapTimeShift, 0);
        chunk->SetMedia(&buf[0]);
        mediaBuffer->Insert(chunk, 0);
        
        int lastChunkPosition = latestReceivedPosition.GetPosition() - 1;
        if (lastChunkPosition < 0)
            lastChunkPosition += latestReceivedPosition.GetBufferSize();
        boost::mutex::scoped_lock bufferMapLock(bufferMapMutex);
        if ((*mediaBuffer)[lastChunkPosition] != NULL)
        {
            if (sampleInterArrivalTime != 0)
                mediaBuffer->SetDuration(lastChunkPosition, (uint32_t)sampleInterArrivalTime);
            (*mediaBuffer)[lastChunkPosition]->SetIntegrity();
            mediaBuffer->SetAvailable(lastChunkPosition);
        }
        bufferMapLock.unlock();
        latestReceivedPosition++;
        chunksGen++;
        
        if(latestReceivedPosition.GetPosition() % 500 == 0)
        {
            time(&lastMediaTime);
            if (lastMediaID)
                delete lastMediaID;
            lastMediaID = new ChunkUniqueID(latestReceivedPosition);    //needs to be a copy
        }
   }
   //close(sock);
   
}

void Client::ConsomeMedia()
{
    while(true)
    {
        if (XPConfig::Instance()->GetBool("playerEnabled"))
            playerTCPServer->Start_accept();
            
        int feedPlayerReturn = FeedPlayer(0);
        cout<< "Client::FeedPlayer() quit with code" << feedPlayerReturn << endl;
        
        if (XPConfig::Instance()->GetBool("playerEnabled"))
            playerTCPServer->Stop_connection(0);
    }
}

void Client::Exit()
{
    quit = true;   
}

int Client::FeedPlayer(int32_t id)
{
    uint32_t maxWaitTime = 3000;
    uint32_t waitTime = 0;
    boost::xtime xt;
  
    while (!quit)
    {
        //SOFT RESET
        if ((latestReceivedPosition > playerDeliveryPosition) &&
            ((latestReceivedPosition - playerDeliveryPosition).GetCycle() > 0))
        {
            //Logging
            if (chunkMissFile != NULL)
            {
                time_t rawtime;
                time(&rawtime);
                string chunkMissLog = "RESET PLAYER ";
                chunkMissLog += boost::lexical_cast<string>(rawtime + bootstrapTimeShift) + " ";
                chunkMissLog += boost::lexical_cast<string>(requestPosition.GetCycle()) + ":" + boost::lexical_cast<string>(requestPosition.GetPosition()) + " ";
                chunkMissLog += boost::lexical_cast<string>(playerDeliveryPosition.GetCycle()) + ":" + boost::lexical_cast<string>(playerDeliveryPosition.GetPosition()) + " ";
                chunkMissLog += boost::lexical_cast<string>(latestReceivedPosition.GetCycle()) + ":" + boost::lexical_cast<string>(latestReceivedPosition.GetPosition()) + " ";
                chunkMissLog += "\n";
                
                fwrite(chunkMissLog.c_str(), 1, chunkMissLog.size(), chunkMissFile);
                fflush(chunkMissFile);
            }

            playerDeliveryPosition.Set(latestReceivedPosition.GetCycle(), latestReceivedPosition.GetPosition());
            uint32_t initialOffset = (Statistics::Instance()->GetEstimatedChunkRate()*tipOffsetTime);
            playerDeliveryPosition = playerDeliveryPosition - initialOffset; //Aprox "tipOffsetTime" segundos atraz do last received position
            cout<<"RESET(FeedPlayer) -> latestReceived["<<latestReceivedPosition<<"] playerDelivery["<<playerDeliveryPosition<<"]"<<endl;
            chunksMissed = -1;
            chunksPlayed = 0;
            maxWaitTime = 3000;
            oldRequestList.clear();
            requestList.clear();
        }

        //Maximize maxWaitTime whenever playerDeliveryPosition = tip
        if (latestReceivedPosition == playerDeliveryPosition)
        {
            maxWaitTime = 3000;
        }

        uint16_t pos_media = playerDeliveryPosition.GetPosition();
        if ((latestReceivedPosition >= playerDeliveryPosition) &&
            (mediaBuffer->Available(pos_media)))
        {
            uint32_t dataDuration = mediaBuffer->GetDuration(pos_media);
            if (XPConfig::Instance()->GetBool("playerEnabled"))
            {
                MessageData* chunk = (*mediaBuffer)[pos_media];
                chunk->Decode(chunk->GetHeaderSize(), chunk->GetSize());
                uint8_t* data = chunk->GetFirstByte() + chunk->GetHeaderSize();
                try
                {
                    if (playerTCPServer->Sync_write(data,chunk->GetBodySize(),id) <= 0)
                        return 0;
                }
                catch (std::exception& e)
                {
                    std::cerr << "Exception in thread: " << e.what() << "\n";
                    return 0;
                }
            }

            playerBufferDuration += dataDuration;

            //Delete request info
            boost::mutex::scoped_lock oldRequestListLock(oldRequestListMutex);
            delete oldRequestList[playerDeliveryPosition];
            oldRequestList.erase(playerDeliveryPosition);
            oldRequestListLock.unlock();
            
            //Statistic chunk sampling
            if (pos_media % 500 == 0)
            {
                cout<<"Lido -> Player["<< playerDeliveryPosition <<"] LastReceived["<< latestReceivedPosition <<"]"<<endl;
                time(&lastMediaTime);
                lastMediaID = new ChunkUniqueID(playerDeliveryPosition);    //needs to be a copy
                if (chunksMissed < 0)
                    chunksMissed = 0;
            }
            chunksPlayed++;
            playerDeliveryPosition++;
            waitTime = 0;
        }
        else
        {
            //Should i sleep or should i discard that chunk?
            if (waitTime < maxWaitTime) //[50ms or 3s]
            {
                boost::xtime_get(&xt, boost::TIME_UTC);
                xt.nsec += 10000000; //10ms
                //cout<<"SEM MEDIA ";
                boost::thread::sleep(xt);
                waitTime+=10;
            }
            else if (playerDeliveryPosition < latestReceivedPosition)
            {
                //Logging
                if (chunkMissFile != NULL)
                {
                    time_t rawtime;
                    time(&rawtime);
                    string chunkMissLog = "MISS ";
                    chunkMissLog += boost::lexical_cast<string>(rawtime + bootstrapTimeShift) + " ";
                    chunkMissLog += boost::lexical_cast<string>(playerDeliveryPosition.GetCycle()) + ":" + boost::lexical_cast<string>(playerDeliveryPosition.GetPosition()) + " ";
                    chunkMissLog += boost::lexical_cast<string>(latestReceivedPosition.GetCycle()) + ":" + boost::lexical_cast<string>(latestReceivedPosition.GetPosition()) + " ";
                    boost::mutex::scoped_lock oldRequestListLock(oldRequestListMutex);
                    Request* requestData = oldRequestList[playerDeliveryPosition];
                    if (requestData)
                    {
                        list<RequestAttempt*> requestAttempts = requestData->GetAttempts();
                        chunkMissLog += boost::lexical_cast<string>(requestAttempts.size()) + " ";
                        for (list<RequestAttempt*>::iterator attemptIt = requestAttempts.begin(); attemptIt != requestAttempts.end();attemptIt++)
                        {
                            RequestAttempt* attempt = *attemptIt;
                            chunkMissLog += boost::lexical_cast<string>(attempt->GetChosenPeerID())  + " ";
                            chunkMissLog += boost::lexical_cast<string>(attempt->GetLastRegisteredDelay()) + " ";
                            chunkMissLog += boost::lexical_cast<string>(attempt->GetCandidates()) + " ";
                        }
                        oldRequestListLock.unlock();
                    }
                    else
                    {
                        oldRequestListLock.unlock();
                        chunkMissLog += "0 ";
                    }
                    chunkMissLog += "\n";
                
                    fwrite(chunkMissLog.c_str(), 1, chunkMissLog.size(), chunkMissFile);
                    fflush(chunkMissFile);
                }

                //Chunk Miss
                cout<<"Chunk Miss!!"<<endl;
                cout<<"CM: Latest Received "<<latestReceivedPosition<<endl;
                cout<<"CM: Player Delivery "<<playerDeliveryPosition<<endl;
                boost::mutex::scoped_lock oldRequestListLock(oldRequestListMutex);
                delete oldRequestList[playerDeliveryPosition];
                oldRequestList.erase(playerDeliveryPosition);
                oldRequestListLock.unlock();
                playerDeliveryPosition++;
                if (chunksMissed < 0)
                    chunksMissed = 0;
                chunksMissed++;
                
                maxWaitTime = 50;
                waitTime = 0;
            }
        }
    }//while
    return 1;
}

Client::~Client()
{
    cout<<endl<<"Shutting down ..."<<endl;
}

void Client::UDPStart()
{
    udp->Start();
    udp->Run();
}

void Client::UDPReceive()
{
    while(true)
    {
        AddressedMessage* aMessage = udp->GetNextMessageToReceive();
        if (aMessage)
        {
            HandleUDPMessage(aMessage->GetMessage(), aMessage->GetAddress());
            if (aMessage->GetMessage()->GetOpcode() != OPCODE_DATA)
            {
                delete aMessage->GetMessage();
            }    
            delete aMessage;
        }
    }
}

void Client::UDPSend()
{
    while(true)
    {
        AddressedMessage* aMessage = udp->GetNextMessageToSend();
        if (aMessage)
        {
            if (aMessage->GetAge() < 0.5) // If message older than 500 ms
            {
                if (leakyBucketUpload) //If do exist leaky bucket 
                {
                    //If only data passes the leaky bucket
                    if (!XPConfig::Instance()->GetBool("leakyBucketDataFilter") || aMessage->GetMessage()->GetOpcode() == OPCODE_DATA) 
                        while (!leakyBucketUpload->DecToken(aMessage->GetMessage()->GetSize())); //while leaky bucket cannot provide
                }
                udp->Send(aMessage->GetAddress(),aMessage->GetMessage()->GetFirstByte(),aMessage->GetMessage()->GetSize());
                /*ECM Correção no controle de banda.
                 * Inicialmente, a variável chunksSent estava sendo identada quando era chegava um pedido por chunk, e não
                 * quando efetivamente o chunck era enviado. Assim, movemos a identação para esse código, que configura realiza o controle.
                 * Neste metodo, inserimos apenas as duas linhas que se seguem.
                 */
                if (aMessage->GetMessage()->GetOpcode() == OPCODE_DATA)
                   chunksSent++;
            }
        }
    }
}

void Client::CreateLogFiles()
{
    time_t rawtime;
    struct tm * timeinfo;
    char timestr[20];
    if(channelCreationTime == 0)
        time(&rawtime);
    else
        rawtime = channelCreationTime;
    timeinfo = localtime(&rawtime);
    strftime (timestr,20,"%Y%m%d%H%M",timeinfo);
    string logFilename = "log-";
    logFilename += boost::lexical_cast<string>(this->idChannel) + "-";
    logFilename += externalIp + "_";
    logFilename += boost::lexical_cast<string>((uint32_t)externalPort) + "-";
    logFilename += timestr;
    logFilename += "-";
    string logFilenameChunkMiss = logFilename + "chunkMiss.txt";
    string logFilenameChunkRcv = logFilename + "chunkRcv.txt";
    chunkMissFile = fopen(logFilenameChunkMiss.c_str(),"w");
    chunkRcvFile = fopen(logFilenameChunkRcv.c_str(),"w");
}

uint32_t Client::GetAutentication(){
	return this->bootStrapID_Autentic;
}
void Client::SetAutentication(uint32_t bootID){
	bootStrapID_Autentic = bootID;

}
