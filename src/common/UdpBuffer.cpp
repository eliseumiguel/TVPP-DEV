#include "UdpBuffer.hpp"

unsigned char UDPBuffer::Buffer[10000][MTU];

UDPBuffer::UDPBuffer()
{
    addr_len = sizeof address[0];
}

void UDPBuffer::Recv_data(int i, int socket){
    size[i] = recvfrom(socket, Buffer[i], MTU , 0, (struct sockaddr *)&address[i], (socklen_t*)&addr_len);
    if (size[i] <= 0)
    {
        //std::cout << "Possivelmente algo deu errado em UDPBuffer::Recv_data (erro na recepção ou msg vazia)!" <<std::endl; 
    }
}

unsigned char *UDPBuffer::Get_buffer(int i)
{
    return Buffer[i];
}

sockaddr_storage UDPBuffer::Get_addr(int i)
{
    return address[i];
}            
string UDPBuffer::Get_ip(int i)
{
    char IP[45];
    j_inet_ntop(&address[i], IP, 45);
    string out;
    if ( IP != NULL)
        out = IP;
    else
        out = "";
    return out;

}
string UDPBuffer::Get_port(int i)
{
    return boost::lexical_cast<string>(j_inet_getport(&address[i]));
}

int UDPBuffer::Get_size(int i)
{
    return size[i];
}

int UDPBuffer::j_inet_getport(struct sockaddr_storage *sa)
{
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;
    
    switch (sa->ss_family)
    {
        case AF_INET:
            sin = (struct sockaddr_in *)sa;
            return ntohs(sin->sin_port);
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *)sa;
            return ntohs(sin6->sin6_port);
        default:
            return 0;
    }
}

const char *UDPBuffer::j_inet_ntop(struct sockaddr_storage *src, char *dst, size_t size)
{
     struct sockaddr_in *sin;
     struct sockaddr_in6 *sin6;
 
     sin = (struct sockaddr_in *)src;
     sin6 = (struct sockaddr_in6 *)src;
 
     switch(src->ss_family)
     {
     case AF_UNSPEC:
         return NULL;
     case AF_INET:
         return inet_ntop(AF_INET, &sin->sin_addr, dst, size);
     case AF_INET6:
         return inet_ntop(AF_INET6, &sin6->sin6_addr, dst, size);
     default:
         return NULL;
     }
 }

