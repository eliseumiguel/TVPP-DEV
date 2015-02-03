#include <iostream>

#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>

#include <map>
#include <errno.h>
#include <string>


#include "servidor.hpp"

#define STANDARD_UDP_PORT "4950"

using namespace std;




/** 
 *Função principal do cliente
 *chama as funções e abre as threads necessárias pra funcionamento do programa
 */
int main (int argc, char* argv[]){

     char *udp_port = STANDARD_UDP_PORT;
     char *tcp_port = STANDARD_TCP_PORT;	

     /** pega o endereço IP do servidor ao qual o cliente deseja se conectar.*/	
     if (argc != 2) {
        fprintf(stderr,"usage: client hostname TCP-port UDP-port\n");
        exit(1);
    }
    
    
    Servidor S(argv[1], tcp_port);
    boost::thread TPING(boost::bind(&Servidor::Ping, &S,argv[1],udp_port));
    boost::thread TGERAR(boost::bind(&Servidor::GerarDados,&S));
     	

    TPING.join();
    TGERAR.join();
 
    
    cout<<endl<<"ENDMAIN"<<endl;
}


