#include "bootstrap.hpp"

Bootstrap :: Bootstrap(string s) {
	chunk_pos = 0;
	chunk_id = 0;
	IP_SERVIDOR = s;
}

/**
Limpa um vetor de char
o paramentro buf é o própio vetor e size o tamanho desse vetor
*/
void Bootstrap :: ClearBuf(char* buf, int size){
    for(int i = 0; i<size; i++){
        buf[i] = '\0';
    }
}

/** 
Função que garante que todos os bytes da mensagem são realmente enviados
send é colocado dentro de um loop que roda até todos os bytes definidos no
tamanho da mensagem são enviados
*/
int Bootstrap :: sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}

// get sockaddr, IPv4 or IPv6:
void *Bootstrap :: get_in_addr(struct sockaddr *sa)
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
int Bootstrap :: j_inet_getport(struct sockaddr_storage *sa)
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
 *Informa o cliente da sua situação
 *Desmembra a lista de peers candidatos e a envia para o cliente
 */	
void Bootstrap :: conex(int new_fd, string s) {
    
    string lista;	
    int len,n;
    bool invalid = false;
    map<string,PeerData> :: iterator k;
    char *buf;
	char punch_info[MAXDATASIZE];
	const char* message;
    string str,recived;
	PeerData d;

	cout<<"TCPServer: got connection from "<<s<<endl;
	//recv(new_fd,punch_info,1,0);
	bool terminou = false;
	//mode::myexternalport::
	while(!terminou){	
		ClearBuf(punch_info, MAXDATASIZE);
        recv(new_fd,punch_info,MAXDATASIZE,0);
        for (int j = 0; j < MAXDATASIZE; j++){
        	if (punch_info[j] != NULL){
        		recived += punch_info[j];
        		
        	}
        }
        
        string version;

        version.append(recived.begin(), recived.begin()+6);
        if( version != "v-0.03"){
        	invalid = true;
        	cout<<endl<<"INVALID CLIENT VERSION!!!"<<endl;
        	break;	
        }     	          	
       	if (recived.find(FINAL) != string::npos){// Verifica se todos os dados já foram recebidos.
        	terminou = true;
        } 	
	}
	if(!invalid){
		char* frame = (char*)recived.c_str();
		vector<int> vetor = decode_msg(frame,SEP,FINAL,4,4,recived.size());
		if ( frame[vetor[0]] == 'P'){
			if( peerList.find(s) == peerList.end()){
				d.SetID(s);
				peerList[s]= d;
				message = "Added to peer list";
				cout<<"TCPServer: peer "<<s<<" added to peer list"<<endl;
			}
			else {
				message = "Already in the peer list";
				cout<<"peer "<<s<< " is already in the peer list"<<endl;
			}
		}
		else if (vetor.size() == 3) {
			message = "OK";
			size_t found = s.find(":");
			string s1;
			s1.append(s.begin(),s.begin()+found+1);
			string port;
			for ( int i =vetor[1]; i<vetor[2]; i++){
				port+= frame[i];
			}
			s1+=port;
			if( peerList.find(s1) == peerList.end()){
				d.SetID(s1);
				peerList[s1]= d;
				message = "Added to peer list";
				cout<<"TCPServer: peer "<<s1<<" added to peer list"<<endl;
			}
			else {
				message = "Already in the peer list";
				cout<<"peer "<<s1<< " is already in the peer list"<<endl;
			}
		}
		
		else {
			cout<<endl<<"Recived Invalid Conex message"<<endl;
			message = "INV";
		}
	}
	else {
		message = "CLIENT INVALID VERSION!!!";
	}

 	len = strlen(message) + 1;	
    if (send(new_fd,message,len, 0) == -1)
        perror("send");
        
  
    string port;
    size_t found = s.find(":");
    port.append(s.begin()+found+1,s.end());
       
    str = boost::lexical_cast<string>(chunk_pos);
	str+=SEP;
	str+=boost::lexical_cast<string>(chunk_id);
    lista.clear();
    lista += SEP;
    lista += str;
    lista += SEP;
    lista += port;
    len = lista.size();
 
     
    if (send(new_fd,lista.c_str(),(int)lista.size(), 0) == -1)
        perror("send");   
            
     /**
     * Varre a lista de peers canditados, separando cada campo por um caracter de seperação	
     * caso esse campo for ultimo campo a ser enviado insere um caracter que sinaliza o fim da mensagem
     *envia para o cliente
     */	
	boost::mutex::scoped_lock sl(mutex); 	     
    for( map<string,PeerData> :: iterator i = peerList.begin(); i != peerList.end(); i++) {
     	
          
		if(i->first != s) { //Não manda IP de quem fez o pedido na lista
			/*if (!flag_primeira){//primeira mensagem enviada, precisa do sinalizador de inicio				      
		   	flag_primeira = true;
		   	n = send(new_fd,SEP,4, 0);
		   	cout<<SEP;
			if (n == -1){
				perror("send");
				exit(1);
			}		   	
		   }*/	 
	 	   lista.clear();
	 	   lista += SEP;	
		   lista += i->first;
		   /*k = i;
		   k++;
		   if ( k == peerList.end()){//Se o proximo for o final, envia sinalizador de final
				lista += FINAL;// Caracter de fim de mensagem
				flag_final = true;
		   }
		   else if ( (k->first != s) || (k++ != peerList.end()))
		   	lista += SEP; // Caracter de separação de campo*/
		   	
	 	   len = lista.size();	
		   buf = new char [len];	

		   strcpy(buf, lista.c_str());
		   //cout<<buf;  
		   n = send(new_fd,buf,len, 0);
		   //cout<<buf;
		   if (n == -1){
			perror("send");
			exit(1);
		   }
		   delete[] buf;
		}
     }
	 sl.unlock();
     //if (!flag_final) {
	   n = send(new_fd,FINAL,4, 0);
	   //cout<<FINAL;
           if (n == -1){
	       perror("send");
	       exit(1);
           }
     //}
     close(new_fd);
}


