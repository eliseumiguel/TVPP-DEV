#ifndef PEERDATA_H_INCLUDED
#define PEERDATA_H_INCLUDED

#include <string>
#include <iostream>
#include <boost/dynamic_bitset.hpp>
#include "HeadedBitset.hpp"
#include "Defines.hpp"
#include "Peer.hpp"
#include "ChunkUniqueID.hpp"

using namespace std;

/**
* This class implements the information about the peers
* OBS. Quanto ao channelId_Sub, este pode ter os seguintes valores:
*      channelId_Sub >= 0. O Peer está no canal principal ou em um subcanal quando flash crowd
*      channelId_Sub = -1  O peer está em estado de mesclagem. isso é importante para ele não ser
*                          enviado a um peer que é servidor de sub canal em execução. Um peer em mesclagem
*                          deve obter parceiros na rede principal do canal. Caso contrário, ele entra em outro subcanal
*
*/
class PeerData
{
    public:
        /**
        * @param string
        * @param int Max TTL value
        */
        PeerData(Peer* p = NULL, int ttlIn = TTLIn, int ttlOut = TTLOut, int ttlChannel = TTLChannel, int size = STD_BUFFERSIZE);

        /**
        * @param channelId_Sub usado para criar novo sub canal na classe Channel
        */
        Peer* GetPeer();
        
        /**
        * This method returns the current TTL value
        * @return A string with the
        */
        //ECM

        int GetTTLIn();
        void SetTTLIn(int v);
        void DecTTLIn();
        int GetTTLOut();
        void SetTTLOut(int v);
        void DecTTLOut();

        int GetTTLChannel();
        void SetTTLChannel(int v);
        void DecTTLChannel();

        int GetChannelId_Sub();
        void SetChannelId_Sub( int channelId_Sub);

        void SetSpecialPeer(bool special);
        bool GetSpecialPeer();

        //ECM


        void SetMode(PeerModes mode);
        PeerModes GetMode();
        void SetChunkMap(ChunkUniqueID chunkMapHead, boost::dynamic_bitset<>x);
        bool GetChunkMapPos(int i) const;
        uint32_t GetChunkMapSize() const;
        void IncPendingRequests();
        void DecPendingRequests();
        int  GetPendingRequests();
        void SetDelay(float value);
        float GetDelay();
        void SetUploadScore(int value);
        int  GetUploadScore();
        
		bool GetPeerWaitListServer();
        void SetPeerWaitListServer(bool peerWaitListServer);


        ChunkUniqueID GetChunkMapHead();
        void SetChunkMapHead(ChunkUniqueID chunkMapHead);
       
        friend bool operator==(const PeerData &a, const PeerData &b) {return a.peer==b.peer;};
        friend bool operator!=(const PeerData &a, const PeerData &b) {return a.peer!=b.peer;};
        friend std::ostream& operator<<(std::ostream& os, const PeerData& pd);
        
    private:
        //PeerId
        Peer* peer;
        PeerModes mode;

        //PartnerChunkMap
        HeadedBitset chunkMap;
        
        // Este campo é usado pelo Channel. Quando ele vai entrar em estado de flash crowd, ele informa a todos os peerList quais são os servidores
        // auxliares da rede. Com isso, os servidores auxiliares podem ter privilégios, como não serem removidos da lista de ativos de um participante
        // caso seja sorteado para desconectar pelo disconnect aleatório. Quando essa opção é verdadeira, o bootstrap informa a todos os clientes da
        // rede principal que, em suas listas internas peerList (em peerManage) que o modo do cliente informado é MODE_AUXILIAR_SERVER. Com isso, toda
        // ação enquanto o canal estiver em flash crowd pode ser cautelosa em relação aos servidores auxiliares.
        bool peerWaitListServer;
        bool specialPeer; // Informa ao cliente que alguém de sua lista de parceiros é especial

        //TimeToLive of the partnership or request
        //ECM ****
        int ttlIn;
        int ttlOut;
        int ttlChannel;


        /* channelId_Sub é do tipo int e não unsigned int para permitir
         * que o peer informe o channelId_Sub = -1 que é o estado de
         * mesclagem a um peer qualquer na rede
         */
        int channelId_Sub;

        //**** ECM
        //ECM

        //Peer classification technologies =D
        int uploadScore;
        int pendingRequests;
        int quality;
        float delay;
};
#endif // PEERDATA_H_INCLUDED
