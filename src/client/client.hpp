#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <iostream>

#include <map>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <list>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "../common/Defines.hpp"
#include "../common/XPConfig.hpp"
#include "../common/PeerData.hpp"
#include "../common/udp.hpp"
#include "Request.hpp"
#include "RequestAttempt.hpp"
#include "MediaBuffer.hpp"
#include "PeerManager.hpp"
#include "PeerManagerServerAux.hpp"
#include "Temporizable/LeakyBucket.hpp"
#include "Temporizable/Disconnector.hpp"
#include "Temporizable/Connector.hpp"
#include "Temporizable/PeerRequester.hpp"
#include "client-tcp-server.hpp"
#include "client-bootstrap-session.hpp"

#include "../common/Messages/Message.hpp"
#include "../common/Messages/MessageChannel.hpp"
#include "../common/Messages/MessageServerSub.hpp"
#include "../common/Messages/MessagePeerlist.hpp"
#include "../common/Messages/MessagePeerlistShare.hpp"
#include "../common/Messages/MessagePeerlistLog.hpp"
#include "../common/Messages/MessageError.hpp"
#include "../common/Messages/MessagePingBoot.hpp"
#include "../common/Messages/MessagePingBootPerf.hpp"

#include "../common/Strategy/Strategy.hpp"
#include "../common/Strategy/MinimumFaultStrategy.hpp"
#include "../common/Strategy/NullStrategy.hpp"
#include "../common/Strategy/RandomStrategy.hpp"
#include "../common/Strategy/RandomStrategyWhitoutPoorBand.hpp"

#include "../common/Messages/MessageData.hpp"
#include "../common/Messages/MessageRequest.hpp"

#include "../common/Scheduler/IMessageScheduler.hpp"
#include "../common/Scheduler/FIFOMessageScheduler.hpp"
#include "../common/Scheduler/RRMessageScheduler.hpp"
#include "../common/Scheduler/CDFMessageScheduler.hpp"
#include "../common/Scheduler/RandomMessageScheduler.hpp"


using namespace std;

class ClientBootstrapSession;
class ClientTCPServer;
class PeerRequester;
class PeerManagerServerAux;

class Client
{
    public:
        Client();
        ~Client();
        void ClientInit(char *host_ip, string TCP_server_port, string udp_port, uint32_t idChannel, 
             string peers_udp_port,  string streamingPort, PeerModes mode, uint32_t buffer, 
            int maxpeersIn, int maxpeersOut, int janela, int num, int ttlIn, int ttlOut, int maxRequestAttempt, int tipOffsetTime, int limitDownload, int limitUpload,
            string disconnectorStrategyIn, string disconnectorStrategyOut, int quantityDisconnect,
			string connectorStrategy, unsigned int minimalBandwidthToBeMyIN, int timeToRemovePeerOutWorseBand,
			string chunkSchedulerStrategy, string messageSendScheduler, string messageReceptionScheduler,
			int maxPartnersOutFREE, unsigned int outLimitToSeparateFree);
        virtual void Ping();
        void CyclicTimers();
        void PeerCtoPeerA();
        bool ColocarNaListaDePedidos();
        bool isServerCandidate(); //ECM
        Request* CriaRequest();
        void GerarDados();
        void MontarListaPedidos();
        bool ConnectToBootstrap();
        void ConsomeMedia();
        int FeedPlayer(int32_t id);
        void UDPStart();
        void UDPReceive();
        void UDPSend();

        void Exit();

    private:
        //Internal Functions
        Message *HandleTCPMessage(Message* byteBuffer, string sourceAddress, uint32_t socket);
        void HandleUDPMessage(Message* message, string sourceAddress = "");
        
        boost::dynamic_bitset<> BytesToBitset(uint8_t* byteVector, int size);
        uint8_t* BitsetToBytes(boost::dynamic_bitset<>);
        boost::dynamic_bitset<> ByteToBitset(uint8_t);            
        
        void FazPedidos(int stepInMs);
        void EnviaDados(string s, string msg);
        
