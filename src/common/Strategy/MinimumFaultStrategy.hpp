#ifndef MINIMUMFAULTSTRATEGY_H
#define MINIMUMFAULTSTRATEGY_H

#include "Strategy.hpp"
#include "RandomStrategy.hpp"

class MinimumFaultStrategy: public Strategy
{
    private:

	    void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity,
	    		         unsigned int minimalBandwidthOut){}
        void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity)
        {
            //Randomize peers first in order to obtain a different peer everytime PendingRequests metric draw
            Strategy* random = new RandomStrategy();
            random->Execute(peers, srcPeer, quantity);
            
            int pos = 0;
            for (int i = 0; i < (int)peers->size(); i++) {
                if ((*peers)[i]->GetPendingRequests() < (*peers)[pos]->GetPendingRequests()) {
                    pos = i;
                }
            }

            PeerData* temp = (*peers)[0];
            (*peers)[0] = (*peers)[pos];
            (*peers)[pos] = temp;
        };
};
#endif
