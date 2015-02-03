#include "bootstrap-tcp-server.hpp"

BootstrapTCPServer::BootstrapTCPServer(short port,Bootstrap *bootstrap):TCPServer(port)
{
  bootstrap_ = bootstrap;
  Start_accept();
}

void BootstrapTCPServer::Start_accept()
{
  connections_[connection_id_] = new BootstrapSession(io_service_,this,connection_id_);
  Accept();
}

Message *BootstrapTCPServer::Handle_message(Message *message, std::string SourceAddress)
{
  return bootstrap_->HandleTCPMessage(message, SourceAddress,0);
}

void BootstrapTCPServer::Handle_accept(const boost::system::error_code& e)
{
  // Check whether the server was stopped by a signal before this completion
  // handler had a chance to run.
  if (!acceptor_.is_open())
  {
    return;
  }

  if (!e)
  {
    connections_[connection_id_]->Start();
    connection_id_++;
  }

  Start_accept();
}
