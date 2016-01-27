#include "Sub-Channel-Data.hpp"

SubChannelServerAuxData::SubChannelServerAuxData(unsigned int channelId, unsigned int ServerAux_ChannelId_Sub, Peer* serverPeer_Sub,  bool gSubLogs)
{
    //TODO ECM ... esta condição deve ser avaliada. Inicialmente, um canal cria o sub, assim, não teria que testar o channelID..
	if (ServerAux_ChannelId_Sub != 0 || serverPeer_Sub != NULL) //Avoid creation by map[]
    {
        this->ServerAux_ChannelId_Master     = channelId;
        this->ServerAux_ChannelId_Sub        = ServerAux_ChannelId_Sub;
        this->serverPeer_Sub                 = serverPeer_Sub;
        this->GenerateSubLogs                = gSubLogs;

        //Logging
         struct tm * timeinfo;
         char timestr[20];
         time(&creationTime);
         timeinfo = localtime(&creationTime);
         strftime (timestr,20,"%Y%m%d%H%M",timeinfo);
         string logFilename = "log-";
         logFilename += boost::lexical_cast<string>(ServerAux_ChannelId_Master) + "-";

         if (this->GenerateSubLogs){

        	 string ip = serverPeer_Sub->GetIP();
        	 int tamanho = ip.size();
        	 for (int j = 0; j < 20 - tamanho ; j++)
        	 {
        		 ip = "0" + ip;
        	 }
        	 logFilename += ip +  "_" ;


        	 logFilename += serverPeer_Sub->GetPort() + "-";
        	 logFilename += timestr;
        	 logFilename += "-";

        	 string channelS = boost::lexical_cast<string>(ServerAux_ChannelId_Sub);
        	 tamanho = channelS.size();
        	 for (int j = 0; j < 3 - tamanho ; j++)
        	 {
        		 channelS ="0" + channelS;
        	 }

        	 logFilename += channelS;

        	 string logFilenamePerf = logFilename + "-perf.txt";
        	 string logFilenameOverlay = logFilename + "-overlay.txt";
        	 performanceFile = fopen(logFilenamePerf.c_str(),"w");
        	 overlayFile = fopen(logFilenameOverlay.c_str(),"w");
         }
    } 
}

Peer* SubChannelServerAuxData::GetServer_Sub()           {return serverPeer_Sub;}

void SubChannelServerAuxData::PrintPeerList(map<string, PeerData>* peerList_Master)
{
	cout<<"#"<<endl;
    cout<<"Auxiliar Server: ["<<serverPeer_Sub->GetID()<<"] in Channel_Sub ["<<ServerAux_ChannelId_Master<<":"<<ServerAux_ChannelId_Sub<<"]  Peer List:"<<endl;
    for (map<string,PeerData>::iterator i = peerList_Master->begin(); i != peerList_Master->end(); i++)
        cout<<"PeerID: "<<i->first<<" Mode: "<<(int)i->second.GetMode()<<" TTL: "<<i->second.GetTTLChannel()<<endl;
    cout<<"#";
}

unsigned int SubChannelServerAuxData::Get_ServerAuxChannelId_Sub()
{
  	return ServerAux_ChannelId_Sub;
}

FILE* SubChannelServerAuxData::GetPerformanceFile()
{
    return performanceFile;
}

FILE* SubChannelServerAuxData::GetOverlayFile()
{
    return overlayFile;
}

//***********************************
SubChannelCandidateData::SubChannelCandidateData(ServerAuxTypes serverState, bool peerWaitInform)
{
	this->serverState = serverState;
	this->peerWaitInform = peerWaitInform;

}
ServerAuxTypes SubChannelCandidateData::GetState(){return this->serverState;}
void SubChannelCandidateData::SetState(ServerAuxTypes serverState){this->serverState = serverState;}

bool SubChannelCandidateData::GetPeerWaitInform(){return this->peerWaitInform;}
void SubChannelCandidateData::SetPeerWaitInform(bool peerWaitInform){this->peerWaitInform = peerWaitInform;}

