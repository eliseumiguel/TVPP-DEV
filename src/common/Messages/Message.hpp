// MESSAGE!
//| OPCODE | HEADERSIZE | BODY_SIZE | CHECKSUM | HEADER | BODY |
//|   1    |      1     |     2     |     2    |   ?    |   ?  | TOTAL: 6 Bytes

// CHANNEL MESSAGE
//| OPCODE | HEADERSIZE | BODY_SIZE | CHECKSUM | CHANNELCODE | PUNCH | VERSION | EXTPORT | CHANNELID | NOWTIME | SERVERAUX |CHANNELSTATE | PEEROUT | PEEROUT_FREE|
//|   1    |      1     |     2     |     2    |      1      |   1   |    2    |    2    |     4     |     4   |     1     |     1       |    2    |      2      |TOTAL: 26 Bytes

// PEERLIST MESSAGE                                         ?     ?
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  EXTPORT | CHUNKGUID | QTDPEERS | PEERLIST |  
//|   1    |     1      |     2    |     2    |     2    |  4  |  2  |    2     |  6/peer  |  TOTAL: 16 Bytes + 6*QTDPEERS

// PING MESSAGE
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  MAXOUT  |MAXOUTFREE  | PINGCODE | PEERMODE | CHUNKGUID |
//|   1    |     1      |     2    |     2    |     2    |     2      |     1    |     1    |  4  |  2  | TOTAL: 18

// PING BOOT MESSAGE
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  MAXOUT  |MAXOUTFREE  | PINGCODE | PEERMODE | CHUNKGUID | STREAMRATE |CHANNELID |
//|   1    |     1      |     2    |     2    |     2    |     2      |     1    |     1    |  4  |  2  |      4     |   0|4    | TOTAL: 26

// PING CHUNKMAP MESSAGE
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  OUTLIST  |  PINGCODE | PEERMODE | CHUNKGUID |    BITMAP    |
//|   1    |     1      |     2    |     2    |      2    |     1     |     1    |  4  |  2  | BUFFERSIZE/8 | TOTAL: 16 + (BUFFERSIZE/8) Bytes

// ERRO MESSAGE 
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  ERRORCODE | X | 
//|   1    |     1      |     2    |     2    |      1     | 1 | TOTAL: 8 Bytes

// SERVER SUB MESSAGE
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  STATECODE | X |
//|   1    |     1      |     2    |     2    |      1     | 1 | TOTAL: 8 Bytes

// CHANNEL STATE MESSAGE
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  STATECODE | X |
//|   1    |     1      |     2    |     2    |      1     | 1 | TOTAL: 8 Bytes

// REQUEST MESSAGE 
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  CHUNKGUID |
//|    1   |     1      |     2    |     2    |   4  |  2  |  TOTAL: 12 Bytes

// DATA MESSAGE        
//| OPCODE | HEADERSIZE | BODYSIZE | CHECKSUM |  HOP | TRIES | CHUNKGUID | TIME_STAMP | DATA |  **************************************
//|   1    |     1      |     2    |     2    |   1  |   1   |  4  |  2  |     4      |   x  |  TOTAL: 18 Bytes + DATA  **************/ 

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>

#include "../Defines.hpp" 

using namespace std;

class Message
{
    private:
        Message(const Message&) {};
        void operator= (const Message&) {};
    protected:
        //Attributes
        uint8_t* firstByte;
        //Functions
        uint8_t ShortGetHigh(uint16_t target);
        uint16_t ShortSetHigh(uint16_t target, uint8_t value);
        uint8_t ShortGetLow(uint16_t target);
        uint16_t ShortSetLow(uint16_t target, uint8_t value);
        uint8_t GetCodedByteValue(uint32_t byteAddress);
        void SetCodedByteValue(uint32_t byteAddress, uint8_t value);
        uint16_t CalcChecksum();
        vector<int> GetMessageHeaderValues();
        vector<int> SplitHeaderValues(uint8_t* fromByte, vector<uint8_t> sizes);
        virtual vector<uint8_t> GetHeaderValuesSizeVector();
        uint8_t AssembleHeaderValues(uint8_t* fromByte, vector<uint8_t> sizes, vector<int> data);
        uint8_t AssembleMessageHeader(uint8_t opcode, uint8_t headerSize, uint16_t bodySize);
        uint8_t AssembleHeader(uint8_t opcode, uint8_t headerSize, uint16_t bodySize, vector<int> data);
        void Print(uint64_t from, uint64_t size);
    public:
        Message();
        Message(uint8_t* firstByte);
        virtual ~Message();
        uint8_t* GetFirstByte();
        void Encode(uint32_t pos, uint32_t n);
        void Decode(uint32_t pos, uint32_t n);
        void SetIntegrity();
        bool CheckIntegrity();
        uint8_t GetOpcode();
        uint8_t GetHeaderSize();
        uint16_t GetBodySize();
        uint16_t GetChecksum();
        uint32_t GetSize();
        vector<int> GetHeaderValues();
        void PrintHeader();
};

#endif // MESSAGE_H
