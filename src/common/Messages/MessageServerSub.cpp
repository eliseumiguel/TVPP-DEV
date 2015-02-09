/*
 * MessageServerSub.cpp
 *
 *  Created on: Feb 8, 2015
 *      Author: eliseu
 */


#include "MessageServerSub.hpp"

MessageServerSub::MessageServerSub(ServerAuxTypes serverAux)
{
    vector<int> data = GetHeaderValuesDataVector(serverAux);

	firstByte = new uint8_t[MESSAGE_SERVERAUX_HEADER_SIZE];
	Message::AssembleHeader(OPCODE_SERVERAUX, MESSAGE_SERVERAUX_HEADER_SIZE, 0, data);
}

vector<int> MessageServerSub::GetHeaderValuesDataVector(ServerAuxTypes serverAux)
{
    vector<int> data(1);
    data[0] = serverAux;
    return data;
}

vector<uint8_t> MessageServerSub::GetHeaderValuesSizeVector()
{
	vector<uint8_t> sizes(1);
	sizes[0] = 8;													//SERVER_AUX FLAG
    return sizes;
}
