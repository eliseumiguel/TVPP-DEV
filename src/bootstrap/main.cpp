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

    unsigned int maxSubChannel = 6;
    unsigned int maxServerAuxCandidate = 200; //ECM quando ocorre o flash crowd, são selecionados o total de pares necessários para os subcanais
    unsigned int maxPeerInSubChannel = 60;
    unsigned int sizeCluster = 1; //ECM auxiliary server total in each subChannel

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
        cout <<endl;
        cout <<"                           FLASH CROWD"<<endl;

        cout <<"  -maxSubChannel               define the sub channel limit                  (default: "<<maxSubChannel<<")"<<endl;
        cout <<"  -maxServerAuxCandidate       define the server limit to create sub channel (default: "<<maxServerAuxCandidate<<")"<<endl;
        cout <<"  -maxPeerInSubChannel         define the peer number in new sub channel     (default: "<<maxPeerInSubChannel<<")"<<endl;
        cout <<"  -sizeCluster                 define the cluster size for each sub channel  (default: "<<sizeCluster<<")"<<endl;
        cout <<"  --isolaVirtutalPeerSameIP    permit only different IP partner "<<endl;

        exit(1);
    }


    XPConfig::Instance()->OpenConfigFile("");
    XPConfig::Instance()->SetBool("isolaVirtutalPeerSameIP", false);

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

        else {
            cout << "Invalid Arguments"<<endl; 
            exit(1);
        }
        optind++;
    }



    Bootstrap bootstrapInstance(myUDPPort, peerlistSelectorStrategy, maxSubChannel, maxServerAuxCandidate, maxPeerInSubChannel, sizeCluster);
    
    boost::thread TTCPSERVER(boost::bind(&Bootstrap::TCPStart, &bootstrapInstance, myTCPPort.c_str()));
    boost::thread TUDPSERVER(boost::bind(&Bootstrap::UDPStart, &bootstrapInstance));
    boost::thread TUDPCONSUME(boost::bind(&Bootstrap::UDPReceive, &bootstrapInstance));
    boost::thread TVERIFICA(boost::bind(&Bootstrap::CheckPeerList, &bootstrapInstance));
    boost::thread TCONTAPEERS(boost::bind(&Bootstrap::HTTPLog, &bootstrapInstance));

    TTCPSERVER.join();
    //TUDPSERVER.join();
    //TUDPRECV.join();
    //TUDPCONSUME.join();
    //TVERIFICA.join();
    //TCONTAPEERS.join();
    return 0;

}
