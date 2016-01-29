CC=g++
CFLAGS=-m32 -c -g3 -Wall
STATIC=-static
LDFLAGS=-lboost_thread-mt -lpthread -lboost_system-mt
BIN_DIR=../bin
C_DIR=./client
B_DIR=./bootstrap
EM_DIR=./externalMessage
COMMON_DIR=./common
MESSAGE_DIR=$(COMMON_DIR)/Messages
SCHEDULER_DIR=$(COMMON_DIR)/Scheduler
TEMPORIZABLE_DIR=$(C_DIR)/Temporizable
MESSAGE_SOURCES=$(MESSAGE_DIR)/Message.cpp $(MESSAGE_DIR)/MessageServerSub.cpp $(MESSAGE_DIR)/MessageStateChannel.cpp $(MESSAGE_DIR)/MessagePing.cpp $(MESSAGE_DIR)/MessagePingBoot.cpp $(MESSAGE_DIR)/MessagePingBootPerf.cpp $(MESSAGE_DIR)/MessageRequest.cpp $(MESSAGE_DIR)/MessageData.cpp $(MESSAGE_DIR)/MessageError.cpp $(MESSAGE_DIR)/MessagePeerlist.cpp $(MESSAGE_DIR)/MessagePeerlistShare.cpp $(MESSAGE_DIR)/MessagePeerlistLog.cpp  $(MESSAGE_DIR)/MessageChannel.cpp
SCHEDULER_SOURCES=$(SCHEDULER_DIR)/FIFOMessageScheduler.cpp $(SCHEDULER_DIR)/RRMessageScheduler.cpp $(SCHEDULER_DIR)/CDFMessageScheduler.cpp $(SCHEDULER_DIR)/RandomMessageScheduler.cpp
COMMON_SOURCES=$(MESSAGE_SOURCES) $(SCHEDULER_SOURCES) $(COMMON_DIR)/XPConfig.cpp $(COMMON_DIR)/AddressedMessage.cpp $(COMMON_DIR)/Sub-Channel-Data.cpp $(COMMON_DIR)/Channel.cpp $(COMMON_DIR)/PeerData.cpp $(COMMON_DIR)/Peer.cpp $(COMMON_DIR)/UdpBuffer.cpp $(COMMON_DIR)/ChunkUniqueID.cpp $(COMMON_DIR)/HeadedBitset.cpp $(COMMON_DIR)/tcp-server.cpp $(COMMON_DIR)/tcp-session.cpp $(COMMON_DIR)/udp.cpp 
TEMPORIZABLE_SOURCES=$(TEMPORIZABLE_DIR)/LeakyBucket.cpp $(TEMPORIZABLE_DIR)/Disconnector.cpp $(TEMPORIZABLE_DIR)/PeerRequester.cpp $(TEMPORIZABLE_DIR)/Connector.cpp
C_SOURCES=$(C_DIR)/main.cpp $(COMMON_SOURCES) $(TEMPORIZABLE_SOURCES) $(C_DIR)/client.cpp $(C_DIR)/Statistics.cpp $(C_DIR)/MediaBuffer.cpp $(C_DIR)/PeerManager.cpp $(C_DIR)/PeerManagerServerAux.cpp $(C_DIR)/Request.cpp $(C_DIR)/RequestAttempt.cpp $(C_DIR)/client-player-session.cpp $(C_DIR)/client-tcp-server.cpp $(C_DIR)/client-bootstrap-session.cpp
B_SOURCES=$(B_DIR)/main.cpp $(COMMON_SOURCES) $(B_DIR)/bootstrap.cpp $(B_DIR)/bootstrap-session.cpp $(B_DIR)/bootstrap-tcp-server.cpp 
EM_SOURCES=$(EM_DIR)/main.cpp $(COMMON_SOURCES) $(EM_DIR)/externalMessageTCP.cpp
C_OBJECTS=$(C_SOURCES:.cpp=.o)
B_OBJECTS=$(B_SOURCES:.cpp=.o)
EM_OBJECTS=$(EM_SOURCES:.cpp=.o)

all: bootstrap client externalMessage

bootstrap: $(B_OBJECTS)
	$(CC) $(B_OBJECTS) $(LDFLAGS) -I/usr/include/boost -o $(BIN_DIR)/$@

client: $(C_OBJECTS)
	$(CC) $(C_OBJECTS) $(LDFLAGS) -I/usr/include/boost -o $(BIN_DIR)/$@
	
externalMessage: $(EM_OBJECTS)
	$(CC) $(EM_OBJECTS) $(LDFLAGS) -I/usr/include/boost -o $(BIN_DIR)/$@

sbootstrap: $(B_OBJECTS)
	$(CC) $(B_OBJECTS) $(STATIC) $(LDFLAGS) -I/usr/include/boost -o $(BIN_DIR)/$@

sclient: $(C_OBJECTS)
	$(CC) $(C_OBJECTS) $(STATIC) $(LDFLAGS) -I/usr/include/boost -o $(BIN_DIR)/$@
	
sexternalMessage: $(EM_OBJECTS)
	$(CC) $(EM_OBJECTS) $(STATIC) $(LDFLAGS) -I/usr/include/boost -o $(BIN_DIR)/$@

clean:
	find . -name "*.o" | xargs rm
	rm -f $(BIN_DIR)/client $(BIN_DIR)/bootstrap $(BIN_DIR)/sclient $(BIN_DIR)/sbootstrap $(BIN_DIR)/externalMessage $(BIN_DIR)/sexternalMessage
	clear

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
