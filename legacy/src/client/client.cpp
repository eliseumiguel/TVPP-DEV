#include "client.hpp"

#define PING 1
#define REQUEST 2
#define DATA 3

using namespace std;

Client :: Client(char *host_ip, char *TCP_server_port, char* udp_port, 
	char* p_udp_port,string md,int maxdata,int buffer,int maxpeers,int janela,int num,int ttl ){
	
    Bootstrap_IP = host_ip;
    TCP_server_PORT = TCP_server_port;
    UDP_server_PORT = udp_port;
    peers_UDP_PORT = p_udp_port;
    MODE = md;          
    MAXDATASIZE = maxdata;
    BUFFER_SIZE = buffer;
    MAX_PEERS_ATIVOS = maxpeers;
    JANELA = janela;
    NUM_PEDIDOS = num;
    TTL_MAX = ttl;
	num_recived_packets = 0;
    
    boost::dynamic_bitset<> x(BUFFER_SIZE);
    chunk_map = x;
	
	for ( int i = 0; i<BUFFER_SIZE; i++){
		chunk_map[i] = 0;
		Buffer_id[i] = 0;
	}

	
    gerador = 0;
    chunk_pos = 0;
	chunk_id = 0;
    pos_atual = 0;
    receber_pos_atual= true;
	udp_punch = true;
	srand ( time(NULL) );
	//system("rm output.asf");
	//system("mknod output.asf p");
	//pipe(mypipe);
   
	if(MODE == SMODE) {
    	//pfile = fopen ("IN","rb");
	}
	else { //mode Cliente
		//system("rm output.mpeg");
		//system("mknod output.mpeg p");
		//pfile = fopen("output.mpeg","wb");
	}
    //pfile1 = fdopen (mypipe[1], "wb");

	for( int i =0; i<BUFFER_SIZE; i++){
		Buffer_piece[i] = 0;
	}
	TCP_Conex();
	
    /*Recebe_Lista();
	PeerCtoPeerA();
	Start_UDP_Connection();*/

    
}

/** 
 *Função para Limpar um vetor de caracteres
 * param char* buf vetor a ser limpo
 * param int size tamanho do vetor a ser limpo
 */
void Client :: ClearBuf(char* buf, int size){
    for(int i = 0; i < size; i++){
        buf[i] = '\0';
    }  
}

/** Recupera o endereço IP no formato legivel ex: 255.255.255.255 */
void *Client :: get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * get the port number out of a struct sockaddr_storage
 *
 * @param sa the struct sockaddr_storage where we want to read the port
 * @return the port number (already converted to host byte order!)
 */
int Client :: j_inet_getport(struct sockaddr_storage *sa)
{
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;
    
    switch(sa->ss_family)
    {
    case AF_INET:
        sin = (struct sockaddr_in *)sa;
        return ntohs(sin->sin_port);
    case AF_INET6:
        sin6 = (struct sockaddr_in6 *)sa;
        return ntohs(sin6->sin6_port);
    default:
        return 0;
    }
}

/** 
 *Mostra os peers contidos no stlmap
 * mostrando seu ID e TTL
 */
void Client :: mostraPeerList(map<string, PeerData> List) {
    int j = 0;
    for( map<string,PeerData> :: iterator i = List.begin(); i != List.end(); i++) {
            if( j == 0) cout<<endl<<"Peer List:"<<endl;
            cout<<"ID: "<<i->first<<" TTL: "<<i->second.GetTTL()<<endl;
            j++;
    }
    if(j != 0)cout<<endl;
}




/** Essa função se conecta via TCP com o servidor de boot, envia um pedido requerindo 
 *  que este cliente seja inserido na lista de peers do servidor
 *  pede uma lista de peers candidatos
 *  recebe e remonta a lista
 */
