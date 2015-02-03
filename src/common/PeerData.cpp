#include "PeerData.hpp"

using namespace std;

/** 
 *Metodo construtor
 * Recebe o TTL padrão (Tempo de vida na lista de peers caso o o cliente pare de se comunicar com o servidor)
 * ID que o o endereço IP do peer
 */
PeerData::PeerData(Peer* peer, int ttl, int size) : chunkMap(size)
{
	this->ttl = ttl;
    this->peer = peer;
    uploadScore = 0;
    mode = MODE_CLIENT;
    pendingRequests = 0;
    delay = 0;
}

int PeerData::GetTTL()
{
    return ttl;
}

/** Retorna o ID*/
Peer* PeerData::GetPeer()
{
    return peer;
}

/** Altera o TTL*/
void PeerData::SetTTL(int ttl)
{
	this->ttl = ttl;
}

/** Decrementa o TTL em 1*/
void PeerData::DecTTL()
{
    ttl--;
}

/** Altera o Modo do Peer */
void PeerData::SetMode(PeerModes mode)
{
    this->mode = mode;
}

/** Retorna o Modo do Peer */
PeerModes PeerData::GetMode()
{
    return this->mode;
}

void PeerData::SetChunkMap(ChunkUniqueID chunkMapHead, boost::dynamic_bitset<> map)
{
    chunkMap = HeadedBitset(chunkMapHead, map);
    //SetChunkMapHead(chunkMapHead);
}

bool PeerData::GetChunkMapPos(int i) const
{
    return (bool)chunkMap[i];
}

void PeerData::SetChunkMapHead(ChunkUniqueID chunkMapHead)
{
    chunkMap.SetHead(chunkMapHead);
}

ChunkUniqueID PeerData::GetChunkMapHead()
{
    return chunkMap.GetHead();
}

uint32_t PeerData::GetChunkMapSize() const
{
    return chunkMap.size();
}

void PeerData::IncPendingRequests()
{
    pendingRequests++;
}

void PeerData::DecPendingRequests()
{
    pendingRequests--;
}

int PeerData::GetPendingRequests()
{
    return pendingRequests;
}

void PeerData::SetDelay(float value)
{
    delay = value;
}

float PeerData::GetDelay()
{
    return delay;
}

void PeerData::SetUploadScore(int value)
{
    uploadScore = value;
}

int PeerData::GetUploadScore()
{
    return uploadScore;
}

std::ostream& operator<<(std::ostream& os, const PeerData& pd)
{
    os << "PeerID: " << pd.peer << " Mode: " << (int)pd.mode << endl;
    os << pd.chunkMap; 
    return os;
}