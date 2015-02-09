/*
 * MessageServerSub.hpp
 *
 *  Created on: Feb 8, 2015
 *      Author: eliseu
 */




#ifndef MESSAGE_SERVERSUB_H
#define MESSAGE_SERVERSUB_H

#include "Message.hpp"

class MessageServerSub : public Message
{

public:
	MessageServerSub(Message* message) : Message(message->GetFirstByte()) {};
	MessageServerSub(ServerAuxTypes serverAux);
	vector<int> GetHeaderValuesDataVector(ServerAuxTypes serverAux);
	vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif //MESSAGE_SERVERSUB_H




