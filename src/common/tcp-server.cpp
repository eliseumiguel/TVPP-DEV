#include "tcp-server.hpp"

TCPServer::TCPServer(short port)
  : io_service_(),
    acceptor_(io_service_, tcp::endpoint(tcp::v4(), port))
{
  connection_id_ = 0;
}

void TCPServer::Accept()
{
  acceptor_.async_accept(connections_[connection_id_]->Socket(),
      boost::bind(&TCPServer::Handle_accept, this,
        boost::asio::placeholders::error)); 
}

void TCPServer::Start_accept()
{

}

void TCPServer::Handle_accept(const boost::system::error_code& e)
{

}

void TCPServer::Run()
{
  io_service_.run();
}

void TCPServer::Remove_connection(uint32_t connex_id)
{
  connections_.erase(connex_id);
}

Message *TCPServer::Handle_message(Message*,std::string)
{
  return NULL;
}

