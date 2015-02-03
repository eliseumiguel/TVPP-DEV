#ifndef TCPSESSION_HPP
#define TCPSESSION_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>
#include "../common/Messages/Message.hpp"

using boost::asio::ip::tcp;

class TCPServer;

/// Represents a single connection from a client.
class TCPSession
{
public:
  /// Construct a connection with the given io_service, associated with the given TCP-Server.
  TCPSession(boost::asio::io_service& io_service, TCPServer *tcp_server, uint32_t id);

  ///Construct a connection with the given io_service.
  TCPSession(boost::asio::io_service& io_service);
  
  /// Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& Socket();
  
  ///Get the address of the endpoint of this connection
  std::string Get_address();
  
  /// start the session
  virtual void Start();
  
  /// Stop all asynchronous operations associated with the connection.
  virtual void Stop();

  ///Synchronous Write
  int32_t Sync_write(unsigned char *data, int size);

  ///Synchronous Read
  int32_t Sync_read(unsigned char* buffer,int size, int start_pos);
 
protected:
  /// asynchronous Read
  void Read_header(int);
  
   /// asynchronous Read
  void Read_body(int);

  /// asynchronous Write
  void Write(unsigned char *,int);
  
  int32_t GetTvtideMessageHeader();

  virtual void Handle_read_header(const boost::system::error_code& e, std::size_t bytes_transferred);
  
  virtual void Handle_read_body(const boost::system::error_code& e, std::size_t bytes_transferred);
  
  virtual void Handle_write(const boost::system::error_code& e);
  
  /// Socket for the connection.
  boost::asio::ip::tcp::socket socket_;

  /// Buffer for incoming data.
  enum { max_length = 8192 };
  unsigned char buffer_[max_length];
  
  Message* rcvMessage;
  Message* replyMessage;
  
  /// This connection identifier.
  uint32_t id_;
  
  /// Owner of this session.
  TCPServer *tcp_server_;
};

#endif //TCPSESSION_HPP
