/*
 * RandomStrategyWhitoutPoorBand.h
 *
 *  Created on: 15 de jul de 2016
 *      Author: eliseu
 */

#ifndef RANDOMSTRATEGYWHITOUTPOORBAND_H
#define RANDOMSTRATEGYWHITOUTPOORBAND_H

#include "Strategy.hpp"
#include <time.h>

class RandomStrategyWhitoutPoorBand: public Strategy
{
    private:

		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity, unsigned int minimalBandwidthOut){
			cout<<"fazendo a estratégia"<<endl;
			vector<PeerData*> peersAUX;
			for (unsigned int i=0; i < peers->size(); i++)
			{
				if (((*peers)[i]->GetSizePeerListOutInformed() >= (int)minimalBandwidthOut) || ((*peers)[i]->GetSizePeerListOutInformed() < 0))
				{
					cout<<"Peer "<<((*peers)[i]->GetPeer())->GetID()<<" tem lista out size = "<<(*peers)[i]->GetSizePeerListOutInformed()<<endl;
					peersAUX.push_back((*peers)[i]);
				}
			}

			*peers = peersAUX;
			shuffle(*peers);
		}

		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity)
        {
			cout<<"acaba de cometer um grande erro"<<endl;
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
