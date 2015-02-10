#ifndef SUB_CHANNEL_DATA_H
#define SUB_CHANNEL_DATA_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include <time.h>
#include <vector>
#include <map>
#include "PeerData.hpp"
#include "Strategy/Strategy.hpp"

#define CHANNEL_LIFE 50
#define RENEW_SERVER 90

using namespace std;

/**
* ECM- esta classe implementa o sub canal
* 01-02-2015
*/
class SubChannelData
{
    public:
 	    SubChannelData(unsigned int channelId = 0, unsigned int channelIdSub = 0, Peer* serverPeer_Sub = NULL,
 	    		       int clife = CHANNEL_LIFE, int rnew = RENEW_SERVER);

        Peer* GetServer_Sub();
        void CheckActivePeers();

        int GetChannelLife();
        void SetChannelLife(int v);
        void DecChannelLif();

        int GetReNewServerSub();
        void SetReNewServerSub(int v);
        void DecReNewServerSub();

        bool GetMesclando();
        void SetMesclando(bool mesclar);

        void PrintPeerList(map<string, PeerData>* peerList_Master);


        unsigned int GetchannelId_Sub();

        FILE* GetPerformanceFile();
        FILE* GetOverlayFile();

    private:

        unsigned int channelId_Master;
        unsigned int channelId_Sub;
        Peer* serverPeer_Sub;
        bool mesclando;

        /* controla o tempo de vida
         * do sub canal até mesclar
         */
        int channelLife;

        /* controla o tempo que o serverSub
         * não poderá ser selecionando novamente
         * como serverSub de outro canal
         * Com isso, a rede pode se estabilizar
         * ao remover um subcanal
         */
        int reNewServerSub;

        time_t creationTime;

        FILE* performanceFile;
        FILE* overlayFile;
};


class SubChannelCandidateData
{
    public:
		SubChannelCandidateData(ServerAuxTypes serverState = NO_SERVER_AUX, bool peerWaitInform = false);

		ServerAuxTypes GetState();
        void SetState(ServerAuxTypes serverState);

		bool GetPeerWaitInform();
        void SetPeerWaitInfor(bool peerWaitInform);

    private:

        ServerAuxTypes serverState;
        bool peerWaitInform;
};

#endif // SUB_CHANNEL_DATA_H

