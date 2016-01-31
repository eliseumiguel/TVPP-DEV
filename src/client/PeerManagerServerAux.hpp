/*
 * PeerManagerServerAux.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: eliseu
 */

#ifndef SRC_CLIENT_PEERMANAGERSERVERAUX_HPP_
#define SRC_CLIENT_PEERMANAGERSERVERAUX_HPP_

#include <map>
#include <set>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>             // para gerar numeros aleatorios
#include <time.h>               // para gerar a 'random seed' baseada no tempo
#include "../common/Defines.hpp"
#include "../common/PeerData.hpp"
#include "../common/Strategy/Strategy.hpp"
#include "../common/Strategy/RandomStrategy.hpp"

#include <boost/thread/mutex.hpp>

#include "PeerManager.hpp"

class PeerManagerServerAux : public PeerManager
{
public:

	void SetPeerManagerState(ServerAuxTypes peerManagerState);
	uint8_t ExecMesc (uint8_t timeMerge);

	PeerManagerServerAux();
	virtual ~PeerManagerServerAux();

	MesclarModeServer Get_MergeType();
	uint8_t Get_QT_PeerMergeType();
	uint8_t Get_TimeDescPeerMerge();

	void Set_MergeType(MesclarModeServer MergeType);
	void Set_QT_PeerMergeType(uint8_t QT_PeerMergeType);
	void Set_TimeDescPeerMerge(uint8_t TimeDescPeerMerge);

	set<string> Get_Random_Merge_Peers(set<string>* peerList, uint8_t QT);


private:

    //ECM Merge configuration
	MesclarModeServer MergeType;
	uint8_t QT_PeerMergeType;
	uint8_t TimeDescPeerMerge;
	bool mergeAux_KillEnd;         //auxiliar to mix two merge types

};

#endif /* SRC_CLIENT_PEERMANAGERSERVERAUX_HPP_ */
