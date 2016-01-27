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
#include "../common/Defines.hpp"
#include "../common/PeerData.hpp"
#include <boost/thread/mutex.hpp>

#include "PeerManager.hpp"

class PeerManagerServerAux : public PeerManager
{
public:

	set<string>* GetPeerActiveOut_Master();
	void SetPeerManagerState(ServerAuxTypes peerManagerState);
	uint8_t ExecMesc (uint8_t timeMix);

	PeerManagerServerAux();
	virtual ~PeerManagerServerAux();

	MesclarModeServer Get_MixType();
	uint8_t Get_QT_PeerMixType();
	uint8_t Get_TimeDescPeerMix();

	void Set_MixType(MesclarModeServer MixType);
	void Set_QT_PeerMixType(uint8_t QT_PeerMixType);
	void Set_TimeDescPeerMix(uint8_t TimeDescPeerMix);


private:

	map<string, PeerData> peerListMasterChannel;
	set<string> peerActiveOut_Master;
    set<string> peerListMixRejected;
    boost::mutex peerActiveMutexOut_Master;

    //ECM configuração da mesclagem do servidor auxiliar.
	MesclarModeServer MixType;   //tipo de mesclagem
	uint8_t QT_PeerMixType;      //quantidade de pares a serem desconectados durante a mesclagem
	uint8_t TimeDescPeerMix;     //intervalo de tempo para cada desconexão

};

#endif /* SRC_CLIENT_PEERMANAGERSERVERAUX_HPP_ */
