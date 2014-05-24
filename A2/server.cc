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

int hashf( int g, int s)
{
	return (int)(0.5 * (g + s) * (g + s + 1) + s);
}

int sockDscrptr;

void term( int signum )
{
	
	shutdown( sockDscrptr, SHUT_RDWR );
	close(sockDscrptr);
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


	#if defined ( UDP )
	// create a socket for UDP
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
	
	// define params for bind and bind the socket and the port
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
	
	cout << hostname <<  " " <<  ntohs(a.sin_port) << endl;
	
	#if defined (TCP )
	listen(sockDscrptr, 1024);
	#endif
	
	
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
				int hashNumber = hashf( groupId, studentId);
				studentMap[hashNumber] = studentName;
			}
			else 
			{
				perror("Wrong format for student");
			}
		}
		
	}

	
	// Accept Commands:

	char buf[256];	
	int pid = 0;
	memset(&client, 0, sizeof(struct sockaddr_in));
	int childSockDscrptr = sockDscrptr;		// for UDP, every child process uses the same socket
	while(1)
	{
		#if defined( TCP )
		childSockDscrptr = accept( sockDscrptr, (struct sockaddr*)(&client), &sockLength );
		pid = fork();
		while(pid == 0)
		#endif
		{
			memset( &buf, 0, sizeof(buf) );
			if( int errno = recvfrom(childSockDscrptr, buf, 256, 0, (struct sockaddr*)(&client), &sockLength ) > 0 )
			{
				#if defined( UDP )
				pid = fork();
				if(pid ==0 )
				#endif
				{
					string req = string(buf);
					if( req.find("GET")!= string::npos )
					{
						unsigned pos = req.find(" ");
						string sub = req.substr(pos + 1);
						pos = sub.find(" ");
						string id = sub.substr(0, pos);
						int groupId = atoi( id.c_str() );
						id = sub.substr( pos+1 );
						int studentId = atoi( id.c_str() );

						// hash and find
						int hashed = hashf( groupId, studentId );
						map<int, string>::iterator it = studentMap.find(hashed);
						if( it != studentMap.end() )
						{
							string studentName = it->second;
							const char* response =  studentName.c_str();
							int len;
							if((len = sendto(childSockDscrptr, response, strlen(response), 0, (const struct sockaddr*) &client, sizeof(client))) < strlen(response) )	
							{
								cout << "Send failed. Sent only " << len << " of " << strlen(response) << endl;
							}
						}
						else 
						{
							stringstream ss;
							ss << "ERROR_" << groupId << "_" << studentId;
							const char* response = ss.str().c_str();   
							int len;
							if((len = sendto(childSockDscrptr, response, strlen(response) , 0, (const struct sockaddr*) &client, sizeof(client))) < strlen(response) )
							{
								cout << " Send failed." << endl;
							}
						}
						
						#if defined( UDP )
						_exit(0);
						#endif
					}
					else if( req.compare("STOP_SESSION") ==0 )
					{
						#if defined( TCP )
						close(childSockDscrptr);
						#endif	
						_exit(0);
					}
					else if( req.compare("STOP") == 0 )
					{
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
