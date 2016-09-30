#ifndef NEARESTIPSTRATEGY_H
#define NEARESTIPSTRATEGY_H

#include "Strategy.hpp"

typedef struct NearestNode {
    unsigned int posVector;
    unsigned int dist[4];
} NearestNode;

class NearestIPStrategy: public Strategy
{
	private:
        void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity,
        		          unsigned int minimalBandwidthOut){}
		void SelectPeers(vector<PeerData*>* peers, Peer* srcPeer, int quantity)
		{
			vector<PeerData*> results;
			
			unsigned int dif = 256;
			vector<string> vectorIpOctets;

			string query = srcPeer->GetIP();
			vector<string> vectorIpQueryOctets;
			Tokenize(query, vectorIpQueryOctets, ".");

			NearestNode nearest;
			for (int i = 0; i < quantity; i++)
			{
				initializeNearest(nearest);
				//PARA CADA VETOR DA LISTA
				for (int j = 0; j < (int)peers->size(); j++)
				{

					vector<string> vectorIpOctets;
					Tokenize((*peers)[j]->GetPeer()->GetIP(), vectorIpOctets, ".");

					//NUMERO DE OCTETOS  - SEMPRE TERA O TAMANHO 4
					int tam = vectorIpOctets.size();
					for (int g = 0; g < tam; g++)
					{
						dif = calculateDistance(atoi(vectorIpQueryOctets[g].c_str()), atoi(vectorIpOctets[g].c_str()));

						//SE A DISTANCIA DO OCTETO ATUAL FOR MAIOR DE UM IP ANTERIOR, DESCONSIDERA O IP E VAI PARA O PROX DA LISTA
						if (dif > nearest.dist[g])
							break;
						
						// SE A DIST DO OCTETO DA LISTA FOR MENOR QUE A DISTANCIA DE UM IP SELECIONADO, O IP MAIS PROXIMO EH O VERIFICADO E VAI PARA O PROXIMO
						if (dif < nearest.dist[g])
						{
							updateNearest(nearest, vectorIpOctets, vectorIpQueryOctets, j);
							break;
						}

					}
				}
				results.push_back((*peers)[nearest.posVector]);
				peers->erase(peers->begin()+nearest.posVector);
			}

			delete peers;
			peers = &results;
		}
		
		void Tokenize(const string& str,
							  vector<string>& tokens,
							  const string& delimiters = " ")
		{
			// Skip delimiters at beginning.
			string::size_type lastPos = str.find_first_not_of(delimiters, 0);
			// Find first "non-delimiter".
			string::size_type pos     = str.find_first_of(delimiters, lastPos);

			while (string::npos != pos || string::npos != lastPos)
			{
				// Found a token, add it to the vector.

				tokens.push_back(str.substr(lastPos, pos - lastPos));
				// Skip delimiters.  Note the "not_of"
				lastPos = str.find_first_not_of(delimiters, pos);
				// Find next "non-delimiter"
				pos = str.find_first_of(delimiters, lastPos);
			}
		}


		int calculateDistance(int ip1, int ip2)
		{
			if(ip1 > ip2)
				return ip1 - ip2;
			else
				return ip2 - ip1;
		}

		void updateNearest(NearestNode& nearest, vector<string>& newNearest, vector<string>& query, int indice)
		{
			nearest.posVector = indice;
			nearest.dist[0] = calculateDistance(atoi(newNearest[0].c_str()), atoi(query[0].c_str()));
			nearest.dist[1] = calculateDistance(atoi(newNearest[1].c_str()), atoi(query[1].c_str()));
			nearest.dist[2] = calculateDistance(atoi(newNearest[2].c_str()), atoi(query[2].c_str()));
			nearest.dist[3] = calculateDistance(atoi(newNearest[3].c_str()), atoi(query[3].c_str()));
		}

		void initializeNearest(NearestNode& nearest)
		{
			nearest.posVector = 0;
			nearest.dist[0] = 256;
			nearest.dist[1] = 256;
			nearest.dist[2] = 256;
			nearest.dist[3] = 256;
		};
};
#endif
