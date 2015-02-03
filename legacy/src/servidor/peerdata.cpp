#include "peerdata.hpp"

using namespace std;

/** 
 *Metodo construtor
 * Recebe o TTL padrão (Tempo de vida na lista de peers caso o o cliente pare de se comunicar com o servidor)
 * ID que o o endereço IP do peer
 */
PeerData :: PeerData ( string id, int ttl) {

    ID = id;
    TTL = ttl;

}
/** Retorna o ID*/
string PeerData :: GetID(){

    return ID;
}
/** Retorna o TTL */
int PeerData :: GetTTL(){
    return TTL;
}
/** Altera o ID */
void PeerData :: SetID(string id) {
    ID = id;

}
/** Altera o TTL*/
void PeerData:: SetTTL(int ttl){
    TTL = ttl;
}
/** Decrementa o TTL em 1*/
void PeerData :: DecTTL(){
    TTL--;
}