void Client :: TCP_Conex(){
    
    int rv;
	char s[INET6_ADDRSTRLEN];
    struct addrinfo hints, *servinfo, *myinfo;
      	 
    

    /** Configuração da conexão TCP */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(Bootstrap_IP, TCP_server_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "TCP_Conex: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
            perror("client_TCP_CONEX: socket");
            //continue;
			exit(1);
    }


	
   	/*if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    	close(sockfd);
        perror("client_TCP_CONEX: connect");
        //continue;
		exit(1);
   	}*/

	if( udp_punch) {
		memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_INET;
    	hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE; // use my IP
	

		if ((rv = getaddrinfo(NULL, peers_UDP_PORT, &hints, &myinfo)) != 0) {
			fprintf(stderr, "udp-punch: getaddrinfo: %s\n", gai_strerror(rv));
		    exit(1);
		}
	
		if (bind(sockfd, myinfo->ai_addr, myinfo->ai_addrlen) == -1) {
            close(sockfd);
            perror("udp-punch: bind");
            //continue;
			exit(1);
        }
		
	}
	if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    	close(sockfd);
        perror("client_TCP_CONEX: connect");
        //continue;
		exit(1);
   	}
   	if (send(sockfd,"v-0.06XSXS",10, 0) == -1)
        	perror("send");
	if (udp_punch){
		if (send(sockfd,"PXFXF",5, 0) == -1)
        	perror("send");
		//udp_punch = false;

	}
	else{
		string msg;
		msg+="R";
		msg+=SEP;
		msg+=external_port;
		msg+=FINAL;

		if (send(sockfd,msg.c_str(),msg.size(), 0) == -1)
        	perror("send");
	}
    // loop through all the results and connect to the first we can
    /*for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client_TCP_CONEX: socket");
            continue;
        }




        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client_TCP_CONEX: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client_TCP_CONEX: failed to connect\n");
        exit(1);
    }*/

    inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr),
            s, sizeof s);
    printf("client_TCP_CONEX: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

}
/** 
*Loop de recebimento de mensagem.
*Recebe todo streaming de dados, passado pelo servidor.
*fecha a conexão ao receber o string de finalização da transferencia de dados
*/
void Client :: Recebe_Lista() {    
    
	int numbytes;
    char buf[MAXDATASIZE+100];	
    ClearBuf(buf, MAXDATASIZE+100);
    recived1.clear();
    //buf.clear();
    terminou = false; //se todos os dados já foram recebidos.  		  
    while(!terminou){
        numbytes = recv(sockfd,buf,MAXDATASIZE+100,0);
        for (int j = 0; j < MAXDATASIZE+100; j++){
        	if (buf[j] != NULL)
        		recived1 += buf[j];
        }
               	    
       	//recived1 += buf; 
       	
       	if (recived1.find(FINAL) != string::npos){// Verifica se todos os dados já foram recebidos.
        	terminou = true;
        } 
          
       	//buf.clear();
     	ClearBuf(buf, MAXDATASIZE+100);
		//cout<<"A";
    }
    //cout<<recived1;
    close(sockfd);
    Organiza_Lista();
}
/**
*Organiza os dados
*Separa todo streaming que foi recebido na mensagem sobre situação do client no servidor 
*e a lista de peers candidatos no formato padrão stlmap
*/
void Client :: Organiza_Lista(){
    
	 vector<int> vetor;	
	 string message,ID,str;
     PeerData D("0:0",10,BUFFER_SIZE);
     peerList.clear();
	 string pos_atual_str, chunk_id_str;
	 char *frame = (char*)recived1.c_str();
	
	 /*int k = 0;
	 while( (frame[k] != 'X') || (frame[k+1] != 'F') || (frame[k+2] != 'X') || (frame[k+3] != 'F')){
		//data_size++;
		k++;
	 }*/
	 vetor = decode_msg(frame,SEP,FINAL,4,4,(int)recived1.size());
	 if (vetor.size() < 4){
		cout<<"INVALID CONNEX MESSAGE"<<endl;
	 }
	 else {
		//BOOTSTRAP MESSAGE
		for(int i = 0; i<vetor[0]; i++){
			message+=frame[i];
		}
		cout<<"Client: "<<message<<endl;
		if(message == "INVALID CLIENT VERSION!!!"){
			exit(1);
		}
		//ACTUAL POSITION	 
		for(int i = vetor[0]; i<vetor[1]; i++){
			pos_atual_str+=frame[i];
		}
		//CHUNK_ID
		pos_atual = boost::lexical_cast<int>(pos_atual_str);				
		for(int i = vetor[1]; i<vetor[2]; i++){
			chunk_id_str+=frame[i];
		}
		//MY EXTERNAL PORT
		if(udp_punch){
			external_port.clear();
			for(int i = vetor[2]; i<vetor[3]; i++){
				external_port+=frame[i];
			}
			udp_punch = false;
		}
 
		//PEER LIST
		chunk_id =  boost::lexical_cast<unsigned int>(chunk_id_str);
		for(int j = 3; j<vetor.size()-1; j++){
			ID.clear();
			for(int i = vetor[j]; i<vetor[j+1]; i++){
				ID+=frame[i];
			}  
			D.SetID(ID);
	  		//cout<<D.GetID()<<endl;
	      	if (peerList.find(D.GetID()) == peerList.end()){
		 		//cout<<"A";
	      		peerList[ID].SetID(ID);
	      	}
		}
	 }
	 //cout<<D.GetID()<<"\t"<<D.GetAddr()<<"\t"<<D.GetPort()<<endl;
    
     /*while(recived1.find(FINAL) != string::npos) {	
     	found = recived1.find(SEP);
		cout<<endl<<recived1<<endl;
     	if ( found == string::npos){
     	 	found = recived1.find(FINAL);
     	}	
     	
     	if ( primeira) {
     		message.append(recived1, 0, found);
     		primeira = false;
     	}
     	else {
     	      if ( msg_chunk_pos){
				  string str1;
				  //size_t found1;
				  //cout<<endl<<"AAAAAAA: "<<recived1<<endl;
				  //cin.get();
	     	      str.clear();
	     	      str.append(recived1, 0, found);
				  recived1.erase(0,found + 4);	
				  str1.clear();
				  found = recived1.find(SEP);
				  if( found == string :: npos){
				  	found = recived1.find(FINAL);
				  }		
				  str1.append(recived1,0,found);				  

	     	      if ( receber_pos_atual){
	     	      	pos_atual = boost::lexical_cast<int>(str);
					chunk_id =  boost::lexical_cast<unsigned int>(str1);
	     	      	receber_pos_atual = false;
	     	      }
	     	      msg_chunk_pos = false;
				 		
     	      }
     	      else {	
	     	      ID.append(recived1, 0, found);
				  //cout<<ID<<endl;
	     	      D.SetID(ID);
				  //cout<<D.GetID()<<endl;
	     	      i = peerList.find(D.GetID());
	     	      if (i == peerList.end()){
					 //cout<<"A";
	     	      	 peerList[ID].SetID(ID);
	     	      }
	     	}
     	}
     	
     	
     	
     	recived1.erase(0,found + 4);
     	ID.clear();
     	
     	
     }*/
     
     cout<<"Client POS_atual: "<<pos_atual<<endl;
	 cout<<"Chunk_ID: "<<chunk_id<<endl;
	 mostraPeerList(peerList);
 }
 
