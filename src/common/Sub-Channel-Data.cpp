#include "Sub-Channel-Data.hpp"

SubChannelData::SubChannelData(unsigned int channelId, unsigned int channelIDSub, Peer* serverPeer_Sub)
{
    if (channelId != 0 || serverPeer_Sub != NULL) //Avoid creation by map[]
    {
        this->channelId_Master = channelId;
        this->channelId_Sub    = channelIDSub;
        this->serverPeer_Sub = serverPeer_Sub;

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

Peer* SubChannelData::GetServer_Sub()
{
    return serverPeer_Sub;
}

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

