/*
 * client-bootstrap-session.hpp
 *
 *  Created on: Oct 21, 2011
 *      Author: mantini
 */

#ifndef CLIENTBOOTSTRAPSESSION_HPP_
#define CLIENTBOOTSTRAPSESSION_HPP_

#include "../common/tcp-session.hpp"
#include "client.hpp"
#include <string>

class Client;

class ClientBootstrapSession: public TCPSession
{
public:
  ClientBootstrapSession(boost::asio::io_service& io_service);
  ClientBootstrapSession(boost::asio::io_service&, string server_address, string server_port, string punch_port, Client *client);

  bool Connect();

  int32_t Punch_connect();

  Message *Receive();

  void Stop();


protected:

  bool Bind();

  Client *client_;
  string server_address_;
  int32_t server_port_;
  int32_t punch_port_;

};

#endif /* CLIENTBOOTSTRAPSESSION_HPP_ */
