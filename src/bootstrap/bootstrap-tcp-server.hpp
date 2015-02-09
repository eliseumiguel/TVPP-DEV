#ifndef BOOTSTRAPTCPSERVER_H_INCLUDED
#define BOOTSTRAPTCPSERVER_H_INCLUDED

#include "../common/tcp-server.hpp"
//#include "../common/core.hpp"
#include "bootstrap.hpp"
#include "bootstrap-session.hpp"

class Bootstrap;
class BootstrapTCPServer:public TCPServer
{
public:
  BootstrapTCPServer(short port,Bootstrap *bootstrap);
  Message *Handle_message(Message*,std::string);

private:  
  ///Start Accepting Connexions
  void Start_accept();
  void Handle_accept(const boost::system::error_code& e);
  Bootstrap *bootstrap_;
};

#endif //BOOTSTRAPTCPSERVER_H_INCLUDED
