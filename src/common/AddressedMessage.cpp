#include "AddressedMessage.hpp"

AddressedMessage::AddressedMessage(string address, Message* message)
{
    this->address = address;
    this->message = message;
    gettimeofday(&creationTime, NULL);
}

string AddressedMessage::GetAddress()
{
    return address;
}

struct timeval AddressedMessage::GetCreationTime()
{
    return creationTime;
}

float AddressedMessage::GetAge()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    float delay = (now.tv_sec - creationTime.tv_sec);
    delay += (now.tv_usec - creationTime.tv_usec) / 1000000;
    return delay;
}


Message* AddressedMessage::GetMessage()
{
    return message;
}