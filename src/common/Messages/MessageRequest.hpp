#ifndef MESSAGE_REQUEST_H
#define MESSAGE_REQUEST_H

#include "Message.hpp" 
#include "../ChunkUniqueID.hpp"

class MessageRequest : public Message
{
	public:
		MessageRequest(Message* message) : Message(message->GetFirstByte()) {};
		MessageRequest(ChunkUniqueID requestedChunkUID, ChunkUniqueID playbackTipChunkUID);
		vector<int> GetHeaderValuesDataVector(ChunkUniqueID requestedChunkUID, ChunkUniqueID playbackTipChunkUID);
		vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_REQUEST_H	