void Client :: Ping(){

	int numbytes;
    const char* m;
    char * cmap, *ping_msg;
    boost:: xtime xt;
    string str,chunk_id_str;
      		
    m = "r";
    /** 
     * Loop principal
     * Envia uma mensagem ao servidor de bootstrap dizendo que este cliente está vivo
     * a cada xt.sec segundos
     */	
    boost::mutex::scoped_lock sl(mutex);
    sl.unlock();		
    while(1){
		/*cout<<endl;
		for( int i = 0; i<BUFFER_SIZE; i++){
			cout<<chunk_map[i];
		}
		cout<<endl;*/
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec+=1;
		boost::thread::sleep(xt);
		float packets_per_second = (float)num_recived_packets;
		num_recived_packets = 0;
		cout<<endl<<"Recived packets per second: "<<packets_per_second<<endl;
		str = external_port;
		if( MODE == SMODE) {
				str +=SEP;
		 		str += boost::lexical_cast<string>(chunk_pos);
				str +=SEP;
				str+= boost::lexical_cast<string>(chunk_id);
				//cout<<m;
		}
		str+=FINAL;
		m = str.c_str();
		string a = Bootstrap_IP;
		string b = UDP_server_PORT;
		numbytes = UDP_MSG(udp_sock,(char*)m,(int)strlen(m),a,b);

		//printf("ping to bootstrap: sent %d bytes to %s\n", numbytes, Bootstrap_IP);
		chunk_id_str = boost::lexical_cast<string>(chunk_id);
		//cout<<endl<<"CHUNK_ID: "<<chunk_id_str<<endl;
		/*ping to Active Peer List*/
		sl.lock();
		//cout<<"Ping to: ";
	 	for( map<string,PeerData> :: iterator i = peerAtivo.begin(); i != peerAtivo.end(); i++) {
				int j = 0;				
				int msg_size = (BUFFER_SIZE/8)+chunk_id_str.size();	
				ping_msg = new char[msg_size];
				cmap = (char *)BitToChar(chunk_map);

				for( j = 0; j<BUFFER_SIZE/8; j++){
					ping_msg[j] = cmap[j];
				}
				//cout<<endl<<"chunk_id: ";
				for(;j<msg_size;j++){
					ping_msg[j] = chunk_id_str[j-(BUFFER_SIZE/8)];
					//cout<<ping_msg[j];
				}
				//cout<<endl;
				//cout<<i->second.GetAddr()<<" ";
				//cout<<":"<<i->second.GetPort()<<endl;
				
				UDP_MSG(udp_sock,ping_msg,msg_size,i->second.GetAddr(),i->second.GetPort());
		}
	   	sl.unlock();

    }
}
/** Verifica se algum peer da lista está inativo (TTL <= 0) caso esteja, removo esse peer */
void Client :: verificaLista() {
    map<string,PeerData> :: iterator i;	
    boost::xtime xt;
    boost::mutex::scoped_lock sl(mutex);
    sl.unlock();
    while (1) {

        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 1;

        for( i = peerAtivo.begin(); i != peerAtivo.end(); i++) {	
            i->second.DecTTL();
	}
	
        i = peerAtivo.begin();

        sl.lock();       
        while ( i != peerAtivo.end()){
        	if ( i->second.GetTTL() <= 0) {
		        cout<<"Peer "<<i->first<<" removed"<<endl;
		        peerAtivo.erase(i);
		        i = peerAtivo.begin();   
            }
            else 
            	i++;
        }
        sl.unlock();
        boost::thread::sleep(xt);
    }

}


/**
 *Cria o servidor TCP.
 *Fica em loop esperando mensagens.
 *Caso receba uma mensagem de um peer que esteja na lista 
 *atualiza o valor de seu TTL
 */
