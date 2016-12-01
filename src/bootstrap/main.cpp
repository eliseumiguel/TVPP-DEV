#include <iostream>
#include <string.h>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>

#include "../common/XPConfig.hpp"

#include "bootstrap.hpp"

#define TCPPORT "5111"    // the port users will be connecting to
#define UDPPORT "4950"    // the port users will be connecting to

#define SERVER_UDPPORT "4951" // the default port that server uses

using namespace std;

/** 
 * Função principal
 *  chama as threads necessárias para execução do programa
 */
int main(int argc, char* argv[]) {
    string myTCPPort = TCPPORT;
    string myUDPPort = UDPPORT;
    string peerlistSelectorStrategy = "";

    unsigned int maxSubChannel = 10;
    unsigned int maxServerAuxCandidate = 200;   //ECM quando ocorre o flash crowd, são selecionados o total de pares necessários para os subcanais
    unsigned int maxPeerInSubChannel = 2000;
    unsigned int sizeCluster = 1;               //ECM auxiliary server total in each subChannel
    unsigned int peerListSharedSize = 20;

    uint8_t minimumBandwidth = 0;               //ECM minimum bandwidth to share peer to other to be neighbor
    uint8_t minimumBandwidth_FREE = 0;          // only if --separatedFreeOutList

    // config merge mode...
	MesclarModeServer mergeType          = (MesclarModeServer) 0x02;       //tipo de mesclagem
	uint8_t           qt_PeerMergeType   =  2;                             //quantidade de pares a serem desconectados durante a mesclagem
	uint8_t           timeDescPeerMerge  = 10;                             //intervalo de tempo para cada desconexão


    string arg1 = "";
    if( argv[1] != NULL)
    	arg1 = argv[1];

    if(arg1 == "--help")
    {
        cout << "\nUsage: ./bootstrap [OPTIONS]" <<endl;
        cout <<"\nMain operation mode:"<<endl;
        cout <<"\n";
        cout <<"  -tcpPort                     define the tcp bootstrap port (default: "<<myTCPPort<<")"<<endl;
        cout <<"  -udpPort                     define the tcp bootstrap port (default: "<<myUDPPort<<")"<<endl;
        cout <<"  -peerlistSelectorStrategy    define the tcp bootstrap port (default: RandomStrategy)"<<endl;
        cout <<"  -peerListSharedSize          define the peer quantity to be shared each time between bootstrap and peer ()(default: "<<peerListSharedSize<<")"<<endl;
        cout <<"  -minimalOUTsend              define the minimum OUT to share a peer                     (defautl: "<<(int)minimumBandwidth<<")"<<endl;
        cout <<"  -minimalOUTFREEsend          define the minimum OUT_FRER to share a peer to Free Rider  (defautl: "<<(int)minimumBandwidth_FREE<<")"<<endl;
        cout <<endl;
        cout <<"      ****  FLASH CROWD ***"<<endl;

        cout <<"  -maxSubChannel               define the sub channel limit                  (default: "<<maxSubChannel<<")"<<endl;
        cout <<"  -maxServerAuxCandidate       define the server limit to create sub channel (default: "<<maxServerAuxCandidate<<")"<<endl;
        cout <<"  -maxPeerInSubChannel         define the peer number in new sub channel     (default: "<<maxPeerInSubChannel<<")"<<endl;
        cout <<"  -sizeCluster                 define the cluster size for each sub channel  (default: "<<sizeCluster<<")"<<endl;
        cout <<"  -mergeType                   define the server_aux merge mode              (default: "<<mergeType<<")"<<endl;
        cout <<"                                **(s-kill 0, s-avoid-p 1, s-permit-p 2, s-avoid-kill 3, s-permit-kill 4, s-noting 5)"<<endl;
        cout <<"  -qt_PeerMergeType            define the peer quantity to be disconnected   (default: "<<(int)qt_PeerMergeType<<")"<<endl;
        cout <<"  -timeDescPeerMerge           define the time interval to disconnect peers  (default: "<<(int)timeDescPeerMerge<<")"<<endl;
        cout <<"                                **(should be timeDescPeerMerge > 0)"<<endl;
        cout <<endl;
        cout <<"  --isolaVirtutalPeerSameIP    permit only different IP partner "<<endl;
        cout <<"  --separatedFreeOutList       share peer per listOut or ListOut_FREE "<<endl;
        cout <<"  --subChannelMixed            create sub channel mixed whit the master network "<<endl;
        cout <<"  --timeMergeSubchannel        merge each subchannel after each externalMessage "<<endl;

        exit(1);
    }


    XPConfig::Instance()->OpenConfigFile("");
    XPConfig::Instance()->SetBool("isolaVirtutalPeerSameIP", false);
    XPConfig::Instance()->SetBool("separatedFreeOutList",false);
    XPConfig::Instance()->SetBool("TIMEMERGE",false);

    int optind=1;

    // decode arguments
    while ((optind < argc) && (argv[optind][0]=='-')) {
        string swtc = argv[optind];
        if (swtc=="-tcpPort") {
            optind++;
            myTCPPort = argv[optind];
        }
        else if (swtc=="-udpPort") {
            optind++;
            myUDPPort = argv[optind];
        }
        else if (swtc=="-peerlistSelectorStrategy")
        {
            optind++;
            peerlistSelectorStrategy = argv[optind];
        }
        else if (swtc=="-peerListSharedSize")
        {
            optind++;
            peerListSharedSize = atoi(argv[optind]);
        }

        //flash crowd
        else if (swtc=="-maxSubChannel") {
            optind++;
            maxSubChannel = (unsigned int) atoi(argv[optind]);
        }
        else if (swtc=="-maxServerAuxCandidate") {
            optind++;
            maxServerAuxCandidate = (unsigned int) atoi(argv[optind]);
        }
        else if (swtc=="-maxPeerInSubChannel") {
            optind++;
            maxPeerInSubChannel = (unsigned int) atoi(argv[optind]);
        }
        else if (swtc=="-sizeCluster") {
            optind++;
            sizeCluster = (unsigned int) atoi(argv[optind]);
        }
        else if (swtc=="--isolaVirtutalPeerSameIP")
        {
            XPConfig::Instance()->SetBool("isolaVirtutalPeerSameIP", true);
        }
        else if (swtc=="--separatedFreeOutList")
        {
            XPConfig::Instance()->SetBool("separatedFreeOutList", true);
        }
        else if (swtc=="--subChannelMixed")
        {
            XPConfig::Instance()->SetBool("subChannelMixed", true);
        }
        else if (swtc=="--timeMergeSubchannel")
        {
            XPConfig::Instance()->SetBool("TIMEMERGE", true);
        }
        else if (swtc=="-minimalOUTsend") {
            optind++;
            minimumBandwidth = atoi(argv[optind]);
         }
        else if (swtc=="-minimalOUTFREEsend") {
            optind++;
            minimumBandwidth_FREE = atoi(argv[optind]);
            XPConfig::Instance()->SetBool("separatedFreeOutList",true);
         }
        else if (swtc=="-mergeType") {
            optind++;
            mergeType = (MesclarModeServer) atoi(argv[optind]);
            if (mergeType < 0 || mergeType > 5){
            	cout<<"-mergeType = "<<mergeType<<endl;
            	cout<<"Make sure that -mergeType in [0 .. 5]"<<endl;
            	exit (1);
            }
        }
        else if (swtc=="-qt_PeerMergeType") {
            optind++;
            int value = atoi(argv[optind]);
            if (value < 0) {
            	cout<<"-qt_PeerMergeType = "<<value<<endl;
            	cout<<"Make sure that -qt_PeerMergeType in [0 .. ]"<<endl;
            	exit (1);
            }
            qt_PeerMergeType = (unsigned int) value;
        }
        else if (swtc=="-timeDescPeerMerge") {
            optind++;
            int value = atoi(argv[optind]);
            if (value < 1) {
            	cout<<"-timeDescPeerMerge = "<<value<<endl;
            	cout<<"Make sure that -timeDescPeerMerge in [1 .. ]"<<endl;
            	exit (1);
            }
            timeDescPeerMerge = (unsigned int) value;
        }
        else {
            cout << "Invalid Arguments"<<endl; 
            exit(1);
        }
        optind++;
    }



    Bootstrap bootstrapInstance(myUDPPort, peerlistSelectorStrategy, peerListSharedSize, maxSubChannel, maxServerAuxCandidate,
    		                    maxPeerInSubChannel, sizeCluster, mergeType , qt_PeerMergeType, timeDescPeerMerge, minimumBandwidth,minimumBandwidth_FREE);
    
    boost::thread TTCPSERVER(boost::bind(&Bootstrap::TCPStart, &bootstrapInstance, myTCPPort.c_str()));
    boost::thread TUDPSERVER(boost::bind(&Bootstrap::UDPStart, &bootstrapInstance));
    boost::thread TUDPCONSUME(boost::bind(&Bootstrap::UDPReceive, &bootstrapInstance));
    boost::thread TVERIFICA(boost::bind(&Bootstrap::CheckPeerList, &bootstrapInstance));
//    passei a fazer a contagem dos pares no log perf...
//    boost::thread TCONTAPEERS(boost::bind(&Bootstrap::HTTPLog, &bootstrapInstance));

    TTCPSERVER.join();
    //TUDPSERVER.join();
    //TUDPRECV.join();
    //TUDPCONSUME.join();
    //TVERIFICA.join();
    //TCONTAPEERS.join();
    return 0;

}
