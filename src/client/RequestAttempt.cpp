#include "RequestAttempt.hpp"

RequestAttempt::RequestAttempt(string chosenPeerID, float lastRegisteredDelay, int candidates)
{
    this->chosenPeerID = chosenPeerID;
    this->lastRegisteredDelay = lastRegisteredDelay;
    this->candidates = candidates;
    gettimeofday(&timestamp,NULL);
}

string RequestAttempt::GetChosenPeerID()
{
    return chosenPeerID;
}

float RequestAttempt::GetLastRegisteredDelay()
{
    return lastRegisteredDelay;
}

int RequestAttempt::GetCandidates()
{
    return candidates;
}

struct timeval RequestAttempt::GetTimestamp()
{
    return timestamp;
}