/**
 *Cria o servidor de TCP que fica em loop esperando receber uma conexão
 * quando a coneção é recebida abre uma thread para tratar essa conexão
 */
void Bootstrap :: TCPServer(const char *Port){
    /** Configura conexão tcp */
    
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes;
    char addr[INET6_ADDRSTRLEN];
	string s;
	string client_port;
    int rv;
    const char* conexmessage;
    
 
    cout<<"PORT:"<<Port<<"\n";	
    yes = 1;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, Port, &hints, &servinfo)) != 0) {
        cout<<"TCPServer: "<<stderr<<" getaddrinfo: "<<gai_strerror(rv)<<endl;
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        cout<<stderr<<" TCPServer: failed to bind"<<endl;
        exit(1);
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    cout<<"TCPServer: waiting for connections..."<<endl;
     	
    while(1) { 
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
            		addr, sizeof addr);
		
		/*inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr,true),
            		client_port, sizeof client_port);*/
		s = addr;
		client_port = boost::lexical_cast<string>(j_inet_getport(&their_addr));
		s+=":";
		s+=client_port;
		//conexmessage = "A";
        
	/** Abre nova thread para tratar a conexão aceita*/
        boost::thread TCONEX(boost::bind(&Bootstrap::conex,this,new_fd,s));
        TCONEX.join();
        close(new_fd); 
    }

    exit(1);
}


/**
 *Cria o servidor UDP.
 *Fica em loop esperando mensagens.
 *Caso receba uma mensagem de um peer que esteja na lista 
 *atualiza o valor de seu TTL
 */
