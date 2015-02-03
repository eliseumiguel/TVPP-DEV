#ifndef MESSAGE_DATA_H
#define MESSAGE_DATA_H

#include "Message.hpp"
#include "../ChunkUniqueID.hpp"

class MessageData : public Message
{
    public:
        MessageData(Message* message) : Message(message->GetFirstByte()) {};
        MessageData(uint16_t bodySize, uint8_t hop, uint8_t tries, ChunkUniqueID chunkuid, uint32_t generationTime, uint32_t estimatedDuration);
        vector<int> GetHeaderValuesDataVector(uint8_t hop, uint8_t tries, ChunkUniqueID chunkuid, uint32_t generationTime, uint32_t estimatedDuration);
        vector<uint8_t> GetHeaderValuesSizeVector();
        vector<int> GetHeaderValues();
        void SetHop(uint8_t hop);
        void SetTries(uint8_t tries);
        void SetDuration(uint32_t duration);
        void SetMedia(uint8_t* buffer);
};

#endif // MESSAGE_DATA_H    