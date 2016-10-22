#include "MessagePingBoot.hpp"

MessagePingBoot::MessagePingBoot(PeerModes mode, ChunkUniqueID chunkuid,

		//uint16_t neighborhoodSizeIn,
        //uint16_t neighborhoodSizeOut,
        //uint16_t neighborhoodSizeOut_FREE,


		uint16_t maxPeerListOut,
		uint16_t maxPeerListOut_FREE,

		uint32_t estimatedStreamRate, uint32_t channelId)
{
	vector<int> data = GetHeaderValuesDataVector(mode, chunkuid,

                      //neighborhoodSizeIn,
                      //neighborhoodSizeOut,
                      //neighborhoodSizeOut_FREE,

			           maxPeerListOut,
			           maxPeerListOut_FREE,
					   estimatedStreamRate, channelId);
	
    firstByte = new uint8_t[MESSAGE_PING_BOOT_HEADER_SIZE];
	Message::AssembleHeader(OPCODE_PING, MESSAGE_PING_BOOT_HEADER_SIZE, 0, data);
}

vector<int> MessagePingBoot::GetHeaderValuesDataVector(PeerModes mode, ChunkUniqueID chunkuid,
                //uint16_t neighborhoodSizeIn,
                //uint16_t neighborhoodSizeOut,
                //uint16_t neighborhoodSizeOut_FREE,

		        uint16_t maxPeerListOut,
		        uint16_t maxPeerListOut_FREE,

				uint32_t estimatedStreamRate, uint32_t channelId)
{
	return GetHeaderValuesDataVector(PING_BOOT, mode, chunkuid,
            //neighborhoodSizeIn,
            //neighborhoodSizeOut,
            //neighborhoodSizeOut_FREE,

			maxPeerListOut,
			maxPeerListOut_FREE,
			estimatedStreamRate, channelId);
}

vector<int> MessagePingBoot::GetHeaderValuesDataVector(PingTypes pingType, PeerModes mode, ChunkUniqueID chunkuid,
        //uint16_t neighborhoodSizeIn,
        //uint16_t neighborhoodSizeOut,
        //uint16_t neighborhoodSizeOut_FREE,


		uint16_t maxPeerListOut,
		uint16_t maxPeerListOut_FREE,

		uint32_t estimatedStreamRate, uint32_t channelId)
{
    vector<int> data = MessagePing::GetHeaderValuesDataVector(pingType, mode, chunkuid,
            //neighborhoodSizeIn,
            //neighborhoodSizeOut,
            //neighborhoodSizeOut_FREE,

    		maxPeerListOut,
			maxPeerListOut_FREE);

            int prevSize = data.size();
	        data.resize(prevSize + 2);
            data[prevSize + 0] = estimatedStreamRate;
            data[prevSize + 1] = channelId;
            return data;
}

vector<uint8_t> MessagePingBoot::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes = MessagePing::GetHeaderValuesSizeVector();
	int prevSize = sizes.size();
    sizes.resize(prevSize + 2);
    sizes[prevSize + 0] = 32;                                                    //STREAMRATE
    sizes[prevSize + 1] = 32;                                                    //CHANNEL ID
    return sizes;
}
