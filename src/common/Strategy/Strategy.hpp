#ifndef STRATEGY_H
#define STRATEGY_H

#include <vector>
#include <stdlib.h>

#include "../Peer.hpp"
#include "../PeerData.hpp"

class Strategy
{
    public:    
        void Execute(vector<PeerData*>* peers, Peer* srcPeer, int quantity)
        {
            if (!peers->empty())
                SelectPeers(peers, srcPeer, quantity);
        }
    private:
        virtual void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity) = 0;
};

#endif