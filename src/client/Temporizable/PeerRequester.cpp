#include "PeerRequester.hpp"

PeerRequester::PeerRequester(Client *client, uint64_t timerPeriod) : Temporizable(timerPeriod)
{
	this->client = client;
}

void PeerRequester::TimerAlarm(uint64_t timerPeriod, string timerName)
{
	client->ConnectToBootstrap();
}
