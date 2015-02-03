#ifndef TCPSERVER_H_INCLUDED
#define TCPSERVER_H_INCLUDED

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <map>
#include "Messages/Message.hpp"
//#include "../bootstrap/bootstrap.hpp"
//#include "../client/client.hpp"
#include "tcp-session.hpp"
//#include "../bootstrap/bootstrap-session.hpp"


using boost::asio::ip::tcp;
//class Bootstrap;

class TCPServer
{
public:
  TCPServer(short port);
  void Run();
  void Remove_connection(uint32_t);
  virtual Message *Handle_message(Message*,std::string);
  
protected:
  ///Async Accept
  virtual void Accept();
  
  ///Start Accepting Connexions
  virtual void Start_accept()= 0;
  
  ///Handle an incoming connetion
  virtual void Handle_accept(const boost::system::error_code&);
  
  boost::asio::io_service io_service_;
  tcp::acceptor acceptor_;
  //uint32_t server_type_;
  //enum {BOOTSTRAP_CLIENT, BOOTSTRAP_PLAYER};
  //Bootstrap *bootstrap_;
  //Client *client_;
  uint32_t connection_id_;
  std::map<uint32_t, TCPSession*> connections_;

};




#endif // TCPSERVER_H_INCLUDED
