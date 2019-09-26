CC = g++
CFLAGS = -pthread

compile: server.o client.o
	$(CC) server.o $(CFLAGS) -o server
	$(CC) client.o $(CFLAGS) -o client

server.o: server.cpp
	$(CC) -c server.cpp

client.o: client.cpp
	$(CC) -c client.cpp

clean:
	rm -f *.o
