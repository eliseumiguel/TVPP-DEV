#include "MessageRequest.hpp"

MessageRequest::MessageRequest(ChunkUniqueID requestedChunkUID, ChunkUniqueID playbackTipChunkUID)
{
	vector<int> data = GetHeaderValuesDataVector(requestedChunkUID, playbackTipChunkUID);
	
    firstByte = new uint8_t[MESSAGE_REQUEST_HEADER_SIZE];
    Message::AssembleHeader(OPCODE_REQUEST, MESSAGE_REQUEST_HEADER_SIZE, 0, data);
}

vector<int> MessageRequest::GetHeaderValuesDataVector(ChunkUniqueID requestedChunkUID, ChunkUniqueID playbackTipChunkUID) 
{
    vector<int> data(4);
    data[0] = requestedChunkUID.GetCycle();
    data[1] = requestedChunkUID.GetPosition();
    data[2] = playbackTipChunkUID.GetCycle();
    data[3] = playbackTipChunkUID.GetPosition();
    return data;
}

vector<uint8_t> MessageRequest::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(4);
    sizes[0] = 32;                                                    //CHUNKMAP.CHUNKUID.Cycle
    sizes[1] = 16;                                                    //CHUNKMAP.CHUNKUID.Position
    sizes[2] = 32;                                                    //CHUNKMAP.CHUNKUID.Cycle
    sizes[3] = 16;                                                    //CHUNKMAP.CHUNKUID.Position
    return sizes;
}