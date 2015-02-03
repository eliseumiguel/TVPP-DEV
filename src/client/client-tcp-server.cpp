#include "client-tcp-server.hpp"

ClientTCPServer::ClientTCPServer(short  port, Client *client):TCPServer(port)
{

}

void ClientTCPServer::Start_accept()
{
    connections_[connection_id_] = new ClientPlayerSession(io_service_,this,connection_id_);
    Accept();
    if(acceptor_.is_open())
    {
      connections_[connection_id_]->Start();
    }
}

void ClientTCPServer::Accept()
{
  boost::system::error_code ec;
  acceptor_.accept(connections_[connection_id_]->Socket(),ec);
  if (ec)
  {
    std::cout<<"Sync Accept Error"<<std::endl;
  }
}

int32_t ClientTCPServer::Sync_write(unsigned char *data, int size, int id)
{
  return connections_[id]->Sync_write(data,size);
}

void ClientTCPServer::Stop_connection(uint32_t id)
{
  connections_[id]->Stop();
}
