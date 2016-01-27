/* ECM 25-01-2015.
 * As classes SubChannelServerAuxData e SubChannelCandidateData poderiam agregar todos os campos e métodos em apenas uma classe.
 * A decisão por separá-las deu-se para organizar as listas de servidores auxiliares candidatos disponíveis e a lista de servidores auxliares ativos.
 * Desta forma, o channel gerencia com mais facilidade o estado de cada servidor Auxiliar em listas distintas.
 */






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

using namespace std;

/**
* Manage the Server_Aux active list
*/
class SubChannelServerAuxData
{
    public:
 	    SubChannelServerAuxData(unsigned int channelId = 0, unsigned int channelIdSub = 0, Peer* serverPeer_Sub = NULL, bool gSubLogs = false);

        Peer* GetServer_Sub();

        void PrintPeerList(map<string, PeerData>* peerList_Master);

        bool Finishing_Server (PeerData* server);

        unsigned int Get_ServerAuxChannelId_Sub();

        FILE* GetPerformanceFile();
        FILE* GetOverlayFile();

    private:

        /* ECM All common peer in the master channel set their channelId_Sub in PeerData Class.
         * However, each Server_Aux are in two networks master and a one sub_Channel. This way, becomes necessary
         * that Server_Aux PeerData set the master's channelID on field channelId_Sub to permit the server_aux keeps
         * its relationship in master network. That is why only for Server_Aux' ID_Sub_channel is configured in a
         * separate structure. About the Server_Aux, PeerData channelId_Sub is equal here ServerAux_ChannelId_Master,
         * that only locally used
         */

        unsigned int ServerAux_ChannelId_Master;
        unsigned int ServerAux_ChannelId_Sub;

        Peer* serverPeer_Sub;
        bool GenerateSubLogs;

        time_t creationTime;

        FILE* performanceFile;
        FILE* overlayFile;
};

/*
 * Manage the Server_Aux candidates list
 */
class SubChannelCandidateData
{
    public:
		SubChannelCandidateData(ServerAuxTypes serverState = NO_SERVER_AUX, bool peerWaitInform = false);

		ServerAuxTypes GetState();
        void SetState(ServerAuxTypes serverState);

		bool GetPeerWaitInform();
        void SetPeerWaitInform(bool peerWaitInform);


    private:

        ServerAuxTypes serverState;
        bool peerWaitInform;
};

#endif // SUB_CHANNEL_DATA_H

