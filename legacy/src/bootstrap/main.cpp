#include <iostream>
#include <string.h>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>

#include "bootstrap.hpp"

#define TCPPORT "5111"   // the port users will be connecting to
#define UDPPORT "4950"    // the port users will be connecting to

//#include "udpserver.hpp"
//#include "tcpserver.hpp"

using namespace std;


//void inicializaDados(map<string, PeerData> &peerList );
//void mostraPeerList(map<string, PeerData> &peerList );
//void verificaLista(map<string, PeerData> &peerList);

/** 
 *Função principal
 *  chama as threads necessárias para execução do programa
 */
int main(int argc, char* argv[]) {

	char *tcp_port = TCPPORT;
    char *udp_port = UDPPORT;
	string s = "127.0.0.1";

	int optind=1;
    // decode arguments
    while ((optind < argc) && (argv[optind][0]=='-')) {
    	string swtc = argv[optind];
        if (swtc=="-t") {
            optind++;
            tcp_port = argv[optind];

        }
        else if (swtc=="-u") {
            optind++;
            udp_port = argv[optind];
        }
        else if (swtc=="-s") {
            optind++;
            s = argv[optind];
        }
        else {
            cout << "Invalid Arguments"<<endl; 
            exit(1);
        }
		optind++;
    }
	
    Bootstrap B(s);
    // B.InicializaDados();
    
    boost::thread TTCPSERVER( boost::bind( &Bootstrap::TCPServer,  &B, tcp_port));	
    boost::thread TUDPSERVER( boost::bind( &Bootstrap::UDPServer,  &B, udp_port));
    boost::thread TVERIFICA(boost::bind(&Bootstrap::VerificaLista, &B));    	
    //UDPServer UDP;
    //TCPServer TCP;	
    //map<string, PeerData> peerList;

    //inicializaDados(peerList);
    //boost::thread TTCPSERVER( boost::bind( &TCPServer::Mainloop, &TCP,boost::ref(peerList)));
    //boost::thread TUDPSERVER( boost::bind( &UDPServer::Mainloop, &UDP,boost::ref(peerList)));	
    //boost::thread TVERIFICA(boost::bind(&verificaLista,boost::ref(peerList)));

    TTCPSERVER.join();
    TUDPSERVER.join();
    TVERIFICA.join();
    return 0;

}

/** Verifica se algum peer da lista está inativo (TTL <= 0) caso esteja, removo esse peer 
void verificaLista(map<string, PeerData> &peerList) {
    boost::xtime xt;
    while (1) {

        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 10;
        for( map<string,PeerData> :: iterator i = peerList.begin(); i != peerList.end(); i++) {
            i->second.DecTTL();
            if ( i->second.GetTTL() <= 0) {
                cout<<"Peer "<<i->first<<" removed"<<endl;
                peerList.erase(i->first);
            }
        }
        mostraPeerList(peerList);
        boost::thread::sleep(xt);
    }

}*/
/** 
 * Inicializa peers na lista
 * usada apenas para testes
 *
void inicializaDados(map<string, PeerData> &peerList){

    PeerData D("192.168.1.103",20);
    PeerData E("192.168.1.102",50);
    PeerData F("189.13.124.86",30);
    peerList["192.168.1.103"] = D;
    peerList["192.168.1.102"] = E;
    peerList["189.13.124.86"] = F;


}*/
 /** Varre a lista mostrando todos os peers e suas informções *
void mostraPeerList(map<string, PeerData> &peerList) {
    int j = 0;
    for( map<string,PeerData> :: iterator i = peerList.begin(); i != peerList.end(); i++) {
            if( j == 0) cout<<endl<<"Peer List:"<<endl;
            cout<<"ID: "<<i->first<<" TTL: "<<i->second.GetTTL()<<endl;
            j++;
    }
    if(j != 0)cout<<endl;
}*/
/** Conta o numero de peers na lista *
void contaPeers(map<string, PeerData> &peerList) {
    int n;
    boost::xtime xt;
    while(1){
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec+=30;
        n = 0;
        for( map<string,PeerData> :: iterator i = peerList.begin(); i != peerList.end(); i++) {
            n++;
        }
        cout<<"Numero de peers: "<<n<<endl;
        boost::thread::sleep(xt);
    }
}*/
