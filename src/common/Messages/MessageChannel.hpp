#ifndef MESSAGE_CHANNEL_H
#define MESSAGE_CHANNEL_H

#include "Message.hpp" 

class MessageChannel : public Message
{
	public:
		MessageChannel(Message* message) : Message(message->GetFirstByte()) {};
		MessageChannel(uint8_t channelFlag, bool performingPunch, uint16_t extPort, uint32_t idChannel, uint32_t nowTime);
		vector<int> GetHeaderValuesDataVector(uint8_t channelFlag, bool performingPunch, uint16_t extPort, uint32_t idChannel, uint32_t nowTime);
		vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_CHANNEL_H	