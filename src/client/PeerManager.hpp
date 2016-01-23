/* Alterações: Eliseu César Miguel
 * 13/01/2015
 * Controle de channelId_Sub
 * 				0 -> peer está na rede principal
 * 				1 -> peer está na rede auxiliar
 * 				2 -> peer está em mesclagem
 */


#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <map>
#include <set>
#include <string.h>
#include <stdio.h>
#include "../common/Defines.hpp"
#include "../common/PeerData.hpp"
#include <boost/thread/mutex.hpp>

#define PEER_ACTIVE_COOLDOWN 3                           //Tempo original = 5. Foi passado para 3 para dar oportunidade a bons parceiros de retornar...
#define SERVER_AUX_SUB_CHANNEL_ID 1                      // ... caso sejam desconectados pela estratégia aleatória que flexibiliza a rede.

class PeerManager
{
	private:

	ServerAuxTypes peerManagerState;                     //Gerencia o estado do servidor auxiliar de redes paralelas. Exclusivo do cliente servidor
	unsigned int maxActivePeersIn;
	unsigned int maxActivePeersOut;

		map<string, PeerData> peerList;                  //todos conhecidos (vizinhos)
	map<string, PeerData> peerListMasterChannel;         //vizinhos do canal principal (em caso de servidor auxiliar). Armazenamento temporário

	set<string> peerActiveIn;                             //ativos que enviam dados a este par
	set<string> peerActiveOut;                            // ativos que recebem dados deste par
	set<string> peerActiveOut_Master;                     // ativos temporários da rede principal enquanto é Servidor Auxiliar

	map<string, unsigned int> peerActiveCooldownIn;       // pares que podem ser removidos por pouca atividade. Não é usado para controle de ...
	map<string, unsigned int> peerActiveCooldownOut;      // ... free-rider pelo SURE. Pode-se evitar enviar um participante a esta lista caso ...
		//Mutexes                                         // ... ele tenha sido removido apenas para flexibilização da rede
		boost::mutex peerListMutex;
    boost::mutex peerActiveMutexIn;
    boost::mutex peerActiveMutexOut;
    boost::mutex peerActiveMutexOut_Master;

    //faz a checagem das duas listas de peerActiveCooldown em checkPeerList
    void CheckpeerActiveCooldown(map<string, unsigned int>* peerActiveCooldown);
    // auxilia na funcao showPeerList
    int showPeerActive(set<string>* peerActive);

    public:
		PeerManager();

	ServerAuxTypes GetPeerManagerState();
	void SetPeerManagerState(ServerAuxTypes peerManagerState);

	unsigned int GetMaxActivePeers(set<string>* peerActive);
		void SetMaxActivePeersIn(unsigned int maxActivePeers);
		void SetMaxActivePeersOut(unsigned int maxActivePeers);

		bool AddPeer(Peer* newPeer); //add na lista de vizinhos

    set<string>* GetPeerActiveIn();
    set<string>* GetPeerActiveOut();
    set<string>* GetPeerActiveOut_Master();
    map<string, unsigned int>* GetPeerActiveCooldown(set<string>* peerActive);

	bool ConnectPeer(string peer, set<string>* peerActive);
	void DisconnectPeer(string peer, set<string>* peerActive);
		void RemovePeer(string peer);//remove na lista de vizinhos

	/*ECM - nao deve implementar as funções:
	 * unsigned int GetPeerActiveSizeIn()
	 * unsigned int GetPeerActiveSizeOut()
	 * porque, assim o método Connector::connector() deverá ser dois, ao contrário de único. */
	unsigned int GetPeerActiveSize(set<string>* peerActive);
	unsigned int GetPeerActiveSizeTotal();  //usada para fornecer o total de pares ativos em In + Out sem repetição. Será removida!!!

	bool IsPeerInPeerList(string peer);

	bool IsPeerActive(string peer,set<string>* peerActive);
		PeerData* GetPeerData(string peer); // a lista de vizinhos é a única que tem os dados do peer

		map<string, PeerData>* GetPeerList();
		boost::mutex* GetPeerListMutex();

	boost::mutex* GetPeerActiveMutex(set<string>* peerActive);

		void CheckPeerList();
        void ShowPeerList();
};
#endif

