/*
 * RandomStrategyWhitoutPoorBand.h
 *
 *  Created on: 15 de jul de 2016
 *      Author: eliseu
 */

#ifndef RANDOMSTRATEGYWHITOUTPOORBAND_H
#define RANDOMSTRATEGYWHITOUTPOORBAND_H

#include "Strategy.hpp"
#include "RandomStrategy.hpp"
#include <time.h>

class RandomStrategyWhitoutPoorBand: public Strategy
{
    private:

		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity, unsigned int minimalBandwidthOut){
			vector<PeerData*> peersAUX;
			for (unsigned int i=0; i < peers->size(); i++)
			{
				cout<<"Peer "<<((*peers)[i]->GetPeer())->GetID()<<" tem lista out size = "<<(*peers)[i]->GetSizePeerListOutInformed()<<endl;
				if (((*peers)[i]->GetSizePeerListOutInformed() >= (int)minimalBandwidthOut) || ((*peers)[i]->GetSizePeerListOutInformed() < 0))
					peersAUX.push_back((*peers)[i]);

			}

			*peers = peersAUX;

			RandomStrategy* randomPeers = new RandomStrategy();
			randomPeers->Execute(&(*peers), srcPeer, quantity);
		}
		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity) {}
};
#endif
