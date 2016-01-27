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
	MessageServerSub(ServerAuxTypes serverAux, MesclarModeServer MixType = (MesclarModeServer) 2, uint8_t QT_PeerMixType = 3, uint8_t TimeDescPeerMix = 7);
	vector<int> GetHeaderValuesDataVector(ServerAuxTypes serverAux, MesclarModeServer MixType, uint8_t QT_PeerMixType, uint8_t TimeDescPeerMix);
	vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif //MESSAGE_SERVERSUB_H




