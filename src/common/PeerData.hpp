#ifndef PEERDATA_H_INCLUDED
#define PEERDATA_H_INCLUDED

#include <string>
#include <iostream>
#include <boost/dynamic_bitset.hpp>
#include "HeadedBitset.hpp"
#include "Defines.hpp"
#include "Peer.hpp"
#include "ChunkUniqueID.hpp"

using namespace std;

/**
* This class implements the information about the peers
*/
class PeerData
{
    public:
        /**
        * @param string
        * @param int Max TTL value
        */
        PeerData(Peer* p = NULL, int ttl = TTL, int size = STD_BUFFERSIZE);
        
        Peer* GetPeer();
        
        /**
        * This method returns the current TTL value
        * @return A string with the
        */
        int GetTTL();
        void SetTTL(int v);
        void DecTTL();
        void SetMode(PeerModes mode);
        PeerModes GetMode();
        void SetChunkMap(ChunkUniqueID chunkMapHead, boost::dynamic_bitset<>x);
        bool GetChunkMapPos(int i) const;
        uint32_t GetChunkMapSize() const;
        void IncPendingRequests();
        void DecPendingRequests();
        int  GetPendingRequests();
        void SetDelay(float value);
        float GetDelay();
        void SetUploadScore(int value);
        int  GetUploadScore();
        
        ChunkUniqueID GetChunkMapHead();
        void SetChunkMapHead(ChunkUniqueID chunkMapHead);
       
        friend bool operator==(const PeerData &a, const PeerData &b) {return a.peer==b.peer;};
        friend bool operator!=(const PeerData &a, const PeerData &b) {return a.peer!=b.peer;};
        friend std::ostream& operator<<(std::ostream& os, const PeerData& pd);
        
    private:
        //PeerId
        Peer* peer;
        PeerModes mode;

        //PartnerChunkMap
        HeadedBitset chunkMap;
        
        //TimeToLive of the partnership or request
        int ttl;
        
        //Peer classification technologies =D
        int uploadScore;
        int pendingRequests;
        int quality;
        float delay;
};
#endif // PEERDATA_H_INCLUDED