#ifndef REQUEST_ATTEMPT_H
#define REQUEST_ATTEMPT_H

#include <sys/time.h>
#include <string>

using namespace std;

class RequestAttempt
{
private:
    string chosenPeerID;
    float lastRegisteredDelay;
    int candidates;
    struct timeval timestamp;
    
public:
    RequestAttempt(string chosenPeerID, float lastRegisteredDelay, int candidates);
    string GetChosenPeerID();
    float GetLastRegisteredDelay();
    int GetCandidates();
    struct timeval GetTimestamp();
};
#endif
