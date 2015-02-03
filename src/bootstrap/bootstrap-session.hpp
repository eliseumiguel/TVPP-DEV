#ifndef BOOTSTRAPSESSION_H_INCLUDED
#define BOOTSTRAPSESSION_H_INCLUDED

#include "../common/tcp-session.hpp"
#include "../common/tcp-server.hpp"

class BootstrapSession : public TCPSession
{
public:
  
  BootstrapSession(boost::asio::io_service& io_service, TCPServer *tcp_server, uint32_t id);  
  
  /// start the session
  void Start();
  
  /// stop the session
  void Stop();

protected:
  
  void Handle_read_header(const boost::system::error_code& e, std::size_t bytes_transferred);
  
  void Handle_read_body(const boost::system::error_code& e, std::size_t bytes_transferred);
  
  void Handle_write(const boost::system::error_code& e);
  
  uint32_t bytes_to_recv_;
};

#endif // BOOTSTRAPSESSION_H_INCLUDED
