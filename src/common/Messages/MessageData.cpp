#include "MessageData.hpp"

MessageData::MessageData(uint16_t bodySize, uint8_t hop, uint8_t tries, ChunkUniqueID chunkuid, uint32_t generationTime, uint32_t estimatedDuration)
{
    vector<int> data = GetHeaderValuesDataVector(hop, tries, chunkuid, generationTime, estimatedDuration);
    
    firstByte = new uint8_t[MESSAGE_DATA_HEADER_SIZE + bodySize];
    Message::AssembleHeader(OPCODE_DATA, MESSAGE_DATA_HEADER_SIZE, bodySize, data);
}

vector<int> MessageData::GetHeaderValuesDataVector(uint8_t hop, uint8_t tries, ChunkUniqueID chunkuid, uint32_t generationTime, uint32_t estimatedDuration) 
{
    vector<int> data(6);
    data[0] = hop;
    data[1] = tries;
    data[2] = chunkuid.GetCycle();
    data[3] = chunkuid.GetPosition();
    data[4] = generationTime;
    data[5] = estimatedDuration;
    return data;
}

vector<uint8_t> MessageData::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(6);
    sizes[0] = 8;                                                     //HOP
    sizes[1] = 8;                                                     //TRIES
    sizes[2] = 32;                                                    //CHUNKUID.Cycle
    sizes[3] = 16;                                                    //CHUNKUID.Position
    sizes[4] = 32;                                                    //GENERATIONTIMESTAMP
    sizes[5] = 32;                                                    //TIMEDURATION
    return sizes;
}

vector<int> MessageData::GetHeaderValues()
{
    //TODO: Decode Header BEFORE!
    vector<int> headerValues = SplitHeaderValues(this->firstByte + MESSAGE_HEADER_SIZE, GetHeaderValuesSizeVector());
    //TODO: Encode Header AFTER!
    return headerValues;
}

void MessageData::SetHop(uint8_t hop)
{
    SetCodedByteValue(MESSAGE_HEADER_SIZE + 0, hop);
}

void MessageData::SetTries(uint8_t tries)
{
    SetCodedByteValue(MESSAGE_HEADER_SIZE + 1, tries);
}

void MessageData::SetDuration(uint32_t duration)
{
    vector<uint8_t> sizes(1);
    sizes[0] = 32;
    vector<int> data(1);
    data[0] = duration;
    AssembleHeaderValues(this->firstByte + MESSAGE_HEADER_SIZE + 12, sizes, data);
}

void MessageData::SetMedia(uint8_t* buffer)
{
    for (uint16_t i = 0; i < this->GetBodySize(); i++)
        this->firstByte[this->GetHeaderSize() + i] = buffer[i];
}