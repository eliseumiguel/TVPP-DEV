#ifndef MESSAGE_PEERLIST_H
#define MESSAGE_PEERLIST_H

#include "Message.hpp" 
#include "../Peer.hpp"
#include <boost/lexical_cast.hpp>

class MessagePeerlist : public Message
{
    protected:
        //Attributes
        uint16_t peersAdded;
        uint16_t qtdPeers;
        //Methods
        void WriteIpStringToArray(string ip, uint8_t* arrayBegin);
    public:
        MessagePeerlist() : Message() {};
        MessagePeerlist(Message* message) : Message(message->GetFirstByte()) {};
        MessagePeerlist(PeerlistTypes peerlistType, uint16_t qtdPeers);
        vector<int> GetHeaderValuesDataVector(PeerlistTypes peerlistType, uint16_t qtdPeers);
        vector<uint8_t> GetHeaderValuesSizeVector();
        void AddPeer(Peer* peer);
        Peer* GetPeer(uint16_t index);
};

#endif // MESSAGE_PEERLIST_H    
