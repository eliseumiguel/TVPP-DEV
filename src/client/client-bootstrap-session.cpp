/*
 * client-bootstrap-session.cpp
 *
 *  Created on: Oct 21, 2011
 *      Author: mantini
 */

#include "client-bootstrap-session.hpp"

ClientBootstrapSession::ClientBootstrapSession(boost::asio::io_service& io_service):TCPSession(io_service)
{

}
ClientBootstrapSession::ClientBootstrapSession(boost::asio::io_service& io_service, string server_address, string server_port, string punch_target_port, Client *client):TCPSession(io_service)
{
  server_address_ = server_address;
  server_port_ = boost::lexical_cast<int32_t>(server_port);
  punch_port_ = boost::lexical_cast<int32_t>(punch_target_port);
  client_ = client;
}

bool ClientBootstrapSession::Connect()
{
  boost::asio::ip::tcp::endpoint endpoint(
      boost::asio::ip::address::from_string(server_address_), server_port_);
  boost::system::error_code ec;
  socket_.connect(endpoint, ec);
  if (ec)///Connect failure
  {
    std::cout<<"Failed to connect to bootstrap"<<std::endl;
    return false;
  }
  return true;

}

bool ClientBootstrapSession::Bind()
{
  socket_.open(boost::asio::ip::tcp::v4());
  boost::system::error_code ec;
  /*uint32_t attemps = 0;
  while(attemps < 100)
  {
    socket_.bind(boost::asio::ip::tcp::endpoint(
          boost::asio::ip::tcp::v4(), punch_port_), ec);
    if (ec)///Error
    {
      punch_port_++;
      attemps++;
    }
    else///Success
      return true;
  }*/
  socket_.bind(boost::asio::ip::tcp::endpoint(
        boost::asio::ip::tcp::v4(), punch_port_), ec);
  if (ec)///Error
  {
    std::cout<<"Failed to bind punch port"<<std::endl;
    return false;
  }
  return true;

}

int32_t ClientBootstrapSession::Punch_connect()
{
  if(Bind() && Connect())
    return punch_port_;
  else
    return -1;
}

Message *ClientBootstrapSession::Receive()
{
  if (Sync_read(buffer_, MESSAGE_HEADER_SIZE, 0) > 0)
  {
    uint32_t msgSize = GetTvtideMessageHeader();
	
	rcvMessage = new Message(new uint8_t[msgSize]);
    for(int i = 0; i < MESSAGE_HEADER_SIZE; i++)
    {
      rcvMessage->GetFirstByte()[i] = buffer_[i];
    }
	
    if (Sync_read(rcvMessage->GetFirstByte(), msgSize - MESSAGE_HEADER_SIZE, MESSAGE_HEADER_SIZE) > 0)
    {
      return rcvMessage;
    }
  }
  std::cout<<"Failed to receive data from bootstrap"<<std::endl;
  return NULL;
}

void ClientBootstrapSession::Stop()
{
  socket_.close();
  delete rcvMessage;
}
