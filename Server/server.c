#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<string.h>
#include<netinet/in.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include"readManifest.h"
#include"server.h"
pthread_mutex_t lock;

void* clientConnect(void* clientSockfd) {
	pthread_mutex_lock(&lock);
	write(1, "New client connected!\n", 22);
	int sockfd = *((int *) clientSockfd);
	int n;
	int op = readNum(sockfd);
	switch(op) {
		case 0:
			break;
		case 1: 
			n = createProject(sockfd);
			break;
		case 2: 
			n = destroyProject(sockfd);
			break;
		case 5: 
			n = currentVersion(sockfd);
			break;
		case 6:
			n = checkout(sockfd);
		case 7:
			n = commit(sockfd);
			break;
		case 8:
			n = push(sockfd);
			break;
		case 9:
			n = update(sockfd);
			break;
		case 10:
			n = upgrade(sockfd);
			break;
		case 11:
			n = history(sockfd);
			break;
		case 12:
			n = rollback(sockfd); 
			break;

		default:
			write(1,"Invalid operation\n", 18);
			break;
	}
	write(1, "Client disconnected.\n", 21);
	pthread_mutex_unlock(&lock);
	return NULL;
}
int main(int argc, char** argv) {
	pthread_mutex_init(&lock, NULL);
	projRoot = NULL;
	write(1, "Starting server\n", 16);
	int sockfd = -1; //fd for socket
	int newsockfd = -1; //fd for client socket
	int portno = -1; //server port to connect to
	int clilen = -1; //utility variable - size of clientAddressInfo below
	int n = -1; //utility variable - monitoring reading/writing to/from socket
	char buffer[256]; //char array to store data moving to and from socket
	
	struct sockaddr_in sAddrInfo; //holds address info for building server socket
	struct sockaddr_in cAddrInfo; //holds address info for building client socket
	
	if(argc < 2) { 
		write(1, "no port provided.\n",strlen("no port provided.\n")); 
		return -1;
	}

	portno = atoi(argv[1]); //Convert text representation of port number
	
	/***SOCKET SET UP***/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) { 
		write(1, "could not open socket.\n", strlen("could not open socket.\n")); 
		return -1;
	}

	bzero((char *) &sAddrInfo, sizeof(sAddrInfo));
	sAddrInfo.sin_port = htons(portno);
	sAddrInfo.sin_family = AF_INET;
	sAddrInfo.sin_addr.s_addr = INADDR_ANY;

	/***SOCKET BINDING***/
	if (bind(sockfd, (struct sockaddr *) &sAddrInfo, sizeof(sAddrInfo)) < 0) { 
		write(1, "Could not bind socket to port.\n", strlen("Could not bind socket to port.\n")); 
		return -1;
	}

	listen(sockfd, 0);

	clilen = sizeof(cAddrInfo);
	while(1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cAddrInfo, &clilen);
	
		if(newsockfd < 0) { write(1, "accept failed.\n", strlen("accept failed.\n")); }
	
		pthread_t newthread;
		pthread_create(&newthread, NULL, &clientConnect, (void *)&newsockfd);
		
	}
	write(1, "Closing server\n", 15);
	return 0;
}
