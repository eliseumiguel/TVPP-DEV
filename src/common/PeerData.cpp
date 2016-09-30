#include "PeerData.hpp"

using namespace std;

/** 
 *Metodo construtor
 * ID que o o endereço IP do peer
 * ECM - Inclusao do ttlIn e ttlOut
 */
PeerData::PeerData(Peer* peer, int ttlIn, int ttlOut, int ttlChannel, int size) : chunkMap(size)
{

	//ECM
	this->ttlIn = ttlIn;
	this->ttlOut = ttlOut;
	this->ttlChannel = ttlChannel;
	this->channelId_Sub = 0;
	this->peerWaitListServer = false;


    this->peer = peer;
    uploadScore = 0;
    mode = MODE_CLIENT;
    pendingRequests = 0;
    delay = 0;

    specialPeer = false; // usado para dar privilégios aos servidores especiais
}

int PeerData::GetSizePeerListOutInformed ()
{
   return this->GetPeer()->GetSizePeerListOutInformed();
}

int PeerData::GetSizePeerListOutInformed_FREE ()
{
   return this->GetPeer()->GetSizePeerListOutInformed_FREE();
}


void PeerData::SetSizePeerListOutInformed(int sizePeerListOutInformed)
{
	this->GetPeer()->SetSizePeerListOutInformed(sizePeerListOutInformed);
}

void PeerData::SetSizePeerListOutInformed_FREE(int sizePeerListOutInformed_FREE)
{
	this->GetPeer()->SetSizePeerListOutInformed_FREE(sizePeerListOutInformed_FREE);
}


/** Retorna o TTL*****************/
int PeerData::GetTTLIn()
{
    return ttlIn;
}

int PeerData::GetTTLOut()
{
    return ttlOut;
}
int PeerData::GetTTLChannel()
{
    return ttlChannel;
}

/** Altera o TTL*****************/
void PeerData::SetTTLIn(int ttlIn)
{
	this->ttlIn = ttlIn;
}
void PeerData::SetTTLOut(int ttlOut)
{
	this->ttlOut = ttlOut;
}
void PeerData::SetTTLChannel(int ttlChannel)
{
	this->ttlChannel = ttlChannel;
}

/** Decrementa o TTL em 1*********/
void PeerData::DecTTLIn()
{
    ttlIn--;
}
void PeerData::DecTTLOut()
{
    ttlOut--;
}
void PeerData::DecTTLChannel()
{
    ttlChannel--;
}

int PeerData::GetChannelId_Sub()
{
	return channelId_Sub;
}

void PeerData::SetChannelId_Sub(int channelId_Sub)
{
	this->channelId_Sub = channelId_Sub;
}

/** Retorna o ID*/
Peer* PeerData::GetPeer()
{
    return peer;
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

void PeerData::SetSpecialPeer(bool special)
{
	this->specialPeer = special;
}
bool PeerData::GetSpecialPeer()
{
	return this->specialPeer;
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

bool PeerData::GetPeerWaitListServer()
{
	return peerWaitListServer;
}

void PeerData::SetPeerWaitListServer(bool InformListServer)
{
	this->peerWaitListServer = InformListServer;
}
std::ostream& operator<<(std::ostream& os, const PeerData& pd)
{
    os << "PeerID: " << pd.peer << " Mode: " << (int)pd.mode << endl;
    os << pd.chunkMap; 
    return os;
}

