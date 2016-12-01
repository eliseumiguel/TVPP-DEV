#ifndef CHANNEL_H
#define CHANNEL_H
/* Mudanças aguardadas no Channel:
 *
 *  1) Modificar a forma de ser avisado do Flash Crowd. Solução é tansformar
 *
 *			  enum ChannelModes
				{
					NULL_MODE               	=0x00,          // Usado para desconsiderar esse campo em mensagesn ao Channel
					MODE_NORMAL             	=0x01,          // Modo Normal
					MODE_FLASH_CROWD        	=0x02,          // Modo que ativa os sub-canais
					MODE_FLASH_CROWD_MESCLAR	=0X03,
				};

				em:
*
*			  enum ChannelModes
				{
					NULL_MODE               	=0x00,          // Usado para desconsiderar esse campo em mensagesn ao Channel
					MODE_NORMAL             	=0x01,          // Modo Normal
					MODE_FLASH_CROWD_E2        	=0x02,          // flash crowd Estratégia 2
					MODE_FLASH_CROWD_E3     	=0X03,          // flash crowd Estratégia 3
					.
					.
					.
					MODE_FLASH_CROWD_EN         =0X0N           // flash crowd Estratégia N
				};
Com isso, a estratégia é passada diretamente ao servidor auxiliar. Importante é lembrar que o atributo bool mesclarRedes é local.
Ele deve ser configurado pela estratégia ou deverá ser eliminado. Uma solução é o campo mesclarRedes ser configurado na estratégia base
assim o channel precisa conhecer apenas a estratégia base, enquanto os servidores auxiliares poderão ver a estratégia derivada.
 *
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <time.h>
#include <vector>
#include <map>
#include <set>
#include <stdlib.h>             // para gerar numeros aleatorios
#include <time.h>               // para gerar a 'random seed' baseada no tempo
#include "PeerData.hpp"
#include "Sub-Channel-Data.hpp"
#include "Strategy/Strategy.hpp"

using namespace std;

/**
* This class implements the Channel abstraction
* ECM-mudanças para que o canal perimta subcanais.
* Com isso, um par do canal poderá ser servidor auxiliar de um sub canal
*/
class SubChannelServerAuxData;
class SubChannelCandidateData;

class Channel
{
    public:
        /**
        * Constructor
        * @param idServer server ip and port (ip:port)
        */
        Channel(unsigned int channelId = 0, Peer* serverPeer = NULL,
        		unsigned int maxSubChannel = 0,
        		unsigned int maxServerAuxCandidate = 0,
        		unsigned int maxPeerInSubChannel = 0,
        		unsigned int sizeCluster = 1,
				bool modeFlasCrowdSemSubChannel = false,
        		bool mesclar = false);

        ChunkUniqueID GetServerNewestChunkID();
		void SetServerNewestChunkID(ChunkUniqueID serverNewestChunkID);
        int GetServerEstimatedStreamRate();
		void SetServerEstimatedStreamRate(int serverEstimatedStreamRate);
		
        Peer* GetServer();
        Peer* GetPeer(Peer* peer);
		bool HasPeer(Peer* peer);
		void AddPeer(Peer* peer);
		void AddPeerMasterChannel(Peer* peer);

        void RemovePeer(Peer* peer);
		void RemovePeer(string peerID);
		PeerData& GetPeerData(Peer* peer);
        time_t GetCreationTime();

        void SetChannelMode(ChannelModes channelMode, bool TIMEMERGE = false);
        ChannelModes GetChannelMode();

        bool GetServerSubWaitInform(Peer* peer);
        void SetServerSubWaitInform(Peer* peer, bool waiting);

        bool GetWaitServerList(Peer* peer);
        void SetWaitServerList(Peer* peer, bool waitServerList);

        ServerAuxTypes GetServerSubNewMode (Peer* peer);

        void SetmaxPeer_ChannelSub(int unsigned maxpeerChannelSub);

        vector<PeerData*> MakeServerAuxList(); //Usado para informar os peer da rede principal quem são os serverAux
        vector<PeerData*> SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity,bool virtualPeer, uint8_t minimumBandwidth, uint8_t minimumBandwidth_FREE, bool separatedFreeOutList);
        unsigned int GetPeerListSize(); //usada apenas pelo bootstrap em HHTLog

        unsigned int GetPeerListSizeChannel_Sub(int channelId_Sub); //retorna quantos pares estão em um subcanal
        unsigned int GetTotalChannel_Sub(); //retorna quantos subcanis existem

        void CheckActivePeers();
        void PrintPeerList();
        void analizePeerToBeServerAux(Peer* source);

        vector<FILE*> GetPerformanceFile(Peer* srcPeer);
        vector<FILE*> GetOverlayFile(Peer* srcPeer);
		
		friend bool operator<(const Channel &a, const Channel &b) {return a.channelId<b.channelId;};
		
    private:
        unsigned int channelId;
        unsigned int sizeCluster; //ECM usado para definir o tamanho do cluster se servidores auxiliares
        Peer* serverPeer; 
        map<string, PeerData> peerList;  //ECM Em PeerData existe o campo channelID_Sub que informa o subcanal do peer

        //ECM ***
        boost::mutex* channel_Sub_List_Mutex;
        boost::mutex* channel_Sub_Candidates_Mutex;

        map<string, SubChannelServerAuxData> channel_Sub_List;      //ECM manage Server_Aux active in Sub_Channel
        map<string, SubChannelCandidateData> server_Sub_Candidates; //ECM manage Server_Aux candidates to be Server_Aux active.

        map<string, SubChannelServerAuxData>::iterator posInsertSubChannel; //ECM interator auxiliar para inserir peer em subChannel
        bool modeFlasCrowdSemSubChannel;                             //ECM usado para flash crowd com subnais não isolados.

        ChannelModes channelMode;
        unsigned int maxPeerInSubChannel;
        unsigned int maxSubChannel;
        unsigned int maxServerAuxCandidate;
        bool mesclarRedes;
        bool GenerateAllLogs;


        bool AddPeerChannel(Peer* peer);
        //bool Create_New_ChannelSub();     //ECM antigo usado para manusear preenchimento de subChannel por demanda.
        void Create_All_ChannelSub();       //ECM Usado para manusear preenchimento de subChannel em paralelos

        bool allsubChannelFull();
        void Remove_AllChannelSub();
        bool Finishing_Server (string* server);
        void Remove_ChannelSub(const string* source, bool mesclar = false);

        void CheckServerSubState();

        void printChannelProfile(); //ECM método auxiliar para testes
        //*** ECM

        ChunkUniqueID serverNewestChunkID;
        int serverEstimatedStreamRate;

		time_t creationTime;

        vector<FILE*> vPerformanceFile;
        vector<FILE*> vPoverlayFile;

        FILE* performanceFile_Master;
        FILE* poverlayFile_Master;

        FILE* performanceFile_Total;
        FILE* poverlayFile_Total;

};


#endif // CHANNEL_H