        void HandleServerAuxListMessage(MessagePeerlist* message, string sourceAddress = "", uint32_t socket = 0);
        void HandlePeerlistMessage(MessagePeerlist* message, string sourceAddress = "", uint32_t socket = 0);
        void HandlePingMessage(MessagePing* message, string sourceAddress = "", uint32_t socket = 0);
        void HandlePingMessageIn(vector<int>* pingHeader, MessagePing* message, string sourceAddress = "", uint32_t socket = 0);
        void HandlePingMessageOut(vector<int>* pingHeader, MessagePing* message, string sourceAddress = "", uint32_t socket = 0);
        //ECM
        void HandleErrorMessage(MessageError* message, string sourceAddress = "", uint32_t socket = 0);
        void HandleMessageServerSub(MessageServerSub* message, string sourceAddress = "", uint32_t socket = 0);
        void HandleRequestMessage(MessageRequest* message, string sourceAddress = "", uint32_t socket = 0);
        void HandleDataMessage(MessageData* message, string sourceAddress = "", uint32_t socket = 0);

        uint32_t GetAutentication();
        void SetAutentication(uint32_t bootID);


        void CreateLogFiles();

        //PARAMETROS-------------
        char* Bootstrap_IP;
        string TCP_server_PORT;
        string UDP_server_PORT;
        string peers_UDP_PORT;
        string streamingPort;
        PeerModes peerMode;
        uint32_t BUFFER_SIZE;   //Tamanho do buffer de dados, e chunk_map
        unsigned int JANELA;    //Janela de interesse
        int NUM_PEDIDOS;        //Número de chunks a ser colocado na lista de pedidos a cada chamada
        int TTL_MAX_In;          //tempo até o cliente ser removido da lista de peer ativos na lista In
        int TTL_MAX_Out;         //tempo até o cliente ser removido da lista de peer ativos na lista Out
        int maxRequestAttempt;  //Maximum number of attempts to perform a request
        int tipOffsetTime;      //Time from where to start requesting once reseting to tip
        uint32_t idChannel;
		list<Temporizable*> temporizableList;
		bool configurarBootID;   //controla a autenticação do bootstrapID no primeiro contato com o bootstrap
		uint32_t bootStrapID_Autentic;


		//ECM
		Disconnector* disconnectorIn;
		Disconnector* disconnectorOut;
		Connector* connectorIn;
		int timeToRemovePeerOutWorseBand;            // ECM only if XPConfig::Instance()->GetBool("removeWorsePartner")== true;
		                                             // used for connect a new partner whit good band than other in peerActiveList.

		PeerRequester* requester;
        Strategy* chunkSchedulerStrategy;
        //-----------------------         
        boost::asio::io_service cbSession_io_service;
        ClientBootstrapSession* cbSession;
        ClientTCPServer* playerTCPServer;
        
        UDPServer* udp;
        UDPServer* sourceVideoReceiver;
        //Media Buffer Data structure
        MediaBuffer* mediaBuffer;

        //Peer Manager (alterado para classe derivada)
		PeerManagerServerAux peerManager;

		//Lists
        list<Request*> requestList;
        map<ChunkUniqueID,Request*> oldRequestList;
        //LeakyBucket related
        LeakyBucket* leakyBucketDownload;
        LeakyBucket* leakyBucketUpload;
        //Socket Descriptors
        int tcp_sock, nweb_listenfd, nweb_socketfd;          
        //Yes or No variables (1: YES 0: NO)
        bool receiveRequestPosition;
        bool perform_udp_punch;
		bool serverActive;   //ECM usado apenas para o servidor principal
        bool quit;
        //External Address
        string externalIp;
        uint16_t externalPort;
        uint32_t channelCreationTime;
        int bootstrapTimeShift;
        //Mutexes
        boost::mutex bufferMapMutex;
        boost::mutex requestListMutex;
        boost::mutex oldRequestListMutex;
        //Player Related
        uint64_t playerBufferDuration;
        //ChunkIds
        ChunkUniqueID requestPosition;
        ChunkUniqueID latestReceivedPosition;
        ChunkUniqueID playerDeliveryPosition;
        //Files
        FILE *chunkRcvFile;
        FILE *chunkMissFile;
        //Instrumentation Variables
        ChunkUniqueID* lastMediaID;
        int lastMediaHop;
        time_t lastMediaTime;
        int chunksGen, chunksSent, chunksRecv, chunksOverload;
        int requestsSent, requestsRecv, requestRetries;
        int estimatedChunkRate, devEstimatedChunkRate;
        int chunksMissed, chunksPlayed;
        unsigned int downloadPerSecDone, uploadPerSecDone;
        list<int> triesPerRequest;

        friend class ClientTCPServer;
};
#endif // TCPCONEX_H_INCLUDED
