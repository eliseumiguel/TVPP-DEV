#include "MessageError.hpp"

MessageError::MessageError(ErrorTypes error)
{
    vector<int> data = GetHeaderValuesDataVector(error);
	
	firstByte = new uint8_t[MESSAGE_ERROR_HEADER_SIZE];
	Message::AssembleHeader(OPCODE_ERROR, MESSAGE_ERROR_HEADER_SIZE, 0, data);
}

vector<int> MessageError::GetHeaderValuesDataVector(ErrorTypes error) 
{
    vector<int> data(1);
    data[0] = error;
    return data;
}

vector<uint8_t> MessageError::GetHeaderValuesSizeVector()
{
	vector<uint8_t> sizes(1);
	sizes[0] = 8;													//ERROR FLAG
    return sizes;
}