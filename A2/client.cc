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

	struct addrinfo *res, *temp, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	#if defined ( UDP )	
	//create UDP socket and obtain socket descriptor
	int sockDscrptr = socket(AF_INET, SOCK_DGRAM, 0);
	hints.ai_socktype = SOCK_DGRAM;
	#else
	//create TCP socket and obtain socket descriptor
	int sockDscrptr = socket(AF_INET, SOCK_STREAM, 0);
	hints.ai_socktype = SOCK_STREAM;
	#endif

	//get host address
	if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) {
		perror("getaddrinfo error");
	}

	//find the correct addrinfo that is in the INET address family	
	for (temp = res; temp != NULL; temp = temp->ai_next) {
		if (temp->ai_family == AF_INET) {
			memcpy (&sa, temp->ai_addr, sizeof(struct sockaddr_in));
			break;
		}
	}

	//assign port to sa. sa has the ip address already from the memcpy
	sa.sin_port = htons(portNumber);	

	#if defined ( TCP )
	connect(sockDscrptr, (struct sockaddr *) &sa, sizeof(sa));
	#endif

	//construct Request and response buffers
	char requestBuffer[1000], responseBuffer[1000];
	
	bool isGetRequest = false;

	//Loop until user inputs STOP
	while(1) {
		//Reset the buffers and state variables
		memset(&requestBuffer, 0, sizeof(requestBuffer));
		memset(&responseBuffer, 0, sizeof(responseBuffer));				
		isGetRequest = false;

		//Get Student ID and group id from stdin
		cin >> groupId;
		if( groupId != "STOP") {
			cin >> studentId;
		}

		//if EOF, send STOP_SESSION
		if(cin.eof()) {
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
			strcpy (requestBuffer, requestString.c_str());
			isGetRequest = true;
		}

		//sending to host
		int len;
		if ((len = sendto(sockDscrptr, requestBuffer, strlen(requestBuffer) , 0, (const struct sockaddr*) &sa, sizeof(sa))) < strlen(requestBuffer)) {
			cout<<"Send Failed. Sent Only " << len << " of " << strlen(requestBuffer) << endl;
		}

		//Only receive if a GET was sent
		if(isGetRequest) {
			//receive the servers response and output it
			recvfrom(sockDscrptr, responseBuffer, 1000, 0, NULL, NULL);
			cout<< responseBuffer << endl;
		 }
		else {
			break;
		}
	}
	//Close socket
	close (sockDscrptr);
	return 0;
}
