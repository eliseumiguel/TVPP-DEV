#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED


#include <iostream>
#include <cstdlib>

#include <map>
#include <vector>
#include <errno.h>
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


#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "peerdata.hpp"


#define _IPV6_SIZE 40

#define SEP "XSXS"         // Flag de separação de mensagem.
#define FINAL "XFXF"       // Flag de final da transmissão de dados.
#define CMODE "0"          // Modo cliente
#define SMODE "1"	       //Modo Servidor		


using namespace std;


class Client{

    public:

	Client(char*,char*,char*,char*,string,int, int, int, int, int, int);
    void TCP_Conex();
    void Recebe_Lista();
	void Start_UDP_Connection();
    virtual void Ping();
    void verificaLista();
	void UDP_Server();
	void PeerCtoPeerA();
	void VerificaPos();
	void Colocar_na_lista_de_pedidos();
	void GerarDados();
	void MontarListaPedidos();
 	void PedeListaPeers();
 	void ConsomeMedia();
	int principal(char*,char*);
	int web(int, int);
	void log(int, char*, char*, int);
	
	
	map<int,char*> Buffer;
	map<int,int> Buffer_piece;//tamanho de cada parte do buffer
	map<int,unsigned long> Buffer_id;
	boost::dynamic_bitset<> chunk_map;


    private:
    
	void *get_in_addr(struct sockaddr *sa);
	int j_inet_getport(struct sockaddr_storage *sa);
	void ClearBuf(char*, int);
	void mostraPeerList(map<string, PeerData>);
	void Organiza_Lista();
	void Pedir();
	int UDP_MSG(int, char*, int, string,string);
	boost :: dynamic_bitset<> charToBit(unsigned char* , int );
	unsigned char* BitToChar(boost :: dynamic_bitset<> );
	boost :: dynamic_bitset<> intToBit(unsigned );    	
	string Procura_chunk(int);
	void Fazer_pedidos();
	void EnviaDados(string s, string msg);
	void RecebeDados(char *, string address, int maxsize);
	vector<int> decode_msg(char *msg,char* separator,char *endmarker,int separator_size,int endmarker_size,int maxsize);
	//PARAMETROS-------------
	char* Bootstrap_IP;
	char *TCP_server_PORT;
	char *UDP_server_PORT;
	char *peers_UDP_PORT;
	string MODE;
	int MAXDATASIZE;
	int BUFFER_SIZE;//Tamanho do buffer de dados, e chunk_map
	int MAX_PEERS_ATIVOS;
	int JANELA;//Janela de interesse
	int NUM_PEDIDOS;//Número de chunks a ser colocado na lista de pedidos a cada chamada
	int TTL_MAX;//tempo até o cliente ser removido da lista de peer ativos
	//-----------------------   	
	
    int sockfd, pos_atual;
    bool receber_pos_atual;
	string recived1;
	int udp_sock;
	string external_port;

	
	bool terminou;
	bool udp_punch;   	
	map<string, PeerData> peerList;
	map<string, PeerData> peerAtivo;
	list<PeerData> lista_de_pedidos;
    	
	
	boost :: mutex mutex;
	
	int chunk_pos;
	int num_recived_packets;
	unsigned int chunk_id;
	unsigned int next_chunk_id;
	int gerador;
	FILE * pfile;

};


#endif // TCPCONEX_H_INCLUDED

