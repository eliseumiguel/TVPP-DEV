#ifndef BOOTSTRAP_H_INCLUDED
#define BOOTSTRAP_H_INCLUDED

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <map>
#include <vector>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "../common/XPConfig.hpp"
#include "../common/PeerData.hpp"
#include "../common/Channel.hpp"
#include "../common/udp.hpp"
#include "bootstrap-tcp-server.hpp"

#include "../common/Messages/Message.hpp"
#include "../common/Messages/MessageChannel.hpp"
#include "../common/Messages/MessagePeerlist.hpp"
#include "../common/Messages/MessagePeerlistShare.hpp"
#include "../common/Messages/MessagePeerlistLog.hpp"
#include "../common/Messages/MessageError.hpp"
#include "../common/Messages/MessagePingBoot.hpp"
#include "../common/Messages/MessagePingBootPerf.hpp"
#include "../common/Messages/MessageStateChannel.hpp"
#include "../common/Messages/MessageServerSub.hpp"

#include "../common/Strategy/Strategy.hpp"
#include "../common/Strategy/TournamentStrategy.hpp"
#include "../common/Strategy/NearestIPStrategy.hpp"
#include "../common/Strategy/RandomStrategy.hpp"

#include "../common/Scheduler/IMessageScheduler.hpp"
#include "../common/Scheduler/FIFOMessageScheduler.hpp"
#include "../common/Scheduler/RRMessageScheduler.hpp"

/**
* This class implements the bootstrap server
*/
class BootstrapTCPServer;
class Bootstrap 
{
    public:
        /**
        * @param string 
        * @param int Max TTLChannel value
        */
        Bootstrap(string udpPort, string peerlistSelectorStrategy, unsigned int peerListSharedSize, unsigned int maxSubChannel,
        		   unsigned int maxServerAuxCandidate, unsigned int maxPeerInSubChannel, unsigned int sizeCluster,
					MesclarModeServer MixType,	uint8_t QT_PeerMixType,	uint8_t TimeDescPeerMix, uint8_t minimumBandwidth, uint8_t minimumBandwidth_FREE);

        void TCPStart(const char *);
        void UDPStart();
        void UDPReceive();
        void Mainloop(map<string, PeerData> &);
        void CheckPeerList();
        void ShowPeerList();
        void HTTPLog();
        void InicializaDados();

    private:

        vector<bool> RandomlySelectPartners(int num_peer, int num_draft);

        Message *HandleTCPMessage(Message* message, string sourceAddress, uint32_t socket);
        void HandleUDPMessage(Message* message, string sourceAddress = "");
        Message *HandleChannelMessage(MessageChannel* message, string sourceAddress = "");
        void HandlePeerlistMessage(MessagePeerlist* message, string sourceAddress = "");
        void HandlePingMessage(MessagePingBoot* message, string sourceAddress = "", uint32_t socket = 0);
        void setChannelState(uint32_t channelId, uint8_t channelState);

        map<unsigned int, Channel> channelList;
        BootstrapTCPServer *tcpServer;
        UDPServer *udp;

        boost::mutex channelListMutex;
        FILE *pfile;
        Strategy* peerlistSelectorStrategy;
        unsigned int peerListSharedSize;

        uint8_t minimumBandwidth;
        uint8_t minimumBandwidth_FREE;

        //para o canal em flash crowd
		unsigned int maxSubChannel;
		unsigned int maxServerAuxCandidate;
		unsigned int maxPeerInSubChannel;
		unsigned int sizeCluster;

		//exclusivo para configurar os servidores auxiliares na mesclagem
		MesclarModeServer MixType;  //tipo de mesclagem
		uint8_t QT_PeerMixType;     //quantidade de pares a serem desconectados durante a mesclagem
		uint8_t TimeDescPeerMix;    //intervalo de tempo para cada desconexão

		//usado por segurnaça para que os clientes autentiquem cada mensagem do bootstrap
		//Caso mude, significa que o cliente estava comunicando com um bootstrap que já foi fechado
		//automaticamente, o cliente deve finalizar sua execução
		//isso evita clientes zumbi interfirindo em outros experimentos e consumindo banda no ambiente.
		uint32_t bootStrap_ID;

        friend class BootstrapTCPServer;    
};

#endif // BOOTSTRAP_H_INCLUDED
