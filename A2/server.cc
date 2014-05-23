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
#include <signal.h>

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

int serverSocket;

void term( int signum )
{
	close(serverSocket);
	exit(0);
}

int main (int argc, char* argv[] )
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction) );
	action.sa_handler = term;
	sigaction(SIGTERM, &action, NULL);
	
	int port;
	
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
	struct sockaddr_in a, client;
	a.sin_family = AF_INET;
	a.sin_port = htons(port);
	a.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t sockLength;

	if (bind (serverSocket, (const struct sockaddr *)(&a), sizeof (a)) < 0) {
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
				studentMap[hashNumber] = studentName;
			}
			else 
			{
				perror("Wrong format for student");
			}
		}
		
	}

	cout << "All input over" << endl;	
	
	// Accept Commands:

	char buf[256];	
	
	memset(&client, 0, sizeof(struct sockaddr_in));
	while(1)
	{
		if( recvfrom(serverSocket, buf, 256, 0, (struct sockaddr*)(&client), &sockLength ) > 0 )
		{
 			cout << "Request: " << buf << endl;
			string req = string(buf);
			if( req.find("GET")!= string::npos )
			{
				unsigned pos = req.find(" ");
				string sub = req.substr(pos + 1);
				cout << sub << endl;
				pos = sub.find(" ");
				string id = sub.substr(0, pos);
				int groupId = atoi( id.c_str() );
				cout << "groupId" << groupId << endl;
				id = sub.substr( pos+1 );
				int studentId = atoi( id.c_str() );
				cout << "studentId" << studentId << endl;				

				// hash and find
				int hashed = hashf( groupId, studentId );
				map<int, string>::iterator it = studentMap.find(hashed);
				if( it != studentMap.end() )
				{
					string studentName = it->second;
					cout << "Response: " << studentName << endl;
					const char* response =  studentName.c_str();
					int len;
					if((len = sendto(serverSocket, response, strlen(response)+1, 0, (const struct sockaddr*) &client, sizeof(client))) < strlen(response) + 1)
					{
						cout << "Send failed. Sent only " << len << " of " << strlen(response) << endl;
					}
				}
				else 
				{
					perror("Check group number and Student number");
				}
			}
			else if( req.compare("STOP_SESSION") ==0 )
			{
				_exit(0);
			}
			else if( req.compare("STOP") == 0 )
			{
				int parent = getppid();
				kill(parent, SIGKILL);
			}
			
		}
		else
		{
			perror("error in recieving");
		} 
		
	}	
}
