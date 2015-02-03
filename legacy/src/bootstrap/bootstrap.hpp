#ifndef BOOTSTRAP_H_INCLUDED
#define BOOTSTRAP_H_INCLUDED

#include <iostream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

/*#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>*/
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>




#include "peerdata.hpp"


#define MAXBUFLEN 10000
//#define IP_SERVIDOR "150.164.11.71"

#define BACKLOG 10       // how many pending connections queue will hold
#define SEP "XSXS"       // Flag de separação de mensagem.
#define FINAL "XFXF"       // Flag de final da transmissão de dados.
#define MAXDATASIZE 512  // max number of bytes we can send at once

/**
* This class implements the information about the peers
*/
class Bootstrap{

    public:
	/**
	* @param string 
	* @param int Max TTL value
	*/
	Bootstrap(string);
   	void TCPServer(const char *);
   	void UDPServer(char*);
   	void Mainloop(map<string, PeerData> &);
   	void VerificaLista();
   	void MostraPeerList();
   	void ContaPeers();
   	void InicializaDados();
   	

    private:
    
   	void *get_in_addr(struct sockaddr *sa);
	int j_inet_getport(struct sockaddr_storage *sa);
   	void ClearBuf(char* buf, int size);
   	void conex(int, string);
   	int sendall(int s, char *buf, int *len);
	vector<int> decode_msg(char*msg,char*separator, char*endmarker, int separator_size,int endmarker_size,int maxsize); 

   	map<string, PeerData> peerList;
   	int chunk_pos;
	unsigned int chunk_id;
   	string IP_SERVIDOR;   
	boost :: mutex mutex;	

};















#endif // BOOTSTRAP_H_INCLUDED
