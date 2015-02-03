#include "bootstrap.hpp"

using namespace std;

/** Construtor **/
Bootstrap::Bootstrap(string udpPort, string peerlistSelectorStrategy) 
{
    if (peerlistSelectorStrategy == "TournamentStrategy")
        this->peerlistSelectorStrategy = new TournamentStrategy();
    else if (peerlistSelectorStrategy == "NearestIPStrategy")
        this->peerlistSelectorStrategy = new NearestIPStrategy();
    else
        this->peerlistSelectorStrategy = new RandomStrategy();

    srand (time(NULL));

    udp = new UDPServer(boost::lexical_cast<uint32_t>(udpPort),0,NULL,new FIFOMessageScheduler());

    cout <<"Starting Bootstrap Version["<<VERSION<<"]" <<endl;
}

Message *Bootstrap::HandleTCPMessage(Message* message, string sourceAddress, uint32_t socket)
{
    uint8_t opcode = message->GetOpcode();

    switch (opcode)
    {
    case OPCODE_CHANNEL:
        return HandleChannelMessage(new MessageChannel(message), sourceAddress);
        break;
    default:
        return NULL;
        break;
    }
    //delete message;
}

Message *Bootstrap::HandleChannelMessage(MessageChannel* message, string sourceAddress)
{
    Peer* source = new Peer(sourceAddress);

    vector<int> channelHeader = message->GetHeaderValues();
    uint8_t channelFlag = channelHeader[0];
    bool performingPunch = channelHeader[1];
    uint16_t version = channelHeader[2];
    uint16_t externalPort = channelHeader[3];
    uint32_t channelId = channelHeader[4];
    uint32_t clientTime = channelHeader[5];

    cout<<"Channel MSG: "<<(uint32_t)channelFlag<<", "<<performingPunch<<", "<<version<<", "<<externalPort<<", "<<channelId<<endl;
    if (!performingPunch)
        source->SetPort(boost::lexical_cast<string>(externalPort));

    Message* messageReply = NULL;
    if (version == VERSION)
    {
        boost::mutex::scoped_lock channelListLock(channelListMutex);
        switch (channelFlag)
        {
        case CHANNEL_CREATE:
            if (channelList.find(channelId) == channelList.end())
            {
                channelList[channelId] = Channel(channelId, source);
            }
            else
            {
                messageReply = new MessageError(ERROR_CHANNEL_CANT_BE_CREATED);
                //message = "Channel "+idChannelStr+" cant be created, ID already in use";
            }
            break;
        case CHANNEL_CONNECT:
            if (channelList.count(channelId) == 0)
            {
                messageReply = new MessageError(ERROR_CHANNEL_UNAVAILABLE);
                //message = "Channel "+idChannelStr+" unavailable";
            }
            else 
            {
                if (!channelList[channelId].HasPeer(source))
                {
                    channelList[channelId].AddPeer(source);
                }
            }
            break;
        default:
            cout<<"Unknown error, flags de channel erradas?"<<endl;
            messageReply = new MessageError(ERROR_NONE);
            break;
        }

        if (!messageReply)
        {
            vector<PeerData*> selectedPeers = channelList[channelId].SelectPeerList(peerlistSelectorStrategy, source, 20);

            time_t nowtime;
            time(&nowtime);
            messageReply = new MessagePeerlistShare(selectedPeers.size(), source->GetIP(), externalPort, 
                channelList[channelId].GetServerNewestChunkID(), channelList[channelId].GetServerEstimatedStreamRate(), 
                channelList[channelId].GetCreationTime(), nowtime, clientTime);
            /**
            * Varre a lista de peers canditados, separando cada campo por um caracter de seperação    
            * caso esse campo for ultimo campo a ser enviado insere um caracter que sinaliza o fim da mensagem
            * envia para o cliente
            */    
            for (uint16_t i = 0; i < selectedPeers.size(); i++) 
            {
                ((MessagePeerlist*)messageReply)->AddPeer(selectedPeers[i]->GetPeer());
            }
        }
        channelListLock.unlock();
    } 
    else
    {
        messageReply = new MessageError(ERROR_INVALID_CLIENT_VERSION);
    }

    messageReply->SetIntegrity();
    return messageReply;
}

