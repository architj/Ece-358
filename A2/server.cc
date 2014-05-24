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
#include <sstream>
#include <net/if.h>

using namespace std;

// takes in group ID and student ID and returns unique key
int hashf( int g, int s)
{
	return (int)(0.5 * (g + s) * (g + s + 1) + s);
}

int sockDscrptr;

// Closes the socket and exits the server
void term( int signum )
{
	close(sockDscrptr);
	exit(0);
}

int main (int argc, char* argv[] )
{
	// binding the handler for SIGTERM
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction) );
	action.sa_handler = term;
	sigaction(SIGTERM, &action, NULL);
	
	// grab port number
	int port;
	if(argc < 2)
	{
		port = 0;
	}
	else
	{ 
		port = atoi((const char*)argv[1]);	
	}

	// create a socket
	#if defined ( UDP )
	if ((sockDscrptr = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("udp socket error");
		exit(0);
	}

	#else
	if ((sockDscrptr = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("tcp socket error");
		exit(0);
	}
	#endif
	
	// define params for binding
	struct sockaddr_in a, client;
	a.sin_family = AF_INET;
	a.sin_addr.s_addr = htonl(INADDR_ANY);
	if( port !=0 )
	{
		a.sin_port = htons(port);
	}
	else 
	{
		a.sin_port = htons(0);
	}

	socklen_t sockLength;
	
	// bind the socket 
	if ( bind (sockDscrptr, (const struct sockaddr *)(&a), sizeof (a)) < 0) {
		perror("bind");
		exit(0);
	}
	
	
	// Get IP and port number
	if( getsockname(sockDscrptr, (struct sockaddr *)(&a), &sockLength) < 0) 
	{
		perror("getsockname");
		exit(0);
	}

	//get host address
	struct ifaddrs *ifap;
	
	if (getifaddrs(&ifap) != 0)
	{
		perror("No interfaces found");
	} 
	
	struct ifaddrs *temp;
	char hostname[256];
	
	//find the correct addrinfo that is in the INET address family
	for (temp = ifap; temp != NULL; temp = temp->ifa_next) {
		string interfaceName = string(temp->ifa_name);
		if ( interfaceName.compare("eth0") == 0 && ( temp->ifa_flags & IFF_BROADCAST) && (( (struct sockaddr_in*)temp->ifa_addr)->sin_family == AF_INET  ))
		 {
			char ip[256];
			inet_ntop(AF_INET, &(((struct sockaddr_in *)(temp->ifa_addr))->sin_addr), ip, 256 );
			struct sockaddr_in ifaSockAddr = *((struct sockaddr_in*) (temp->ifa_addr));
			ifaSockAddr.sin_family = AF_INET;
			int errno = getnameinfo(((struct sockaddr*) &ifaSockAddr), sizeof(ifaSockAddr), hostname, sizeof(hostname), NULL, 0, 0);
			if( errno != 0)
			{
				// failure
				printf(gai_strerror(errno));
				memcpy(hostname, ip, sizeof(ip) );
			}
			break;
		}
	}
	
	// display server and port
	cout << hostname <<  " " <<  ntohs(a.sin_port) << endl;
	
	// listen only for TCP
	#if defined (TCP )
	listen(sockDscrptr, 1024);
	#endif
	
	// Initialize group and student info
	int groupId = 0, studentId =0;
	string studentName;
	map< int, string > studentMap;
	string input;
	
	// Accept input until EOF is read
	while( getline(cin, input) && !cin.eof() )
	{
		size_t found = input.find("Group ");				// Find "Group" and pasre group number
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
				// Separate string before and after " "
				unsigned pos = input.find(" ");
				string id = input.substr(0, pos);
				studentId = atoi( id.c_str() );			//convert studentId to int
				studentName = input.substr( pos+1 );
				
				// Calculate unique key from the combination of grouId and studentId and store a pair of key, studentName
				int hashNumber = hashf( groupId, studentId);	
				studentMap[hashNumber] = studentName;
			}
			else 
			{
				perror("Wrong format for student");
			}
		}
		
	}
	
	char buf[256];	
	int pid = 0;
	memset(&client, 0, sizeof(struct sockaddr_in));
	int childSockDscrptr = sockDscrptr;		// for UDP, every child process uses the same socket
	
	// Stays on till STOP request is made by the client
	while(1)
	{
		// accept and set childSockDscrptr if target is TCP
		#if defined( TCP )
		childSockDscrptr = accept( sockDscrptr, (struct sockaddr*)(&client), &sockLength );
		
		// fork and start the child process for TCP
		pid = fork();
		while(pid == 0)
		#endif
		{
			// clear the buffer
			memset( &buf, 0, sizeof(buf) );
			
			// recieve messages
			if( int errno = recvfrom(childSockDscrptr, buf, 256, 0, (struct sockaddr*)(&client), &sockLength ) > 0 )
			{
				#if defined( UDP )
				
				// fork and start the child process for UDP
				pid = fork();
				if(pid ==0 )
				#endif
				{
					
					string req = string(buf);
					
					// Check for Get and parse for groupId and studentId
					if( req.find("GET")!= string::npos )			
					{
						unsigned pos = req.find(" ");
						string sub = req.substr(pos + 1);
						pos = sub.find(" ");
						string id = sub.substr(0, pos);
						int groupId = atoi( id.c_str() );
						id = sub.substr( pos+1 );
						int studentId = atoi( id.c_str() );

						// hash the combination of groupId and studentId and find studentName from map
						int hashed = hashf( groupId, studentId );
						map<int, string>::iterator it = studentMap.find(hashed);
						
						// Match found
						if( it != studentMap.end() )
						{
							string studentName = it->second;
							const char* response =  studentName.c_str();
							
							// send the student name as the response
							int len;
							if((len = sendto(childSockDscrptr, response, strlen(response), 0, (const struct sockaddr*) &client, sizeof(client))) < strlen(response) )	
							{
								cout << "Send failed. Sent only " << len << " of " << strlen(response) << endl;
							}
						}
						// Student Name not found
						else 
						{
							stringstream ss;
							ss << "ERROR_" << groupId << "_" << studentId;
							const char* response = ss.str().c_str();
							
							// send error code as the response
							int len;
							if((len = sendto(childSockDscrptr, response, strlen(response) , 0, (const struct sockaddr*) &client, sizeof(client))) < strlen(response) )
							{
								cout << " Send failed." << endl;
							}
						}
						
						// exit child process only for UDP because TCP keeps the connection alive
						#if defined( UDP )
						_exit(0);
						#endif
					}
					else if( req.compare("STOP_SESSION") ==0 )
					{
						// exit the child process
						#if defined( TCP )
						close(childSockDscrptr);
						#endif	
						_exit(0);
					}
					else if( req.compare("STOP") == 0 )
					{
						// kill the parent and exit the child process
						#if defined( TCP )
						close(childSockDscrptr);
						#endif
						int parent = getppid();
						kill(parent, SIGTERM);
						_exit(0);
					}
	
				}
			}
		} 
		
	}	
}
