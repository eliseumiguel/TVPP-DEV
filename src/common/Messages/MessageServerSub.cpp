/*
 * MessageServerSub.cpp
 *
 *  Created on: Feb 8, 2015
 *      Author: eliseu
 */


#include "MessageServerSub.hpp"

MessageServerSub::MessageServerSub(ServerAuxTypes serverAux, MesclarModeServer MixType, uint8_t QT_PeerMixType, uint8_t TimeDescPeerMix)
{
    vector<int> data = GetHeaderValuesDataVector(serverAux, MixType, QT_PeerMixType, TimeDescPeerMix);

	firstByte = new uint8_t[MESSAGE_SERVERAUX_HEADER_SIZE];
	Message::AssembleHeader(OPCODE_SERVERAUX, MESSAGE_SERVERAUX_HEADER_SIZE, 0, data);
}

vector<int> MessageServerSub::GetHeaderValuesDataVector(ServerAuxTypes serverAux, MesclarModeServer MixType, uint8_t QT_PeerMixType, uint8_t TimeDescPeerMix)
{
    vector<int> data(4);
    data[0] = serverAux;
    data[1] = MixType;
    data[2] = QT_PeerMixType;
    data[3] = TimeDescPeerMix;
    return data;
}

vector<uint8_t> MessageServerSub::GetHeaderValuesSizeVector()
{
	vector<uint8_t> sizes(4);
	sizes[0] = 8;													//SERVER_AUX FLAG
	sizes[1] = 8;													//SERVER_AUX MIX TYPE
	sizes[2] = 8;													//SERVER_AUX QUANTITY PEER DISCONNECTION
	sizes[3] = 8;													//SERVER_AUX TIME OF DISCONNECTION EACH PEER GROUP

    return sizes;
}
