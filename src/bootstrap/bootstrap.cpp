/* Para evitar as informações que o bootstrap imprime na tela para outros testes, comentar as chamadas
 * channel->second.PrintPeerList(); e channel->second.PrintPeerList();
 */

#include "bootstrap.hpp"

using namespace std;

/** Construtor **/
Bootstrap::Bootstrap(string udpPort, string peerlistSelectorStrategy, unsigned int peerListSharedSize,
		unsigned int maxSubChannel, unsigned int maxServerAuxCandidate,
		unsigned int maxPeerInSubChannel, unsigned int sizeCluster,
		MesclarModeServer MixType,	uint8_t QT_PeerMixType,	uint8_t TimeDescPeerMix, uint8_t minimumBandwidth, uint8_t minimumBandwidth_FREE){

	if (peerlistSelectorStrategy == "TournamentStrategy")
		this->peerlistSelectorStrategy = new TournamentStrategy();
	else if (peerlistSelectorStrategy == "NearestIPStrategy")
		this->peerlistSelectorStrategy = new NearestIPStrategy();
	else
		this->peerlistSelectorStrategy = new RandomStrategy();
	this->peerListSharedSize=peerListSharedSize;

	srand(time(NULL));

	udp = new UDPServer(boost::lexical_cast<uint32_t>(udpPort), 0, NULL,
			new FIFOMessageScheduler());

	cout << "Starting Bootstrap Version[" << VERSION << "]" << endl;

	this->minimumBandwidth = minimumBandwidth;
	this->minimumBandwidth_FREE = minimumBandwidth_FREE;
	// para configurar o channel para o flash crowd.
	this->maxSubChannel = maxSubChannel;
	this->maxServerAuxCandidate = maxServerAuxCandidate;
	this->maxPeerInSubChannel = maxPeerInSubChannel;
	this->sizeCluster = sizeCluster;

	// config Mix mode...
	this->MixType             = MixType;
	this->QT_PeerMixType      = QT_PeerMixType;
	this->TimeDescPeerMix     = TimeDescPeerMix;


	time_t boot_ID;
	time(&boot_ID);
	bootStrap_ID = boot_ID;

	cout<<"--Merge Settings --"<<endl;
	cout<<"Merge Type: "<<MixType<<endl;
	cout<<"Quantity of Peers: "<<(int)QT_PeerMixType<<endl;
	cout<<"Time Interval: "<<(int)TimeDescPeerMix<<endl;
}

Message *Bootstrap::HandleTCPMessage(Message* message, string sourceAddress,
		uint32_t socket) {
	uint8_t opcode = message->GetOpcode();

	switch (opcode) {
	case OPCODE_CHANNEL:
		return HandleChannelMessage(new MessageChannel(message), sourceAddress);
		break;
	default:
		return NULL;
		break;
	}
}

