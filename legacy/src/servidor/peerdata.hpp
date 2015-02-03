#ifndef PEERDATA_H_INCLUDED
#define PEERDATA_H_INCLUDED

#include <string>
#define _IPV6_SIZE 40
#define _TTL_  9

using namespace std;

/**
* This class implements the information about the peers
*/
class PeerData{

    public:
	/**
	* @param string 
	* @param int Max TTL value
	*/
        PeerData(string = "0", int = _TTL_);
	/**
	* This method retrives the ID of the peer
	* @return A string with the
	*/
        string GetID();
	/**
	* This method returns the current TTL value
	* @return A string with the
	*/
        int GetTTL();
        void SetID(string IP);
        void SetTTL(int v);
        void DecTTL();

    private:
        int TTL;
        string ID;

};


#endif // PEERDATA_H_INCLUDED

