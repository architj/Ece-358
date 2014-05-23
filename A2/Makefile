OBJECTS = udpclient.o udpserver.o

EXEC = default

.PHONY : clean

all : ${EXEC}

.PHONY : ${EXEC}

${EXEC} :
	g++ client.cc -o "udpclient" -DUDP
	g++ server.cc -o "udpserver" -DUDP
	g++ client.cc -o "tcpclient" -DTCP
	g++ server.cc -o "tcpserver" -DTCP

clean :
	rm -f *.d *.o client server *.class