Message *Bootstrap::HandleChannelMessage(MessageChannel* message, string sourceAddress) {


	vector<int> channelHeader = message->GetHeaderValues();
	uint8_t channelFlag = channelHeader[0];
	bool performingPunch = channelHeader[1];
	uint16_t version = channelHeader[2];
	uint16_t externalPort = channelHeader[3];
	uint32_t channelId = channelHeader[4];
	uint32_t clientTime = channelHeader[5];
	bool auxiliarServerCandidate = channelHeader[6]; //ECM inform bootstrap if peer is or not auxiliary server candidate
	uint8_t channelMode = channelHeader[7]; //ECM used if channelFlag == CHANGE_STATE
	uint16_t sizePeerListOut = channelHeader[8];
	uint16_t sizePeerListOut_FREE = channelHeader[9];

	Peer* source = new Peer(sourceAddress, sizePeerListOut, sizePeerListOut_FREE);

	cout << "Channel MSG: " << (uint32_t) channelFlag << ", " << performingPunch
			<< ", " << version << ", " << externalPort << ", " << channelId
			<< endl;

	if (!performingPunch)
		source->SetPort(boost::lexical_cast<string>(externalPort));

	Message* messageReply = NULL;
	if (version == VERSION) {
		boost::mutex::scoped_lock channelListLock(channelListMutex);
		switch (channelFlag) {

		case CHANNEL_CREATE:
			if (channelList.find(channelId) == channelList.end()) {
				channelList[channelId] = Channel(channelId, source,
						maxSubChannel, maxServerAuxCandidate, maxPeerInSubChannel, this->sizeCluster,
						XPConfig::Instance()->GetBool("subChannelMixed"), false);
			} else {
				messageReply = new MessageError(ERROR_CHANNEL_CANT_BE_CREATED);
			}
			break;

		case CHANNEL_CONNECT:
			if (channelList.count(channelId) == 0) {
				messageReply = new MessageError(ERROR_CHANNEL_UNAVAILABLE);
			} else {
				//deve ser revisto. Caso já exista e esteja em um sub canal, ele tentará entrar novamente no sub canal...
				if (!channelList[channelId].HasPeer(source)) {
					channelList[channelId].AddPeer(source);

				}

				if (auxiliarServerCandidate){
					channelList[channelId].analizePeerToBeServerAux(source);
				}
			}
			break;

		case CHANGE_STATE:
			if (channelMode != NULL_MODE) {
				this->setChannelState(channelId, channelMode);

				//mensagem devolvida ao externalMessage de estado... não usada para nada, mas o método exige replay
				messageReply = new MessageStateChannel(
						(ChannelModes) channelMode);
			} else {
				cout << "No Channel State altered..." << endl;
				messageReply = new MessageError(ERROR_CHANNEL_STATE_NULL);
			}
			break;

		default:
			cout << "Unknown error, flags de channel erradas?" << endl;
			messageReply = new MessageError(ERROR_NONE);
			break;
		}

		// ECM não pode ser CHANGE_STATE porque apenas o externalmessage
		// envia esse código e ele não é um peer válido
		if (!messageReply && (channelFlag != CHANGE_STATE)) {

			if (channelList[channelId].GetServerSubWaitInform(source)) {
				messageReply = new MessageServerSub(
						channelList[channelId].GetServerSubNewMode(source),
						this->MixType,	this->QT_PeerMixType, this->TimeDescPeerMix);
				channelList[channelId].SetServerSubWaitInform(source, false);
			} else {
				vector<PeerData*> selectedPeers;
				time_t nowtime;
				time(&nowtime);

				// Mensagem de servidores auxiliares para participante da rede principal
				if (channelList[channelId].GetWaitServerList(source))
				{
				     selectedPeers = channelList[channelId].MakeServerAuxList();

				     messageReply = new MessagePeerlistShare(selectedPeers.size(),
					    	source->GetIP(), externalPort,
						    channelList[channelId].GetServerNewestChunkID(),
						    channelList[channelId].GetServerEstimatedStreamRate(),
						    channelList[channelId].GetCreationTime(), nowtime,
						    clientTime, this->bootStrap_ID, OPCODE_SERVERAUXLIST);

				}
				else // Mensagem de novos vizinhos
				{
				     selectedPeers = channelList[channelId].SelectPeerList( peerlistSelectorStrategy, source, peerListSharedSize,
				                                             XPConfig::Instance()->GetBool("isolaVirtutalPeerSameIP"),
							                                 minimumBandwidth,minimumBandwidth_FREE, XPConfig::Instance()->GetBool("separatedFreeOutList"));

				      messageReply = new MessagePeerlistShare(selectedPeers.size(),
					    	source->GetIP(), externalPort,
						    channelList[channelId].GetServerNewestChunkID(),
						    channelList[channelId].GetServerEstimatedStreamRate(),
						    channelList[channelId].GetCreationTime(), nowtime,
						    clientTime,this->bootStrap_ID);
				}
				/**
				 * Varre a lista de peers canditados, separando cada campo por um caracter de separação
				 * caso esse campo seja ultimo campo a ser enviado insere um caracter que sinaliza o fim da mensagem
				 * envia para o cliente
				 */
				for (uint16_t i = 0; i < selectedPeers.size(); i++) {
						((MessagePeerlist*) messageReply)->AddPeer(selectedPeers[i]->GetPeer());

				}
				/* Se foi mensagem de lista de servidores, remover o vetor e
				* configurar WaitServerList para false
				*/
				if (channelList[channelId].GetWaitServerList(source)){
				     channelList[channelId].SetWaitServerList(source,false);
				     for (uint16_t i = 0; i < selectedPeers.size(); i++)
				    	 delete selectedPeers.at(i);
				}

			}
		}

		channelListLock.unlock();
	} else {
		messageReply = new MessageError(ERROR_INVALID_CLIENT_VERSION);
	}
	messageReply->SetIntegrity();
	return messageReply;
}