void Bootstrap::HandleUDPMessage(Message* message, string sourceAddress)
{
    uint8_t opcode = message->GetOpcode();
    switch (opcode)
    {
    case OPCODE_PING:
        HandlePingMessage(new MessagePingBoot(message), sourceAddress);
        break;

    case OPCODE_PEERLIST:
        HandlePeerlistMessage(new MessagePeerlist(message), sourceAddress);
        break;

    default:
        cout<<"UNKNOWN OPCODE[" << opcode << "] from [" << sourceAddress << "]: " << message->GetFirstByte() <<endl;
        break;
    }
}

/* PEERLIST PACKET:    | OPCODE | HEADERSIZE | BODYSIZE | EXTPORT | CHUNKGUID | QTDPEERS | PEERLIST |  **************************************
** Sizes(bytes):       |   1    |     1      |     2    |    2    |  4  |  2  |    2     |    6     |  TOTAL: 14 Bytes + 6*QTDPEERS *********/ 
void Bootstrap::HandlePeerlistMessage(MessagePeerlist* message, string sourceAddress)
{
    Peer* srcPeer = new Peer(sourceAddress);

    vector<int> peerlistHeader = message->GetHeaderValues();
    PeerlistTypes messageType = (PeerlistTypes)peerlistHeader[0];
    uint16_t peersReceived = peerlistHeader[1];

    if (messageType == PEERLIST_LOG)
    {
        MessagePeerlistLog* messageWrapper = new MessagePeerlistLog(message);
        peerlistHeader = messageWrapper->GetHeaderValues();

        uint32_t channelId = peerlistHeader[2];
        uint32_t msgTime = peerlistHeader[3];

        boost::mutex::scoped_lock channelListLock(channelListMutex);
        if (channelList.count(channelId) != 0) //Channel exists
        {
            if (channelList[channelId].HasPeer(srcPeer))
            {
                FILE* overlayFile = channelList[channelId].GetOverlayFile();
                channelListLock.unlock();

                string overlayLog  = sourceAddress + " ";                           //PeerID
                overlayLog += boost::lexical_cast<string>(msgTime) + " ";           //MsgTime
                //PEER LIST
                for (uint16_t i = 0; i < peersReceived; i++)
                {
                    if (Peer* peer = message->GetPeer(i))
                    {    
                        overlayLog += peer->GetID() + " ";
                    }
                }
                overlayLog += "\n";

                if (overlayFile)
                {
                    fwrite(overlayLog.c_str(), 1, overlayLog.size(), overlayFile);
                    fflush(overlayFile);
                }
            }
            else
            {
                channelListLock.unlock();
                cout<<"UDPServer: Message received from a peer that is not on the list ("<<sourceAddress<<")"<<endl<<endl;
            }
        }
        else
        {
            channelListLock.unlock();
        }
    }
}


