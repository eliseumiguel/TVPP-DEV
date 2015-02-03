#include "Message.hpp"

Message::Message()
{
    this->firstByte = NULL;
}

Message::Message(uint8_t* firstByte)
{
    this->firstByte = firstByte;
}

Message::~Message()
{
    delete[] this->firstByte;
}

uint8_t* Message::GetFirstByte()
{
    return this->firstByte;
}

uint8_t Message::ShortGetHigh(uint16_t target)
{
    return (uint8_t)((target & 0xFF00) >> 8);
}

uint16_t Message::ShortSetHigh(uint16_t target, uint8_t value)
{
    return (target & 0x00FF) + (value << 8);
}

uint8_t Message::ShortGetLow(uint16_t target)
{
    return (uint8_t)(target & 0x00FF);
}

uint16_t Message::ShortSetLow(uint16_t target, uint8_t value)
{
    return (target & 0xFF00) + (value);   
}

void Message::Encode(uint32_t pos, uint32_t n)
{
    //htons
    return;
}

void Message::Decode(uint32_t pos, uint32_t n)
{
    //ntohs
    return;
}

uint16_t Message::CalcChecksum()
{
    uint16_t checksum = 0;
    uint16_t* firstShort = (uint16_t*)this->firstByte;
    for (uint32_t i = 0; i < GetHeaderSize()/2; i++)
    {
        checksum += firstShort[i];
    }
    
    return checksum;
}

void Message::SetIntegrity()
{
    //[4 + i] is checksum address on the msg
    for (uint8_t i = 0; i < 2; i++) 
        this->firstByte[4 + i] = 0;
    uint16_t checksum = ~CalcChecksum();
    for (uint8_t i = 0; i < 2; i++) 
        this->firstByte[4 + i] = ((uint8_t*)&checksum)[i];
}

bool Message::CheckIntegrity()
{
    uint16_t checksum = this->CalcChecksum()+1;
    if (checksum == 0)
        return true;
    else
    {
        cout << "CheckError: " << checksum << " | ";
        PrintHeader();
        return false;
    }
}

uint8_t Message::GetCodedByteValue(uint32_t byteAddress)
{
    if (byteAddress % 2 == 0)        //HIGH
        return ShortGetHigh(ntohs(((uint16_t*)(this->firstByte+((byteAddress / 2) * 2)))[0]));
    else                             //LOW
        return ShortGetLow(ntohs(((uint16_t*)(this->firstByte+((byteAddress / 2) * 2)))[0]));
}

void Message::SetCodedByteValue(uint32_t byteAddress, uint8_t value)
{
    uint16_t newValue = 0;
    if (byteAddress % 2 == 0)        //HIGH
        newValue = htons(ShortSetHigh(ntohs(((uint16_t*)(this->firstByte+((byteAddress / 2) * 2)))[0]), value));
    else                             //LOW
        newValue = htons(ShortSetLow(ntohs(((uint16_t*)(this->firstByte+((byteAddress / 2) * 2)))[0]), value));
    
    for (uint8_t i = 0; i < 2; i++) 
        this->firstByte[((byteAddress / 2) * 2) + i] = ((uint8_t*)&newValue)[i];
}

vector<int> Message::SplitHeaderValues(uint8_t* fromByte, vector<uint8_t> sizes)
{
    vector<int> out(sizes.size());

    for (uint16_t i = 0, j = 0; i < sizes.size(); i++)
    {
        switch(sizes[i]) 
        {
            case 8:
                out[i] = ((uint8_t*)(fromByte+j))[0];
                break;
            case 16:
                if (j % 2 != 0) 
                    j = 2 * ((j / 2) + 1);
                out[i] = ((uint16_t*)(fromByte+j))[0];
                break;
            case 32:
                if (j % 2 != 0) 
                    j = 2 * ((j / 2) + 1);
                out[i] = ((uint32_t*)(fromByte+j))[0];
                break;
            default:
                break;
        }
        //cout<<"Split: "<<out[i]<<" j:"<<j<<" i:"<<i<<" size:"<<(uint32_t)sizes[i]<<endl;
        j+=sizes[i]/8;
    }
    return out;
}

