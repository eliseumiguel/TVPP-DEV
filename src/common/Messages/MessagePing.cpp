#include "MessagePing.hpp"

MessagePing::MessagePing(PingTypes pingType, uint16_t bodySize, PeerModes mode, ChunkUniqueID chunkuid)
{
    vector<int> data = GetHeaderValuesDataVector(pingType, mode, chunkuid);

	firstByte = new uint8_t[MESSAGE_PING_HEADER_SIZE + bodySize];
	Message::AssembleHeader(OPCODE_PING, MESSAGE_PING_HEADER_SIZE, bodySize, data);
}

vector<int> MessagePing::GetHeaderValuesDataVector(PingTypes pingType, PeerModes mode, ChunkUniqueID chunkuid) 
{
    vector<int> data(4);
    data[0] = pingType;
	data[1] = mode;
    data[2] = chunkuid.GetCycle();
    data[3] = chunkuid.GetPosition();
    return data;
}

vector<uint8_t> MessagePing::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(4);
    sizes[0] = 8;                                                     //PING FLAG
	sizes[1] = 8;                                                     //PEER MODE
    sizes[2] = 32;                                                    //CHUNKMAP.CHUNKUID.Cycle
    sizes[3] = 16;                                                    //CHUNKMAP.CHUNKUID.Position
    return sizes;
}