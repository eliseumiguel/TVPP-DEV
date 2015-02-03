/*
 * client-player-session.cpp
 *
 *  Created on: Oct 19, 2011
 *      Author: mantini
 */

#include "client-player-session.hpp"

ClientPlayerSession::ClientPlayerSession(boost::asio::io_service & io_service,
    TCPServer *tcp_server, uint32_t id):TCPSession(io_service,tcp_server,id)
{

}

void ClientPlayerSession::Start()
{
  Hand_shake();
}

void ClientPlayerSession::Stop()
{
  socket_.close();
  tcp_server_->Remove_connection(id_);
}

void ClientPlayerSession::Hand_shake()
{
  socket_.read_some(boost::asio::buffer(buffer_, max_length));
  string accept = "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n";
  Sync_write((unsigned char*)accept.c_str(),accept.size());
}
