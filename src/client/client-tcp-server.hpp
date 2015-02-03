#ifndef CLIENTTCPSERVER_H_INCLUDED
#define CLIENTTCPSERVER_H_INCLUDED

#include "../common/tcp-server.hpp"
#include "client.hpp"
#include "client-player-session.hpp"

class Client;
class ClientTCPServer:public TCPServer
{
public:
  ClientTCPServer(short port,Client *client);
  int32_t Sync_write(unsigned char* data, int size, int id);
  void Start_accept();
  void Stop_connection(uint32_t id);
private:  
  ///Start Accepting Connexions
  
  void Accept();
  Client *client_;
};

#endif //CLIENTPLAYERTCPSERVER_H_INCLUDED