void Bootstrap::setChannelState(uint32_t channelId, uint8_t channelState) {
	for (map<unsigned int, Channel>::iterator channel = channelList.begin();
			channel != channelList.end(); channel++)
		if (channel->first == channelId)
			channel->second.SetChannelMode((ChannelModes) channelState, XPConfig::Instance()->GetBool("TIMEMERGE"));

	if (channelState == MODE_FLASH_CROWD)
		cout << " ***** CHANNEL STATE NEW * FLASH CROWD ****" << endl;
	if (channelState == MODE_FLASH_CROWD_MESCLAR)
		cout << " ***** CHANNEL STATE NEW * FLASH CROWD MESCLAR ****" << endl;
	if (channelState == MODE_NORMAL)
		cout << " **** CHANNEL STATE NEW * MODE_NORMAL ****" << endl;
}

void Bootstrap::HandleUDPMessage(Message* message, string sourceAddress) {
	uint8_t opcode = message->GetOpcode();
	switch (opcode) {
	case OPCODE_PING:
		HandlePingMessage(new MessagePingBoot(message), sourceAddress);
		break;

	case OPCODE_PEERLIST:
		HandlePeerlistMessage(new MessagePeerlist(message), sourceAddress);
		break;

	default:
		cout << "UNKNOWN OPCODE[" << opcode << "] from [" << sourceAddress
				<< "]: " << message->GetFirstByte() << endl;
		break;
	}
}

/* PEERLIST PACKET:    | OPCODE | HEADERSIZE | BODYSIZE | EXTPORT | CHUNKGUID | QTDPEERS | PEERLIST |  **************************************
 ** Sizes(bytes):      |   1    |     1      |     2    |    2    |  4  |  2  |    2     |    6     |  TOTAL: 14 Bytes + 6*QTDPEERS *********/
void Bootstrap::HandlePeerlistMessage(MessagePeerlist* message,
		string sourceAddress) {
	Peer* srcPeer = new Peer(sourceAddress);

	vector<int> peerlistHeader = message->GetHeaderValues();
	PeerlistTypes messageType = (PeerlistTypes) peerlistHeader[0];
	uint16_t peersReceived = peerlistHeader[1];

	if (messageType == PEERLIST_LOG) {
		MessagePeerlistLog* messageWrapper = new MessagePeerlistLog(message);
		peerlistHeader = messageWrapper->GetHeaderValues();

		uint32_t channelId = peerlistHeader[2];
		uint32_t msgTime = peerlistHeader[3];

		boost::mutex::scoped_lock channelListLock(channelListMutex);
		if (channelList.count(channelId) != 0) //Channel exists
				{
			if (channelList[channelId].HasPeer(srcPeer)) {
				vector<FILE*> overlayFile =
						channelList[channelId].GetOverlayFile(srcPeer);
				channelListLock.unlock();

				string overlayLog = sourceAddress + " ";                //PeerID
				overlayLog += boost::lexical_cast<string>(msgTime) + " "; //MsgTime
				//PEER LIST
				for (uint16_t i = 0; i < peersReceived; i++) {
					if (Peer* peer = message->GetPeer(i)) {
						overlayLog += peer->GetID() + " ";
					}
				}
				overlayLog += "\n";

				for (vector<FILE*>::iterator f = overlayFile.begin();
						f != overlayFile.end(); f++)  //if (overlayFile)
						{
					fwrite(overlayLog.c_str(), 1, overlayLog.size(), *f); //overlayFile);
					fflush(*f);                                  //overlayFile);
				}

			} else {
				channelListLock.unlock();
				cout
						<< "UDPServer: Message received from a peer that is not on the list ("
						<< sourceAddress << ")" << endl << endl;
			}
		} else {
			channelListLock.unlock();
		}
	}
}

/* PING PACKET:        | OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM | LISTOUT | PINGCODE | PEERMODE | CHUNKGUID |    BITMAP    | **************************************
 ** Sizes(bytes):      |   1    |     1      |     2    |     2    |    2    |    1     |     1    |  4  |  2  | BUFFERSIZE/8 | TOTAL: 8 || 16 + (BUFFERSIZE/8) Bytes */
