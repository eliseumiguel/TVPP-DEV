#ifndef DEFINES_H
#define DEFINES_H

#define VERSION 0x010B

#define MTU 1500
#define STD_BUFFERSIZE 1600
#define TTL 10

#define SECONDS 1000000000

//Message Header Sizes
#define MESSAGE_HEADER_SIZE                      6
#define MESSAGE_CHANNEL_HEADER_SIZE             20
#define MESSAGE_REQUEST_HEADER_SIZE             18
#define MESSAGE_ERROR_HEADER_SIZE                8
#define MESSAGE_PEERLIST_HEADER_SIZE            10
#define MESSAGE_PEERLIST_SHARE_HEADER_SIZE      38
#define MESSAGE_PEERLIST_LOG_HEADER_SIZE        18
#define MESSAGE_PING_HEADER_SIZE                14
#define MESSAGE_PING_BOOT_HEADER_SIZE           22
#define MESSAGE_PING_BOOT_PERF_HEADER_SIZE      94
#define MESSAGE_DATA_HEADER_SIZE                22

//Client Operation Modes
enum PeerModes
{
    MODE_CLIENT            = 0x00,          // Modo Client
    MODE_SERVER            = 0x01,          // Modo Server
    MODE_FREERIDER         = 0x02,          // Modo Freerider
    MODE_FULLCHUNKMAP      = 0x03,          // Modo Anunciando FULL ChunkMap
    MODE_SUPERNODE         = 0x04,          // Modo Supernode
};

//Operation Codes
enum Opcodes
{
    OPCODE_CHANNEL      = 0x00,
    OPCODE_PEERLIST     = 0x01,
    OPCODE_PING         = 0x02,
    OPCODE_REQUEST      = 0x03,
    OPCODE_DATA         = 0x04,
    OPCODE_ERROR        = 0x05,
};

//CHANNEL MESSAGES FLAGS
enum ChannelFlags
{
    CHANNEL_CREATE      = 0x00,
    CHANNEL_CONNECT     = 0x01,
};

//PING MESSAGES FLAGS
enum PingTypes
{
    PING_BASE           = 0x00,     // PING BASE
    PING_BOOT           = 0x01,     // PING TO BOOTSTRAP
    PING_BOOT_PERF      = 0x02,     // PING TO BOOTSTRAP WITH PERFORMANCE INFORMATION
    PING_PART_PERF      = 0x03,     // PING TO PARTNER WITH PERFORMANCE INFORMATION
    PING_PART_CHUNKMAP  = 0x04      // PING TO PARTNER WITH CHUNK MAP
};

//PEERLIST MESSAGES FLAGS
enum PeerlistTypes
{
    PEERLIST_BASE          = 0x00,     // PEERLIST BASE
    PEERLIST_SHARE         = 0x01,     // PEERLIST TO SHARE PARTNERS
    PEERLIST_LOG           = 0x02      // PEERLIST TO LOG
};

//ERROR MESSAGES FLAGS
enum ErrorTypes
{
    ERROR_NONE                      = 0x00,
    ERROR_INVALID_CLIENT_VERSION    = 0x01,
    ERROR_CHANNEL_CANT_BE_CREATED   = 0x02,
    ERROR_CHANNEL_UNAVAILABLE       = 0x03,
    ERROR_CHUNK_UNAVAILABLE         = 0x04,
    ERROR_UPLOAD_LIMIT_EXCEEDED     = 0x05,
    ERROR_NO_PARTNERSHIP            = 0x06,
};

#endif // DEFINES_H
