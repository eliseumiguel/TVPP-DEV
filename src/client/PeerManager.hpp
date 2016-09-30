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

#define PEER_ACTIVE_COOLDOWN 5                           // each 5 seconds list peer to waiting to able restart connections
#define SERVER_AUX_SUB_CHANNEL_ID 1

class PeerManager
{
private:
	bool ConnectSpecial(string peer, set<string>* peerActive);

protected:
	ServerAuxTypes peerManagerState;
	unsigned int maxActivePeersIn;
	unsigned int maxActivePeersOut;
	unsigned int maxActivePeersOutFREE;                 //tamanho da lista de pares com pouca banda
	unsigned int outLimitToSeparateFree;                //insere pares pobres em lista outFree se out do par for menor que o limite

	bool removeWorsePartner;                             // used to remove the worse peer if new connection asked

	map<string, PeerData> peerList;		                 // All know peers

	set<string> peerActiveIn;
	set<string> peerActiveOut;
	set<string> peerActiveOutFREE;                        // lista de pares com pouca banda
	set<string> peerList_Rejected;                        // Avoided connection peer list

	map<string, unsigned int> peerActiveCooldownIn;       // pares que podem ser removidos por pouca atividade. Não é usado para controle de ...
	map<string, unsigned int> peerActiveCooldownOut;      // ... free-rider pelo SURE. Pode-se evitar enviar um participante a esta lista caso ...
	map<string, unsigned int> peerActiveCooldownOutFREE;  // ... ele tenha sido removido apenas para flexibilização da rede

	//Mutexes
	boost::mutex peerListMutex;
    boost::mutex peerActiveMutexIn;
    boost::mutex peerActiveMutexOut;
    boost::mutex peerActiveMutexOutFREE;                   //mutex out pares com pouca banda
    boost::mutex peerListRejectedMutexOut;


    void CheckpeerActiveCooldown(map<string, unsigned int>* peerActiveCooldown);
    int showPeerActive(set<string>* peerActive);

public:

    PeerManager();

	ServerAuxTypes GetPeerManagerState();


	unsigned int GetMaxActivePeers(set<string>* peerActive);
	void SetMaxActivePeersIn(unsigned int maxActivePeers);
	void SetMaxActivePeersOut(unsigned int maxActivePeers);
	void SetMaxActivePeersOutFREE(unsigned int maxActivePeersFREE);
	void SetMaxOutFreeToBeSeparated(unsigned int outLimitToSeparateFree);

	bool AddPeer(Peer* newPeer, int sizePeerListOut = -1, int sizePeerListOut_FREE = -1); //add na lista de vizinhos

    set<string>* GetPeerActiveIn();
    set<string>* GetPeerActiveOut(bool separatedFree = false, uint16_t peerOut = 0);
    map<string, unsigned int>* GetPeerActiveCooldown(set<string>* peerActive);

	bool ConnectPeer(string peer, set<string>* peerActive);
	void DisconnectPeer(string peer, set<string>* peerActive);
	void RemovePeer(string peer);                             //remove na lista de vizinhos

	unsigned int GetPeerActiveSize(set<string>* peerActive);
	unsigned int GetPeerActiveSizeTotal();                   //fornecer o total de pares ativos em In + Out sem repetição.

	bool IsPeerInPeerList(string peer);
	void SetRemoveWorsePartner (bool removeWorsePartner);
	bool GetRemoveWorsePartner ();

	bool IsPeerActive(string peer,set<string>* peerActive);
	PeerData* GetPeerData(string peer);

	map<string, PeerData>* GetPeerList();

	boost::mutex* GetPeerListMutex();
	boost::mutex* GetPeerActiveMutex(set<string>* peerActive);

	void CheckPeerList();
    void ShowPeerList();
};
#endif

