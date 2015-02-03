#include "bootstrap-session.hpp"

BootstrapSession::BootstrapSession(boost::asio::io_service& io_service, TCPServer *tcp_server, uint32_t id):
TCPSession(io_service,tcp_server,id)
{  
}

void BootstrapSession::Start()
{
  Read_header(MESSAGE_HEADER_SIZE);
}

void BootstrapSession::Stop()
{
  socket_.close();
  delete rcvMessage;
  delete replyMessage;
  tcp_server_->Remove_connection(id_);
}

void BootstrapSession::Handle_read_header(const boost::system::error_code& e, std::size_t bytes_transferred)
{
  if(!e)
  {
    uint32_t msgSize = GetTvtideMessageHeader();
	
	rcvMessage = new Message(new uint8_t[msgSize]);
    for(int i = 0; i < MESSAGE_HEADER_SIZE; i++)
    {
      rcvMessage->GetFirstByte()[i] = buffer_[i];
    }
	
    Read_body(msgSize - MESSAGE_HEADER_SIZE);
  }
  else
    std::cout<<"read_header failure"<<"Session id: "<<id_<<"endpoint address: "<<Get_address()<<std::endl;
  
}

void BootstrapSession::Handle_read_body(const boost::system::error_code& e, std::size_t bytes_transferred)
{
  if (!e) ///Success
  {
    replyMessage = tcp_server_->Handle_message(rcvMessage, Get_address());
    if (replyMessage)
		Write(replyMessage->GetFirstByte(), replyMessage->GetSize());
  }
  else
    std::cout<<"read_body failure"<<"Session id: "<<id_<<"endpoint address: "<<Get_address()<<std::endl;
}
  
void BootstrapSession::Handle_write(const boost::system::error_code& e)
{
  if(e)///Failure
    std::cout<<"write failure"<<"Session id: "<<id_<<"endpoint address: "<<Get_address()<<std::endl;
  Stop();
}

