/*
 * changeChannelState.cpp
 *
 *  Created on: Feb 5, 2015
 *      Author: eliseu
 */


#include "externalMessageTCP.hpp"


    ExternalMessageTCP::ExternalMessageTCP(std::string ip, std::string port)
        : io_service()
        , socket(io_service)
    {
        boost::asio::ip::tcp::resolver resolver(this->io_service);
        boost::asio::ip::tcp::resolver::query query(ip, port);
        boost::asio::ip::tcp::resolver::iterator end;

        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        boost::system::error_code error = boost::asio::error::host_not_found;

        while(error && endpoint_iterator != end)
        {
            this->socket.close();
            this->socket.connect(*endpoint_iterator, error);
            ++endpoint_iterator;
        }

        if (error)
            throw boost::system::system_error(error);
    }

    size_t ExternalMessageTCP::Sync_read(uint8_t* data, uint32_t dataSize)
    {
        return boost::asio::read(this->socket, boost::asio::buffer(data, dataSize));
    }

    size_t ExternalMessageTCP::Sync_write(uint8_t* data, uint32_t dataSize)
    {
        return boost::asio::write(this->socket, boost::asio::buffer(data, dataSize));
    }

