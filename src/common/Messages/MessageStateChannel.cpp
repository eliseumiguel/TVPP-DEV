/*
 * MessageConfirmStateChannel.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: eliseu
 */




#include "MessageStateChannel.hpp"

MessageStateChannel::MessageStateChannel(ChannelModes state)
{
    vector<int> data = GetHeaderValuesDataVector(state);

	firstByte = new uint8_t[MESSAGE_STATE_CHANNEL_SIZE];
	Message::AssembleHeader(OPCODE_CHANNEL, MESSAGE_STATE_CHANNEL_SIZE, 0, data);
}

vector<int> MessageStateChannel::GetHeaderValuesDataVector(ChannelModes state)
{
    vector<int> data(1);
    data[0] = state;
    return data;
}

vector<uint8_t> MessageStateChannel::GetHeaderValuesSizeVector()
{
	vector<uint8_t> sizes(1);
	sizes[0] = 8;													//STATE FLAG
    return sizes;
}
