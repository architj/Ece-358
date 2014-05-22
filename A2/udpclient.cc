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
	if (argc < 3) {
		cout << "usage: " << argv[0] << " <server name/ip> <server port>"<<endl;
		exit(0);
	}
	int portNumber = atoi(argv[2]);
	string studentId = "";
	string groupId = "";
	
	//Get Student ID from stdin
	cin >> groupId;
	if( groupId != "STOP") {
		cin >> studentId;
	}
	
	//construct address structs
	struct sockaddr_in a, sa;
	a.sin_family = AF_INET;
	a.sin_port = htons(portNumber);
	a.sin_addr.s_addr = INADDR_ANY;

	struct addrinfo *res, *cai, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	//get socket
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	//get host address
	cout<<"GETTING ADDRESS INFORMATION"<<endl;	
	if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) {
		perror("getaddrinfo error");
	}
	
	for (cai = res; cai != NULL; cai = cai->ai_next) {
		if (cai->ai_family == AF_INET) {
			printf("server ip: %s\n", inet_ntoa(((struct sockaddr_in *) (cai->ai_addr))->sin_addr));
			memcpy (&sa, cai->ai_addr, sizeof(struct sockaddr_in));
			break;
		}
	}

	//construct Request
	char requestBuffer[256], responseBuffer[256];

	//if EOF, send STOP_SESSION
	if(cin.eof()) {
		cout << "EOF" << endl;
		strcpy (requestBuffer, "STOP_SESSION");	
	} else if (studentId == "STOP" || groupId == "STOP"){
		strcpy (requestBuffer, "STOP");
	} else {
		stringstream ss;
		ss << "GET " << groupId << " " << studentId;
		string requestString = ss.str();
		cout<< "\n REQUEST: " << requestString<<endl;
		strcpy (requestBuffer, requestString.c_str());
	}

	//sending to host
	int len;
	if ((len = sendto(s, requestBuffer, strlen(requestBuffer) + 1, 0,  cai->ai_addr, sizeof(struct sockaddr_in))) < strlen(requestBuffer) + 1) {
		cout<<"Send Failed. Sent Only " << len << " of " << strlen(requestBuffer) << endl;
}

	memset(responseBuffer, 0, 256);
	cout<<"ABOUT TO RECEIVE: " <<endl;
	recvfrom(s, responseBuffer, 256, 0, NULL, NULL);
	cout<<"Client received: " << responseBuffer;
	close (s);
	return 0;
}
