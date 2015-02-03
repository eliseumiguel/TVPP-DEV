#ifndef MESSAGE_PING_BOOT_H
#define MESSAGE_PING_BOOT_H

#include "MessagePing.hpp" 

class MessagePingBoot : public MessagePing
{
	protected: 
		vector<int> GetHeaderValuesDataVector(PingTypes pingType, PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId);
    public:
        MessagePingBoot() : MessagePing() {};
        MessagePingBoot(Message* message) : MessagePing(message) {};
        MessagePingBoot(PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId);
        vector<int> GetHeaderValuesDataVector(PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId);
        virtual vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_PING_BOOT_H    