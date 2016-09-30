#ifndef RANDOMSTRATEGY_H
#define RANDOMSTRATEGY_H

#include "Strategy.hpp"
#include <time.h>

class RandomStrategy: public Strategy
{
    private:

		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity,	unsigned int minimalBandwidthOut)
		{
			shuffle(*peers);
		}

		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity)
        {
            shuffle(*peers);
        }
        
        template<class c> void shuffle(std::vector<c>& thevec)
        {
            for (unsigned int i = 0; i < thevec.size(); i++)
            {
                int r = i + drand48()*(thevec.size() - i);
                c temp = thevec[i];
                thevec[i] = thevec[r];
                thevec[r] = temp;
            }
        };
};
#endif
