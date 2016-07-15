#include "MessageChannel.hpp"

MessageChannel::MessageChannel(uint8_t channelFlag, bool performingPunch, uint16_t externalPort, uint32_t idChannel, uint32_t nowTime, bool serverCandidate, uint8_t channelState, uint16_t sizePeerListOutInformed)
{
    vector<int> data = GetHeaderValuesDataVector(channelFlag, performingPunch, externalPort, idChannel, nowTime, serverCandidate, channelState, sizePeerListOutInformed);
	
    firstByte = new uint8_t[MESSAGE_CHANNEL_HEADER_SIZE];
    Message::AssembleHeader(OPCODE_CHANNEL, MESSAGE_CHANNEL_HEADER_SIZE, 0, data);
}

vector<int> MessageChannel::GetHeaderValuesDataVector(uint8_t channelFlag, bool performingPunch, uint16_t externalPort, uint32_t idChannel, uint32_t nowTime, bool serverCandidate, uint8_t channelState, uint16_t sizePeerListOutInformed)
{
    vector<int> data(9);
    data[0] = channelFlag;
    data[1] = performingPunch;
    data[2] = VERSION;
    data[3] = externalPort;
    data[4] = idChannel;
    data[5] = nowTime;
    data[6] = serverCandidate;           //ECM client information about is or not a server candidate t
    data[7] = channelState;              //ECM
    data[8] = sizePeerListOutInformed;   //ECM
    return data;
}


vector<uint8_t> MessageChannel::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(9);
    sizes[0] = 8;                                                    //CHANNEL FLAG
    sizes[1] = 8;                                                    //CHANNEL FLAG
    sizes[2] = 16;                                                    //VERSION
    sizes[3] = 16;                                                    //EXTERNAL PORT
    sizes[4] = 32;                                                    //CHANNEL ID
    sizes[5] = 32;                                                    //NOWTIME
    sizes[6] = 8;                                                     //INFORM IF IS AUXILIAR SERVER
    sizes[7] = 8;                                                     //USED FOR CHANNEL STATE
    sizes[8] = 16;                                                    //INFORME LISTOUT SIZE
    return sizes;
}
