#ifndef MESSAGE_ERROR_H
#define MESSAGE_ERROR_H

#include "Message.hpp" 

class MessageError : public Message
{
	public:
		MessageError(Message* message) : Message(message->GetFirstByte()) {};
		MessageError(ErrorTypes error);
		vector<int> GetHeaderValuesDataVector(ErrorTypes error);
		vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_ERROR_H	