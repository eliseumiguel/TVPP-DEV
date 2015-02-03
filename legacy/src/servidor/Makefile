CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lpthread -lboost_thread
SOURCES=peerdata.cpp ../client/client.cpp servidor.cpp  main.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=servidor

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@



