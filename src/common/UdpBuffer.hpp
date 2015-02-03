#ifndef UDP_BUFFER_H
#define UDP_BUFFER_H

#include <string>
#include <sys/socket.h>
#include <boost/lexical_cast.hpp>
#include <netdb.h>
#include <arpa/inet.h>

#include "Defines.hpp"

using namespace std;

class UDPBuffer {

    private:
        static unsigned char Buffer[10000][MTU];
        struct sockaddr_storage address[10000];
        int size[10000];
        size_t addr_len;
        const char *j_inet_ntop(struct sockaddr_storage *src, char *dst, size_t size);
        int j_inet_getport(struct sockaddr_storage *sa);
        
    public:
        UDPBuffer();
        void Recv_data(int, int);
        unsigned char *Get_buffer(int);
        sockaddr_storage Get_addr(int);            
        string Get_ip(int);
        string Get_port(int);
        int Get_size(int);
};























#endif // CHANNEL_H

