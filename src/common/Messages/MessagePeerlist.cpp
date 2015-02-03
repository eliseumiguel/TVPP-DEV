#include "MessagePeerlist.hpp"

MessagePeerlist::MessagePeerlist(PeerlistTypes peerlistType, uint16_t qtdPeers)
{
    vector<int> data = GetHeaderValuesDataVector(peerlistType, qtdPeers);

    firstByte = new uint8_t[MESSAGE_PEERLIST_HEADER_SIZE + qtdPeers*6];
    this->peersAdded = 0;
    this->qtdPeers = qtdPeers;
    Message::AssembleHeader(OPCODE_PEERLIST, MESSAGE_PEERLIST_HEADER_SIZE, qtdPeers*6, data);
}

vector<int> MessagePeerlist::GetHeaderValuesDataVector(PeerlistTypes peerlistType, uint16_t qtdPeers)
{
    vector<int> data(2);
    data[0] = peerlistType;
    data[1] = qtdPeers;
    return data;
}

vector<uint8_t> MessagePeerlist::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(2);
    sizes[0] = 8;                                                     //PEERLIST.Type
    sizes[1] = 16;                                                    //PEERLIST.Size
    return sizes;
}

void MessagePeerlist::AddPeer(Peer* peer)
{
    if (peersAdded < qtdPeers)
    {
        uint8_t* peerToWrite = firstByte + GetHeaderSize() + peersAdded*6;

        WriteIpStringToArray(peer->GetIP(), peerToWrite);

        uint16_t port = boost::lexical_cast<uint16_t>(peer->GetPort());
        peerToWrite[4] = ShortGetHigh(port);
        peerToWrite[5] = ShortGetLow(port);

        peersAdded++;
    }
}

Peer* MessagePeerlist::GetPeer(uint16_t index)
{
    uint8_t* peerReceived = firstByte + GetHeaderSize() + index*6;
    string ip = boost::lexical_cast<string>((uint32_t)peerReceived[0]) + "." 
        + boost::lexical_cast<string>((uint32_t)peerReceived[1]) + "." 
        + boost::lexical_cast<string>((uint32_t)peerReceived[2]) + "." 
        + boost::lexical_cast<string>((uint32_t)peerReceived[3]);
    string port = boost::lexical_cast<string>(ShortSetLow(ShortSetHigh(0, peerReceived[4]), peerReceived[5]));
    return new Peer(ip, port);
}

void MessagePeerlist::WriteIpStringToArray(string ip, uint8_t* arrayBegin)
{
    for (uint8_t octetI = 0; octetI < 4; octetI++)
    {
        size_t dotI = ip.find('.');
        string octet = ip.substr(0, dotI);
        if (dotI!=string::npos)
        {
            ip = ip.substr(dotI+1);
        }
        arrayBegin[octetI] = (uint8_t)boost::lexical_cast<uint16_t>(octet);
    }
}