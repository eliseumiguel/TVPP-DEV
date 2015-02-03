#include "servidor.hpp"

Servidor :: Servidor (char* host_ip, char* tcp_port) : Client(host_ip, tcp_port) {
	chunk_pos = 0;
	gerador = 0;
}	

void  Servidor :: Ping(char* host_ip, char *UDP_server_PORT){

     char* IP;		
     int sockfd;
     struct addrinfo hints, *servinfo, *p;
     int rv;
     int numbytes;
     const char* m;
     string str;
     boost:: xtime xt;



    IP = host_ip;  
    str = boost::lexical_cast<string>(chunk_pos);
    m = str.c_str();
    cout<<m<<endl;



    /** Configuração da conxão UDP */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(host_ip, UDP_server_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("ping: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "ping: failed to bind socket\n");
        exit(1);
    }

    /** 
     * Loop principal
     * Envia uma mensagem ao servidor de bootstrap dizendo que este cliente está vivo contendo a posição do chunk
     * a cada xt.sec segundos
     */	
		
    while(1){
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec+=2;
        if ((numbytes = sendto(sockfd,m,strlen(m), 0,
                 p->ai_addr, p->ai_addrlen)) == -1) {
            perror("ping: sendto");
            exit(1);
        }
        printf("ping: sent %d bytes to %s\n", numbytes, IP);
        boost::thread::sleep(xt);
    }
    freeaddrinfo(servinfo);
    close(sockfd);

}

void Servidor :: GerarDados(){
	boost::xtime xt;
	while(1) {
		boost::xtime_get(&xt, boost::TIME_UTC);
        	xt.sec+=5;
		for ( int i = gerador; i< gerador + 5; i++) {
			chunk_map[i % 1600] = 1;
			Buffer[i%1600] = i;
		}
		gerador = gerador + 5;
		chunk_pos = gerador;
		boost::thread::sleep(xt);
	} 
}