vector<int> Message::GetMessageHeaderValues()
{
    vector<uint8_t> sizes(4,0);
    sizes[0] = 8;                                                    //OPCODE
    sizes[1] = 8;                                                    //HEADERSIZE
    sizes[2] = 16;                                                   //BODYSIZE
    sizes[3] = 16;                                                   //CHECKSUM
    return SplitHeaderValues(this->firstByte, sizes);
}

uint8_t Message::GetOpcode()
{
    return (uint8_t)GetMessageHeaderValues()[0];
}

uint8_t Message::GetHeaderSize()
{
    return (uint8_t)GetMessageHeaderValues()[1];
}

uint16_t Message::GetBodySize()
{
    return (uint16_t)GetMessageHeaderValues()[2];
}

uint16_t Message::GetChecksum()
{
    return (uint16_t)GetMessageHeaderValues()[3];
}

uint32_t Message::GetSize()
{
    return GetHeaderSize() + GetBodySize();
}

vector<uint8_t> Message::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(0);
    return sizes;
}

vector<int> Message::GetHeaderValues()
{
    return SplitHeaderValues(this->firstByte + MESSAGE_HEADER_SIZE, GetHeaderValuesSizeVector());
}

uint8_t Message::AssembleHeaderValues(uint8_t* fromByte, vector<uint8_t> sizes, vector<int> data) 
{
    uint16_t byteBufferIter = 0;

    for (uint16_t i = 0; i < sizes.size(); i++)
    {
        // Data is encoded in pair of bytes (short), for that reason we avoid 
        // writing a pair beginning at the second half of another pair, so this 
        // condition cares about incrementing the iterator when needed
        if ((sizes[i]!=8) && (byteBufferIter % 2 != 0))
            byteBufferIter = 2 * ((byteBufferIter / 2) + 1);

        for (uint8_t k = 0; k < sizes[i]/8; k++) 
            fromByte[byteBufferIter + k] = ((uint8_t*)&data[i])[k];
        
        byteBufferIter+=sizes[i]/8;
    }
    return byteBufferIter;
}

uint8_t Message::AssembleMessageHeader(uint8_t opcode, uint8_t headerSize, uint16_t bodySize)
{
    vector<uint8_t> sizes(4);
    vector<int> data(4);
    sizes[0] = 8;
    sizes[1] = 8;
    sizes[2] = 16;
    sizes[3] = 16;
    data[0]  = opcode;
    data[1]  = headerSize;
    data[2]  = bodySize;
    data[3]  = 0;
    return AssembleHeaderValues(this->firstByte, sizes, data);
}

uint8_t Message::AssembleHeader(uint8_t opcode, uint8_t headerSize, uint16_t bodySize, vector<int> data)
{
    uint8_t messageHeaderSize = AssembleMessageHeader(opcode, headerSize, bodySize);
    //DEBUG CODE
    uint32_t messageSize = messageHeaderSize + AssembleHeaderValues(this->firstByte + messageHeaderSize, GetHeaderValuesSizeVector(), data);
    //Print(0,messageSize);
    return messageSize;
    //ORIGINAL
    //return messageHeaderSize + AssembleHeaderValues(this->firstByte + messageHeaderSize, GetHeaderValuesSizeVector(), data);
}

void Message::Print(uint64_t from, uint64_t size)
{
    cout<<"Message in Bytes ["<<size-from<<"][";
    for (uint32_t i = from; i < from+size; i++)
    {
        cout<<(uint32_t)firstByte[i]<<" ";
    }
    cout<<"]"<<endl;
}

void Message::PrintHeader()
{
    Print(0,GetHeaderSize());
}
