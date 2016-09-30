/*
 * RandomStrategyWhitoutPoorBand.h
 *  Com esta estratégia, o par pode fazer a escolha aleatória de
 *  novos parceiros mas considerando o limite inferior de banda
 *  out que o parceiro oferece.
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

		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity,
				         unsigned int minimalBandwidthOut){
			vector<PeerData*> peersAUX;
			for (unsigned int i=0; i < peers->size(); i++)
			{
				if (((*peers)[i]->GetSizePeerListOutInformed() >= (int)minimalBandwidthOut) ||
				    ((*peers)[i]->GetSizePeerListOutInformed() < 0))
					peersAUX.push_back((*peers)[i]);
			}

			*peers = peersAUX;

			RandomStrategy* randomPeers = new RandomStrategy();
			randomPeers->Execute(&(*peers), srcPeer, quantity);
		}
		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity) {}
};

class RandomStrategyWhitoutPoorBand_FREE: public Strategy
{
    private:

		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity,
				         unsigned int minimalBandwidthOut){
			vector<PeerData*> peersAUX;
			for (unsigned int i=0; i < peers->size(); i++)
			{
				if (((*peers)[i]->GetSizePeerListOutInformed_FREE() >= (int)minimalBandwidthOut) ||
				    ((*peers)[i]->GetSizePeerListOutInformed() < 0))
					peersAUX.push_back((*peers)[i]);
			}

			*peers = peersAUX;

			RandomStrategy* randomPeers = new RandomStrategy();
			randomPeers->Execute(&(*peers), srcPeer, quantity);
		}
		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity) {}
};

#endif
