#include "tcp-session.hpp"

TCPSession::TCPSession(boost::asio::io_service& io_service, TCPServer *tcp_server, uint32_t id)
  : socket_(io_service)
{
  id_ = id;
  tcp_server_ = tcp_server;
}

TCPSession::TCPSession(boost::asio::io_service& io_service)
  : socket_(io_service)
{
  id_ = 0;
}

void TCPSession::Read_header(int size)
{
  boost::asio::async_read(socket_, boost::asio::buffer(buffer_, size),
    boost::bind(&TCPSession::Handle_read_header, 
    this,
    boost::asio::placeholders::error,
    boost::asio::placeholders::bytes_transferred));
  
}

void TCPSession::Read_body(int size)
{
  boost::asio::async_read(socket_, boost::asio::buffer(rcvMessage->GetFirstByte() + MESSAGE_HEADER_SIZE, size),
    boost::bind(&TCPSession::Handle_read_body, 
    this,
    boost::asio::placeholders::error,
    boost::asio::placeholders::bytes_transferred));
  
}

int32_t TCPSession::Sync_read(unsigned char *buffer,int size,int start_pos)
{
    int32_t read;
    boost::system::error_code ec;
    read = boost::asio::read(socket_, boost::asio::buffer(buffer+start_pos, size),boost::asio::transfer_all(), ec);
    if(ec)//Failure
      return -1;
    else//Success
      return read;
}

void TCPSession::Write(unsigned char *data,int size)
{
  boost::asio::async_write(socket_, boost::asio::buffer(data, size),
    boost::bind(&TCPSession::Handle_write, 
    this,
    boost::asio::placeholders::error));
}

int32_t TCPSession::Sync_write(unsigned char *data, int size)
{
   boost::system::error_code e;
   int32_t sent = boost::asio::write(socket_,boost::asio::buffer(data,size),boost::asio::transfer_all(),e);
   if(!e)
     return sent;
   else
   {
     std::cout<<"sync_write failure"<<"Session id: "<<id_<<"endpoint address: "<<Get_address();
     return -1;
   }
}

boost::asio::ip::tcp::socket& TCPSession::Socket()
{
  return socket_;
}

std::string TCPSession::Get_address()
{
  std::string addr = socket_.remote_endpoint().address().to_string();
  addr+=":";
  addr+= boost::lexical_cast<std::string>(socket_.remote_endpoint().port());
  return addr;
}

int32_t TCPSession::GetTvtideMessageHeader()
{
  Message preMessage(new uint8_t[MESSAGE_HEADER_SIZE]);
  for(int i = 0; i < MESSAGE_HEADER_SIZE; i++)
  {
    preMessage.GetFirstByte()[i] = buffer_[i];
  }
  return preMessage.GetHeaderSize() + preMessage.GetBodySize();
}

void TCPSession::Stop()
{
}

void TCPSession::Start()
{
}

void TCPSession::Handle_read_header(const boost::system::error_code & e, std::size_t bytes_transferred)
{
}

void TCPSession::Handle_read_body(const boost::system::error_code & e, std::size_t bytes_transferred)
{
}

void TCPSession::Handle_write(const boost::system::error_code & e)
{
}