void Bootstrap::HandlePingMessage(MessagePingBoot* message,
		string sourceAddress, uint32_t socket) {
	Peer* srcPeer = new Peer(sourceAddress);

	//Trata a mensagem de ping
	vector<int> pingHeader = message->GetHeaderValues();
	uint8_t pingType = pingHeader[0];
	PeerModes peerMode = (PeerModes) pingHeader[1];
	ChunkUniqueID peerTipChunk = ChunkUniqueID(pingHeader[2],
			(uint16_t) pingHeader[3]);
	//int maxPeerOut = pingHeader[4];
	//int maxPeerOut_FREE = pingHeader[5];
	//int sizePeerListIn = pingHeader[6];
	//int sizePeerListOut = pingHeader[7];
	//int sizePeerListOut_FREE = pingHeader[8];
	//int serverStreamRate = pingHeader[9];
	//uint32_t channelId = pingHeader[10];

	int serverStreamRate = pingHeader[6];
	uint32_t channelId = pingHeader[7];

	unsigned short totalPeer = 0;


	boost::mutex::scoped_lock channelListLock(channelListMutex);
	if (channelList.count(channelId) != 0) //Channel exists
			{

		totalPeer = channelList[channelId].GetPeerListSize()-1; //subtrai um para excluir o servidor
		if (channelList[channelId].HasPeer(srcPeer)) {
			channelList[channelId].GetPeerData(srcPeer).SetMode(peerMode);
			channelList[channelId].GetPeerData(srcPeer).SetTTLChannel(
					TTLChannel);

			//If ping from server
			if (*(srcPeer) == *(channelList[channelId].GetServer())) {
				channelList[channelId].SetServerNewestChunkID(peerTipChunk);
				channelList[channelId].SetServerEstimatedStreamRate(
						serverStreamRate);
			}

			vector<FILE*> performanceFile =
					channelList[channelId].GetPerformanceFile(srcPeer);
			channelListLock.unlock();

			//If it has performance measures
			if (pingType == PING_BOOT_PERF) {
				uint8_t indexPerfStart = 8; // original era 6


				MessagePingBootPerf* messageWrapper = new MessagePingBootPerf(
						message);
				pingHeader = messageWrapper->GetHeaderValues();

				time_t rawtime;
				time(&rawtime);
				string performanceLog = sourceAddress + " ";            //PeerID
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 0]) + " ";  //ChunkGenerated
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 1]) + " ";       //ChunkSent
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 2]) + " ";   //ChunkReceived
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 3]) + " ";   //ChunkOverload
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 4]) + " ";     //RequestSent
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 5]) + " ";     //RequestRecv
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 6]) + " ";  //RequestRetries
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 7]) + " ";     //ChunkMissed
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 8]) + " ";   //ChunkExpected
				performanceLog += boost::lexical_cast<string>(
						(float) *reinterpret_cast<float*>(&pingHeader[indexPerfStart
										+ 9])) + " ";    //MeanHop
				performanceLog += boost::lexical_cast<string>(
						(float) *reinterpret_cast<float*>(&pingHeader[indexPerfStart
										+ 10])) + " ";    //MeanTries
				performanceLog += boost::lexical_cast<string>(
								(float) *reinterpret_cast<float*>(&pingHeader[indexPerfStart
										+ 11])) + " ";    //MeanTriesPerRequest
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 12]) + " "; //SampleChunk.Cycle
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 13]) + " "; //SampleChunk.Position
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 14]) + " "; //SampleChunk.HopCount
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 15]) + " "; //SampleChunk.TriesCount
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 16]) + " "; //SampleChunk.Time
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 17]) + " "; //MsgTime

				performanceLog += boost::lexical_cast<string>(
						rawtime) + " ";                         //NowTime
/*
				performanceLog += boost::lexical_cast<string>(
						pingHeader[6]) + " ";                   //ECM  NeighborhoodSizeIn
				performanceLog += boost::lexical_cast<string>(
						pingHeader[7]) + " ";                   //ECM  NeighborhoodSizeOUT
				performanceLog += boost::lexical_cast<string>(
						pingHeader[8]) + " ";                   //ECM  NeighborhoodSizeOUT_FREE
*/
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 18]) + " ";                   //ECM  NeighborhoodSizeIn
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 19]) + " ";                   //ECM  NeighborhoodSizeOUT
				performanceLog += boost::lexical_cast<string>(
						pingHeader[indexPerfStart + 20]) + " ";                   //ECM  NeighborhoodSizeOUT_FREE


				performanceLog += boost::lexical_cast<string>(
						pingHeader[4]) + " ";                   //ECM maxPeerListOut
				performanceLog += boost::lexical_cast<string>(
						pingHeader[5]) + " ";                   //ECM maxPeerListOut_FREE
				performanceLog += boost::lexical_cast<string>(
						channelList[channelId].GetPeerData(srcPeer).GetChannelId_Sub())	+ " "; //ECM
				performanceLog += boost::lexical_cast<string>(
						totalPeer) + "\n";

				//if (performanceFile)
				for (vector<FILE*>::iterator f = performanceFile.begin();
						f != performanceFile.end(); f++) {
					fwrite(performanceLog.c_str(), 1, performanceLog.size(),
							*f);
					fflush(*f);
				}

				int peerUpload = pingHeader[indexPerfStart + 1];
				int neighborhoodSize = pingHeader[indexPerfStart + 12];
				PeerData pd = channelList[channelId].GetPeerData(srcPeer);
				if (neighborhoodSize)
					pd.SetUploadScore(
							(pd.GetUploadScore() + peerUpload)
									/ neighborhoodSize);
			}
		} else {
			channelListLock.unlock();
			cout
					<< "UDPServer: Message received from a peer that is not on the list ("
					<< sourceAddress << ")" << endl << endl;
		}
	} else {
		channelListLock.unlock();
		//packetSize = EHS;
		//packet = new uint8_t[packetSize];
		//AssembleErrorHeader(packet, ERROR_CHANNEL_UNAVAILABLE);
		//message = "Channel "+idChannelStr+" unavailable";
	}
}

