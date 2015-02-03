#include "MediaBuffer.hpp"
using namespace std;

MediaBuffer::MediaBuffer(uint16_t bsize)
{
    this->bufferSize = 0;
    this->maxBufferSize = bsize;
    this->buffer = new MessageData*[maxBufferSize]; 
    this->chunkSize = new uint16_t[maxBufferSize]; //HEADER SIZE + DATA SIZE
    this->headerSize = new uint16_t[maxBufferSize];
    this->hop = new uint8_t[maxBufferSize];
    this->tries = new uint8_t[maxBufferSize];
    this->duration = new uint32_t[maxBufferSize];
    this->id = new uint32_t[maxBufferSize];
    boost::dynamic_bitset<> x(maxBufferSize);
    this->map = x;
    for(int i = 0; i < maxBufferSize; i++)
    {
        this->buffer[i] = NULL;
        this->map[i] = 0;       
    }
}

void MediaBuffer::Insert(MessageData* chunkMessage, bool available)
{   
    vector<int> chunkHeader = chunkMessage->GetHeaderValues();
    uint16_t pos = chunkHeader[3];
    
    MessageData* oldMessage = NULL;
    if (buffer[pos] == NULL) 
        bufferSize++;
    else
    {
        oldMessage = buffer[pos];
    }
    
    buffer[pos] = chunkMessage;
    if (oldMessage) 
        delete oldMessage;

    this->chunkSize[pos] = chunkMessage->GetSize();
    this->headerSize[pos] = chunkMessage->GetHeaderSize();
    this->id[pos] = chunkHeader[2];
    this->hop[pos] = chunkHeader[0];
    this->tries[pos] = chunkHeader[1];
    this->duration[pos] = chunkHeader[4];
    this->map[pos] = available;
}

uint32_t MediaBuffer::GetID(uint16_t pos)
{
    return id[pos];
}

uint16_t MediaBuffer::GetChunkSize(uint16_t pos)
{
    return chunkSize[pos];
}

uint8_t MediaBuffer::GetHop(uint16_t pos)
{
    return hop[pos];
}

uint8_t MediaBuffer::GetTries(uint16_t pos)
{
    return tries[pos];
}

uint16_t MediaBuffer::GetSize()
{
    return bufferSize;
}

uint32_t MediaBuffer::GetDuration(uint16_t pos)
{
    return duration[pos];
}

boost::dynamic_bitset<> MediaBuffer::GetMap()
{     
    return map;
}        

void MediaBuffer::SetAvailable(uint16_t pos)
{
    map[pos] = 1;
}

void MediaBuffer::UnsetAvailable(uint16_t pos)
{
    map[pos] = 0;
}

bool MediaBuffer::Available(uint16_t pos)
{
    if (buffer[pos] != NULL)
        return map[pos];
    else
        return 0;
}

void MediaBuffer::SetDuration(uint16_t pos, uint32_t duration)
{
    this->duration[pos] = duration;
    buffer[pos]->SetDuration(duration);
}

MessageData* MediaBuffer::operator[](uint16_t index)
{
    if (buffer[index] == NULL)
    {
        cout<<"MediaBuffer: NULL pointer access."<<endl;
        //exit(1);
    }
    else if (index < 0 || index > maxBufferSize)
    {
        cout<<"MediaBuffer: Index out of range."<<endl;
        return NULL;
        //exit(1);
    }
    return buffer[index]; 
}