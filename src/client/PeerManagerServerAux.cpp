/*
 * PeerManagerServerAux.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: eliseu
 */

#include "PeerManagerServerAux.hpp"
#include "PeerManager.hpp"

/*constructor*/
PeerManagerServerAux::PeerManagerServerAux(): PeerManager (){
	this->peerManagerState = NO_SERVER_AUX;
}

/*destructor*/
PeerManagerServerAux::~PeerManagerServerAux() {}


MesclarModeServer PeerManagerServerAux::Get_MixType(){return this->MixType;}
uint8_t PeerManagerServerAux::Get_QT_PeerMixType(){return this->QT_PeerMixType;}
uint8_t PeerManagerServerAux::Get_TimeDescPeerMix(){return this->TimeDescPeerMix;}

void PeerManagerServerAux::Set_MixType(MesclarModeServer MixType){this->MixType = MixType;}
void PeerManagerServerAux::Set_QT_PeerMixType(uint8_t QT_PeerMixType){this->QT_PeerMixType = QT_PeerMixType;}
void PeerManagerServerAux::Set_TimeDescPeerMix(uint8_t TimeDescPeerMix){this->TimeDescPeerMix = TimeDescPeerMix;}

set<string>* PeerManagerServerAux::GetPeerActiveOut_Master()
{
	return &peerActiveOut_Master;
}

void PeerManagerServerAux::SetPeerManagerState(ServerAuxTypes newPeerManagerState)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	boost::mutex::scoped_lock peerActiveLock(peerActiveMutexOut);
	switch (newPeerManagerState)
	{

	case SERVER_AUX_ACTIVE:
		if (this->peerManagerState == NO_SERVER_AUX) //iniciando o processo de servidor auxiliar
		{
			peerListMasterChannel = peerList;        //separa a lista dos pares do canal principal

			peerActiveOut_Master.clear();
			/* caso queira enviar chunkmap vazio aos parceiros antigos durante o flash crowd,
			 * basta descomentar esta linha...
			 * Isso mantém o TTLIn do servidor auxiliar a seus antigos parceiros durante o flash crowd.
			 * Com isso, o servidor auxiliar pode voltar a ser parceiro dos antigos parceiros na rede principal
			*/
			//if(true) // implementar controle para esta opção. Isso deverá ser implementado na estratégia.
			 	  peerActiveOut_Master = peerActiveOut;

			peerActiveOut.clear();
		    this->peerManagerState = newPeerManagerState;
		}
		break;

	case SERVER_AUX_MESCLAR:
		if (this->peerManagerState == SERVER_AUX_ACTIVE) //entrando em processo de mesclagem
		{
            /*neste momento, lá no bootstrap, o Channel já colocou todos os pares deste servidor
             * auxiliar na rede principal. Isso faz com que esses vizinhos possam iniciar as parcerias
             * na rede principal e iniciar o processo de deixar o subCanal.
             * Assim, o servidor auxiliar mantém os pares do canalAuxiliar (de forma interna) neste subcanal
             * e começa a aceitar conecção Out apenas do canal principal. Desta forma, se a estratégia de mesclagem
             * removeu algum par do canal auxiliar, este par não deve conseguir voltar ao servidor auxiliar durante a mesclagem
             */

			/* importante: implementar uma lista de pares removidos pelo servidor auxiliar durante a mesclagem e não permitir que esses
			 * pares voltem a ser parceiros do servidor durante a mesclagem....
			 *
			 */
			int size = (int) peerActiveOut.size()/2;
		    while (size > 0)
			{
		    	size--;
		    	//peerList[*(peerActiveOut.begin())].SetChannelId_Sub(CHANNEL_ID_MESCLANDO);
		    	//peerList.erase(*(peerActiveOut.begin()));

		    	//errado, deve usar um controle com lista de nós proibidos de retornar ao servidor.
		    	//além disso, deve-se juntar a lista master com peerlist.

		    	//peerActiveOut.erase(peerActiveOut.begin());
		    }
		    peerListMasterChannel.clear();
			this->peerManagerState = newPeerManagerState;
		}
		break;

	case NO_SERVER_AUX:

	    for (set<string>::iterator i = peerActiveOut.begin(); i != peerActiveOut.end(); i++)
	    	peerList[*i].SetChannelId_Sub(0);

	    //acho que peerListMaster deve voltar a ser peerlist para que o servidor encontre os parceiros in na principal antiga
	    //seria assim...
	    //peerList.clear() e peerList = peerListMasterChannel; (isso pode ser feito na mesclagem...

	    peerListMasterChannel.clear();
	    peerActiveOut_Master.clear();

		this->peerManagerState = newPeerManagerState;
		break;

    default:
        cout<<"Não houve mudança no Estado do PeerManager"<<endl;
        break;
	}
	peerActiveLock.unlock();
	peerListLock.unlock();
}

uint8_t PeerManagerServerAux::ExecMesc (uint8_t timeMix){
	timeMix--;
	if (timeMix == 0){
		cout<<"executou mesclagem tempo "<<(int)timeMix<<endl;

	}
	return timeMix;
}

