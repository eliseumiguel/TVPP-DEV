#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <map>
#include <set>
#include <string.h>
#include <stdio.h>
#include "../common/PeerData.hpp"

#include <boost/thread/mutex.hpp>

#define PEER_ACTIVE_COOLDOWN 5

class PeerManager
{
	private:
		unsigned int maxActivePeers;
		map<string, PeerData> peerList;
        set<string> peerActive;
		map<string, unsigned int> peerActiveCooldown;
		//Mutexes
		boost::mutex peerListMutex;
        boost::mutex peerActiveMutex;

    public:
		PeerManager();
		unsigned int GetMaxActivePeers();
		void SetMaxActivePeers(unsigned int maxActivePeers);

		bool AddPeer(Peer* newPeer);
		bool ConnectPeer(string peer);
		void DisconnectPeer(string peer);
		void RemovePeer(string peer);

		unsigned int GetPeerActiveSize();
		bool IsPeerActive(string peer);
		PeerData* GetPeerData(string peer);

		map<string, PeerData>* GetPeerList();
        set<string>* GetPeerActive();
		boost::mutex* GetPeerListMutex();
		boost::mutex* GetPeerActiveMutex();

		void CheckPeerList();
        void ShowPeerList();
};

#endif