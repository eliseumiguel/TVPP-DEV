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

#define MAXPEER_CHANNELSUB 2 //30               //máximo de pares em um sub canal
#define MAX_CHANNELSUB 2                        //máximo de subcanais permitido
#define MAXPEER_CHANNEL_SUB_CANDIDATE 2 //10    //máximo de candidatos a servidor auxiliar

//#define MESCLAR_REDES 1 //true

using namespace std;

/**
* This class implements the Channel abstraction
* ECM-mudanças para que o canal perimta subcanais.
* Com isso, um par do canal poderá ser servidor auxiliar de um sub canal
*/
class SubChannelData;

class Channel
{
    public:
        /**
        * Constructor
        * @param idServer server ip and port (ip:port)
        */
        Channel(unsigned int channelId = 0, Peer* serverPeer = NULL,
        		unsigned int maxPeerInSubChannel = MAXPEER_CHANNELSUB,
        		unsigned int maxSubChannel = MAX_CHANNELSUB,
        		unsigned int maxSubServerAux = MAXPEER_CHANNEL_SUB_CANDIDATE,
        		bool mesclar = false);

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

        bool GetServerSubWaitInform(Peer* peer);
        void SetServerSubWaitInform(Peer* peer, bool waiting);

        ServerAuxTypes GetServerSubNewMode (Peer* peer);

        void SetmaxPeer_ChannelSub(int unsigned maxpeerChannelSub);

        vector<PeerData*> SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity);
        unsigned int GetPeerListSize(); //usada apenas pelo bootstrap em HHTLog

        unsigned int GetPeerListSizeChannel_Sub(unsigned int channelId_Sub); //retorna quantos pares estão em um subcanal
        unsigned int GetTotalChannel_Sub(); //retorna quantos subcanis existem

        void CheckActivePeers();
        void PrintPeerList();
        void analizePeerToBeServerAux(Peer* source);

        FILE* GetPerformanceFile(Peer* srcPeer);
        FILE* GetOverlayFile(Peer* srcPeer);
		
		friend bool operator<(const Channel &a, const Channel &b) {return a.channelId<b.channelId;};
		
    private:
        unsigned int channelId;
        Peer* serverPeer; 
        map<string, PeerData> peerList;

        //ECM ***
        boost::mutex* channel_Sub_List_Mutex;
        boost::mutex* channel_Sub_Candidates_Mutex;

        map<string, SubChannelData> channel_Sub_List;
        map<string, SubChannelCandidateData> server_Sub_Candidates;

        ChannelModes channelMode;
        unsigned int maxPeer_ChannelSub;
        unsigned int max_channelSub;
        unsigned int max_ServerSubCandidate;
        bool mesclarRedes;

        bool AddPeerChannel(Peer* peer);
        bool Create_New_ChannelSub();
        void Remove_AllChannelSub(bool mesclar = false);
        void Remove_ChannelSub(const string* source, bool mesclar = false);
        void CheckAllSubChannel();

        void CheckServerSubState();

        void printChannelProfile(); //ECM método auxiliar para testes
        //*** ECM

        ChunkUniqueID serverNewestChunkID;
        int serverEstimatedStreamRate;

		time_t creationTime;

        FILE* performanceFile;
        FILE* overlayFile;
};


#endif // CHANNEL_H

