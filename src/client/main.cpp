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
    /*void (*prev_fn)(int);//TODO CONCERTAR O SIGNAL TREATMENT

    prev_fn = signal (SIGTERM,terminate);
    if (prev_fn==SIG_IGN) signal (SIGTERM,SIG_IGN);
    prev_fn = signal (SIGINT,terminate);
    if (prev_fn==SIG_IGN) signal (SIGINT,SIG_IGN);*/

    char *ip = argv[1];
    string tcpPort = BOOTSTRAP_TCP_PORT;
    string udpPort = BOOTSTRAP_UDP_PORT;
    unsigned int idChannel = 0;
    string peerPort = PEERS_UDP_PORT;
    string streamingPort = STANDARD_EXHIBITION_PORT;
    PeerModes mode = MODE_CLIENT;
    int bufferSize = BUFFER_SIZE;
    int maxPartners = MAX_PEER_ATIVO;
    int windowOfInterest = JANELA;
    int requestLimit = NUM_PEDIDOS;
    int ttl = TTL_MAX;
    int maxRequestAttempt = 3;
    int tipOffsetTime = 3;
    int limitDownload = -1;
    int limitUpload = -1;
	string disconnectorStrategy = "None";
	string connectorStrategy = "Random";
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
            cout <<"  -bufferSize                   define the buffermap message size (default: "<<bufferSize<<" )"<<endl;
            cout <<"  -channelId                    select a channel to transmit/receive (default: "<<idChannel<<" )"<<endl;
			cout <<"  -disconnectorStrategy         select a strategy for peer disconnection (default: "<<disconnectorStrategy<<" )"<<endl;
            cout <<"                                (Options: None, Random)"<<endl;
			cout <<"  -connectorStrategy            select a strategy for peer connection (default: "<<connectorStrategy<<" )"<<endl;
            cout <<"                                (Options: Random)"<<endl;
            cout <<"  -chunkSchedulerStrategy       select a strategy for chunk scheduling (default: "<<chunkSchedulerStrategy<<" )"<<endl;
            cout <<"                                (Options: MinimumFaultStrategy, NullStrategy, RandomStrategy)"<<endl;
            cout <<"  -messageSendScheduler         select a strategy for message reception (default: "<<messageSendScheduler<<" )"<<endl;
            cout <<"                                (Options: FIFO, RR - RoundRobin, Random, CDF - Closest Deadline First)"<<endl;
            cout <<"  -messageReceiveScheduler      select a strategy for message reception (default: "<<messageReceiveScheduler<<" )"<<endl;
            cout <<"                                (Options: FIFO, RR - RoundRobin, Random, CDF - Closest Deadline First)"<<endl;
            cout <<"  -limitDownload                limits the download bandwidht usage in B/s (default: "<<limitDownload<<" )"<<endl;
            cout <<"  -limitUpload                  limits the upload bandwidht usage in B/s (default: "<<limitUpload<<" )"<<endl;
            cout <<"  -maxPartners                  maximum number of neighbors(default: "<<maxPartners<<" )"<<endl;
            cout <<"  -mode                         define if the peer is a client (0) or a server (1) (default: "<<mode<<" )"<<endl;
            cout <<"  -peerPort                     port for inter peer comunication (default: "<<peerPort<<" )"<<endl;
            cout <<"  -maxRequestAttempt            maximum number of attempts to perform a request(default: "<<maxRequestAttempt<<" )"<<endl;;
            cout <<"  -tipOffsetTime                amount of seconds from where to start requesting chunks prior to stream tip (default: "<<tipOffsetTime<<" )"<<endl;;
            //cout <<"  -requestLimit               define the amount of chunks that can be simultaneously asked (default: "<<requestLimit<<" )"<<endl;
            cout <<"  -streamingPort                port used by media stream (mode-dependent) (default: "<<streamingPort<<" )"<<endl;
            cout <<"  -tcpPort                      bootstrap tcp port (default: "<<tcpPort<<" )"<<endl;
            cout <<"  -ttl                          partnership time to live (default: "<<ttl<<" )"<<endl;
            cout <<"  -udpPort                      bootstrap udp port (default: "<<udpPort<<" )"<<endl;
            cout <<"\n"<<endl;
            cout <<"  --playerDisabled              disables stream dispatch to player"<<endl;
            cout <<"  --blockFreeriders             blocks requests to freeriders"<<endl;
            cout <<"  --clientLogsDisabled          disables client logging service"<<endl;
            cout <<"  --leakyBucketDataFilter       forces data packets only to pass through upload leaky bucket"<<endl;
            exit(1);
        }
        else
        {
            cout << "Invalid Arguments. Try --help"<<endl;
            exit(1);
        }
    }

    XPConfig::Instance()->OpenConfigFile("");
    
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
        else if (swtc=="-maxPartners")
        {
            optind++;
            maxPartners = atoi(argv[optind]);
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
        else if (swtc=="-ttl")
        {
            optind++;
            ttl = atoi(argv[optind]);
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
		else if (swtc=="-disconnectorStrategy")
        {
            optind++;
            disconnectorStrategy = argv[optind];
        }
		else if (swtc=="-connectorStrategy")
        {
            optind++;
            connectorStrategy = argv[optind];
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
        else
        {
            cout << "Invalid Arguments. Try --help"<<endl;
            exit(1);
        }
        optind++;
    }

    clientInstance.ClientInit(ip, tcpPort, udpPort, idChannel, 
                                peerPort, streamingPort, mode, bufferSize, 
                                maxPartners, windowOfInterest, requestLimit, ttl, maxRequestAttempt, tipOffsetTime, limitDownload, limitUpload, 
                                disconnectorStrategy, connectorStrategy, chunkSchedulerStrategy, 
                                messageSendScheduler, messageReceiveScheduler);
    
    boost::thread TPING(boost::bind(&Client::Ping, &clientInstance));
    boost::thread TUDPSTART(boost::bind(&Client::UDPStart, &clientInstance));
    boost::thread TUDPRECEIVE(boost::bind(&Client::UDPReceive, &clientInstance));
    boost::thread TUDPSEND(boost::bind(&Client::UDPSend, &clientInstance));
    boost::thread TTIMER(boost::bind(&Client::CyclicTimers, &clientInstance));
    if (mode == 1) //MODE_SERVER
    {
        boost::thread TGERAR(boost::bind(&Client::GerarDados, &clientInstance));
    }    
    else //MODE_CLIENT, MODE_FREERIDER, MODE_SUPERNODE
    {
        boost::thread TCONSOME(boost::bind(&Client::ConsomeMedia,&clientInstance));
        boost::thread TPEDIR(boost::bind(&Client::MontarListaPedidos,&clientInstance));
    }
    TPING.join();
    
    cout<<endl<<"ENDMAIN"<<endl;
    return 0;
}

