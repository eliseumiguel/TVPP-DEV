#include "Request.hpp"

Request::Request(ChunkUniqueID chunkUID)
{
    peers.clear();
    this->chunkUID = chunkUID;
    this->ttl = 500;
    this->attempts.clear();
}

Request::~Request()
{
    for (list<RequestAttempt*>::iterator attemptIt = attempts.begin(); attemptIt != attempts.end();attemptIt++)
    {
        delete *attemptIt;
    }
    this->attempts.clear();
}

void Request::AddPeer(PeerData* peer)
{
    peers.push_back(peer);
}

void Request::RemovePeer(PeerData* peer)
{
    for (int i=0; i<(int)peers.size(); i++)
    {
        if(peers[i] == peer)
        {
            peers.erase(peers.begin()+i);
            break;
        }
    }
}

Peer* Request::GetPeer(Strategy* strategy)
{
    strategy->Execute(&peers, NULL, 1);
    if (!peers.empty() && peers[0])
        return peers[0]->GetPeer();
    return NULL;
}

ChunkUniqueID Request::GetChunkID()
{
return chunkUID;
}

int Request::GetSize()
{
    return (int)peers.size();
}

void Request::SearchPeers(map<string,PeerData*> *peerActive)
{
    peers.clear();
    for (map<string,PeerData*>::iterator j = peerActive->begin(); j != peerActive->end(); j++)
    {
        PeerData* peer = j->second;
        if (peer->GetChunkMapPos(chunkUID.GetPosition()))
        {
            ChunkUniqueID theirChunk = peer->GetChunkMapHead();
            
            if ((theirChunk >= chunkUID) && ((theirChunk-chunkUID).GetCycle() == 0)) //1 cycle max diff
            {
                //BLOCK FREERIDER
                if (!XPConfig::Instance()->GetBool("blockFreeriders") 
					|| peer->GetPendingRequests() <= Statistics::Instance()->GetEstimatedChunkRate())
				{
                    this->AddPeer(peer);
				}
            }    
        }
    }
}

int Request::GetTTL()
{
    return ttl;
}

void Request::SetTTL(int ttl)
{
    this->ttl = ttl;
}

void Request::DecTTL(int diffTtl)
{
    ttl -= diffTtl;
}

RequestAttempt* Request::CreateAttempt(PeerData* chosenPeer)
{
    RequestAttempt* attempt = new RequestAttempt(chosenPeer->GetPeer()->GetID(), chosenPeer->GetDelay(), GetSize());
    attempts.push_back(attempt);
    return attempt;
}

list<RequestAttempt*> Request::GetAttempts()
{
    return attempts;
}