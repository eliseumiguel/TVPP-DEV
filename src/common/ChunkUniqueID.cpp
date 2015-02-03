#include "ChunkUniqueID.hpp"

ChunkUniqueID::ChunkUniqueID(uint32_t cycle, uint16_t pos, uint16_t size)
{
    this->position = pos;
    this->cycle = cycle;
    this->bufferSize = size;
}

uint16_t ChunkUniqueID::GetPosition() const
{
    return position;
}

uint32_t ChunkUniqueID::GetCycle() const
{
    return cycle;
}

uint16_t ChunkUniqueID::GetBufferSize() const
{
    return bufferSize;
}

void ChunkUniqueID::Set(uint32_t cycle, uint16_t pos)
{
    this->position = pos;
    this->cycle = cycle;
}        

void ChunkUniqueID::SetPosition(uint16_t pos)
{
    this->position = pos;
}

void ChunkUniqueID::SetCycle(uint32_t cycle)
{
    this->cycle = cycle;
}

void ChunkUniqueID::SetBufferSize(uint32_t size)
{
    this->bufferSize = size;
}

void ChunkUniqueID::IncID()
{
    this->position++;
    if (this->position >= this->bufferSize)
    {
        this->position = 0;
        this->cycle++;
    }
}

bool ChunkUniqueID::operator==(const ChunkUniqueID param) const
{
    return ((this->cycle == param.GetCycle()) && (this->position == param.GetPosition())); 
}

bool ChunkUniqueID::operator!=(const ChunkUniqueID param) const
{
    return !((this->cycle == param.GetCycle()) && (this->position == param.GetPosition())); 
}

bool ChunkUniqueID::operator<(const ChunkUniqueID param) const
{
    if (this->cycle < param.GetCycle())
        return true;
    else if ((this->cycle == param.GetCycle()) && (this->position < param.GetPosition()))
        return true;
    return false;    
}

bool ChunkUniqueID::operator<=(const ChunkUniqueID param) const
{
    if (this->cycle < param.GetCycle())
        return true;
    else if ((this->cycle == param.GetCycle()) && (this->position < param.GetPosition()))
        return true;
    return ((this->cycle == param.GetCycle()) && (this->position == param.GetPosition()));
}

bool ChunkUniqueID::operator>(const ChunkUniqueID param) const
{
    if (this->cycle > param.GetCycle())
        return true;
    else if ((this->cycle == param.GetCycle()) && (this->position > param.GetPosition()))
        return true;
    return false;    
}

bool ChunkUniqueID::operator>=(const ChunkUniqueID param) const
{
    if (this->cycle > param.GetCycle())
        return true;
    else if ((this->cycle == param.GetCycle()) && (this->position > param.GetPosition()))
        return true;
    return ((this->cycle == param.GetCycle()) && (this->position == param.GetPosition()));
}

void ChunkUniqueID::operator++(int unused)
{
    IncID();
}

ChunkUniqueID ChunkUniqueID::operator-(ChunkUniqueID param)
{
    int64_t newCycle = this->GetCycle() - param.GetCycle();
    if (newCycle < 0)
      newCycle = 0;
    ChunkUniqueID ret((uint32_t)newCycle, this->GetPosition(),this->GetBufferSize());
    return ret - param.GetPosition();
}

ChunkUniqueID ChunkUniqueID::operator-(int param)
{
    ChunkUniqueID ret(*this);
    uint16_t cycleToSub = param / ret.GetBufferSize();
    uint16_t posToSub = param % ret.GetBufferSize();
    uint16_t newPos = ret.GetPosition();
    if (newPos < posToSub)
    {
        cycleToSub++;
        newPos += ret.GetBufferSize();
    }
    newPos -= posToSub;
    if(ret.GetCycle() > cycleToSub)
        ret.SetCycle(ret.GetCycle()-cycleToSub);
    else
        ret.SetCycle(0);
    ret.SetPosition(newPos);
    return ret;
}

std::ostream& operator<<(std::ostream& os, const ChunkUniqueID& c)
{
   os << c.GetCycle() << ":" << c.GetPosition();
   return os;
}