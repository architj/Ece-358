#include <stdio.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

struct student
{
	int studentno;
	char name[256];
};

struct group
{
	int number;
	student students[256];
};


int main (int argc, char* argv() )
{
	// Create Socket, port = x
	int serverSocket;
	unsigned short port;
	if(argc < 2)
		port = 0;//(unsigned short) atoi(argv[1]);
	else 
		port = 0;	

	if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(0);
	}
	else 
	{
		printf("Opened socket: %d\n", serverSocket);
	}
	
	// define params for bind and use bind
	struct sockaddr_in in;
	in.sin_family = AF_INET;
	in.sin_port = htons(port);
	in.sin_addr.s_addr = INADDR_ANY;
	int addrlen = sizeof(struct sockaddr_in);
	

	// stdout server and port
	
	// stdin info on groups	

	// while for commands:
		// Get
		// Stop_session
		// Stop
	char buf[256];
	char getkey[4];
	char stopkey[12];
	memset(&in, 0, sizeof(struct sockaddr_in));
	/*
	while(1)
	{
		recvfrom(serverSocket, buf, 256, 0, (struct sockaddr*) (&in), &addrlen );
 		memcpy(key, buf, 4);
		
		if( strcmp(getkey, "GET " )
		{
			
		}
		else if( strcmp(stopkey, "STOP" )
		{
			
		}
		else if( strcmp(stopkey, "STOP_SESSION" )
		{
			
		}
	}
	*/
}
