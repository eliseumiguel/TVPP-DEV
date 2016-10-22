#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>
#include <map>
#include <errno.h>
#include <signal.h>
#include <string>
#include <arpa/inet.h>
#include "client.hpp"
#include "../common/Defines.hpp"
#include "../common/XPConfig.hpp"

#define BOOTSTRAP_UDP_PORT "4950"
#define BOOTSTRAP_TCP_PORT "5111"
#define PEERS_UDP_PORT "4951"
#define STANDARD_EXHIBITION_PORT "8181"
#define MAX_PEER_ATIVO 60
#define TTL_MAX  4
#define JANELA 120         // Janela de interesse
#define BUFFER_SIZE 1600
#define NUM_PEDIDOS 64   //Número de pacotes pedidos por segundo no caso do cliente. 
                         //Número de pacotes lidos por do arquivo por segundo, no caso do servidor             

using namespace std;

Client clientInstance;

bool validateIpAddress(const string ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

void terminate (int param)
{
    clientInstance.Exit();
    exit(1);    
}

/** 
 *Função principal do cliente
 *chama as funções e abre as threads necessárias pra funcionamento do programa
 */
int main (int argc, char* argv[])
{

    char *ip = argv[1];
    string tcpPort = BOOTSTRAP_TCP_PORT;
    string udpPort = BOOTSTRAP_UDP_PORT;
    unsigned int idChannel = 0;
    string peerPort = PEERS_UDP_PORT;
    string streamingPort = STANDARD_EXHIBITION_PORT;
    PeerModes mode = MODE_CLIENT;
    int bufferSize = BUFFER_SIZE;
    int maxPartnersIn = MAX_PEER_ATIVO;
    int maxPartnersOut = MAX_PEER_ATIVO;
    int maxPartnersOutFREE = 0;
    int windowOfInterest = JANELA;
    int requestLimit = NUM_PEDIDOS;
    int ttlIn = TTL_MAX;
    int ttlOut = TTL_MAX;
    int maxRequestAttempt = 3;
    int tipOffsetTime = 3;
    int limitDownload = -1;
    int limitUpload = -1;

    																// ** Used for disconnector
	string disconnectorStrategyIn = "None";                         //ECM separate In and Out to be possible disconnect only Out or In or both
	string disconnectorStrategyOut = "None";                        //ECM ensures that make peerListOut new connections
	unsigned int quantityDisconnect = 0;                            //ECM quantity of peer to be disconnected

    																// ** Used for connector
	string connectorStrategy = "Random";                            //ECM for In and Out. But, in client, the connectorOut is unable
	unsigned int timeToRemovePeerOutWorseBand = 0;                  //Time interval to remove worse peer band if a good new peer ask for connection
	unsigned int minimalBandwidthToBeMyIN     = 0;                  // minimal new partner size OUT to try connecting IN
	unsigned int outLimitToSeparateFree       = 4;                  // insert peer in PeerListOutFREE if peer Out <= outLimitToSeparateFree and maxPartnersOutFREE > 0

    string chunkSchedulerStrategy = "Random";
    string messageSendScheduler = "FIFO";
    string messageReceiveScheduler = "FIFO";
    int optind=2;
    string arg1 = argv[1];
    
    if (!validateIpAddress(arg1))
    {
        if(arg1 == "--help")
        {    
            cout << "\nUsage: ./client [BOOTSTRAP IP] [OPTIONS]" <<endl;
            cout <<"\nMain operation mode:"<<endl;
            cout <<"\n";
            cout <<"  -bufferSize                   define the buffermap message size (default: "<<bufferSize<<")"<<endl;
            cout <<"  -channelId                    select a channel to transmit/receive (default: "<<idChannel<<")"<<endl;
			cout <<"  -disconnectorStrategyIn       select a strategy for peer disconnection (default: "<<disconnectorStrategyIn<<")"<<endl;
			cout <<"  -disconnectorStrategyOut      select a strategy for peer disconnection (default: "<<disconnectorStrategyOut<<")"<<endl;
            cout <<"                                 **(Options: None, Random, or RandomOnlyNoServerActive)"<<endl;
            cout <<"  -quantityDisconnect           number of peer to be disconnected by disconnectorStrategy (default: "<<quantityDisconnect<<")"<<endl;
            cout <<"                                                                                                                             "<<endl;
			cout <<"  -connectorStrategy            select a strategy for peer connection (default: "<<connectorStrategy<<")"<<endl;
            cout <<"                                 **(Options: Random, RandomWhitoutPoor)"<<endl;
            cout <<"  -minimalOUTtoBeMyIN           minimal partner's peersizeOut to ask for new partner IN (default: "<<minimalBandwidthToBeMyIN<<")"<<endl;
            cout <<"                                 **(if chosen, it sets automatically connectorStrategy = RandomWhitoutPoor)"<<endl;
            cout <<"  -minimalOUTFREEtoBeMyIN       minimal partner's peersizeOut_FREE to ask for new partner IN (default: "<<minimalBandwidthToBeMyIN<<")"<<endl;
            cout <<"                                 **(if chosen, it sets automatically connectorStrategy = RandomWhitoutPoorFREE)"<<endl;
            cout <<"                                   (find the partner only the maxPartnersOutFREE is setted on it)"<<endl;
            cout <<"  -timeToRemovePeerOutWorseBand time to remove someone and connect a better new peer asking to be partner (default: disabled)"<<endl;
            cout <<"                                 **(for to able, choose a positive number)"<<endl;
            cout <<"                                ---"<<endl;
            cout <<"  -chunkSchedulerStrategy       select a strategy for chunk scheduling (default: "<<chunkSchedulerStrategy<<")"<<endl;
            cout <<"                                 **(Options: MinimumFaultStrategy, NullStrategy, RandomStrategy)"<<endl;
            cout <<"  -messageSendScheduler         select a strategy for message reception (default: "<<messageSendScheduler<<")"<<endl;
            cout <<"                                 **(Options: FIFO, RR - RoundRobin, Random, CDF - Closest Deadline First)"<<endl;
            cout <<"  -messageReceiveScheduler      select a strategy for message reception (default: "<<messageReceiveScheduler<<")"<<endl;
            cout <<"                                 **(Options: FIFO, RR - RoundRobin, Random, CDF - Closest Deadline First)"<<endl;
            cout <<"  -limitDownload                limits the download bandwidht usage in b/s (default: "<<limitDownload<<")"<<endl;
            cout <<"  -limitUpload                  limits the upload bandwidht usage in b/s (default: "<<limitUpload<<")"<<endl;
            cout <<"  -maxPartnersIn                maximum number of neighbors-In(default: "<<maxPartnersIn<<")"<<endl;
            cout <<"  -maxPartnersOut               maximum number of neighbors-Out(default: "<<maxPartnersOut<<")"<<endl;
            cout <<"  -maxPartnersOutFREE           maximum number of neighbors-Out in special list Free (default: "<<maxPartnersOutFREE<<")"<<endl;
            cout <<"                                 *(if not setted worse peer is inserted in a conventional peerListOut) "<<endl;
            cout <<"  -outLimitToSeparateFree       maximum peer out to be insert in peerListOutFree (default: "<<outLimitToSeparateFree<<")"<<endl;
            cout <<"  -mode                         define the type of client. (default: "<<mode<<")"<<endl;
            cout <<"                                 **(Options: client (0); server (1); free-rider-good (2))"<<endl;
            cout <<"  -peerPort                     port for inter-peer communication (default: "<<peerPort<<")"<<endl;
            cout <<"  -maxRequestAttempt            maximum number of attempts to perform a request(default: "<<maxRequestAttempt<<")"<<endl;;
            cout <<"  -tipOffsetTime                amount of seconds from where to start requesting chunks prior to stream tip (default: "<<tipOffsetTime<<" )"<<endl;;
            cout <<"  -requestLimit                 define the amount of chunks that can be simultaneously asked (default: "<<requestLimit<<" )"<<endl;
            cout <<"  -streamingPort                port used by media stream (mode-dependent) (default: "<<streamingPort<<")"<<endl;
            cout <<"  -tcpPort                      bootstrap tcp port (default: "<<tcpPort<<")"<<endl;
            cout <<"  -ttlIn                        partnership time to live list In (default: "<<ttlIn<<")"<<endl;
            cout <<"  -ttlOut                       partnership time to live list Out (default: "<<ttlOut<<")"<<endl;
            cout <<"  -udpPort                      bootstrap udp port (default: "<<udpPort<<")"<<endl;
            cout <<"                  ***           "<<endl;
            cout <<"\n"<<endl;
            cout <<"  --playerDisabled              disables stream dispatch to player"<<endl;
            cout <<"  --blockFreeriders             blocks requests to freeriders"<<endl;
            cout <<"  --clientLogsDisabled          disables client logging service"<<endl;
            cout <<"  --leakyBucketDataFilter       forces data packets only to pass through upload leaky bucket"<<endl;
            cout <<"  --serverCandidate             permits that peer becomes a auxiliary server on parallel network"<<endl;
            exit(1);
        }
        else
        {
            cout << "Invalid Arguments. Try --help"<<endl;
            exit(1);
        }
    }

    XPConfig::Instance()->OpenConfigFile("");
    XPConfig::Instance()->SetBool("removeWorsePartner",false);
    XPConfig::Instance()->SetBool("separatedFreeOutList",false);
    
    // decode arguments
    while ((optind < argc) && (argv[optind][0]=='-'))
    {
        string swtc = argv[optind];

        if (swtc=="-tcpPort")
        {
            optind++;
            tcpPort = argv[optind];
        }
        else if (swtc=="-udpPort")
        {
            optind++;
            udpPort = argv[optind];
        }
        else if (swtc=="-channelId")
        {
            optind++;
            idChannel = atoi(argv[optind]);
        }
        else if (swtc=="-peerPort")
        {
            optind++;
            peerPort = argv[optind];
        }
        else if (swtc=="-streamingPort")
        {
            optind++;
            streamingPort = argv[optind];
        }
        else if (swtc=="-bufferSize")
        {
            optind++;
            bufferSize = atoi(argv[optind]);
        }
        else if (swtc=="-mode")
        {
            optind++;
            mode = (PeerModes)atoi(argv[optind]);
        }
        else if (swtc=="-maxPartnersIn")
        {
            optind++;
            maxPartnersIn = atoi(argv[optind]);
        }
        else if (swtc=="-maxPartnersOut")
        {
            optind++;
            maxPartnersOut = atoi(argv[optind]);
        }
        else if (swtc=="-maxPartnersOutFREE")
        {
            optind++;
            maxPartnersOutFREE = atoi(argv[optind]);
            if (maxPartnersOutFREE >= 0)
               XPConfig::Instance()->SetBool("separatedFreeOutList", true);

        }
        else if (swtc=="-outLimitToSeparateFree")
        {
            optind++;
            outLimitToSeparateFree = atoi(argv[optind]);
        }
        else if (swtc=="-windowOfInterest")
        {
            optind++;
            windowOfInterest = atoi(argv[optind]);
        }
        else if (swtc=="-requestLimit")
        {
            optind++;
            requestLimit = atoi(argv[optind]);
        }
        else if (swtc=="-ttlIn")
        {
            optind++;
            ttlIn = atoi(argv[optind]);
        }
        else if (swtc=="-ttlOut")
        {
            optind++;
            ttlOut = atoi(argv[optind]);
        }
        else if (swtc=="-maxRequestAttempt")
        {
            optind++;
            maxRequestAttempt = atoi(argv[optind]);
        }
        else if (swtc=="-tipOffsetTime")
        {
            optind++;
            tipOffsetTime = atoi(argv[optind]);
        }
        else if (swtc=="-limitDownload")
        {
            optind++;
            limitDownload = atoi(argv[optind]);
        }
        else if (swtc=="-limitUpload")
        {
            optind++;
           	limitUpload = atoi(argv[optind]);
         }
		else if (swtc=="-disconnectorStrategyIn")
        {
            optind++;
            disconnectorStrategyIn = argv[optind];
        }
		else if (swtc=="-disconnectorStrategyOut")
        {
            optind++;
            disconnectorStrategyOut = argv[optind];
        }
		else if (swtc=="-quantityDisconnect")
        {
            optind++;
            quantityDisconnect = atoi(argv[optind]);
        }
		else if (swtc=="-connectorStrategy")
        {
            optind++;
            connectorStrategy = argv[optind];
        }
		else if (swtc=="-timeToRemovePeerOutWorseBand")
        {
            optind++;
            timeToRemovePeerOutWorseBand = atoi(argv[optind]);
            XPConfig::Instance()->SetBool("removeWorsePartner", true);
        }
		else if (swtc=="-minimalOUTtoBeMyIN")
        {
            optind++;
            minimalBandwidthToBeMyIN = atoi(argv[optind]);
            connectorStrategy = "RandomWhitoutPoor";
        }
		else if (swtc=="-minimalOUTFREEtoBeMyIN")
        {
            optind++;
            minimalBandwidthToBeMyIN = atoi(argv[optind]);
            connectorStrategy = "RandomWhitoutPoorFREE";
        }

        else if (swtc=="-chunkSchedulerStrategy")
        {
            optind++;
            chunkSchedulerStrategy = argv[optind];
        }
        else if (swtc=="-messageSendScheduler")
        {
            optind++;
            messageSendScheduler = argv[optind];
        }
        else if (swtc=="-messageReceiveScheduler")
        {
            optind++;
            messageReceiveScheduler = argv[optind];
        }
        else if (swtc=="--playerDisabled")
        {
            XPConfig::Instance()->SetBool("playerEnabled", false);
        }
        else if (swtc=="--blockFreeriders")
        {
            XPConfig::Instance()->SetBool("blockFreeriders", true);
        }
        else if (swtc=="--clientLogsEnabled")
        {
            XPConfig::Instance()->SetBool("clientLogsEnabled", true);
        }
        else if (swtc=="--leakyBucketDataFilter")
        {
            XPConfig::Instance()->SetBool("leakyBucketDataFilter", true);
        }
        else if (swtc=="--serverCandidate")
        {
            XPConfig::Instance()->SetBool("serverCandidate", true);
        }
        else
        {
            cout << "Invalid Arguments. Try --help"<<endl;
            exit(1);
        }
        optind++;
    }

    clientInstance.ClientInit(ip, tcpPort, udpPort, idChannel, 
                                peerPort, streamingPort, mode, bufferSize, 
                                maxPartnersIn, maxPartnersOut, windowOfInterest, requestLimit, ttlIn, ttlOut, maxRequestAttempt, tipOffsetTime, limitDownload, limitUpload,
                                disconnectorStrategyIn, disconnectorStrategyOut, quantityDisconnect, connectorStrategy, minimalBandwidthToBeMyIN, timeToRemovePeerOutWorseBand,
								chunkSchedulerStrategy, messageSendScheduler, messageReceiveScheduler, maxPartnersOutFREE, outLimitToSeparateFree);
    
    boost::thread TPING(boost::bind(&Client::Ping, &clientInstance));
    boost::thread TUDPSTART(boost::bind(&Client::UDPStart, &clientInstance));
    boost::thread TUDPRECEIVE(boost::bind(&Client::UDPReceive, &clientInstance));
    boost::thread TUDPSEND(boost::bind(&Client::UDPSend, &clientInstance));
    boost::thread TTIMER(boost::bind(&Client::CyclicTimers, &clientInstance));
    if (mode == 1) //MODE_SERVER
    {
        boost::thread TGERAR(boost::bind(&Client::GerarDados, &clientInstance));
    }    
    else //MODE_CLIENT, MODE_FREERIDER_GOOD, MODE_SUPERNODE
    {
        boost::thread TCONSOME(boost::bind(&Client::ConsomeMedia,&clientInstance));
        boost::thread TPEDIR(boost::bind(&Client::MontarListaPedidos,&clientInstance));
    }
    TPING.join();
    
    cout<<endl<<"ENDMAIN"<<endl;
    return 0;
}

