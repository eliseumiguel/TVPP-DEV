/*
 * MessagePeerListLog.hpp
 *
 *  Created on: Nov 29, 2012
 *      Author: leonardoresende
 */

#ifndef MESSAGEPEERLISTLOG_HPP
#define MESSAGEPEERLISTLOG_HPP

#include "MessagePeerlist.hpp"

class MessagePeerlistLog : public MessagePeerlist
{
    public:
        MessagePeerlistLog(Message* message) : MessagePeerlist(message) {};
        MessagePeerlistLog(uint16_t qtdPeers, uint32_t channelId, uint32_t nowtime);
        vector<int> GetHeaderValuesDataVector(uint16_t qtdPeers, uint32_t channelId, uint32_t nowtime);
        vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif /* MESSAGEPEERLISTLOG_HPP_ */
