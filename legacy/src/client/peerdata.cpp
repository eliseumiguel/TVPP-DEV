#include "peerdata.hpp"

using namespace std;

/** 
 *Metodo construtor
 * Recebe o TTL padrão (Tempo de vida na lista de peers caso o o cliente pare de se comunicar com o servidor)
 * ID que o o endereço IP do peer
 */
PeerData :: PeerData ( string id, int ttl, int size) {

    //ID = id;
	boost::dynamic_bitset<> x(size);
    chunk_map = x;
    TTL = ttl;
    SetID(id);
	Size = size;

}
/** Retorna o ID*/
string PeerData :: GetID(){

    return ID;
}

string PeerData :: GetAddr(){

    return Addr;
}
string PeerData :: GetPort(){

    return Port;
}
/** Retorna o TTL */
int PeerData :: GetTTL(){
    return TTL;
}
/** Altera o ID */
void PeerData :: SetID(string id) {
    ID = id;
	size_t found = id.find(IP_PORT_SEPARATOR);
	if( found == string::npos){
		//cout<<endl<<"INVALID PEER ID"<<endl;
		exit(1);
	}
	Addr.clear();
	Addr.append(id.begin(),id.begin()+found);
	Port.clear();
	Port.append(id.begin()+found+1,id.end());

}
/** Altera o TTL*/
void PeerData:: SetTTL(int ttl){
    TTL = ttl;
}
/** Decrementa o TTL em 1*/
void PeerData :: DecTTL(){
    TTL--;
}

void PeerData :: SetChunk_map(boost :: dynamic_bitset<>x) {
	chunk_map = x;
}

bool PeerData :: GetChunk_pos(int i) {
	bool x = chunk_map[i];
	return x;
}
 
void PeerData :: SetChunk_num(int i) {
	chunk_num = i;
}

int  PeerData :: GetChunk_num() {
	return chunk_num;
}

void PeerData :: SetChunk_id(unsigned int i) {
	chunk_id = i;
}
 
unsigned int PeerData :: GetChunk_id() {
	return chunk_id;
}

int PeerData :: GetLastChunk_pos(){
	int i;
	int pos = 0;
	for ( i = 0; i<Size-1; i++){
		if( chunk_map[i] == 1 && chunk_map[i+1] == 0){
			pos = i;
			break;
		}
	}
	return pos;
}
