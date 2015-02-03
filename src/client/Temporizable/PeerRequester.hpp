#ifndef PEERREQUESTER_H
#define PEERREQUESTER_H

#include "../client.hpp"
#include "../../common/Temporizable.hpp"
#include "../../common/Strategy/Strategy.hpp"

class Client;

class PeerRequester : public Temporizable
{
	private:
		Client *client;

    public:
		PeerRequester(Client *client, uint64_t timerPeriod);
		void TimerAlarm(uint64_t timerPeriod, string timerName);
};

#endif