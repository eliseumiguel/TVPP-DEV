CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lpthread -lboost_thread-mt
SOURCES=main.cpp peerdata.cpp bootstrap.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=bootstrap

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) -I/usr/include/boost $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@



