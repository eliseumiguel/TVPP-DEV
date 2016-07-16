#ifndef FIRSTSTRATEGY_H
#define FIRSTSTRATEGY_H

#include "Strategy.hpp"

class NullStrategy: public Strategy
{
	private:
		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity){}
        void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity, unsigned int minimalBandwidthOut){}

};
#endif
