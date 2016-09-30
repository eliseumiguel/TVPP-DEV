#include "MessagePeerlistShare.hpp"

MessagePeerlistShare::MessagePeerlistShare(uint16_t qtdPeers, string externalIp, uint16_t externalPort, ChunkUniqueID serverTipChunkUId, uint32_t serverStreamRate, uint32_t channelCreationTime, uint32_t nowTime, uint32_t clientTime, uint32_t bootID, Opcodes opcodeMessage)
{
    vector<int> data = GetHeaderValuesDataVector(qtdPeers, externalIp, externalPort, serverTipChunkUId, serverStreamRate, channelCreationTime, nowTime, clientTime, bootID);

    firstByte = new uint8_t[MESSAGE_PEERLIST_SHARE_HEADER_SIZE + qtdPeers*6];
    this->peersAdded = 0;
    this->qtdPeers = qtdPeers;
    Message::AssembleHeader(opcodeMessage, MESSAGE_PEERLIST_SHARE_HEADER_SIZE, qtdPeers*6, data);
}

vector<int> MessagePeerlistShare::GetHeaderValuesDataVector(uint16_t qtdPeers, string externalIp, uint16_t externalPort, ChunkUniqueID serverTipChunkUId, uint32_t serverStreamRate, uint32_t channelCreationTime, uint32_t nowTime, uint32_t clientTime, uint32_t bootID)
{
    vector<int> data = MessagePeerlist::GetHeaderValuesDataVector(PEERLIST_SHARE, qtdPeers);
	int prevSize = data.size();
	data.resize(prevSize + 12); //modificado 11 -> 12
    //IP to byte[] transformation
    uint8_t ipArray [5];
    WriteIpStringToArray(externalIp, &ipArray[0]);
    for (uint8_t octetI = 0; octetI < 4; octetI++)
    {
        data[prevSize + octetI] = ipArray[octetI];//(uint8_t)boost::lexical_cast<uint16_t>(octet);
    }
	data[prevSize + 4] = externalPort;
    data[prevSize + 5] = serverTipChunkUId.GetCycle();
    data[prevSize + 6] = serverTipChunkUId.GetPosition();
    data[prevSize + 7] = serverStreamRate;
    data[prevSize + 8] = channelCreationTime;
    data[prevSize + 9] = nowTime;
    data[prevSize + 10] = clientTime;
    data[prevSize + 11] = bootID;
    return data;
}

vector<uint8_t> MessagePeerlistShare::GetHeaderValuesSizeVector()
{
	vector<uint8_t> sizes = MessagePeerlist::GetHeaderValuesSizeVector();
	int prevSize = sizes.size();
    sizes.resize(prevSize + 12);  //mudou de 11 -> 12
    sizes[prevSize + 0] = 8;                                                     //EXTIP.Oct1
    sizes[prevSize + 1] = 8;                                                     //EXTIP.Oct2
    sizes[prevSize + 2] = 8;                                                     //EXTIP.Oct3
    sizes[prevSize + 3] = 8;                                                     //EXTIP.Oct4
    sizes[prevSize + 4] = 16;                                                    //EXTPORT
    sizes[prevSize + 5] = 32;                                                    //CHUNKMAP.CHUNKUID.Cycle
    sizes[prevSize + 6] = 16;                                                    //CHUNKMAP.CHUNKUID.Position
    sizes[prevSize + 7] = 32;                                                    //STREAMRATE
    sizes[prevSize + 8] = 32;                                                    //CHANNELCREATIONTIME
    sizes[prevSize + 9] = 32;                                                    //BOOTTIME
    sizes[prevSize + 10] = 32;                                                   //CLIENTTIME
    sizes[prevSize + 11] = 32;                                                   //ECM BOOTSTRAP_ID
    return sizes;
}