/* PING PACKET:        | OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM | PINGCODE | PEERMODE | CHUNKGUID |    BITMAP    | **************************************
** Sizes(bytes):       |   1    |     1      |     2    |     2    |    1     |     1    |  4  |  2  | BUFFERSIZE/8 | TOTAL: 6 || 14 + (BUFFERSIZE/8) Bytes */ 
void Bootstrap::HandlePingMessage(MessagePingBoot* message, string sourceAddress, uint32_t socket)
{
    Peer* srcPeer = new Peer(sourceAddress);

    //Trata a mensagem de ping
    vector<int> pingHeader = message->GetHeaderValues();
    uint8_t pingType = pingHeader[0];
    PeerModes peerMode = (PeerModes)pingHeader[1];
    ChunkUniqueID peerTipChunk = ChunkUniqueID(pingHeader[2],(uint16_t)pingHeader[3]);
    int serverStreamRate = pingHeader[4];
    uint32_t channelId = pingHeader[5];

    boost::mutex::scoped_lock channelListLock(channelListMutex);
    if (channelList.count(channelId) != 0) //Channel exists
    {
        if (channelList[channelId].HasPeer(srcPeer))
        {
            channelList[channelId].GetPeerData(srcPeer).SetMode(peerMode);
            channelList[channelId].GetPeerData(srcPeer).SetTTL(TTL);

            //If ping from server
            if (*(srcPeer) == *(channelList[channelId].GetServer())) 
            {
                channelList[channelId].SetServerNewestChunkID(peerTipChunk);
                channelList[channelId].SetServerEstimatedStreamRate(serverStreamRate);
            }

            FILE* performanceFile = channelList[channelId].GetPerformanceFile();
            channelListLock.unlock();

            //If it has performance measures
            if (pingType == PING_BOOT_PERF)
            {
                uint8_t indexPerfStart = 6;
                MessagePingBootPerf* messageWrapper = new MessagePingBootPerf(message);
                pingHeader = messageWrapper->GetHeaderValues();

                time_t rawtime;
                time(&rawtime);
                string performanceLog = sourceAddress + " ";                            //PeerID
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 0]) + " ";        //ChunkGenerated
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 1]) + " ";        //ChunkSent
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 2]) + " ";        //ChunkReceived
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 3]) + " ";        //ChunkOverload
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 4]) + " ";        //RequestSent
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 5]) + " ";       //RequestRecv
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 6]) + " ";       //RequestRetries
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 7]) + " ";       //ChunkMissed
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 8]) + " ";       //ChunkExpected
                performanceLog += boost::lexical_cast<string>(
                    (float)*reinterpret_cast<float*>(&pingHeader[indexPerfStart + 9])) + " ";    //MeanHop
                performanceLog += boost::lexical_cast<string>(
                    (float)*reinterpret_cast<float*>(&pingHeader[indexPerfStart + 10])) + " ";    //MeanTries
                performanceLog += boost::lexical_cast<string>(
                    (float)*reinterpret_cast<float*>(&pingHeader[indexPerfStart + 11])) + " ";    //MeanTriesPerRequest
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 12]) + " ";     //NeighborhoodSize
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 13]) + " ";    //SampleChunk.Cycle
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 14]) + " ";    //SampleChunk.Position
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 15]) + " ";    //SampleChunk.HopCount
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 16]) + " ";    //SampleChunk.TriesCount
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 17]) + " ";    //SampleChunk.Time
                performanceLog += boost::lexical_cast<string>(pingHeader[indexPerfStart + 18]) + " ";    //MsgTime
                performanceLog += boost::lexical_cast<string>(rawtime) + "\n";            //NowTime
                if (performanceFile)
                {
                    fwrite(performanceLog.c_str(), 1, performanceLog.size(), performanceFile);
                    fflush(performanceFile);
                }

                int peerUpload = pingHeader[indexPerfStart + 1];
                int neighborhoodSize = pingHeader[indexPerfStart + 12];
                PeerData pd = channelList[channelId].GetPeerData(srcPeer);
                if (neighborhoodSize)
                    pd.SetUploadScore((pd.GetUploadScore() + peerUpload)/neighborhoodSize);
            }
        }
        else
        {
            channelListLock.unlock();
            cout<<"UDPServer: Message received from a peer that is not on the list ("<<sourceAddress<<")"<<endl<<endl;
        }
    }
    else
    {
        channelListLock.unlock();
        //packetSize = EHS;
        //packet = new uint8_t[packetSize];
        //AssembleErrorHeader(packet, ERROR_CHANNEL_UNAVAILABLE);
        //message = "Channel "+idChannelStr+" unavailable";
    }
}

