/*
 * client-player-session.hpp
 *
 *  Created on: Oct 19, 2011
 *      Author: mantini
 */

#ifndef CLIENTPLAYERSESSION_H_
#define CLIENTPLAYERSESSION_H_

#include "../common/tcp-session.hpp"
#include "../common/tcp-server.hpp"
#include "client-tcp-server.hpp"

class ClientPlayerSession: public TCPSession
{
public:

  ClientPlayerSession(boost::asio::io_service& io_service,
      TCPServer *tcp_server, uint32_t id);

  /// start the session
  void Start();

  /// stop the session
  void Stop();

protected:

  void Hand_shake();

  uint32_t bytes_to_recv_;
};

#endif /* CLIENTPLAYERSESSION_H_ */
