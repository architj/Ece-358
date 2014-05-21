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

int main (int argc, const char* argv[]) {
	if (argc < 3) {
		std::cout << "usage: " << argv[0] << " <server name/ip> <server port>"<<std::endl;
		exit(0);
	}
	int portNumber = atoi(argv[2]);
	int studentId = 0;
	int groupId = 0;
	//Get Student ID from stdin
	std::cin >> groupId >> studentId;

	//if EOF, send STOP_SESSION

	//construct address structs
	struct sockaddr_in a, sa;
	a.sin_family = AF_INET;
	a.sin_port = 0;
	a.sin_addr.s_addr = INADDR_ANY;

	int portnum;
	
	struct addrinfo *res, *cai, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	//get socket
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	//get host address
	std::cout<<"GETTING ADDRESS INFORMATION"<<std::endl;	
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
	std::stringstream ss;
	ss << "GET " << groupId << " " << studentId;
	std::string requestString = ss.str();
	std::cout<< "\n REQUEST: " << requestString<<std::endl;
	char requestBuffer[256], responseBuffer[256];
	strcpy (requestBuffer, requestString.c_str());

	//sending to host
	int len;
	if ((len = sendto(s, requestBuffer, strlen(requestBuffer) + 1, 0,  cai->ai_addr, sizeof(struct sockaddr_in))) < strlen(requestBuffer) + 1) {
		std::cout<<"Send Failed. Sent Only " << len << " of " << strlen(requestBuffer) << std::endl;
}

	memset(responseBuffer, 0, 256);
	recvfrom(s, responseBuffer, 256, 0, NULL, NULL);
	std::cout<<"Client received: " << responseBuffer;
	close (s);
	return 0;
}
