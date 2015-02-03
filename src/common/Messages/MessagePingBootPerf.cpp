#include "MessagePingBootPerf.hpp"

MessagePingBootPerf::MessagePingBootPerf(PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId,
                                    uint32_t chunksGeneratedPerSecond, uint32_t chunksSentPerSecond, uint32_t chunksReceivedPerSecond, uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond, uint32_t requestsRecvPerSecond, uint32_t requestsRetriesPerSecond, uint32_t chunksMissed, uint32_t chunksExpected, float meanHop, float meanTries,  float meanTriesPerRequest,
                                    uint16_t neighborhoodSize, ChunkUniqueID* lastMediaID, int lastMediaHopCount, int lastMediaTriesCount, uint32_t lastMediaTime, uint32_t nowtime)
{
    vector<int> data = GetHeaderValuesDataVector(mode, chunkuid, estimatedStreamRate, channelId,
                    chunksGeneratedPerSecond, chunksSentPerSecond, chunksReceivedPerSecond, chunksOverloadPerSecond,
                    requestsSentPerSecond, requestsRecvPerSecond, requestsRetriesPerSecond, chunksMissed, chunksExpected, meanHop, meanTries, meanTriesPerRequest, neighborhoodSize, 
                    lastMediaID, lastMediaHopCount, lastMediaTriesCount, lastMediaTime, nowtime);
                    
    firstByte = new uint8_t[MESSAGE_PING_BOOT_PERF_HEADER_SIZE];
    Message::AssembleHeader(OPCODE_PING, MESSAGE_PING_BOOT_PERF_HEADER_SIZE, 0, data);
}

vector<int> MessagePingBootPerf::GetHeaderValuesDataVector(PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId,
                                    uint32_t chunksGeneratedPerSecond, uint32_t chunksSentPerSecond, uint32_t chunksReceivedPerSecond, uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond, uint32_t requestsRecvPerSecond, uint32_t requestsRetriesPerSecond, uint32_t chunksMissed, uint32_t chunksExpected, float meanHop, float meanTries, float meanTriesPerRequest,
                                    uint16_t neighborhoodSize, ChunkUniqueID* lastMediaID, int lastMediaHopCount, int lastMediaTriesCount, uint32_t lastMediaTime, uint32_t nowtime)
{
    return GetHeaderValuesDataVector(PING_BOOT_PERF, mode, chunkuid, estimatedStreamRate, channelId,
                    chunksGeneratedPerSecond, chunksSentPerSecond, chunksReceivedPerSecond, chunksOverloadPerSecond,
                    requestsSentPerSecond, requestsRecvPerSecond, requestsRetriesPerSecond, chunksMissed, chunksExpected, meanHop, meanTries, meanTriesPerRequest, neighborhoodSize, 
                    lastMediaID, lastMediaHopCount, lastMediaTriesCount, lastMediaTime, nowtime);
}

vector<int> MessagePingBootPerf::GetHeaderValuesDataVector(PingTypes pingType, PeerModes mode, ChunkUniqueID chunkuid, uint32_t estimatedStreamRate, uint32_t channelId,
                                    uint32_t chunksGeneratedPerSecond, uint32_t chunksSentPerSecond, uint32_t chunksReceivedPerSecond, uint32_t chunksOverloadPerSecond,
                                    uint32_t requestsSentPerSecond, uint32_t requestsRecvPerSecond, uint32_t requestsRetriesPerSecond, uint32_t chunksMissed, uint32_t chunksExpected, float meanHop, float meanTries, float meanTriesPerRequest,
                                    uint16_t neighborhoodSize, ChunkUniqueID* lastMediaID, int lastMediaHopCount, int lastMediaTriesCount, uint32_t lastMediaTime, uint32_t nowtime)
{
    vector<int> data = MessagePingBoot::GetHeaderValuesDataVector(pingType, mode, chunkuid, estimatedStreamRate, channelId);
    int prevSize = data.size();
    data.resize(prevSize + 19);
    //Performance
    data[prevSize + 0]  = chunksGeneratedPerSecond;
    data[prevSize + 1]  = chunksSentPerSecond;
    data[prevSize + 2]  = chunksReceivedPerSecond;
    data[prevSize + 3]  = chunksOverloadPerSecond;
    data[prevSize + 4]  = requestsSentPerSecond;
    data[prevSize + 5]  = requestsRecvPerSecond;
    data[prevSize + 6]  = requestsRetriesPerSecond;
    data[prevSize + 7]  = chunksMissed;
    data[prevSize + 8]  = chunksExpected;
    data[prevSize + 9]  = (int)*reinterpret_cast<int*>(&meanHop);
    data[prevSize + 10]  = (int)*reinterpret_cast<int*>(&meanTries);
	data[prevSize + 11]  = (int)*reinterpret_cast<int*>(&meanTriesPerRequest);
    data[prevSize + 12]  = neighborhoodSize;
    if (lastMediaID)
    {
        data[prevSize + 13] = lastMediaID->GetCycle();
        data[prevSize + 14] = lastMediaID->GetPosition();
        data[prevSize + 15] = lastMediaHopCount;
        data[prevSize + 16] = lastMediaTriesCount;
        data[prevSize + 17] = lastMediaTime;
    } 
    else 
    {
        data[prevSize + 13] = -1;
        data[prevSize + 14] = -1;
        data[prevSize + 15] = -1;
        data[prevSize + 16] = -1;
        data[prevSize + 17] = -1;
    }
    data[prevSize + 18] = nowtime;
    return data;
}

vector<uint8_t> MessagePingBootPerf::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes = MessagePingBoot::GetHeaderValuesSizeVector();
    int prevSize = sizes.size();
    sizes.resize(prevSize + 19);
    sizes[prevSize + 0] = 32;                                                    //STAT.ChunksGenerated
    sizes[prevSize + 1] = 32;                                                    //STAT.ChunksSent
    sizes[prevSize + 2] = 32;                                                    //STAT.ChunksReceived
    sizes[prevSize + 3] = 32;                                                    //STAT.ChunksOverload
    sizes[prevSize + 4] = 32;                                                    //STAT.RequestsSent
    sizes[prevSize + 5] = 32;                                                    //STAT.RequestsRecv
    sizes[prevSize + 6] = 32;                                                    //STAT.RequestsRetries
    sizes[prevSize + 7] = 32;                                                    //STAT.ChunksMissed
    sizes[prevSize + 8] = 32;                                                    //STAT.ChunksExpected
    sizes[prevSize + 9] = 32;                                                    //STAT.MeanHop
    sizes[prevSize + 10] = 32;                                                    //STAT.MeanTries
	sizes[prevSize + 11] = 32;                                                    //STAT.MeanTriesPerRequest
    sizes[prevSize + 12] = 16;                                                    //STAT.NeighborhoodSize
    sizes[prevSize + 13] = 32;                                                    //STAT.SampleChunk.Cycle
    sizes[prevSize + 14] = 16;                                                    //STAT.SampleChunk.Position
    sizes[prevSize + 15] = 32;                                                    //STAT.SampleChunk.HopCount
    sizes[prevSize + 16] = 32;                                                    //STAT.SampleChunk.TriesCount
    sizes[prevSize + 17] = 32;                                                    //STAT.SampleChunk.Time
    sizes[prevSize + 18] = 32;                                                    //STAT.NowTime
    return sizes;
}