#include "udp.hpp"

/*
 *  Method: UDPServer::UDPServer()
 *   Descr: 
 */
UDPServer::UDPServer(uint32_t port,bool mode, IMessageScheduler* sendScheduler, IMessageScheduler* receiveScheduler):io_service_(),socket_(io_service_)
{
    port_ = port;
    this->receiveScheduler = receiveScheduler;
    this->sendScheduler = sendScheduler;
    Bind(mode); 
}


/*
 *  Method: UDPServer::Get_binded_port()
 *   Descr: 
 */
uint32_t UDPServer::Get_binded_port()
{
  return port_;
}


/*
 *  Method: UDPServer::Send()
 *   Descr: 
 */
void UDPServer::Send(std::string IP,uint32_t port,unsigned char *data,uint32_t size)
{
  boost::asio::ip::udp::endpoint destination(
    boost::asio::ip::address::from_string(IP), port);
  
  socket_.async_send_to(
    boost::asio::buffer(data, size), 
    destination,
    boost::bind(&UDPServer::Handle_send, 
    this,
    boost::asio::placeholders::error,
    boost::asio::placeholders::bytes_transferred));
}

/*
 *  Method: UDPServer::Send()
 *   Descr: addr = xxx.xxx.xxx.xxx:xxxxx
 *                 IP:port  
 */
void UDPServer::Send(std::string addr,unsigned char *data,uint32_t size)
{
  std::string ip;
  ip.append(addr.begin(),addr.begin()+addr.find(":"));
  std::string sport;
  sport.append(addr.begin()+addr.find(":")+1,addr.end());
  uint32_t port = boost::lexical_cast<uint32_t>(sport);
  //std::cout<<"sent to: "<<ip<<" "<<port<<std::endl;
  Send(ip,port,data,size);
  
}

/*
 *  Method: UDPServer::Recv()
 *   Descr: 
 */
void UDPServer::Recv()
{
  socket_.async_receive_from(
    boost::asio::buffer(buffer_.c_array(), MTU),remote_endpoint_,  
    boost::bind(&UDPServer::Handle_recv, 
    this,
    boost::asio::placeholders::error,
    boost::asio::placeholders::bytes_transferred));
}

int32_t UDPServer::Sync_recv(uint8_t *buffer,uint32_t size,uint32_t start_pos)
{
    int32_t recv;
    boost::system::error_code ec;
    recv = socket_.receive_from(boost::asio::buffer(buffer+start_pos, size),remote_endpoint_,0,ec);
    if(ec)//Failure
      return -1;
    else//Success
      return recv;
}


/*
 *  Method: UDPServer::Start()
 *   Descr: 
 */
void UDPServer::Start()
{
  Recv();
}


/*
 *  Method: UDPServer::Stop()
 *   Descr: 
 */
void UDPServer::Stop()
{
  socket_.close();
}


/*
 *  Method: UDPServer::Run()
 *   Descr: 
 */
void UDPServer::Run()
{
  io_service_.run();
}


/*
 *  Method: UDPServer::Bind()
 *   Descr: 
 */
bool UDPServer::Bind(bool mode)
{
  socket_.open(boost::asio::ip::udp::v4());
  boost::system::error_code ec;
  uint32_t attemps = 0;
  
  while(attemps < MAX_TRIES)
  {
    socket_.bind(boost::asio::ip::udp::endpoint(
          boost::asio::ip::udp::v4(), port_), ec);
    if (ec)///Error
    {
      if(mode)
        port_++;
      attemps++;
    }
    else///Success
    {
      std::cout<<"UDPServer binded to port: "<<port_<<std::endl;
      return true;
    }
  }
  std::cout<<"Failed to bind punch port"<<std::endl;
  port_ = 0;
  return false;
}


/*
 *  Method: UDPServer::Handle_send()
 *   Descr: 
 */
void UDPServer::Handle_send(const boost::system::error_code &error, std::size_t bytes_transferred)
{
  if(!error)
  {
    
  }
  else
    std::cout<<"UDP send error"<<std::endl;
}


/*
 *  Method: UDPServer::Handle_recv()
 *   Descr: 
 */
void UDPServer::Handle_recv(const boost::system::error_code &error, std::size_t bytes_transferred)
{
    if(!error)
    { 
        string address = remote_endpoint_.address().to_string();
        address += ":";
        address += boost::lexical_cast<std::string>(remote_endpoint_.port());

        uint8_t* data = buffer_.c_array();

        Message preMessage(new uint8_t[MESSAGE_HEADER_SIZE]);
        for(int i = 0; i < MESSAGE_HEADER_SIZE; i++)
        {
            preMessage.GetFirstByte()[i] = data[i];
        }
        uint32_t msgSize = preMessage.GetHeaderSize() + preMessage.GetBodySize();
        Message* message = new Message(new uint8_t[msgSize]);
        for(uint32_t i = 0; i < msgSize; i++)
        {
            message->GetFirstByte()[i] = data[i];
        }
        
        if (message->CheckIntegrity() == true)
        {
            AddressedMessage* aMessage = new AddressedMessage(address, message);
            //Send it to the UDP Receive Scheduler
            receiveScheduler->Push(aMessage);
        }

        emptyReceiveBuff_.notify_one();
    } else
        std::cout<<"UDP recv error"<<std::endl;

    Recv();
}

/*
 *  Method: UDPServer::Processing_begin()
 *   Descr: Starts Data processing  
 */
AddressedMessage* UDPServer::GetNextMessageToReceive()
{
    boost::unique_lock<boost::mutex> lock(emptyReceiveBuffMutex_);
    if(receiveScheduler->GetSize() <= 0)
    {
      emptyReceiveBuff_.wait(lock);
    }
    return receiveScheduler->Pop();
    ///After this functions returns, the data can be processed by the application
}

void UDPServer::EnqueueSend(std::string address, Message* message)
{
    if (message && (message->CheckIntegrity() == true))
    {
        if (sendScheduler)
        {
            AddressedMessage* aMessage = new AddressedMessage(address, message);
            //Send it to the UDP Send Scheduler
            sendScheduler->Push(aMessage);
        
            emptySendBuff_.notify_one();
        }
        else
            Send(address, message->GetFirstByte(), message->GetSize());
    }
}

AddressedMessage* UDPServer::GetNextMessageToSend()
{
    if (sendScheduler)
    {
        boost::unique_lock<boost::mutex> lock(emptySendBuffMutex_);
        if(sendScheduler->GetSize() <= 0)
        {
          emptySendBuff_.wait(lock);
        }
        return sendScheduler->Pop();
        ///After this functions returns, the data can be processed by the application
    }
    else
        return NULL;
}
