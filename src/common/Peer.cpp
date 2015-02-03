#include "Peer.hpp"

using namespace std;

Peer::Peer(string IP, string port)
{
    ConstructorAux(IP,port);
}

Peer::Peer(string IP_port)
{
    string IP, port;
    size_t pos;
    
    pos = IP_port.find(IP_PORT_SEPARATOR);
    IP = IP_port.substr(0, pos);
    port = IP_port.substr(pos+1);

    ConstructorAux(IP,port);
}

void Peer::ConstructorAux(string IP, string port)
{
    this->IP = IP;
    this->port = port;
    ResetID();
}

void Peer::ResetID()
{
    this->ID = this->IP+IP_PORT_SEPARATOR+this->port;
}

void Peer::SetID(string ID)
{
    this->ID = ID;
}
void Peer::SetIP(string IP)
{
    this->IP = IP;
    ResetID();
}
void Peer::SetPort(string port)
{
    this->port = port;
    ResetID();
}

string Peer::GetID()
{
    return ID;
}
string Peer::GetIP()
{
    return IP;
}
string Peer::GetPort()
{
    return port;
}
