OBJECTS = udpclient.o udpserver.o

EXEC = client

.PHONY : clean

all : ${EXEC}

.PHONY : ${EXEC}

${EXEC} :
	g++ udpclient.cc -o "udpclient" -DUDP
#	g++ udpserver.cc -o "udpserver" -DTCP

clean :
	rm -f *.d *.o client server *.class
