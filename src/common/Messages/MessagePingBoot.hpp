#ifndef MESSAGE_PING_BOOT_H
#define MESSAGE_PING_BOOT_H

#include "MessagePing.hpp" 

class MessagePingBoot : public MessagePing
{
	protected: 
		vector<int> GetHeaderValuesDataVector(PingTypes pingType, PeerModes mode, ChunkUniqueID chunkuid,
                //uint16_t neighborhoodSizeIn,
                //uint16_t neighborhoodSizeOut,
                //uint16_t neighborhoodSizeOut_FREE,

				uint16_t maxPeerListOut,
				uint16_t maxPeerListOut_FREE,

				uint32_t estimatedStreamRate, uint32_t channelId);
    public:
        MessagePingBoot() : MessagePing() {};
        MessagePingBoot(Message* message) : MessagePing(message) {};
        MessagePingBoot(PeerModes mode, ChunkUniqueID chunkuid,

                //uint16_t neighborhoodSizeIn,
                //uint16_t neighborhoodSizeOut,
                //uint16_t neighborhoodSizeOut_FREE,

        		uint16_t maxPeerListOut,
				uint16_t maxPeerListOut_FREE,

				uint32_t estimatedStreamRate, uint32_t channelId);

        vector<int> GetHeaderValuesDataVector(PeerModes mode, ChunkUniqueID chunkuid,

        		//uint16_t neighborhoodSizeIn,
                //uint16_t neighborhoodSizeOut,
                //uint16_t neighborhoodSizeOut_FREE,

        		uint16_t maxPeerListOut,
				uint16_t maxPeerListOut_FREE,

				uint32_t estimatedStreamRate,
				uint32_t channelId);
        virtual vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_PING_BOOT_H    
