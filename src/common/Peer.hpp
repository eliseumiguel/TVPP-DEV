#ifndef PEER_H
#define PEER_H

#include <string>
#include <iostream>

#define IP_PORT_SEPARATOR ":"

using namespace std;

class Peer
{
    private:        
        string ID;
        string IP;
        string port;
        
        void ConstructorAux(string IP, string port);
        void ResetID();
    public:
        Peer(string IP_port = "");
        Peer(string IP, string port);
        void SetID(string ID);
        void SetIP(string IP);
        void SetPort(string port);
        string GetID();
        string GetIP();
        string GetPort();
        friend bool operator==(const Peer &a, const Peer &b) {return a.ID==b.ID;};
        friend bool operator!=(const Peer &a, const Peer &b) {return a.ID!=b.ID;};
        friend std::ostream& operator<<(std::ostream& os, const Peer& p) {os << p.ID; return os;};
};

#endif