
CXX= g++
CFLAGS= -std=c++11
LIBS= -lpthread -pthread -lcurl -lcrypto -lssl -lev
OBJS= client.o Protocol.o Requests.o base64.o utilities.o debug.o Jwt.o GameClient.o

all: build image

image:
	$(CXX) $(CFLAGS) $(OBJS) $(LIBS) -o client

build: main Protocol Requests base64 utilities debug Jwt GameClient

main: client.cpp
	$(CXX) $(CFLAGS) -c -g client.cpp -o client.o

Protocol: Protocol.cpp Protocol.h
	$(CXX) $(CFLAGS) -c -g Protocol.cpp -o Protocol.o

Requests: Requests.cpp Requests.h
	$(CXX) $(CFLAGS) -c -g Requests.cpp -o Requests.o

base64: base64.cpp base64.h 
	$(CXX) $(CFLAGS) -c -g base64.cpp -o base64.o

utilities: utilities.cpp utilities.h
	$(CXX) $(CFLAGS) -c -g utilities.cpp -o utilities.o

debug: debug.cpp debug.h
	$(CXX) $(CFLAGS) -c -g debug.cpp -o debug.o

Jwt: Jwt.cpp Jwt.h
	$(CXX) $(CFLAGS) -c -g Jwt.cpp -o Jwt.o

GameClient: GameClient.cpp GameClient.h
	$(CXX) $(CFLAGS) -c -g GameClient.cpp -o GameClient.o

clean:
	rm -rf *.o client

rebuild_all: clean build
