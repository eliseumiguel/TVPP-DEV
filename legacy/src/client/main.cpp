#include <iostream>

#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>

#include <map>
#include <errno.h>
#include <string>


#include "client.hpp"

#define STANDARD_UDP_PORT "4950"
#define PEERS_UDP_PORT "4951"
#define STANDARD_TCP_PORT "5111"
#define MAXDATASIZE 10000 // tamanho do payload. Caso o arquivo termine antes de encher o payload 
						 //	seu tamanho é o tanto que foi enchido.
#define MAX_PEER_ATIVO 60
#define TTL_MAX  4
#define JANELA 120         // Janela de interesse
#define BUFFER_SIZE 1600
#define NUM_PEDIDOS 64   //Número de pacotes pedidos por segundo no caso do cliente. 
						 //Número de pacotes lidos por do arquivo por segundo, no caso do servidor 			

using namespace std;

/** 
 *Função principal do cliente
 *chama as funções e abre as threads necessárias pra funcionamento do programa
 */
int main (int argc, char* argv[]){

	char *ip = argv[1];	
	char *tcp_port = STANDARD_TCP_PORT;
	char *udp_port = STANDARD_UDP_PORT;
	char *p_udp_port = PEERS_UDP_PORT;
	string mode = "0";
	int maxd = MAXDATASIZE;
	int buf = BUFFER_SIZE;
	int maxp = MAX_PEER_ATIVO;
	int jan = JANELA;
	int nump = NUM_PEDIDOS;
	int ttl = TTL_MAX;
      

    int optind=2;
    // decode arguments
    while ((optind < argc) && (argv[optind][0]=='-')) {
    	string swtc = argv[optind];
        if (swtc=="-t") {
            optind++;
            tcp_port = argv[optind];
            //maxd = atoi(argv[optind]);
        }
        else if (swtc=="-u") {
            optind++;
            udp_port = argv[optind];
        }
        else if (swtc=="-p") {
            optind++;
            p_udp_port = argv[optind];
        }
        else if (swtc=="-b"){
            optind++;
            buf = atoi(argv[optind]);
        }
        else if (swtc=="-s"){
            mode = "1";
        }
        else if (swtc=="-m"){
            optind++;
            maxp = atoi(argv[optind]);
        }
        else if (swtc=="-j"){
            optind++;
            jan = atoi(argv[optind]);
        }
        else if (swtc=="-n"){
            optind++;
            nump = atoi(argv[optind]);
        }
        else if (swtc=="-t"){
            optind++;
            ttl = atoi(argv[optind]);
        }
        else {
            cout << "Invalid Arguments"<<endl; 
            exit(1);
        }
		optind++; 
    }
	
    
    Client C(ip,tcp_port,udp_port,p_udp_port,mode,maxd,buf,maxp,jan,nump,ttl);
    if ( mode == "0") {
		boost::thread TCONSOME(boost::bind(&Client::ConsomeMedia,&C));
		C.Recebe_Lista();
		C.PeerCtoPeerA();
		C.Start_UDP_Connection();	
	    boost::thread TPING(boost::bind(&Client::Ping, &C));
	    boost::thread TVERIF(boost::bind(&Client::verificaLista, &C));		
	    boost::thread TDADOS(boost::bind(&Client::PedeListaPeers,&C));
	    boost::thread TUDPS(boost::bind(&Client::UDP_Server, &C));
		boost::thread TPEDIR(boost::bind(&Client::MontarListaPedidos,&C));
			
	    TPING.join();
	    TVERIF.join();
	    TDADOS.join();
	    TUDPS.join();
		TPEDIR.join();
		TCONSOME.join();
    }
    
    else if ( mode == "1") {
		C.Recebe_Lista();
		C.PeerCtoPeerA();
		C.Start_UDP_Connection();
	    boost::thread TPING(boost::bind(&Client::Ping, &C));
	    boost::thread TVERIF(boost::bind(&Client::verificaLista, &C));		
	    //boost::thread TDADOS(boost::bind(&Client::PedeListaPeers,&C));
	    boost::thread TUDPS(boost::bind(&Client::UDP_Server, &C));
	    boost::thread TGERAR(boost::bind(&Client::GerarDados, &C));	
	    TPING.join();
	    TVERIF.join();
	    //TDADOS.join();
	    TUDPS.join();
	    TGERAR.join();
    }		
    
    cout<<endl<<"ENDMAIN"<<endl;
}

