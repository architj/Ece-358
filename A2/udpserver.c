#include <stdio.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>

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


int main (int argc, char* argv[] )
{
	// Create Socket, port = x
	int serverSocket, port, s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	// grab port number
	if(argc < 2)
	{
		port = 0;
	}
	else
	{ 
		port = atoi((const char*)argv[1]);	
	}

	// create a socket for UDP
	if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(0);
	}
	else 
	{
		printf("Opened socket: %d\n", serverSocket);
	}
	
	                  
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	
	s = getaddrinfo(argv[1], NULL, &hints, &result);
	if(s!=0)
	{
		perror("getaddrinfo error");
	}

	
	// define params for bind and use bind
	struct sockaddr_in a;
	a.sin_family = AF_INET;
	a.sin_port = htons(port);
	a.sin_addr.s_addr = INADDR_ANY;
	int addrlen = sizeof(struct sockaddr_in);
	socklen_t sockLength;

	if (bind (serverSocket, (const struct sockaddr *)(&a), sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		exit(0);
	}
	
	// Get port number
	if( getsockname(serverSocket, (struct sockaddr *)(&a), &sockLength) < 0) 
	{
		perror("getsockname");
		exit(0);
	}

	printf("Connecting to port %hu\n", ntohs(a.sin_port));

	char ip[256];
	inet_ntop(AF_INET, &(a.sin_addr), ip, 256);
	printf( ip ); 
	printf("\n");

	// stdout server and port
	
	// stdin info on groups	

	// Accept Commands:
		// Get
		// Stop_session
		// Stop

	char buf[256];
	char getkey[4];
	char stopkey[12];	
	
	
	{
		if( recv(serverSocket, buf, 256, 0 ) <= 0 )
		{
 			printf( buf );
			printf( "\n" );
		}
		else
		{
			perror("error in recieving");
		} 
		/*memcpy(key, buf, 4);
		
		if( strcmp(getkey, "GET " )
		{
			
		}
		else if( strcmp(stopkey, "STOP" )
		{
			
		}
		else if( strcmp(stopkey, "STOP_SESSION" )
		{
			
		}
*/	
	}
	
}