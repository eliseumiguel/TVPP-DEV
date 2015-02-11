#include "Sub-Channel-Data.hpp"

SubChannelData::SubChannelData(unsigned int channelId, unsigned int channelIdSub, Peer* serverPeer_Sub,
		                       int clife, int rnew)
{
    //TODO ECM ... esta condição deve ser avaliada. Inicialmente, um canal cria o sub, assim, não teria que testar o channelID..
	if (channelIdSub != 0 || serverPeer_Sub != NULL) //Avoid creation by map[]
    {
        this->channelId_Master     = channelId;
        this->channelId_Sub        = channelIdSub;
        this->serverPeer_Sub       = serverPeer_Sub;
        this->channelLife          = clife;
        this->reNewServerSub       = rnew;
        this->mesclando            = false;

        //Logging
         struct tm * timeinfo;
         char timestr[20];
         time(&creationTime);
         timeinfo = localtime(&creationTime);
         strftime (timestr,20,"%Y%m%d%H%M",timeinfo);
         string logFilename = "log-";
         logFilename += boost::lexical_cast<string>(channelId_Master) + ":" + boost::lexical_cast<string>(channelId_Sub) + "-";
         logFilename += serverPeer_Sub->GetIP() + "_" + serverPeer_Sub->GetPort() + "-";
         logFilename += timestr;
         logFilename += "-";
         string logFilenamePerf = logFilename + "perf.txt";
         string logFilenameOverlay = logFilename + "overlay.txt";
         performanceFile = fopen(logFilenamePerf.c_str(),"w");
         overlayFile = fopen(logFilenameOverlay.c_str(),"w");

    } 
}

Peer* SubChannelData::GetServer_Sub()           {return serverPeer_Sub;}

int SubChannelData::GetChannelLife()            {return channelLife;}
void SubChannelData::SetChannelLife(int cf)     {this->channelLife = cf;}
void SubChannelData::DecChannelLif()            {this->channelLife--;}


int SubChannelData::GetReNewServerSub()         {return reNewServerSub;}
void SubChannelData::SetReNewServerSub(int rn)  {this->reNewServerSub = rn;}
void SubChannelData::DecReNewServerSub()        {reNewServerSub--;}

bool SubChannelData::GetMesclando()               {return this->mesclando;}
void SubChannelData::SetMesclando(bool mesclar)   {this->mesclando = mesclar;}


void SubChannelData::PrintPeerList(map<string, PeerData>* peerList_Master)
{
	cout<<"#"<<endl;
    cout<<"Auxiliar Server: ["<<serverPeer_Sub->GetID()<<"] in Channel_Sub ["<<channelId_Master<<":"<<channelId_Sub<<"]  Peer List:"<<endl;
    for (map<string,PeerData>::iterator i = peerList_Master->begin(); i != peerList_Master->end(); i++)
        cout<<"PeerID: "<<i->first<<" Mode: "<<(int)i->second.GetMode()<<" TTL: "<<i->second.GetTTLChannel()<<endl;
    cout<<"#";
}

unsigned int SubChannelData::GetchannelId_Sub()
{
  	return channelId_Sub;
}

FILE* SubChannelData::GetPerformanceFile()
{
    return performanceFile;
}

FILE* SubChannelData::GetOverlayFile()
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

