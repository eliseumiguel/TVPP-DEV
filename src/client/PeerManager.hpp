#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <map>
#include <set>
#include <string.h>
#include <stdio.h>
#include "../common/PeerData.hpp"
#include <boost/thread/mutex.hpp>

#define PEER_ACTIVE_COOLDOWN 5

// Alterações: Eliseu César Miguel
// 13/01/2015
class PeerManager
{
	private:
	unsigned int maxActivePeersIn;
	unsigned int maxActivePeersOut;
		map<string, PeerData> peerList; //todos conhecidos (vizinhos)

	set<string> peerActiveIn; //ativos que enviam dados a este par
	set<string> peerActiveOut; // ativos que recebem dados deste par

	map<string, unsigned int> peerActiveCooldownIn; // pares que podem ser removidos por pouca atividade
	map<string, unsigned int> peerActiveCooldownOut;
		//Mutexes
		boost::mutex peerListMutex;
    boost::mutex peerActiveMutexIn;
    boost::mutex peerActiveMutexOut;
    //faz a checagem das duas listas de peerActiveCooldown em checkPeerList
    void CheckpeerActiveCooldown(map<string, unsigned int>* peerActiveCooldown);
    // auxilia na funcao showPeerList
    int showPeerActive(set<string>* peerActive);

    public:
		PeerManager(); //atua na lista de vizinhos
	unsigned int GetMaxActivePeers(set<string>* peerActive);
		void SetMaxActivePeersIn(unsigned int maxActivePeers);
		void SetMaxActivePeersOut(unsigned int maxActivePeers);

		bool AddPeer(Peer* newPeer); //add na lista de vizinhos

    set<string>* GetPeerActiveIn();
    set<string>* GetPeerActiveOut();
    map<string, unsigned int>* GetPeerActiveCooldown(set<string>* peerActive);

	bool ConnectPeer(string peer, set<string>* peerActive);
	void DisconnectPeer(string peer, set<string>* peerActive);
		void RemovePeer(string peer);//remove na lista de vizinhos

	/*ECM - nao deve implementar as funções:
	 * unsigned int GetPeerActiveSizeIn()
	 * unsigned int GetPeerActiveSizeOut()
	 * porque, assim o método Connector::connector() deverá ser dois, ao contrário de único. */
	unsigned int GetPeerActiveSize(set<string>* peerActive);
	unsigned int GetPeerActiveSizeTotal(); //usada para fornecer o total de pares ativos em In + Out sem repetição. Será removida!!!

	bool IsPeerActive(string peer,set<string>* peerActive);
		PeerData* GetPeerData(string peer); // a lista de vizinhos é a única que tem os dados do peer

		map<string, PeerData>* GetPeerList();
		boost::mutex* GetPeerListMutex();

	boost::mutex* GetPeerActiveMutex(set<string>* peerActive);

		void CheckPeerList();
        void ShowPeerList();
};
#endif