/** Verifica se algum peer da lista está inativo (TTL <= 0) caso esteja, removo esse peer */
void Bootstrap::CheckPeerList() 
{
    boost::xtime xt;
    boost::mutex::scoped_lock channelListLock(channelListMutex);
    channelListLock.unlock();
    while (true) 
    {
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 1;

        cout<<"#";
        channelListLock.lock();
        vector<unsigned int> deletedChannel;
        for (map<unsigned int, Channel>::iterator channel = channelList.begin(); channel != channelList.end(); channel++)
        {
            channel->second.PrintPeerList();
            channel->second.CheckActivePeers();

            if (!channel->second.HasPeer(channel->second.GetServer()))
                deletedChannel.push_back(channel->first);
        }
        for (vector<unsigned int>::iterator it = deletedChannel.begin(); it < deletedChannel.end(); it++)
        {
            channelList.erase(*it);
            cout<<"Channel "<<*it<<" removed"<<endl;
        }
        channelListLock.unlock();
        cout<<"#"<<endl;

        boost::thread::sleep(xt);
    }
}

/** Varre a lista mostrando todos os peers e suas informções */
void Bootstrap::ShowPeerList() 
{   
    for (map<unsigned int, Channel>::iterator channel = channelList.begin(); channel != channelList.end(); channel++)
        channel->second.PrintPeerList();
}

/** Conta o numero de peers na lista */
void Bootstrap::HTTPLog() 
{
    boost::xtime xt;
    FILE *pfile;
    time_t rawtime;
    struct tm * timeinfo;
    string head = "<html><body><center><h1>Bootstrap log</h1></center>Peers Conectados - Log time<P>";
    string tail = "</body></html>";
    string log;
    string all;
    list<string> last_logs;
    list<string>::iterator it;
    int html_update_time = 0;
    //vector<bool> vetor;
    while (true)
    {
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 2;

        if (last_logs.size() >= 60)
            last_logs.pop_front();
        unsigned int totalPeers = 0;
        boost::mutex::scoped_lock channelListLock(channelListMutex);
        for (map<unsigned int, Channel>::iterator channel = channelList.begin(); channel != channelList.end(); channel++)
            totalPeers = channel->second.GetPeerListSize();
        channelListLock.unlock();

        cout<<"* Connected Peers: "<<totalPeers<<endl;
        if(html_update_time >= 30)
        {
            log += boost::lexical_cast<string>(totalPeers);
            log += " - ";
            time ( &rawtime);
            timeinfo = localtime(&rawtime);
            log += asctime(timeinfo);
            log += "<P>";
            last_logs.push_back(log);
            all = head;
            it = last_logs.begin();
            while ( it != last_logs.end()){
                all += it->c_str();
                it++;
            }
            all += tail;
            pfile = fopen("public_html/bootstraplog/index.html","w");
            if (pfile)
            {
                fwrite(all.c_str(),1,all.size(),pfile);
                fflush(pfile);
                fclose(pfile);
            }

            all.clear();
            log.clear();
            html_update_time = 0;
        }
        html_update_time++;
        boost::thread::sleep(xt);
    }
}

/**
* Cria o servidor de TCP que fica em loop esperando receber uma conexão
* Quando a coneção é recebida abre uma thread para tratar essa conexão
*/
void Bootstrap::TCPStart(const char *myTCPPort)
{
    /** Configura conexão tcp */
    tcpServer = new BootstrapTCPServer(boost::lexical_cast<short>(myTCPPort),this);
    tcpServer->Run();    
}

void Bootstrap::UDPStart()
{
    udp->Start();
    udp->Run();
}

void Bootstrap::UDPReceive()
{
    while(true)
    {
        AddressedMessage* aMessage = udp->GetNextMessageToReceive();
        if (aMessage)
        {
            HandleUDPMessage(aMessage->GetMessage(), aMessage->GetAddress());
            delete aMessage;
        }
    }
}
