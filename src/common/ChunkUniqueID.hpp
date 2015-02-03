#ifndef CHUNKUNIQUEID_H
#define CHUNKUNIQUEID_H

#include <iostream>
#include <stdint.h>
#define STD_BUFFERSIZE 1600

using namespace std;

class ChunkUniqueID
{
    public:
        ChunkUniqueID(uint32_t cycle = 0, uint16_t pos = 0, uint16_t size = STD_BUFFERSIZE);
        ChunkUniqueID(const ChunkUniqueID &chunkUID) : position(chunkUID.position), cycle(chunkUID.cycle), bufferSize(chunkUID.bufferSize) { };
        uint16_t GetPosition() const;
        uint32_t GetCycle() const;
        uint16_t GetBufferSize() const;
        void Set(uint32_t cycle, uint16_t pos);
        void SetPosition(uint16_t);
        void SetCycle(uint32_t);
        void SetBufferSize(uint32_t);
        void IncID();
        bool operator==(const ChunkUniqueID param) const;
        bool operator!=(const ChunkUniqueID param) const;
        bool operator<(const ChunkUniqueID param) const;
        bool operator<=(const ChunkUniqueID param) const;
        bool operator>(const ChunkUniqueID param) const;
        bool operator>=(const ChunkUniqueID param) const;
        void operator++(int);
        ChunkUniqueID operator-(ChunkUniqueID param);
        ChunkUniqueID operator-(int);
        
        friend std::ostream& operator<<(std::ostream& os, const ChunkUniqueID& c);
    private:
        uint16_t position;
        uint32_t cycle;
        uint16_t bufferSize;
};
#endif // CHUNKUNIQUEID_H