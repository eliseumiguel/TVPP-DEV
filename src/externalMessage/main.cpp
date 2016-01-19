/*
 * main.cpp
 *
 *  Created on: Feb 5, 2015
 *      Author: eliseu
 *      Este executável comunica ao bootstrap que um determinado canal sofrerá flash crowd.
 *      Deverá ser executado por um script passando as características do canal
 */

#include "../common/Messages/Message.hpp"
#include "../common/Messages/MessageChannel.hpp"

#include "../common/Defines.hpp"
#include "externalMessageTCP.hpp"

#define BOOTSTRAP_TCP_PORT "5111"
#define PEERS_UDP_PORT "4951"

using namespace std;

bool validateIpAddress(const string ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

int main(int argc, char* argv[])
{
	char *ip = argv[1];                       //IP do bootstrap
	uint8_t channelFlag   =  CHANGE_STATE;
	uint8_t channelMode   =  MODE_FLASH_CROWD;
	uint32_t idChannel    =  0;
	string tcpPort        =  BOOTSTRAP_TCP_PORT;
	string peers_udp_port =  PEERS_UDP_PORT;

    int optind=2;
    string arg1 = argv[1];

    if (!validateIpAddress(arg1))
    {
        if(arg1 == "--help")
        {
            cout << "\nUsage: [SERVER CHANG MODE] [OPTIONS]" <<endl;
            cout <<"\nMain operation mode:"<<endl;
            cout <<"\n";
            cout <<"  -channelId                    select a channel to transmit/receive (default: "<<idChannel<<")"<<endl;
            cout <<"  -peers_udp_port               port for inter peer comunication (default: "<<peers_udp_port<<")"<<endl;
            cout <<"  -channelMode                  select a new channelMode (default:"<<channelMode<<" )"<<endl;
            cout <<"  -tcpPort                      bootstrap tcp port (default: "<<tcpPort<<")"<<endl;
            exit(1);
        }
        else
        {
            cout << "Invalid Arguments. Try --help"<<endl;
            exit(1);
        }
    }

    while ((optind < argc) && (argv[optind][0]=='-'))
    {
        string swtc = argv[optind];

        if (swtc=="-tcpPort")
         {
             optind++;
             tcpPort = argv[optind];
         }
        if (swtc=="-peers_udp_port")
        {
            optind++;
            peers_udp_port = argv[optind];
        }
        if (swtc=="-channelId")
        {
        	optind++;
        	idChannel = atoi(argv[optind]);
        }
        if (swtc=="-channelMode")
        {
        	optind++;
        	channelMode = atoi(argv[optind]);
        }
        else
        {
            cout << "Invalid Arguments. Try --help"<<endl;
            exit(1);
        }

        optind++;
    }

    Message *message;
    time_t nowtime;
    time(&nowtime);
    uint16_t externalPort = boost::lexical_cast<uint16_t>(peers_udp_port);
    message = new MessageChannel(channelFlag, false, externalPort, idChannel, nowtime, false, channelMode);
    message->SetIntegrity();

    ExternalMessageTCP externalConnect(std::string(ip), tcpPort);
    if (externalConnect.Sync_write(message->GetFirstByte(),message->GetSize()) < 0) //Failed to write
      	cout<<"Falha ao conectar o servidor"<<endl;
    else
    	cout<<"CHANNEL STATE CHANGED: Channel Mode ["<<(int)channelMode<<"]"<<endl;

    cout<<"configurado.... "<<endl;
}


