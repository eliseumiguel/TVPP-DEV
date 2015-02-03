#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <time.h>
#include <vector>
#include <map>
#include <set>
#include "PeerData.hpp"
#include "Sub-Channel-Data.hpp"
#include "Strategy/Strategy.hpp"

#define MAXPEER_CHANNELSUB 30
#define MAXPEER_CHANNEL_SUB_CANDIDATE 2 //10

using namespace std;

/**
* This class implements the Channel abstraction
* ECM-mudanças para que o canal perimta subcanais.
* Com isso, um par do canal poderá ser servidor auxiliar de um sub canal
*/
class Channel
{
    public:
        /**
        * Constructor
        * @param idServer server ip and port (ip:port)
        */
        Channel(unsigned int channelId = 0, Peer* serverPeer = NULL);

        ChunkUniqueID GetServerNewestChunkID();
		void SetServerNewestChunkID(ChunkUniqueID serverNewestChunkID);
        int GetServerEstimatedStreamRate();
		void SetServerEstimatedStreamRate(int serverEstimatedStreamRate);
		
        Peer* GetServer();
        Peer* GetPeer(Peer* peer);
		bool HasPeer(Peer* peer);
		void AddPeer(Peer* peer);

        void RemovePeer(Peer* peer);
		void RemovePeer(string peerId);
		PeerData& GetPeerData(Peer* peer);
        time_t GetCreationTime();

        void SetChannelMode(ChannelModes channelMode);
        ChannelModes GetChannelMode();

        void SetmaxPeer_ChannelSub(int unsigned maxpeerChannelSub);

        vector<PeerData*> SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity);
        unsigned int GetPeerListSize(); //usada apenas pelo bootstrap em HHTLog

        /* ECM ***
         * retorna o tamanho da lista em que o peer pertence (subcanal ou canal principal)
         * caso não esteja em flash crowd, retorna peerList.size (todos peers em canal principal)
         * o tamanho do canal principal
         */
        unsigned int GetPeerListSizeChannel_Sub(unsigned int channelId_Sub);
        void CheckActivePeers();
        void PrintPeerList();
        void analizePeerToBeServerAux(Peer* source);

        FILE* GetPerformanceFile();
        FILE* GetOverlayFile();
		
		friend bool operator<(const Channel &a, const Channel &b) {return a.channelId<b.channelId;};
		
    private:
        unsigned int channelId;
        Peer* serverPeer; 
        map<string, PeerData> peerList;

        //ECM ***
        boost::mutex* channel_Sub_List_Mutex;
        boost::mutex* channel_Sub_Candidates_Mutex;

        map<string, SubChannelData> channel_Sub_List;
        set<string> server_Sub_Candidates;

        ChannelModes channelMode;
        unsigned int maxPeer_ChannelSub;

        bool AddPeerChannel(Peer* peer);
        bool Creat_New_ChannelSub();
        void Remove_ChannelSub(string* source);
        //apagar esse método
        void printPossibleServerAux();

        //*** ECM

        ChunkUniqueID serverNewestChunkID;
        int serverEstimatedStreamRate;

		time_t creationTime;

        FILE* performanceFile;
        FILE* overlayFile;
};


#endif // CHANNEL_H