void Client :: Start_UDP_Connection(){

    struct addrinfo hints, *servinfo, *p;
    int rv;
    
	/** Configuração do servidor UDP*/
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, peers_UDP_PORT, &hints, &servinfo)) != 0) {
        //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        cout<<"UDPServer: "<<stderr<<"getaddrinfo: "<<gai_strerror(rv)<<endl;
        exit(1);
    }


    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((udp_sock = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(udp_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(udp_sock);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        cout<<"UDPServer: "<<stderr<<"listener: failed to bind socket"<<endl;
        exit(1);
    }

    freeaddrinfo(servinfo);

    //printf("listener: waiting to recvfrom...\n");
    cout<<"UDPServer: waiting to recvfrom..."<<endl;
}	
	/** 
	*Loop principal. Fica esperando receber uma mensagem
	*Quando recebe a mensagem atualiza o TTL do peer que enviou
	*/ 
void Client :: UDP_Server(){

	int numbytes, MENSAGEM;
    struct sockaddr_storage their_addr;
    char udp_buf[MAXDATASIZE+100];
    size_t addr_len;
    char addr[INET6_ADDRSTRLEN];
	string port;
    string s;
	PeerData d("0:0",10,BUFFER_SIZE);
	string str;
    while (1) {
        addr_len = sizeof their_addr;
        if ( (numbytes = recvfrom(udp_sock, udp_buf, MAXDATASIZE+100 , 0,
            (struct sockaddr *)&their_addr, (socklen_t*)&addr_len)) == -1) {
                perror("recvfrom");
                exit(1);
        }
        inet_ntop(their_addr.ss_family,
                    get_in_addr((struct sockaddr *)&their_addr),
                    addr, sizeof addr);
		port = boost::lexical_cast<string>(j_inet_getport(&their_addr));
		s = addr;
		s+=IP_PORT_SEPARATOR;
		s+=port;
                    
        //Verifica que tipo de Mensagem foi recebida. Ping, Request etc...              
    
        str.clear();		
        str = udp_buf;
        str.erase(str.begin() + 3, str.end());
        
        //cout<<endl<<"buf-ping: "<<buf<<endl;
        //cout<<endl<<"str-ping: "<<str<<endl;
        
        if ( str == "RQT" ) 
        	MENSAGEM = REQUEST;
        else if (str == "DTA")
        	MENSAGEM = DATA;
        else
        	MENSAGEM = -1;	                   
                    
        switch (MENSAGEM) {
		    case REQUEST:
				//cout<<"REQUEST"<<endl;
				if( peerAtivo.find(s) != peerAtivo.end()){
		    		str.clear();
		    		str = udp_buf;
		    		EnviaDados(s,str);
				}
				else
					cout<<"INVALID REQUEST"<<endl;
		    	//cout<<endl<<buf<<endl;
		    	
		    break;
		    case DATA:
		    	str.clear();
		    	str = udp_buf;
				num_recived_packets++;
		    	RecebeDados(udp_buf,s,numbytes);		    
		    break;
		    default://PING 
				string chunk_id_str;   
				for(int j = BUFFER_SIZE/8; j<numbytes; j++){
					chunk_id_str += udp_buf[j];
				}
				      
			    if( peerAtivo.find(s) != peerAtivo.end()){
					peerAtivo[s].SetTTL(TTL_MAX);
					peerAtivo[s].SetChunk_map(charToBit((unsigned char*)udp_buf,(BUFFER_SIZE/8)));
					//cout<<endl<<"chunk_id: "<<chunk_id_str<<endl;
					unsigned int their_chunk_id = boost::lexical_cast<unsigned int>(chunk_id_str);
					peerAtivo[s].SetChunk_id(their_chunk_id);  
				//cout<<endl<<"Cliencht_UDPServer: got packet from "<<peerAtivo[s].GetID()<<endl;
				         	   	
			    }
			    else {
					if ( (int)peerAtivo.size() < MAX_PEERS_ATIVOS) {
						d.SetID(s);
						d.SetTTL(TTL_MAX);
						peerAtivo[d.GetID()] = d;
						cout<<endl<<"Cliencht_UDPServer: got packet from "<<peerAtivo[s].GetID()<<endl;
						cout<<"Peer "<<d.GetID()<<" adicionado a lista de peers ativos"<<endl;
						peerAtivo[s].SetChunk_map(charToBit((unsigned char*)udp_buf,(BUFFER_SIZE/8)));
						
					
						
						//cout<<endl;
						//cout<<"CHUNK_ID: "<<peerAtivo[s].GetChunk_id()<<endl;
						//cout<<endl;
						//cout<<endl<<"chunk_id: "<<chunk_id_str<<endl;
						unsigned int their_chunk_id = boost::lexical_cast<unsigned int>(chunk_id_str);
						peerAtivo[s].SetChunk_id(their_chunk_id);  
					}
					else 
					   	cout<<"Numero máximo de peers ativos atigindo"<<endl;			              
				}
				/*for( int j = 0; j<BUFFER_SIZE; j++){
					if(peerAtivo[s].GetChunk_pos(j))
						cout<<1;
					else
						cout<<0;
				}*/
		    break;
    	}        
	}
	close(udp_sock);
}
//Transfere os peers da lista de peers para lista de peers ativos.
void  Client :: PeerCtoPeerA(){
	for( map<string,PeerData> :: iterator i = peerList.begin(); i != peerList.end(); i++) {

		if(peerAtivo.size() <= MAX_PEERS_ATIVOS){		
			if( peerAtivo.find(i->first) == peerAtivo.end()) {
				peerAtivo[i->first] = i->second;
			}
		}
		else{ 
			break;
		}
	}
	//peerAtivo = peerList;
	
}

/*Converte um inteiro, para o seu equivalente em binário na estrutura de dados
dynamic_bitset*/
boost :: dynamic_bitset<> Client :: intToBit(unsigned a){
	boost :: dynamic_bitset<> x;
	while( a >= 1) {
		x.push_back(a%2);
		a = a/2;				
	}
	if ( a == 1) {
		x.push_back(1);
	}
	return x;
} 
/*Retorna um vetor de unsigned chars  contendo o número binário do parametro "a"
Cada caracter no vetor representa 8 bits */
unsigned char*  Client :: BitToChar(boost :: dynamic_bitset<>a ) {

	unsigned char *c;
	c = new unsigned char [a.size()/8 + a.size()%8];
	int i,j;	
	boost :: dynamic_bitset<> x(8);
	j = 0;
	for( i = 0; i < a.size(); i++) {
		x[i%8] = a[i];
		
		if ( i%8 == 7) {
			c[j] = x.to_ulong();
			j++;
		}
	}
	while( i%8 != 0) { //Preenche com zero os bits que sobram no ultimo caracter
		x[i%8] = 0;
		i++;
		if( i%8 == 7) {
			c[j] = x.to_ulong();
			j++;
		}
	}	
	return c;
}
		
/*Retorna em binário o conteudo de um vetor de unsigned char */	
boost :: dynamic_bitset<> Client :: charToBit(unsigned char* c, int tam) {
	int i,j,k;
	unsigned inteiro;
	boost :: dynamic_bitset<> bitvector(tam*8);
	boost :: dynamic_bitset<> bitvector8;
	j = 0;
	for(i = 0; i<tam; i++) {
		inteiro = c[i];
		bitvector8 = intToBit(inteiro);
		while(bitvector8.size() < 8){
			bitvector8.push_back(0);
		}
		for(k = 0; k < 8; k++){
			bitvector[j] = bitvector8[k];
			j++;
		}
	}
	return bitvector;
}

void Client :: VerificaPos(){
}

void Client :: MontarListaPedidos(){
	boost::xtime xt;
	
	boost::xtime_get(&xt, boost::TIME_UTC);		
	xt.sec += 3;
	boost::thread::sleep(xt);
	/*int maior;	
	for( map<string,PeerData> :: iterator j = peerAtivo.begin(); j != peerAtivo.end(); j++) {
		for(int i = 0; i<BUFFER_SIZE; i++){
			if(j->second.GetChunk_pos(i) && !j->second.GetChunk_pos((i+1)%BUFFER_SIZE) )
				if (maior < i) {
					maior = i;
				}
		}
	}
	cout<<endl<<"MAIOR: "<<maior<<endl;
	pos_atual = maior;
	pos_media = maior;*/
	while(1){
		
		
		Colocar_na_lista_de_pedidos();
		//xt.sec += 1;	
		boost::xtime_get(&xt, boost::TIME_UTC);		
		xt.nsec += 1000000;
		boost::thread::sleep(xt);
		

		
	}
} 	

//Coloca os chunks a serem pededidos na lista de pedidos
void Client :: Colocar_na_lista_de_pedidos(){
	string str = "";
	PeerData D("0:0",10,BUFFER_SIZE);

	//for(int i = pos_atual; i<pos_atual+NUM_PEDIDOS; i++){
	  //for(int i = 0; i<BUFFER_SIZE; i++){     //Modo transferencia de arquivos
			//if(chunk_map[i] == 0) {	              //Modo Transferencia de arquivos
			str = Procura_chunk(pos_atual);
			if (lista_de_pedidos.size() >= JANELA) {
				lista_de_pedidos.pop_front();
			}
			if ( str != "") {
				D.SetID(str);
				D.SetTTL(1000);// Estado do pedido 1: Esperando. 0 : Recebido
				D.SetChunk_num(pos_atual);
				lista_de_pedidos.push_back(D);
				pos_atual = (pos_atual+1)%BUFFER_SIZE;
				if( pos_atual%BUFFER_SIZE == 0){
					chunk_id++;
				}
			
			}
			int i = JANELA;
			while(i > 0){
				chunk_map[(pos_atual+i)%BUFFER_SIZE] = 0;
				i--;
			}
		//}
	//}
	//cout<<pos_atual;
	/*pos_atual = (pos_atual + NUM_PEDIDOS)%BUFFER_SIZE;
	for ( int i= pos_atual; i < pos_atual + NUM_PEDIDOS; i++) {
		chunk_map[i] = 0;
	}*/
	//if( pos_atual%50 == 0 ){		
		Fazer_pedidos();
		//lista_de_pedidos.clear();
	//}						
}

/*Varre a lista de pedidos, removendo os pedidos que já foram recebidos e 
envia uma mensagem UDP fazendo o pedido*/
void Client :: Fazer_pedidos() {
	list<PeerData>::iterator it;
	string str,msg;
	boost::xtime xt;
	//cout<<endl<<"LISTA DE PEDIDOS: "<<endl;
	
	it = lista_de_pedidos.begin();
	while ( it != lista_de_pedidos.end()){ // Verifica e remove algum pedido que já foi recebido.
			int i = it->GetChunk_num(); 
        	if ( (chunk_map[i] == 1) || (it->GetTTL() <= 0)){ //Pedido Recebido
				lista_de_pedidos.erase(it);
				it = lista_de_pedidos.begin();
		    }
            else 
            	it++;
    }
	
	for( it = lista_de_pedidos.begin(); it != lista_de_pedidos.end(); it++) {//Faz os pedidos
		//cout<<"ID: "<<it->GetID()<<" Status: "<<it->GetTTL()<<" Chunk: "<<it->GetChunk_num()<<endl;	
		if(it->GetTTL()%500==0){
			if(it->GetTTL()<1000){
				//cout<<" repetindo pedido ";
			}
				
			str = boost::lexical_cast<string>(it->GetChunk_num());
			//cout<<endl<<"Fazer_P_str: "<<str<<endl;
			msg.clear();
			msg += "RQT";
			msg += str;
			msg += FINAL;
			//cout<<endl<<"FAZER_PEDIDOS_MSG: "<<msg<<endl;
			UDP_MSG(udp_sock,(char*)msg.c_str(),msg.size(),it->GetAddr(),it->GetPort());
			//boost::xtime_get(&xt, boost::TIME_UTC);
		    //xt.nsec+=1000000;
			//boost::thread::sleep(xt);
		}
		it->DecTTL();
				
	}
	//cout<<endl<<"FIM DA LISTA DE PEDIDOS"<<endl;				
}


/*Verifica se algum peer da lista de peers ativos possui o chunk procurado. Retorna o IP do primeiro peer que possuir o chunk
ou uma string vazia caso o chunk não seja encontrado*/
string Client :: Procura_chunk(int posicao_chunk){

	vector<string> vetor;
	boost::mutex::scoped_lock sl(mutex);
	sl.unlock();
	string vazia = "";
	sl.lock();
	//cout<<endl<<"LISTA DE PEERS"<<endl;
	for( map<string,PeerData> :: iterator j = peerAtivo.begin(); j != peerAtivo.end(); j++) {
		//cout<<j->first<<"\t";
		if( j->second.GetChunk_pos(posicao_chunk)){
			int last_position = j->second.GetLastChunk_pos();
			unsigned int their_id = j->second.GetChunk_id();
			if( (last_position >= posicao_chunk && their_id == chunk_id) 
										|| (last_position < posicao_chunk && their_id-1 == chunk_id)){
				vetor.push_back(j->first);
			}
			//return j->first;	
		}
	}
	//cout<<endl<<"FIM DA LISTA"<<endl;
	sl.unlock();
	if(vetor.empty()){
		return vazia;
	}
	else {
		//cout<<"VETOR"<<endl;
		/*for(int z = 0; z < (int)vetor.size(); z++){
			cout<<vetor[z]<<"\t";
		}*/
		//cout<<endl<<"FIM VETOR"<<endl;
		int i = rand() % (int)vetor.size();
		return vetor[i];	
	}	
}
		
//Envia uma mensagem pelo protocolo UDP para o destino	
int Client :: UDP_MSG(int sock, char *msg,int tamanho, string destino,string port ){
	struct addrinfo hints, *servinfo0;
	int rv1,numbytes;
	/** Configuração da conxão UDP */
   	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_INET;
    	hints.ai_socktype = SOCK_DGRAM;	
    	
	if ((rv1 = getaddrinfo(destino.c_str(),port.c_str(), &hints, &servinfo0)) != 0) {
		fprintf(stderr, "UDP MSG:getaddrinfo: %s\n", gai_strerror(rv1));
		exit(1);
    	}
 
	if ((numbytes = sendto(sock,msg,tamanho, 0,
		servinfo0->ai_addr, servinfo0->ai_addrlen)) == -1) {
		cout<<endl<<" "<<destino<<" "<<port<<" "<<endl;
		perror("ping_active: sendto");
		exit(1);
	}
    freeaddrinfo(servinfo0);
	return numbytes;
}

void Client :: GerarDados(){

	int num_packets = 1;
	int sock;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int tam,numbytes;
    struct sockaddr_storage their_addr;
    char buf[100000];
	char temp[MAXDATASIZE];
    size_t addr_len;
	boost :: xtime start, end;
	unsigned long diff,t1,t2;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, "1234", &hints, &servinfo)) != 0) {
        fprintf(stderr, "Gerar Dados: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(1);
    }

    freeaddrinfo(servinfo);

    //printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
	int kaput = 100000;
	int i = 0;
	while(1){
		boost::xtime_get(&start, boost::TIME_UTC);
		tam =0;
		for( int j = 0; j<num_packets; j++)	{
			if ((numbytes = recvfrom(sock, temp, kaput , 0,
				(struct sockaddr *)&their_addr, (socklen_t*)&addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
			//
			for(int k = 0; k<numbytes; k++){
				buf[tam+k] = temp[k];
			}
			tam += numbytes;
		}
		boost::xtime_get(&end, boost::TIME_UTC);
		/*if ( end.nsec > start.nsec)
			diff = (end.nsec - start.nsec);
		else diff = Buffer_time[(i-1)%BUFFER_SIZE];*/
		//t1 = start.sec*1000000000;
		//t1+= start.nsec;
		//t2 = end.sec*1000000000;
		//t2+= end.nsec;
		//diff = end.nsec - start.nsec;
		//diff = (t2 - t1);
		if( Buffer_piece[i%BUFFER_SIZE] != tam){
			if ( Buffer[i%BUFFER_SIZE] != NULL){
				delete[] Buffer[i%BUFFER_SIZE];
			}
			Buffer[i%BUFFER_SIZE] = new char[tam];
			Buffer_piece[i%BUFFER_SIZE] = tam;
		}
		//Buffer[i%BUFFER_SIZE] = new char[tam];
		//Buffer_piece[i%BUFFER_SIZE] = tam;
		//Buffer_id[i%BUFFER_SIZE] = chunk_id;
		//cout<<endl<<Buffer_id[i%BUFFER_SIZE]<<endl;
		for (int j = 0; j<tam; j++){
			Buffer[i%BUFFER_SIZE][j] = buf[j];
		}
		chunk_map[i % BUFFER_SIZE] = 1;
		chunk_pos = i % BUFFER_SIZE;
		//chunk_id++;

		int j = JANELA;
		while(j > 0){
			chunk_map[(i+j)%BUFFER_SIZE] = 0;
			j--;
		}
		i++;
		if( i%BUFFER_SIZE == 0){
			chunk_id++;
		}
		/*cout<<endl<<"INICIO"<<endl;
		for ( int k = 0; k<BUFFER_SIZE; k++){
			cout<<chunk_map[k];
		}
		cout<<endl<<"FIM"<<endl;*/
		//cout<<endl<<"GERAR_pos:"<<chunk_pos<<endl;
		


        /* 	
		printf("listener: got packet from %s\n",
		    inet_ntop(their_addr.ss_family,
		        get_in_addr((struct sockaddr *)&their_addr),
		        s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);*/
		
	}

    close(sock);
	
	//MODO TRANSFERENCIA DE ARQUIVO ------------------------------------
	/*
	boost::xtime xt;
	char *recipe;
	size_t result;
	while(1) {
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec+=1;
		if(!feof(pfile)){
			
			for ( int i = gerador; i< gerador + NUM_PEDIDOS; i++) {
				chunk_map[i % BUFFER_SIZE] = 1;
			
				
				//Buffer[i%BUFFER_SIZE] = new char[2000];
				//result = fread(Buffer[i%BUFFER_SIZE],1,2000,pfile);
				recipe = new char[MAXDATASIZE];
				result = fread(recipe,1,MAXDATASIZE,pfile);
				if ( Buffer[i%BUFFER_SIZE] != NULL){
					delete[] Buffer[i%BUFFER_SIZE];
				}
				Buffer[i%BUFFER_SIZE] = new char[(int)result];
				Buffer_piece[i%BUFFER_SIZE] = (int)result;
				for (int j = 0; j<(int)result; j++){
					Buffer[i%BUFFER_SIZE][j] = recipe[j];
				}
				cout<<endl<<"GEREI DADO POS:" <<i<<endl;
				if(feof(pfile)) {
					i = gerador+NUM_PEDIDOS;
				}
				delete [] recipe;
				
			
			}
			gerador = gerador + NUM_PEDIDOS;
			chunk_pos = gerador;
		}
		//cout<<endl<<"GERAR DADOS";
		boost::thread::sleep(xt);
	}*/
 	//------------------------------------------------------------------------------------
}

void Client :: EnviaDados(string s, string msg){
	int pos,i;
	char data[MAXDATASIZE+100];
	size_t found;
	string destino = s;
	string dados,str,spos,id;
	msg.erase(msg.begin(), msg.begin() + 3); // Remove o tag da mensagem
	found = msg.find(FINAL);
	spos.clear();
	spos.assign(msg.begin(), msg.begin() + found);//Separa o número do pedido
	//cout<<endl<<"MSG-ENVIA: "<<spos<<endl;
	pos = boost::lexical_cast<int>(spos);
	data[0] = 'D'; 
	data[1] = 'T';
	data[2] = 'A';
	i = 3;
	//Separador XSXS
	data[i] = 'X';
	i++;
	data[i] = 'S';
	i++;
	data[i] = 'X';
	i++;
	data[i] = 'S';
	i++;
	//cout<<endl<<"Montando Número";<<endl;
	for ( int j = 0; j < spos.size(); j++) {
		data[i] = spos[j];
		i++;
		
	}
	//Separador XSXS
	data[i] = 'X';
	i++;
	data[i] = 'S';
	i++;
	data[i] = 'X';
	i++;
	data[i] = 'S';
	i++;
	
	id = boost::lexical_cast<string>(Buffer_id[pos]);	

	for (int j = 0; j < (int)id.size(); j++){
		data[i] = id[j];
		i++;
	}

	//Separador XSXS
	data[i] = 'X';
	i++;
	data[i] = 'S';
	i++;
	data[i] = 'X';
	i++;
	data[i] = 'S';
	i++;
	
	/*for (int j = 0; j<i; j++)
		cout<<data[j];
	cout<<endl;*/
	for (int j = 0; j < Buffer_piece[pos]; j++){
		data[i] = Buffer[pos][j];
		i++;
	}
	
	//Separador de final XFXF
	data[i] = 'X';
	i++;
	data[i] = 'F';
	i++;
	data[i] = 'X';
	i++;
	data[i] = 'F';
	i++;
	
	boost::mutex::scoped_lock sl(mutex);
	if(peerAtivo.find(s) != peerAtivo.end()){
		UDP_MSG(udp_sock, data,i+1,peerAtivo[s].GetAddr(),peerAtivo[s].GetPort() );
	}
	else{
		cout<<endl<<"Message to peer that is not on the list anymore"<<endl;
	}
	sl.unlock();
}

void Client :: RecebeDados(/*string msg*/char *frame, string address,int maxsize){
	//MSG FORMAT: tag::num_pedido::id::data END
	
	string dados;
	int data_size = 0;
	string pedido,id;
	int pos,i,j,k;
	unsigned long tempo;
	vector<int> vetor = decode_msg(frame,SEP,FINAL,4,4,maxsize);
	
	if (vetor.size() != 4){
		cout<<"INVALID DATA RECIVED"<<endl;
	}
	else {
		//Request Number
		for(int i = vetor[0]; i<vetor[1]; i++){
			pedido+=frame[i];
		}
		pos = boost::lexical_cast<int>(pedido);
		if ( Buffer[pos] != NULL){
			delete[] Buffer[pos];
		}
		//Recived Data
		Buffer[pos] = new char[vetor[3]-vetor[2]];
		for(int i = vetor[2]; i<vetor[3]; i++){
			Buffer[pos][i-vetor[2]] = frame[i];
		}
		Buffer_piece[pos] = vetor[3]-vetor[2];
		chunk_map[pos] = 1;
		//cout<<endl<<"Pos: "<<pos<<" recived from: "<<address<<" "<<vetor[3] - vetor[2]<<" bytes"<<endl;
	}
	
			
	
	/*
	i = 3;// pacote começa na posição 3, os 3 primeiros bytes são para indetificar o tipo do pacote.
	k = 0;
	//Encontra o número do pedido.	
	while( (frame[i] != 88) || (frame[i+1] != 83) || (frame[i+2] != 88) ||  (frame[i+3] != 83)){
		//cout<<endl<<frame[i]<<endl;		
		pedido += frame[i];
		i++;		
	}
	i = i+4;// i + 4 bytes do separado
	while( (frame[i] != 88) || (frame[i+1] != 83) || (frame[i+2] != 88) ||  (frame[i+3] != 83)){
		//cout<<endl<<frame[i]<<endl;		
		id += frame[i];
		i++;		
	}

	i = i+4;// i + 4 bytes do separador
	j = i;
	//cout<<endl<<"Recebe-pedido: "<<time<<endl;
	pos = boost::lexical_cast<int>(pedido);
	tempo = boost::lexical_cast<unsigned long>(id);
	//cout<<endl<<tempo;
	//cout<<endl<<"int pos: "<<pos<<endl;

	//Encontra o tamanho da parte de dados 
	while( (frame[i] != 88) || (frame[i+1] != 70) || (frame[i+2] != 88) || (frame[i+3] != 70)){
		data_size++;
		i++;
	}
	//cout<<endl<<"Pos: "<<pos<<" size: "<<data_size<<endl;
	if ( Buffer[pos] != NULL){
		delete[] Buffer[pos];
	}

	Buffer[pos] = new char[data_size];
	k=0;

	//Buffer recebe os dados
	for ( k = 0; k < data_size; k++){
		Buffer[pos][k] = frame[j];
		j++;
	}	
	Buffer_piece[pos] = data_size;	
	Buffer_id[pos] = tempo;
	chunk_map[pos] = 1;
	cout<<endl<<"Pos: "<<pos<<" recived from: "<<address<<endl;
	
	
	//fwrite(Buffer[pos],1,data_size,pfile1);
	//fflush(pfile1); 
	//UDP_MSG(sock_peers,Buffer[pos],data_size,"127.0.0.1","1235");
	//cout<<endl;
	//cout<<"CHUNK_POS: "<<chunk_pos<<endl;
	//cout<<endl;
	*/
}

void Client :: PedeListaPeers(){
    boost::xtime xt;
    while (1) {
		
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 20;
      	boost::thread::sleep(xt);
        TCP_Conex();
        Recebe_Lista();
		PeerCtoPeerA();
        
    }
}



void Client :: ConsomeMedia(){

	principal("8181","/home/mantini/client/");
	
	//MODO TRANSFERENCIA DE ARQUVIO.------------------------------
	/*
	boost::xtime xt;
	while(1){
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.nsec += 10000000;
		if ( (chunk_map[pos_media] == 1) && (Buffer[pos_media] != NULL)){
			fwrite(Buffer[pos_media],1,Buffer_piece[pos_media],pfile);
			fflush(pfile);
			pos_media++;
		}
		boost::thread::sleep(xt);
	}*/ 
	//-----------------------------------------------------------------------
	/*
	boost::xtime xt;
	//boost::xtime_get(&xt, boost::TIME_UTC);
	//xt.sec = 3;
	//boost::thread::sleep(xt);
	while(!((chunk_map[pos_media%BUFFER_SIZE] == 1) && (Buffer[pos_media%BUFFER_SIZE])) ){
	   boost::xtime_get(&xt, boost::TIME_UTC);
		xt.nsec = 5000000;
		boost::thread::sleep(xt);
	}
	while(1) {
		boost::xtime_get(&xt, boost::TIME_UTC);		
		
		if ( (chunk_map[pos_media%BUFFER_SIZE] == 1) && (Buffer[pos_media%BUFFER_SIZE] != NULL)) {
			xt.nsec += (Buffer_time[pos_media%BUFFER_SIZE]);
			boost::thread::sleep(xt);
			UDP_MSG(sock_peers,Buffer[pos_media%BUFFER_SIZE],Buffer_piece[pos_media%BUFFER_SIZE],"127.0.0.1","1234");
			
			//cout<<"mandei";
		}
		else{
			//xt.nsec+= 10000000;
			cout<<"SEM MEDIA ";
			//boost::thread::sleep(xt);
		}
		pos_media++;
		if( pos_media >= BUFFER_SIZE)
			pos_media = 0;
		//cout<<endl<<"POS_CHUNK"<<pos_atual<<endl;
		//cout<<"POS_MEDIA"<<pos_media<<endl;
	}*/

}

vector<int> Client :: decode_msg(char *msg,char* separator, char* endmarker,
int separator_size,int endmarker_size,int maxsize){
	vector<int> arguments_sizes;
	int l = 0;
	while(l<maxsize-endmarker_size){
		int i;
		for(i = l; i<l+endmarker_size; i++){
			if( msg[i] != endmarker[i-l]){
				break;
			}
		}
		if(i == l+endmarker_size){
			break;
		}
		l++;
	}
	int msg_size = l;				
	int j=0;
	for( int i = 0; i<msg_size; i++){
		int k;
		for( k = i; k<i+separator_size; k++){
			if( k >= msg_size ){
				break;
			}
			if( msg[k] != separator[k-i]){
				break;
			}
		}
		if(k == i+separator_size){
			arguments_sizes.push_back(j);
			i = i+separator_size;
		}
	
		msg[j] = msg[i];
		j++;
	}
	arguments_sizes.push_back(j);
	return arguments_sizes;
}
/*vector<int> Client :: decode_msg(char *msg,char* separator,int msg_size,int separator_size){
	vector<int> arguments_sizes;
	int j=0;
	for( int i = 0; i<msg_size; i++){
		int k;
		for( k = i; k<i+separator_size; k++){
			if( k >= msg_size ){
				break;
			}
			if( msg[k] != separator[k-i]){
				break;
			}
		}
		if(k == i+separator_size){
			arguments_sizes.push_back(j);
			i = i+separator_size;
		}
	
		msg[j] = msg[i];
		j++;
	}
	arguments_sizes.push_back(j);
	return arguments_sizes;
}*/	
//:sout=#std{access=udp,mux=ts,dst=127.0.0.1:1234}
	
		  	
