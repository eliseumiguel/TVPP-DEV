CC=g++ 
CFLAGS=-c -Wall 
LDFLAGS=-lpthread -lboost_thread-mt 
SOURCES=main.cpp peerdata.cpp client.cpp nweb.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=client

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) -I/usr/include/boost $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@



