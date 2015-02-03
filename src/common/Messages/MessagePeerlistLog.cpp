#include "MessagePeerlistLog.hpp"

MessagePeerlistLog::MessagePeerlistLog(uint16_t qtdPeers, uint32_t channelId, uint32_t nowtime)
{
    vector<int> data = GetHeaderValuesDataVector(qtdPeers, channelId, nowtime);

    firstByte = new uint8_t[MESSAGE_PEERLIST_LOG_HEADER_SIZE + qtdPeers*6];
    this->peersAdded = 0;
    this->qtdPeers = qtdPeers;
    Message::AssembleHeader(OPCODE_PEERLIST, MESSAGE_PEERLIST_LOG_HEADER_SIZE, qtdPeers*6, data);
}

vector<int> MessagePeerlistLog::GetHeaderValuesDataVector(uint16_t qtdPeers, uint32_t channelId, uint32_t nowtime)
{
    vector<int> data = MessagePeerlist::GetHeaderValuesDataVector(PEERLIST_LOG, qtdPeers);
    int prevSize = data.size();
    data.resize(prevSize + 2);
    data[prevSize + 0] = channelId;
    data[prevSize + 1] = nowtime;
    return data;
}

vector<uint8_t> MessagePeerlistLog::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes = MessagePeerlist::GetHeaderValuesSizeVector();
    int prevSize = sizes.size();
    sizes.resize(prevSize + 2);
    sizes[prevSize + 0] = 32;                                                    //CHANNEL ID
    sizes[prevSize + 1] = 32;                                                    //NOWTIME
    return sizes;
}
