#ifndef MESSAGE_PING_BOOT_PERF_H
#define MESSAGE_PING_BOOT_PERF_H

#include "MessagePingBoot.hpp" 

class MessagePingBootPerf : public MessagePingBoot
{
    protected: 
        vector<int> GetHeaderValuesDataVector(PingTypes pingType, PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId,
                                    uint32_t chunksGeneratedPerSecond, uint32_t chunksSentPerSecond, uint32_t chunksReceivedPerSecond, uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond, uint32_t requestsRecvPerSecond, uint32_t requestsRetriesPerSecond, uint32_t chunksMissed, uint32_t chunksExpected, float meanHop, float meanTries, float meanTriesPerRequest,
                                    ChunkUniqueID* lastMediaID, int lastMediaHopCount, int lastMediaTriesCount, uint32_t lastMediaTime, uint32_t nowtime,

                                    uint16_t neighborhoodSizeIn,
									uint16_t neighborhoodSizeOut,
									uint16_t neighborhoodSizeOut_FREE,

									uint16_t maxPeerListOut,
									uint16_t maxPeerListOut_FREE);

    public:
        MessagePingBootPerf(Message* message) : MessagePingBoot(message) {};

        MessagePingBootPerf(PeerModes mode,
        		            ChunkUniqueID chunkuid,
							uint32_t estimatedStreamRate,
							uint32_t channelId,
                            uint32_t chunksGeneratedPerSecond,
						    uint32_t chunksSentPerSecond,
							uint32_t chunksReceivedPerSecond,
							uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond,
									uint32_t requestsRecvPerSecond,
									uint32_t requestsRetriesPerSecond,
									uint32_t chunksMissed,
									uint32_t chunksExpected,
									float meanHop,
									float meanTries,
									float meanTriesPerRequest,

									ChunkUniqueID* lastMediaID,
									int lastMediaHopCount,
									int lastMediaTriesCount,
									uint32_t lastMediaTime,
									uint32_t nowtime,

                                    uint16_t neighborhoodSizeIn,
									uint16_t neighborhoodSizeOut,
									uint16_t neighborhoodSizeOut_FREE,

									uint16_t maxPeerListOut,
									uint16_t maxPeerListOut_FREE);

        vector<int> GetHeaderValuesDataVector(PeerModes mode, ChunkUniqueID chunkuid,  uint32_t estimatedStreamRate, uint32_t channelId,
                                    uint32_t chunksGeneratedPerSecond, uint32_t chunksSentPerSecond, uint32_t chunksReceivedPerSecond, uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond, uint32_t requestsRecvPerSecond, uint32_t requestsRetriesPerSecond, uint32_t chunksMissed, uint32_t chunksExpected, float meanHop, float meanTries, float meanTriesPerRequest,
                                    ChunkUniqueID* lastMediaID, int lastMediaHopCount, int lastMediaTriesCount, uint32_t lastMediaTime, uint32_t nowtime,

									uint16_t neighborhoodSizeIn,
									uint16_t neighborhoodSizeOut,
									uint16_t neighborhoodSizeOut_FREE,

									uint16_t maxPeerListOut,
									uint16_t maxPeerListOut_FREE);

        virtual vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_PING_BOOT_PERF_H    
