#ifndef MESSAGE_PING_BOOT_PERF_H
#define MESSAGE_PING_BOOT_PERF_H

#include "MessagePingBoot.hpp" 

class MessagePingBootPerf : public MessagePingBoot
{
    protected: 
        vector<int> GetHeaderValuesDataVector(PingTypes pingType, PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId,
                                    uint32_t chunksGeneratedPerSecond, uint32_t chunksSentPerSecond, uint32_t chunksReceivedPerSecond, uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond, uint32_t requestsRecvPerSecond, uint32_t requestsRetriesPerSecond, uint32_t chunksMissed, uint32_t chunksExpected, float meanHop, float meanTries, float meanTriesPerRequest,
                                    uint16_t neighborhoodSize, ChunkUniqueID* lastMediaID, int lastMediaHopCount, int lastMediaTriesCount, uint32_t lastMediaTime, uint32_t nowtime);
    public:
        MessagePingBootPerf(Message* message) : MessagePingBoot(message) {};
        MessagePingBootPerf(PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId,
                                    uint32_t chunksGeneratedPerSecond, uint32_t chunksSentPerSecond, uint32_t chunksReceivedPerSecond, uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond, uint32_t requestsRecvPerSecond, uint32_t requestsRetriesPerSecond, uint32_t chunksMissed, uint32_t chunksExpected, float meanHop, float meanTries, float meanTriesPerRequest,
                                    uint16_t neighborhoodSize, ChunkUniqueID* lastMediaID, int lastMediaHopCount, int lastMediaTriesCount, uint32_t lastMediaTime, uint32_t nowtime);
        vector<int> GetHeaderValuesDataVector(PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId,
                                    uint32_t chunksGeneratedPerSecond, uint32_t chunksSentPerSecond, uint32_t chunksReceivedPerSecond, uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond, uint32_t requestsRecvPerSecond, uint32_t requestsRetriesPerSecond, uint32_t chunksMissed, uint32_t chunksExpected, float meanHop, float meanTries, float meanTriesPerRequest,
                                    uint16_t neighborhoodSize, ChunkUniqueID* lastMediaID, int lastMediaHopCount, int lastMediaTriesCount, uint32_t lastMediaTime, uint32_t nowtime);
        virtual vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_PING_BOOT_PERF_H    