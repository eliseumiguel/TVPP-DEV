/*
 * MessagePeerListShare.hpp
 *
 *  Created on: Nov 29, 2012
 *      Author: leonardoresende
 */

#ifndef MESSAGEPEERLISTSHARE_HPP
#define MESSAGEPEERLISTSHARE_HPP

#include "MessagePeerlist.hpp"
#include "../ChunkUniqueID.hpp"

class MessagePeerlistShare : public MessagePeerlist
{
    public:
	    MessagePeerlistShare(Message* message) : MessagePeerlist(message) {};
	    MessagePeerlistShare(uint16_t qtdPeers, string externalIp, uint16_t externalPort, ChunkUniqueID serverTipChunkUId, uint32_t serverStreamRate, uint32_t channelCreationTime, uint32_t nowTime, uint32_t clientTime, uint32_t bootID, Opcodes opcodeMessage = OPCODE_PEERLIST);
        vector<int> GetHeaderValuesDataVector(uint16_t qtdPeers, string externalIp, uint16_t externalPort, ChunkUniqueID serverTipChunkUId, uint32_t serverStreamRate, uint32_t channelCreationTime, uint32_t nowTime, uint32_t clientTime, uint32_t bootID);
        vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif /* MESSAGEPEERLISTSHARE_HPP_ */
