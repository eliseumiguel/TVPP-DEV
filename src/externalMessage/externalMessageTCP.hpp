/*
 * externalMessageTCP.hpp
 *
 *  Created on: Feb 5, 2015
 *      Author: eliseu
 *      Objeto responsável para enviar uma mensagem externa ao TVPP
 *      para um dos participantes da malha ou servidores (vídeo ou bootstrap)
 */

#ifndef EXTERNALMESSAGETCP_HPP_
#define EXTERNALMESSAGETCP_HPP_

#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/utility.hpp>


class ExternalMessageTCP
{
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket;

public:

    ExternalMessageTCP(std::string ip, std::string port);

    size_t Sync_read(uint8_t* data, uint32_t dataSize);

    size_t Sync_write(uint8_t* data, uint32_t dataSize);

};
#endif //EXTERNALMESSAGETCP_HPP_
