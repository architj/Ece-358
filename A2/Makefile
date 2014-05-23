OBJECTS = udpclient.o udpserver.o

EXEC = default

.PHONY : clean
.PHONY : tcp
.PHONY : udp

all : ${EXEC}

.PHONY : ${EXEC}

${EXEC} :
	g++ client.cc -o "udpclient" -DUDP
	g++ server.cc -o "udpserver" -DUDP
	g++ client.cc -o "tcpclient" -DTCP
	g++ server.cc -o "tcpserver" -DTCP

clean :
	rm -f *.d *.o  udpclient udpserver tcpclient tcpserver client server

tcp :
	g++ client.cc -o "client" -DTCP
	g++ server.cc -o "server" -DTCP

udp :
	g++ client.cc -o "client" -DUDP
	g++ server.cc -o "server" -DUDP
