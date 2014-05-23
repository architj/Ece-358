#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ifaddrs.h>
#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include <unistd.h>

using namespace std;

int main (int argc, const char* argv[]) {
	//Check if correct number of arguments was provided
	if (argc < 3) {
		//output error and terminate
		cout << "usage: " << argv[0] << " <host name/ip> <host port>"<<endl;
		exit(0);
	}

	//obtain port number from argv
	int portNumber = atoi(argv[2]);
	string studentId = "";
	string groupId = "";
	
	//construct address structs
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;

	struct addrinfo *res, *cai, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	#if defined ( UDP )	
	//create UDP socket and obtain socket descriptor
	int sockDscrptr = socket(AF_INET, SOCK_DGRAM, 0);
	#else
	//create TCP socket and obtain socket descriptor
	int sockDscrptr = socket(AF_INET, SOCK_STREAM, 0);
	#endif

	//get host address
	if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) {
		perror("getaddrinfo error");
	}

	//find the correct addrinfo that is in the INET address family	
	for (cai = res; cai != NULL; cai = cai->ai_next) {
		if (cai->ai_family == AF_INET) {
			printf("server ip: %s\n", inet_ntoa(((struct sockaddr_in *) (cai->ai_addr))->sin_addr));
			memcpy (&sa, cai->ai_addr, sizeof(struct sockaddr_in));
			break;
		}
	}

	//assign port to sa. sa has the ip address already from the memcpy
	sa.sin_port = htons(portNumber);	

	#if defined ( TCP )
	connect(sockDscrptr, (struct sockaddr *) &sa, sizeof(sa));
	#endif

	//construct Request and response buffers
	char requestBuffer[256], responseBuffer[1000];

	//Loop until user inputs STOP
	while(1) {
		//Reset the buffers
		memset(&requestBuffer, 0, sizeof(requestBuffer));
		memset(&responseBuffer, 0, sizeof(responseBuffer));		

		//Get Student ID and group id from stdin
		cin >> groupId;
		if( groupId != "STOP") {
			cin >> studentId;
		}

		//if EOF, send STOP_SESSION
		if(cin.eof()) {
			cout << "EOF" << endl;
			strcpy (requestBuffer, "STOP_SESSION");	
		}
		//if STOP received as input, send STOP 
		else if (studentId == "STOP" || groupId == "STOP"){
			strcpy (requestBuffer, "STOP");
		}
		//otherwise, send a GET request 
		else {
			stringstream ss;
			ss << "GET " << groupId << " " << studentId;
			string requestString = ss.str();
			cout<< "\n REQUEST: " << requestString<<endl;
			strcpy (requestBuffer, requestString.c_str());
		}

		//sending to host
		int len;
		if ((len = sendto(sockDscrptr, requestBuffer, strlen(requestBuffer) + 1, 0, (const struct sockaddr*) &sa, sizeof(sa))) < strlen(requestBuffer) + 1) {
			cout<<"Send Failed. Sent Only " << len << " of " << strlen(requestBuffer) << endl;
	}

		cout<<"ABOUT TO RECEIVE: " <<endl;
		//receive the servers response and output it
		recvfrom(sockDscrptr, responseBuffer, 1000, 0, NULL, NULL);
		cout<<"Client received: " << responseBuffer << endl;;
	}
	close (sockDscrptr);
	return 0;
}
