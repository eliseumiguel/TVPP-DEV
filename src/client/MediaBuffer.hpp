#ifndef MEDIABUFFER_H_INCLUDED
#define MEDIABUFFER_H_INCLUDED

#include <iostream>
#include <boost/dynamic_bitset.hpp>
#include <netinet/in.h>

#include "../common/Messages/MessageData.hpp"
  
class MediaBuffer
{
    public:
        MediaBuffer(uint16_t pos);
        void Insert(MessageData* chunkMessage, bool available = 1);
        uint32_t GetID(uint16_t pos);
        uint16_t GetChunkSize(uint16_t pos);
        uint8_t GetHop(uint16_t pos);
        uint8_t GetTries(uint16_t pos);
        uint16_t GetSize();
        uint32_t GetDuration(uint16_t pos);
        boost::dynamic_bitset<> GetMap();
        void SetAvailable(uint16_t pos);
        void UnsetAvailable(uint16_t pos);
        bool Available(uint16_t pos);
        void SetDuration(uint16_t pos, uint32_t);
        void SetTries(uint16_t pos, uint8_t tries);
        MessageData* operator[](uint16_t);
    private:
        MessageData **buffer;    
        uint16_t *chunkSize; // == Total Size: Header + Data
        uint16_t *headerSize;
        uint8_t *hop;
        uint8_t *tries;
        uint32_t *duration;
        uint32_t *id;
        uint16_t bufferSize;
        uint16_t maxBufferSize;
        boost::dynamic_bitset<> map;     
};
#endif // MEDIABUFFER_H_INCLUDED
