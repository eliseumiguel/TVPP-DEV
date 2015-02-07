/*
 * MessageStateChannel.hpp
 *
 *  Created on: Feb 6, 2015
 *      Author: eliseu
 */




#ifndef MESSAGE_STATECHANNEL_H
#define MESSAGE_STATECHANNEL_H

#include "Message.hpp"

class MessageStateChannel : public Message
{
	public:
		MessageStateChannel(Message* message) : Message(message->GetFirstByte()) {};
		MessageStateChannel(ChannelModes state);
		vector<int> GetHeaderValuesDataVector(ChannelModes state);
		vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_STATECHANNEL_H
