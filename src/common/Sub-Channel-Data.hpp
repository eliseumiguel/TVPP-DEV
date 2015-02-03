#ifndef SUB_CHANNEL_DATA_H
#define SUB_CHANNEL_DATA_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include <time.h>
#include <vector>
#include <map>
#include "PeerData.hpp"
#include "Strategy/Strategy.hpp"

using namespace std;

/**
* ECM- esta classe implementa o sub canal
* 01-02-2015
*/
class SubChannelData
{
    public:
 	    SubChannelData(unsigned int channelId = 0, unsigned int subChannelID = 0, Peer* serverPeer_Sub = NULL);

        Peer* GetServer_Sub();
        void CheckActivePeers();
        void PrintPeerList(map<string, PeerData>* peerList_Master);

        unsigned int GetchannelId_Sub();

        FILE* GetPerformanceFile();
        FILE* GetOverlayFile();

    private:
        unsigned int channelId_Master;
        unsigned int channelId_Sub;
        Peer* serverPeer_Sub;

        time_t creationTime;

        FILE* performanceFile;
        FILE* overlayFile;
};

#endif // SUB_CHANNEL_DATA_H

