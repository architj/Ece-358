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
#include <iostream>
#include <vector>
#include <map>

using namespace std;

enum States
{
	START = 0,
	GROUP,
	STUDENT
};

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

int hashf( int g, int s)
{
	return 0.5 * (g + s) * (g + s + 1) + s;
}



int main (int argc, char* argv[] )
{
	int serverSocket, port;

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
	
	// define params for bind and bind the socket and the port
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
	
	// Get IP and port number
	if( getsockname(serverSocket, (struct sockaddr *)(&a), &sockLength) < 0) 
	{
		perror("getsockname");
		exit(0);
	}


	char ip[256];
	inet_ntop(AF_INET, &(a.sin_addr), ip, 256);
	cout << ip << " " << ntohs(a.sin_port) << endl;

	States state = START;
	int groupId = 0, studentId =0;
	string studentName;
	map< int, string > studentMap;
	// stdin info on groups
	string input;
	while( getline(cin, input) && !cin.eof() )
	{

		size_t found = input.find("Group ");
		if(found!= string::npos)
		{
			unsigned pos = input.find(" ");
			string id = input.substr(pos + 1);
			groupId = atoi(id.c_str() );
		}
		else
		{
			size_t found = input.find(" ");
			if(found!= string::npos)
			{
				unsigned pos = input.find(" ");
				string id = input.substr(0, pos);
				studentId = atoi( id.c_str() );
				studentName = input.substr( pos+1 );
				
				// hash and store
				cout << "Group Id: " << groupId << " Student Id: " << studentId << "Student Name: " << studentName << endl;
				int hashNumber = hashf( groupId, studentId);
				studentMap.insert(pair<int, string> (hashNumber, studentName) );
			}
			else 
			{
				perror("Wrong format for student");
			}
		}
		
	}

	cout << "All input over" << endl;	
	// Accept Commands:
		// Get
		// Stop_session
		// Stop

	char buf[256];	
	
	memset(&a, 0, sizeof(struct sockaddr_in));
	
	
		if( recvfrom(serverSocket, buf, 256, 0, (struct sockaddr*)(&a), &sockLength ) > 0 )
		{
 			string req = string(buf);
			unsigned pos1 = req.find(" ");
			
		}
		else
		{
			perror("error in recieving");
		} 
		
	/*
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
	*/	
	
}
