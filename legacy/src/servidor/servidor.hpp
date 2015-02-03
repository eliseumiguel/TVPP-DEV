#ifndef SERVIDOR_H_INCLUDED
#define SERVIDOR_H_INCLUDED

#include "../client/client.hpp"
#include <boost/lexical_cast.hpp>

class Servidor : public Client {

	public:
		Servidor(char* host_ip, char* udp_port);
		void Ping(char*, char*);
		void GerarDados();
	private:
	
	int chunk_pos;
	int gerador;
	
};


#endif // SERVIDOR_H_INCLUDED
