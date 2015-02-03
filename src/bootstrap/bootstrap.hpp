#ifndef BOOTSTRAP_H_INCLUDED
#define BOOTSTRAP_H_INCLUDED


#include <iostream>
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
        * @param int Max TTL value
        */
        Bootstrap(string udpPort, string peerlistSelectorStrategy);
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

        map<unsigned int, Channel> channelList;
        BootstrapTCPServer *tcpServer;
        UDPServer *udp;

        boost::mutex channelListMutex;
        FILE *pfile;
        Strategy* peerlistSelectorStrategy;
        friend class BootstrapTCPServer;    
};

#endif // BOOTSTRAP_H_INCLUDED
