#include "MessageChannel.hpp"

MessageChannel::MessageChannel(uint8_t channelFlag, bool performingPunch, uint16_t externalPort, uint32_t idChannel,
		uint32_t nowTime, bool serverCandidate, uint8_t channelState,
		uint16_t maxPeerListOutInformed, uint16_t maxPeerListOutInformed_FREE)
{
    vector<int> data = GetHeaderValuesDataVector(channelFlag, performingPunch, externalPort, idChannel,
    		nowTime, serverCandidate, channelState,
			maxPeerListOutInformed, maxPeerListOutInformed_FREE);
	
    firstByte = new uint8_t[MESSAGE_CHANNEL_HEADER_SIZE];
    Message::AssembleHeader(OPCODE_CHANNEL, MESSAGE_CHANNEL_HEADER_SIZE, 0, data);
}

vector<int> MessageChannel::GetHeaderValuesDataVector(uint8_t channelFlag, bool performingPunch, uint16_t externalPort,
		uint32_t idChannel, uint32_t nowTime, bool serverCandidate, uint8_t channelState,
		uint16_t maxPeerListOutInformed,  uint16_t maxPeerListOutInformed_FREE)
{
    vector<int> data(10);
    data[0] = channelFlag;
    data[1] = performingPunch;
    data[2] = VERSION;
    data[3] = externalPort;
    data[4] = idChannel;
    data[5] = nowTime;
    data[6] = serverCandidate;                //ECM client information about is or not a server candidate t
    data[7] = channelState;                   //ECM
    data[8] = maxPeerListOutInformed;        //ECM
    data[9] = maxPeerListOutInformed_FREE;   //ECM
    return data;
}


vector<uint8_t> MessageChannel::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(10);
    sizes[0] = 8;                                                    //CHANNEL FLAG
    sizes[1] = 8;                                                    //CHANNEL FLAG
    sizes[2] = 16;                                                    //VERSION
    sizes[3] = 16;                                                    //EXTERNAL PORT
    sizes[4] = 32;                                                    //CHANNEL ID
    sizes[5] = 32;                                                    //NOWTIME
    sizes[6] = 8;                                                     //INFORM IF IS AUXILIAR SERVER
    sizes[7] = 8;                                                     //USED FOR CHANNEL STATE
    sizes[8] = 16;                                                    //INFORME LISTOUT SIZE
    sizes[9] = 16;                                                    //INFORME LISTOUT-FREE SIZE
    return sizes;
}