/** Verifica se algum peer da lista está inativo (TTLChannel <= 0) caso esteja, remove esse peer */
void Bootstrap::CheckPeerList() {
	boost::xtime xt;
	boost::mutex::scoped_lock channelListLock(channelListMutex);
	channelListLock.unlock();
	while (true) {
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += 1;

		cout << "#";
		channelListLock.lock();
		vector<unsigned int> deletedChannel;
		for (map<unsigned int, Channel>::iterator channel = channelList.begin();
				channel != channelList.end(); channel++) {
			//ECM - comentar esse opção para testes com flash crowd. Assim, fica mais fácil acompanhar a tela do bootstrap
			//channel->second.PrintPeerList();
			channel->second.CheckActivePeers(); //ECM (corrigir no subchannel)-> chamada que decrementa o tempo de vida do subCanal

			if (!channel->second.HasPeer(channel->second.GetServer()))
				deletedChannel.push_back(channel->first);
		}
		for (vector<unsigned int>::iterator it = deletedChannel.begin();
				it < deletedChannel.end(); it++) {
			channelList.erase(*it);
			cout << "Channel " << *it << " removed" << endl;
		}
		channelListLock.unlock();
		cout << "#" << endl;

		boost::thread::sleep(xt);
	}
}

/** Varre a lista mostrando todos os peers e suas informções */
void Bootstrap::ShowPeerList() {
	for (map<unsigned int, Channel>::iterator channel = channelList.begin();
			channel != channelList.end(); channel++)
		channel->second.PrintPeerList();
}

/** Conta o numero de peers na lista */
void Bootstrap::HTTPLog() {
	boost::xtime xt;
	FILE *pfile;
	time_t rawtime;
	struct tm * timeinfo;
	string head =
			"<html><body><center><h1>Bootstrap log</h1></center>Peers Conectados - Log time<P>";
	string tail = "</body></html>";
	string log;
	string all;
	list<string> last_logs;
	list<string>::iterator it;
	int html_update_time = 0;
	//vector<bool> vetor;
	while (true) {
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += 2;

		if (last_logs.size() >= 60)
			last_logs.pop_front();
		unsigned int totalPeers = 0;
		boost::mutex::scoped_lock channelListLock(channelListMutex);
		for (map<unsigned int, Channel>::iterator channel = channelList.begin();
				channel != channelList.end(); channel++)
			totalPeers = channel->second.GetPeerListSize();
		channelListLock.unlock();

		cout << "* Connected Peers: " << totalPeers << endl;
		if (html_update_time >= 30) {
			log += boost::lexical_cast<string>(totalPeers);
			log += " - ";
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			log += asctime(timeinfo);
			log += "<P>";
			last_logs.push_back(log);
			all = head;
			it = last_logs.begin();
			while (it != last_logs.end()) {
				all += it->c_str();
				it++;
			}
			all += tail;
			pfile = fopen("public_html/bootstraplog/index.html", "w");
			if (pfile) {
				fwrite(all.c_str(), 1, all.size(), pfile);
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
void Bootstrap::TCPStart(const char *myTCPPort) {
	/** Configura conexão tcp */
	tcpServer = new BootstrapTCPServer(boost::lexical_cast<short>(myTCPPort),
			this);
	tcpServer->Run();
}

void Bootstrap::UDPStart() {
	udp->Start();
	udp->Run();
}

void Bootstrap::UDPReceive() {
	while (true) {
		AddressedMessage* aMessage = udp->GetNextMessageToReceive();
		if (aMessage) {
			HandleUDPMessage(aMessage->GetMessage(), aMessage->GetAddress());
			delete aMessage;
		}
	}
}
