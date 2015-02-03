#ifndef UDP_HPP
#define UDP_HPP

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <string>
#include <iostream>
#include "Defines.hpp"

#include "AddressedMessage.hpp"
#include "Scheduler/IMessageScheduler.hpp"

#define MAX_TRIES 1000

class UDPServer {
  
public:
   UDPServer(uint32_t port, bool mode, IMessageScheduler* sendScheduler, IMessageScheduler* receiveScheduler);
   uint32_t Get_binded_port();
   void Send(std::string address, uint32_t port, unsigned char *data, uint32_t size);
   void Send(std::string addr, unsigned char *data, uint32_t size);
   void Recv();
   int32_t Sync_recv(uint8_t *, uint32_t, uint32_t);
   AddressedMessage* GetNextMessageToReceive();
   void EnqueueSend(std::string address, Message* message);
   AddressedMessage* GetNextMessageToSend();
   void Run();
   virtual void Start();
   virtual void Stop();
     
protected:
  bool Bind(bool mode);
  virtual void Handle_send( const boost::system::error_code& error, std::size_t bytes_transferred); 
  virtual void Handle_recv( const boost::system::error_code& error, std::size_t bytes_transferred);
  boost::asio::io_service io_service_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint remote_endpoint_;
  uint32_t port_;
  typedef boost::array<unsigned char, MTU> frame;
  frame buffer_;
  IMessageScheduler* receiveScheduler;
  boost::condition_variable emptyReceiveBuff_;
  boost::mutex emptyReceiveBuffMutex_;
  IMessageScheduler* sendScheduler;
  boost::condition_variable emptySendBuff_;
  boost::mutex emptySendBuffMutex_;
  
};


#endif // UDP_HPP