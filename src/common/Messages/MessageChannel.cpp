#include "MessageChannel.hpp"

MessageChannel::MessageChannel(uint8_t channelFlag, bool performingPunch, uint16_t externalPort, uint32_t idChannel, uint32_t nowTime)
{
    vector<int> data = GetHeaderValuesDataVector(channelFlag, performingPunch, externalPort, idChannel, nowTime);
	
    firstByte = new uint8_t[MESSAGE_CHANNEL_HEADER_SIZE];
    Message::AssembleHeader(OPCODE_CHANNEL, MESSAGE_CHANNEL_HEADER_SIZE, 0, data);
}

vector<int> MessageChannel::GetHeaderValuesDataVector(uint8_t channelFlag, bool performingPunch, uint16_t externalPort, uint32_t idChannel, uint32_t nowTime) 
{
    vector<int> data(6);
    data[0] = channelFlag;
    data[1] = performingPunch;
    data[2] = VERSION;
    data[3] = externalPort;
    data[4] = idChannel;
    data[5] = nowTime;
    return data;
}

vector<uint8_t> MessageChannel::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(6);
    sizes[0] = 8;                                                    //CHANNEL FLAG
    sizes[1] = 8;                                                    //CHANNEL FLAG
    sizes[2] = 16;                                                    //VERSION
    sizes[3] = 16;                                                    //EXTERNAL PORT
    sizes[4] = 32;                                                    //CHANNEL ID
    sizes[5] = 32;                                                    //NOWTIME
    return sizes;
}