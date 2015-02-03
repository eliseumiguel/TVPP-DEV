#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <map>

#include "../common/XPConfig.hpp"
#include "../common/Peer.hpp"
#include "../common/ChunkUniqueID.hpp"
#include "../common/PeerData.hpp"
#include "../common/Strategy/Strategy.hpp"

#include "RequestAttempt.hpp"
#include "Statistics.hpp"

using namespace std;

class Request
{
private:
    vector<PeerData*> peers;
    ChunkUniqueID chunkUID;
    int ttl;
    list<RequestAttempt*> attempts;
    
public:
    Request(ChunkUniqueID);
    ~Request();
    void AddPeer(PeerData* peer);
    void RemovePeer(PeerData* peer);
    Peer* GetPeer(Strategy* strategy);
    ChunkUniqueID GetChunkID();
    int GetSize();
    void SearchPeers(map<string,PeerData*>* peerActive);
    int GetTTL();
    void SetTTL(int ttl);
    void DecTTL(int diffTtl);
    RequestAttempt* CreateAttempt(PeerData* chosenPeer);
    list<RequestAttempt*> GetAttempts();
};
#endif
