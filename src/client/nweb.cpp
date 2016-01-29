#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include "client.hpp"

#define BUFSIZE 8096
#define ERROR 42
#define SORRY 43
#define LOG   44

void Client::log(int type, const char *s1, const char *s2, int num)
{
    int fd ;
    char logbuffer[BUFSIZE*2];

    switch (type)
    {
        case ERROR:
            (void)sprintf(logbuffer,"ERROR: %s:%s Errno=%d exiting pid=%d",s1, s2, errno,getpid()); break;
        case SORRY:
            (void)sprintf(logbuffer, "<HTML><BODY><H1>nweb Web Server Sorry: %s %s</H1></BODY></HTML>\r\n", s1, s2);
            (void)write(num,logbuffer,strlen(logbuffer));
            (void)sprintf(logbuffer,"SORRY: %s:%s",s1, s2);
            break;
        case LOG:
            (void)sprintf(logbuffer," INFO: %s:%s:%d",s1, s2,num);
            break;
    }
    /* no checks here, nothing can be done a failure anyway */
    if((fd = open("nweb.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0)
    {
        (void)write(fd,logbuffer,strlen(logbuffer));
        (void)write(fd,"\n",1);
        (void)close(fd);
    }
    if(type == ERROR || type == SORRY)
        exit(3);
}

/* this is a child web server process, so we can exit on errors */
int Client::web(int fd, int hit)
{
    struct
    {
        const char *ext;
        const char *filetype;
    }

    extensions [] =
    {
        {"wmv", "video/wmv" },
        {"mpeg","video/mpeg"},
        {"gif", "image/gif" },
        {"jpg", "image/jpeg"},
        {"jpeg","image/jpeg"},
        {"png", "image/png" },
        {"zip", "image/zip" },
        {"gz",  "image/gz"  },
        {"tar", "image/tar" },
        {"htm", "text/html" },
        {"html","text/html" },
        {0,0}
    };

    int j, file_fd, buflen, len;
    long i, ret;
    const char * fstr;
    static char buffer[BUFSIZE+1]; /* static so zero filled */
    //UNUSED: int pos_media,media_id;

    ret =read(fd,buffer,BUFSIZE); 	/* read Web request in one go */

    if(ret == 0 || ret == -1)
    {	/* read failure stop now */
        log(SORRY,"failed to read browser request","",fd);
    }
    if(ret > 0 && ret < BUFSIZE)	/* return code is valid chars */
        buffer[ret]=0;		/* terminate the buffer */
    else
        buffer[0]=0;

    for(i=0;i<ret;i++)	/* remove CF and LF characters */
        if(buffer[i] == '\r' || buffer[i] == '\n')
            buffer[i]='*';

    log(LOG,"request",buffer,hit);

    if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) )
        log(SORRY,"Only simple GET operation supported",buffer,fd);

    for(i=4;i<BUFSIZE;i++)
    { /* null terminate after the second space to ignore extra stuff */
        if(buffer[i] == ' ')
        { /* string is "GET URL " +lots of other stuff */
            buffer[i] = 0;
            break;
        }
    }

    for(j=0;j<i-1;j++) 	/* check for illegal parent directory use .. */
        if(buffer[j] == '.' && buffer[j+1] == '.')
            log(SORRY,"Parent directory (..) path names not supported",buffer,fd);

    if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) /* convert no filename to index file */
        (void)strcpy(buffer,"GET /index.html");

    /* work out the file type and check we support it */
    buflen=strlen(buffer);
    fstr = (char *)0;
    for(i=0;extensions[i].ext != 0;i++)
    {
        len = strlen(extensions[i].ext);
        if( !strncmp(&buffer[buflen-len], extensions[i].ext, len))
        {
            fstr =extensions[i].filetype;
            break;
        }
    }

    if(fstr == 0)
        log(SORRY,"file extension type not supported",buffer,fd);

    log(LOG,"SEND",&buffer[5],hit);
    log(LOG,"TEST",extensions[i].ext,hit);
    (void)sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
    (void)write(fd,buffer,strlen(buffer));
    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    xt.sec+=5;
    //boost::thread::sleep(xt);
    //time_t rawtime;
  	//struct tm * timeinfo;	

    if(strcmp(extensions[i].ext,"mpeg") == 0)
    {
        int wait = 0;
        playerDeliveryPosition.Set(latestReceivedPosition.GetCycle(), latestReceivedPosition.GetPosition());
        playerDeliveryPosition = playerDeliveryPosition - 140;// Aprox 2 segundos atraz do last received position
        boost::xtime_get(&xt, boost::TIME_UTC);
        //xt.sec+=3;
        // boost::thread::sleep(xt);
        /* DATA PACKET:        | FLAG | HOP | POS | ID | TIME_STAMP | DATA |  **************************************
        ** Sizes(bytes):       |  1   |  1  |  2  |  4 |     4      |   x  |  TOTAL: 12 Bytes + DATA  **************/ 

        while(!quit)
        {
            uint16_t pos_media = playerDeliveryPosition.GetPosition();
            if (mediaBuffer->Available(pos_media))
            {
				MessageData* chunk = (*mediaBuffer)[pos_media];
				chunk->Decode(chunk->GetHeaderSize(), chunk->GetSize());
                uint8_t* data = chunk->GetFirstByte() + chunk->GetHeaderSize();
                if (send(fd, data, chunk->GetBodySize(), MSG_NOSIGNAL) == -1)
                {
					close(fd);
					return 1;
				}

                if(pos_media % 500 == 0)
                {
					cout<<"Read -> Player["<< playerDeliveryPosition <<"] LastReceived["<< latestReceivedPosition <<"]"<<endl;
					time(&lastMediaTime);
					lastMediaID = new ChunkUniqueID(playerDeliveryPosition);
				}
                playerDeliveryPosition++;
                wait = 0;
            }
            else
            {
                if (wait < 800)
				{
                    boost::xtime_get(&xt, boost::TIME_UTC);
                    xt.nsec+= 5000000;
                    boost::thread::sleep(xt);
                }
                else if (playerDeliveryPosition < latestReceivedPosition)
                {
                    cout<<"DESCONTINUIDADE DE MEDIA"<<endl;
                    cout<<"DM: Latest Received "<<latestReceivedPosition<<endl;
                    cout<<"DM: Player Delivery "<<playerDeliveryPosition<<endl;
                    playerDeliveryPosition++;
					if (quantDiscontinuity < 0)
						quantDiscontinuity = 0;
                    quantDiscontinuity++;
                }
				 
                if ((latestReceivedPosition - playerDeliveryPosition).GetCycle() > 0)
				{
                    playerDeliveryPosition.Set(latestReceivedPosition.GetCycle(),latestReceivedPosition.GetPosition());
                    playerDeliveryPosition = playerDeliveryPosition - 140; //Aprox 2 segundos atraz do last received position
					cout<<"RESET -> latestReceived["<<latestReceivedPosition<<"] playerDelivery["<<playerDeliveryPosition<<"]"<<endl;
					quantDiscontinuity = -1;
                }
                    
                wait++;

            }
        }//while
    }
    else
    {
        /* send file in 8KB block - last block may be smaller */
        while (	(ret = read(file_fd, buffer, BUFSIZE)) > 0 )
        {
                (void)write(fd,buffer,ret);
        }
    }
    close(fd);

#ifdef LINUX
    //sleep(1);	/* to allow socket to drain */
#endif
    return 0;
}

