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
#include"../ClientStuff/readManifest.h"

void error(char* msg) {
	printf("ERROR: %s\n", msg);
	exit(1);
}
char* getSize(int fd) {
	int fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	char* sizeStr = malloc(10 * sizeof(char)) ;
	sprintf(sizeStr, "%d", fileSize);
	return sizeStr;
}
void sendFile(int sockfd, int fd) {
	int n;
	char* sizeStr = getSize(fd);
	int size = atoi(sizeStr);
	write(sockfd, strcat(sizeStr, " "), strlen(sizeStr) + 1);
	printf("Sent back to client [%s ]\n", sizeStr);
	char* buffer = malloc(size * sizeof(char) + 1);
	n = read(fd, buffer, size);
	if (n <= 0) { error("could not read file.\n"); }

	
	n = write(sockfd, buffer, size);
	printf("Sent back to client %d bytes [%s ]\n",size, buffer);

}
int projectExists(char* projectName) {
	struct stat st;
	if (stat(projectName, &st) != -1) { 
		return 1;
	}
	return 0;	
}
char* getProjName(int sockfd) {
	char* buffer = (char *) malloc(256);
	int n = 0;
	while((n = read(sockfd, buffer, 255)) == 0)
	if(n < 0) { error("Could not read from socket."); }
	buffer[n] = '\0';
	
	return buffer;
}
int destroyProject(int sockfd) { /*Send 1 if the project exists, and will be deleted. 0 if the project doesn't exist */
	char* buffer = getProjName(sockfd);
	if(!projectExists(buffer)) {
		printf("Project [%s] does not exist.\n", buffer);
		write(sockfd, "0", 1);
		return 0;	
	}
	printf("YERRRR\n");
	char* sysCall = (char*) malloc(256);
	strcat(sysCall, "rm -r ./");
	strcat(sysCall, buffer);
	system(sysCall);
	printf("Deleted [./%s]\n", buffer); 
	write(sockfd, "1", 1);
	return 1;
}
int createProject(int sockfd) { /* Send 1 if the project does not exist, and a manifest is to be sent. 0 if project already exists. */
	char* buffer = getProjName(sockfd);

	if(!projectExists(buffer)) {
		write(sockfd, "1", 1);
		mkdir(buffer, 0700);
		printf("Created project [%s]\n", buffer);
		strcat(buffer, "/.Manifest");
		int fd = open(buffer, O_RDWR | O_CREAT, 00600);
		write(fd, "0\n", 2);
		sendFile(sockfd, fd);
		return 0;
	}
	printf("Project already exists.\n");
	write(sockfd, "0", 1);
	return 1;
}
char* extractFileNameFromPath(char* path) {
	char* fileName = strrchr(path, '/');
	return (fileName + 1);
}
int currentVersion(int sockfd) {
	char* buffer = getProjName(sockfd);
	
	
	if(projectExists(buffer)) {
		char manifestPath[256];
		sprintf(manifestPath, "%s/.Manifest", buffer);	
		int manfd = open(manifestPath, O_RDONLY);
		int x = readNum(manfd);
		node* root = readManifest(manfd);
		node* ptr;
		for(ptr = root; ptr!= NULL; ptr = ptr->next) {
			char toWrite[256];
			char* fileName = extractFileNameFromPath(ptr->filePath);
			char version[6];
			sprintf(version, "%d", ptr->version);
			sprintf(toWrite, "%s\t%s\n", version, fileName);
			write(sockfd, toWrite, strlen(toWrite));
		
		}
	}
	printf("Project does not exist on server.\n");	
	write(sockfd, "0", 1);
	return 1;

}
void* clientConnect(void* clientSockfd) {
	int sockfd = *((int *) clientSockfd);
	char* operation = malloc(6 * sizeof(char));
	char c;
//	bzero(*operation, 6);
	int n; 
	int curr = 0;
	while (n = read(sockfd, &c, 1) >= 0) {
		if(c == ' ') { break; }
		operation[curr++] = c;
	}
	operation[curr] = '\0';
	int op = atoi(operation);
	switch(op) {
		case 1: //Create
			n = createProject(sockfd);
			break;
		case 2:
			n = destroyProject(sockfd);
			break;
		case 5:
			n = currentVersion(sockfd);
		default:
			error("Invalid operation");
			break;
	}
//	return (void *) &op;
}
int main(int argc, char** argv) {
	printf("Starting server\n");
	printf("%s\n", extractFileNameFromPath("../ClientStuff/p36/file1.txt"));
	int sockfd = -1; //fd for socket
	int newsockfd = -1; //fd for client socket
	int portno = -1; //server port to connect to
	int clilen = -1; //utility variable - size of clientAddressInfo below
	int n = -1; //utility variable - monitoring reading/writing to/from socket
	char buffer[256]; //char array to store data moving to and from socket
	
	struct sockaddr_in sAddrInfo; //holds address info for building server socket
	struct sockaddr_in cAddrInfo; //holds address info for building client socket
	
	if(argc < 2) { error("no port provided."); }

	portno = atoi(argv[1]); //Convert text representation of port number
	
	/***SOCKET SET UP***/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) { error("could not open socket."); }

	bzero((char *) &sAddrInfo, sizeof(sAddrInfo));
	sAddrInfo.sin_port = htons(portno);
	sAddrInfo.sin_family = AF_INET;
	sAddrInfo.sin_addr.s_addr = INADDR_ANY;

	/***SOCKET BINDING***/
	if (bind(sockfd, (struct sockaddr *) &sAddrInfo, sizeof(sAddrInfo)) < 0) { error("Could not bind socket to port."); }

	listen(sockfd, 0);

	clilen = sizeof(cAddrInfo);
	while(1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cAddrInfo, &clilen);
	
		if(newsockfd < 0) { error("accept failed."); }
	
		pthread_t newthread;
		pthread_create(&newthread, NULL, &clientConnect, (void *)&newsockfd);
		
	}
	return 0;
}
