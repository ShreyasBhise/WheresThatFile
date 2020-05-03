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

int destroyProject(int sockfd) { /*Send 1 if the project exists, and will be deleted. 0 if the project doesn't exist */
	char* buffer = getProjName(sockfd);
	if(!projectExists(buffer)) {
		printf("Project [%s] does not exist.\n", buffer);
		write(sockfd, "0", 1);
		return 0;	
	}
	
	char* sysCall = (char*) malloc(256);
	strcat(sysCall, "rm -r ./");
	strcat(sysCall, buffer);
	system(sysCall);
	
	write(sockfd, "1", 1);
	return 1;
}
int createProject(int sockfd) { /* Send 1 if the project does not exist, and a manifest is to be sent. 0 if project already exists. */
	char* buffer = getProjName(sockfd);

	if(!projectExists(buffer)) {
		write(sockfd, "1", 1);
		mkdir(buffer, 0700);
		
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
int currentVersion(int sockfd) {
	char* buffer = getProjName(sockfd);
	
	if(projectExists(buffer)) {
		char manifestPath[256];
		sprintf(manifestPath, "%s/.Manifest", buffer);	
		int manfd = open(manifestPath, O_RDONLY);
		int x = readNum(manfd);
		node* root = readManifest(manfd);
		char numElements[10];
		sprintf(numElements, "%d\n", manifestSize);
		printf("num elements on server: %s\n", numElements);
		write(sockfd, numElements, strlen(numElements));
		node* ptr;
		for(ptr = root; ptr!= NULL; ptr = ptr->next) {
			char toWrite[256];
			char* fileName = extractFileNameFromPath(ptr->filePath);
			char version[6];
			sprintf(version, "%d", ptr->version);
			sprintf(toWrite, "%s\t%s\n", version, fileName);
			printf("wrote to client [%s]", toWrite);
			write(sockfd, toWrite, strlen(toWrite));
			bzero(toWrite, 256);
			bzero(version, 6);
		}
		return 0;
	}
	printf("Project does not exist on server.\n");	
	write(sockfd, "0", 1);
	return 1;

} 
int checkout(int sockfd) {
	char* pName = getProjName(sockfd);
	
	if(projectExists(pName)) { //If the project exists, compress it and send it to client.
		write(sockfd, "1", 1);
		char sysCall[256];
		char tarName[25];
		sprintf(tarName, "%s.tar.gz", pName);
		sprintf(sysCall, "tar -czf %s ./%s", tarName, pName);
		printf("tar file name: %s\n", tarName);
		system(sysCall);
		
		int toSend = open(tarName, O_RDONLY);
		sendFile(sockfd, toSend);
		close(toSend);
		bzero(sysCall, 256);
		sprintf(sysCall, "rm %s", tarName);
		system(sysCall);
		return 1;
		 	
	}	
	write(sockfd, "0", 1);
}
int commit(int sockfd){
	char* buffer = getProjName(sockfd);
	printf("%s\n", buffer);
	if(!projectExists(buffer)){
		printf("Project does not exist on server.\n");
		write(sockfd, "0", 1);
		return 1;
	}
	projNode* projCommitted = searchPNode(buffer);
	if(projCommitted == NULL) {
		projCommitted = (projNode*) malloc(sizeof(projNode));
		projCommitted->projName = buffer;
		projCommitted->commitListRoot = NULL;
	}
	printf("test\n");
	write(sockfd, "1", 1);
	char manifestPath[256];
	sprintf(manifestPath, "%s/.Manifest", buffer);	
	int manfd = open(manifestPath, O_RDONLY);
	sendFile(sockfd, manfd);
	write(sockfd, "\n\n", 2);
	int size = readNum(sockfd);
	char* commitFile = (char*)malloc(size+1);
	int n = read(sockfd, commitFile, size);
	commitFile[size]='\0';
	commitNode* newnode = (commitNode*) malloc(sizeof(commitNode));
	newnode->next = projCommitted->commitListRoot;
	projCommitted->commitListRoot = newnode;
	newnode->file = commitFile;
	newnode->size = size;
	write(1, projCommitted->commitListRoot->file, size);
	projCommitted->next = projRoot;
	projRoot = projCommitted;
	return 0;
}
int push(int sockfd) {
	char* buffer = getProjName(sockfd);
	printf("%s\n", buffer);
	if(!projectExists(buffer)){
		printf("Project does not exist on server.\n");
		write(sockfd, "0", 1);
		return 1;
	}
	write(sockfd, "1", 1);
	projNode* projCommitted = searchPNode(buffer);
	if(projCommitted==NULL){
		printf("2.Commit file does not match.\n");
		write(sockfd, "0", 1);
		return 1;
	}
	int x = readNum(sockfd);
	char* commitFile = (char*)malloc(x+1);
	int n = read(sockfd, commitFile, x);
	commitFile[x]='\0';
	commitNode* ptr = projCommitted->commitListRoot;
	while(ptr!=NULL){
		printf("%d, %d\n", ptr->size, x);
		printf("%d\n", memcmp(ptr->file, commitFile, x));
		if(ptr->size==x){
			if(memcmp(ptr->file, commitFile, x)==0){
				printf("break successful\n");
				break;
			}
		}
		ptr = ptr->next;
	}
	if(ptr==NULL){
		printf(".Commit file does not match.\n");
		write(sockfd, "0", 1);
		return 1;
	}
	int projNumber = getVersion(buffer);
	write(sockfd, "1", 1);
	int size = readNum(sockfd);
	char* tarFile = (char*)malloc(size+1);
	n = read(sockfd, tarFile, size);
	int tarfd = open("push.tar.gz", O_RDWR | O_CREAT, 00600);
	write(tarfd, tarFile, size);
	char manifestPath[256];
	sprintf(manifestPath, "%s/.Manifest", buffer);	
	int manfd = open(manifestPath, O_RDONLY);
	int manversion = readNum(manfd);
	node* manRoot = readManifest(manfd);
	char cmd[128];
	sprintf(cmd, "mv %s %s_%d", buffer, buffer, projNumber);
	system(cmd);
	char cmd2[128] = "tar -xzf push.tar.gz";
	system(cmd2);
	char cmd3[64] = "rm push.tar.gz";
	system(cmd3);
	char commitPath[256];
	sprintf(commitPath, "%s/.Commit", buffer);	
	int commitfd = open(commitPath, O_RDWR | O_CREAT, 00600);
	write(commitfd, commitFile, x);
	lseek(commitfd, 0, SEEK_SET);
	node* commitRoot = readManifest(commitfd);
	printf("Server Manifest:\n");
	printManifest(manRoot);
	printf("Commit File:\n");
	printManifest(commitRoot);
	int newManfd = open(manifestPath, O_RDWR | O_CREAT, 00600);
	char version[10];
	sprintf(version, "%d\n", manversion+1);
	write(newManfd, version, strlen(version));
	writeCommit(newManfd, commitRoot);
	node* ptr2;
	for(ptr2 = manRoot; ptr2!=NULL; ptr2 = ptr2->next){
		int y = isChanged(ptr2->filePath, commitRoot);
		if(y==0){ // file is not already in the commit
			char moveFile[256];
			char* newPath = removeProjName(ptr2->filePath);
			sprintf(moveFile, "cp %s_%d%s %s", buffer, manversion, newPath, buffer);
			printf(moveFile);
			system(moveFile);
			char toWrite[256];
			sprintf(toWrite, "M\t%d\t%s\t%s\n", ptr2->version, ptr2->filePath, ptr2->hash);
			n = write(newManfd, toWrite, strlen(toWrite)); 
		}
	}
	lseek(newManfd, 0, SEEK_SET);
	sendFile(sockfd, newManfd);
	return 0;

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
		case 0:
			break;
		case 1: //Create
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
		default:
			error("Invalid operation");
			break;
	}
//	return (void *) &op;
}
int main(int argc, char** argv) {
	projRoot = NULL;
	printf("Starting server\n");
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