int Client::principal(string porta)
{
    int port, hit;
    socklen_t length;
    static struct sockaddr_in cli_addr; /* static = initialised to zeros */
    static struct sockaddr_in serv_addr; /* static = initialised to zeros */
    vector<int> socks;
    FILE *pFile;

    log(LOG,"nweb starting",porta.c_str(),getpid());

    /* setup the network socket */
    if((nweb_listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
        log(ERROR, "system call","socket",0);

    port = atoi(porta.c_str());
    if(port < 0 || port >60000)
        log(ERROR,"Invalid port number (try 1->60000)",porta.c_str(),0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    /*if(bind(nweb_listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
        log(ERROR,"system call","bind",0);*/
    int attemps = 0;
    while(bind(nweb_listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)))
    {
        port++;
        serv_addr.sin_port = htons(port);
        attemps++;
        if (attemps > 10000)
        {
            cout<<"nweb bind Failure"<<endl;
            exit(1);
        }          
    }
    
    cout<<"Internal communication port: "<<port<<endl;
    pFile = fopen ("portnum","w");
    if (pFile!=NULL)
    {
        string strPort = boost::lexical_cast<string>(port);
        fputs (strPort.c_str(),pFile);
        fclose (pFile);
    }
    else
    {
        cout<<"Port file failure"<<endl;
        exit(1);
    }
    if( listen(nweb_listenfd,64) <0)
        log(ERROR,"system call","listen",0);

    for(hit=0; ;hit++)
    {
        length = sizeof(cli_addr);
        cout<<"VLC closed?!";
        if((nweb_socketfd = accept(nweb_listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
            log(ERROR,"system call","accept",0);
        web(nweb_socketfd,hit); /* never returns */
        close(nweb_socketfd);
    }
    close(nweb_listenfd);
}
