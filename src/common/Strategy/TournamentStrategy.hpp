#ifndef TOURNAMENTSTRATEGY_H
#define TOURNAMENTSTRATEGY_H

#include "Strategy.hpp"
#include "RandomStrategy.hpp"

class TournamentStrategy: public Strategy
{
    private:

        void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity, unsigned int minimalBandwidthOut){}
        void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity)
        {
            int originalQuantity = quantity;
            vector<PeerData*> selectedPeers;
            Strategy* random = new RandomStrategy();
            random->Execute(peers, srcPeer, quantity);
            if ((int)peers->size() < quantity * 2)
                quantity = peers->size();
            else
                quantity *= 2;
            selectedPeers.insert(selectedPeers.begin(),peers->begin(),peers->begin()+quantity);
            
            PeerData* aux;
            for (int i = 0; i < originalQuantity; i++)
            {
                for (int j = originalQuantity+1; j < quantity; j++)
                {
                    if (selectedPeers[i]->GetUploadScore() < selectedPeers[j]->GetUploadScore())
                    {
                        aux = selectedPeers[i];
                        selectedPeers[i] = selectedPeers[j];
                        selectedPeers[j] = aux;
                    }
                }
            }

            delete peers;
            peers = &selectedPeers;
        };
};
#endif
