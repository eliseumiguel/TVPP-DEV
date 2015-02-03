#ifndef PEERDATA_H_INCLUDED
#define PEERDATA_H_INCLUDED

#include <string>
#include <iostream>
#include <boost/dynamic_bitset.hpp>

#define _IPV6_SIZE 40
#define _CHUNK_MAP_SIZE 1600

#define _TTL_ 10
#define IP_PORT_SEPARATOR ":"
#define _TTL_MAX  9

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
        PeerData(string = "0:0", int = _TTL_,int size = 1600);
	/**
	* This method retrives the ID of the peer
	* @return A string with the
	*/
        string GetID();
	/**
	* This method retrives the IP adrees of the peer
	* @return A string with the
	*/
        string GetAddr();
	/**
	* This method retrives the Port number of the peer
	* @return A string with the
	*/
        string GetPort();
	/**
	* This method returns the current TTL value
	* @return A string with the
	*/
        int GetTTL();
        void SetID(string);
        void SetTTL(int v);
        void DecTTL();
        void SetChunk_map(boost::dynamic_bitset<>x);
        bool GetChunk_pos(int i);
        void SetChunk_num(int i);
        int  GetChunk_num();
		void SetChunk_id(unsigned int);
		unsigned int GetChunk_id();

    private:
		boost::dynamic_bitset<> chunk_map;
        int TTL;
        int chunk_num;
		unsigned int chunk_id;
        string ID;
		string Addr;
		string Port;
		
        

};


#endif // PEERDATA_H_INCLUDED

