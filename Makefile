
CXX= g++
CFLAGS= -std=c++11
LIBS= -lpthread -pthread -lcurl -lcrypto -lssl
OBJS= client.o Protocol.o Requests.o Jwt.o base64.o utilities.o

all: client

client: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) $(LIBS) -o client

client.o: client.cpp
	$(CXX) $(CFLAGS) -c client.cpp -o client.o

Protocol.o: Protocol.cpp Protocol.h
	$(CXX) $(CFLAGS) -c Protocol.cpp -o Protocol.o

Requests.o: Requests.cpp Requests.h Protocol.h
	$(CXX) $(CFLAGS) -c Requests.cpp -o Requests.o

Jwt.o: Jwt.cpp Jwt.h
	$(CXX) $(CFLAGS) -c Jwt.cpp -o Jwt.o

base64.o: base64.cpp base64.h 
	$(CXX) $(CFLAGS) -c base64.cpp -o base64.o

utilities.o: utilities.cpp utilities.h
	$(CXX) $(CFLAGS) -c utilities.cpp -o utilities.o

clean:
	rm -rf *.o

rebuild_all: clean all