void Bootstrap :: UDPServer(char* port){


	
    	int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char addr[INET6_ADDRSTRLEN];
	string s;
	string their_port;
   


/** Configuração do servidor TCP*/
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        cout<<"UDPServer: "<<stderr<<"getaddrinfo: "<<gai_strerror(rv)<<endl;
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
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
	string str;
	while (1) {
	    addr_len = sizeof their_addr;
		//new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	    if ( (numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
	        (struct sockaddr *)&their_addr,(socklen_t*)&addr_len)) == -1) {
	            perror("recvfrom");
	            exit(1);
	    }
	    inet_ntop(their_addr.ss_family,
	                get_in_addr((struct sockaddr *)&their_addr),
	                addr, sizeof addr);
		their_port = boost::lexical_cast<string>(j_inet_getport(&their_addr));
		s.clear();
		s += addr;
		s += ":";
		s += their_port;
	    if( peerList.find(s) != peerList.end()){
	        //peerList[s].SetID(s);
	        peerList[s].SetTTL(_TTL_MAX);
	        cout<<endl<<"UDPServer: got packet from "<<peerList[s].GetID()<<endl;
	        cout<<"UDPServer: peer "<<s<<" TTL updated"<<endl;
	        //cout<<"UDPServer: packet is "<<numbytes<<" bytes long"<<endl;
	        buf[numbytes] = '\0';
	        cout<<"UDPServer: packet contains '"<<buf<<"'"<<endl;
	        
	        if ( peerList[s].GetAddr() == IP_SERVIDOR) {
	    		str.append(buf);
				size_t found = str.find(SEP);
				cout<<str<<endl;
				string chunk_pos_str;
				chunk_pos_str.append(str.begin(), str.begin() + found);
	    		chunk_pos = boost::lexical_cast<int>(chunk_pos_str);
				cout<<endl<<"Chunk_Pos: "<<chunk_pos<<endl;
				string chunk_id_str;
				size_t found1 = str.find(FINAL);
				chunk_id_str.append(str.begin()+found+4,str.begin()+found1);
				chunk_id = boost::lexical_cast<unsigned int>(chunk_id_str);
				cout<<endl<<"Chunk_id: "<<chunk_id<<endl;
				
	    		str.clear();
	    	}
	    }
	    else {
	        cout<<"UDPServer: Message recived from a peer that is not on the list"<<endl;
			cout<<s<<endl;
	    }
	    
	   
	    //cout<<"CHUNK: "<<chunk_pos<<endl;
	    	

	}

}

/** Verifica se algum peer da lista está inativo (TTL <= 0) caso esteja, removo esse peer */
void Bootstrap :: VerificaLista() {
    boost::xtime xt;
	boost::mutex::scoped_lock sl(mutex);
	sl.unlock();
    while (1) {
		sl.lock();
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.sec += 10;
        for( map<string,PeerData> :: iterator i = peerList.begin(); i != peerList.end(); i++) {
            i->second.DecTTL();
            if ( i->second.GetTTL() <= 0) {
                cout<<"Peer "<<i->first<<" removed"<<endl;
                peerList.erase(i->first);
            }
        }
		sl.unlock();
        MostraPeerList();
        boost::thread::sleep(xt);
    }

}

/** 
 * Inicializa peers na lista
 * usada apenas para testes
 */
void Bootstrap :: InicializaDados(){

    PeerData D("192.168.1.103",20);
    PeerData E("192.168.1.102",50);
    PeerData F("189.13.124.86",30);
    peerList["192.168.1.103"] = D;
    peerList["192.168.1.102"] = E;
    peerList["189.13.124.86"] = F;


}

 /** Varre a lista mostrando todos os peers e suas informções */
void Bootstrap :: MostraPeerList() {
    int j = 0;
    for( map<string,PeerData> :: iterator i = peerList.begin(); i != peerList.end(); i++) {
            if( j == 0) cout<<endl<<"Peer List:"<<endl;
            cout<<"ID: "<<i->first<<" TTL: "<<i->second.GetTTL()<<endl;
            j++;
    }
    if(j != 0)cout<<endl;
}
/** Conta o numero de peers na lista */
void Bootstrap :: ContaPeers() {
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
}
vector<int> Bootstrap :: decode_msg(char *msg,char* separator, char* endmarker,
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
