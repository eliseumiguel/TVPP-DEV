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

        int sizePeerListOutInformed;        // ECM used to store the out quantity the peer can manage
        int sizePeerListOutInformed_FREE;   // ECM lista de pares pobres, quando existir
        
        void ConstructorAux(string IP, string port, int sizePeerListOutInformed, int sizePeerListOutInformed_FREE);
        void ResetID();
    public:
        Peer(string IP_port = "", int sizePeerListOutInformed = -1, int sizePeerListOutInformed_FREE = -1);
        Peer(string IP, string port, int sizePeerListOutInformed = -1, int sizePeerListOutInformed_FREE = -1);
        void SetID(string ID);
        void SetIP(string IP);
        void SetPort(string port);
        string GetID();
        string GetIP();
        string GetPort();
        int GetSizePeerListOutInformed();
        int GetSizePeerListOutInformed_FREE();

        void SetSizePeerListOutInformed(int sizePeerListOutInformed);
        void SetSizePeerListOutInformed_FREE(int sizePeerListOutInformed_FREE);

        friend bool operator==(const Peer &a, const Peer &b) {return a.ID==b.ID;};
        friend bool operator!=(const Peer &a, const Peer &b) {return a.ID!=b.ID;};
        friend std::ostream& operator<<(std::ostream& os, const Peer& p) {os << p.ID; return os;};

};

#endif
