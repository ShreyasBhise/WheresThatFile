#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<openssl/md5.h>
#include"readManifest.h"
#include"client.h"
/*char* ipaddress;
int port; */


int projectExists(char* projectName){
	struct stat st;
	if(stat(projectName, &st) != -1){
		return 1;
	}
	return 0;
}
int fileExists(char* filePath) {
	return access(filePath, F_OK);	
	//0 if file exists, -1 if not.
}
char* getSize(int fd) {
	int fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	char* sizeStr = (char *) malloc(10);
	sprintf(sizeStr, "%d", fileSize);
	return sizeStr;
}
void sendFile(int sockfd, int fd) {
	int n;
	char* sizeStr = getSize(fd);
	int size = atoi(sizeStr);
	write(sockfd, strcat(sizeStr, "\t"), strlen(sizeStr) + 1);
	
	char* buffer = (char*) malloc(size);
	n = read(fd, buffer, size);
	if (n <= 0) printf("Error: could not read file.");
	
	n = write(sockfd, buffer, size);
	printf("Sent .Commit to server\n");
	return;
}
void readBytes(int sfd, int x, void* buffer){
	int bytesRead = 0;
	int n;
	while(bytesRead<x){
		n = read(sfd, buffer+bytesRead, x-bytesRead);
		if(n<0){
			printf("Error: unable to read file completely");
			exit(1);
		}
		bytesRead+=n;
	}
	return;
}
char* readFile(int fd) {
	int fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	char* fileContents = malloc(fileSize+1);
	int n = read(fd, fileContents, fileSize);
//This is a comment

	if(n != fileSize) printf("Error reading the file.\n");
	fileContents[n] = '\0';
	return fileContents;

}
int readFileFromServer(int sfd, char** buffer){
	int n = 0;
	char c;
	char* sizeStr = (char *)malloc(256);
	int curr = 0;
	while((n=read(sfd, &c, 1))>=0){
		if(n==0) continue;
		if(c==' ' || c == '\t') break;
		sizeStr[curr++]=c;
	}
	sizeStr[curr]='\0';
	int size = atoi(sizeStr);
	free(sizeStr);
	*buffer = (char*)malloc(size);
	readBytes(sfd, size, (void*)*buffer);
	printf("%d bytes\n", size);
	return size;
}
char* getHash(char* toHash) {
	unsigned char fileHash[MD5_DIGEST_LENGTH];
	MD5_CTX c;
        MD5_Init(&c);
        MD5_Update(&c, toHash, strlen(toHash));
        MD5_Final(fileHash, &c);
	
	int i;
	char* hash = (char*)malloc(33);	
	for( i = 0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(&hash[i * 2], "%02x", fileHash[i]);	
	return hash;
}
int isFileAdded(node* root, char* filePath) {
	printf("File path in isFileAdded: %s\n", filePath);
	node* ptr;
	int count = 0;
	for(ptr = root; ptr != NULL; ptr = ptr->next) {
		printf("isFileAdded: %d %s\n", count++, ptr->filePath);
		if(strcmp(ptr->filePath, filePath) == 0) 
			return 1;	
	}
	return 0;
}
node* getMatch(char* fileName, node* root){
	node* ptr;
	for(ptr = root; ptr!=NULL; ptr=ptr->next){
		if(strcmp(ptr->filePath, fileName)==0){
			return ptr;
		}
	}
	return NULL;
}
void getPath(char* pName, char* fName, char* buffer) {
	sprintf(buffer, "%s/%s", pName, fName);
	return;
}
int writeCommitFileClient(int cfd, node* ptr, node* serverroot) {
	char status = ptr->status;
	char commitbuffer[256+MD5_DIGEST_LENGTH];
	node* match;
	match = getMatch(ptr->filePath, serverroot);
	switch(status) {
		case 'M' : ;// need to recompute hash
			if(match==NULL){
				printf("Unable to find file in server .Manifest\n");
				return 1;
			}
			// creating ptr->hash 
			int fileToHash = open(ptr->filePath, O_RDONLY);
			char* filecontents = readFile(fileToHash);
			char* liveHash = getHash(filecontents); //This is the new hash of the file */
			if(strcmp(liveHash, match->hash)==0) break;
			if(match->version>ptr->version){
				printf("File version doesn't match for %s.\n", ptr->filePath);
				return 1;
			}
			printf("M %s\n", ptr->filePath);
			sprintf(commitbuffer, "M\t%d\t%s\t%s\n", ptr->version+1, ptr->filePath, ptr->hash);
			write(cfd, commitbuffer, strlen(commitbuffer));
			break;
		case 'A' :
			if(match!=NULL){
				printf("Added file that already exists on server\n");
				return 1;
			}
			printf("A %s\n", ptr->filePath);
			sprintf(commitbuffer, "A\t%d\t%s\t%s\n", ptr->version+1, ptr->filePath, ptr->hash);
			write(cfd, commitbuffer, strlen(commitbuffer));
			break;
		case 'D' :
			if(match==NULL){
				printf("Asking to delete file that does not exist\n");
				return 1;
			}
			if(match->version>ptr->version){
				printf("File version doesn't match for %s.\n", ptr->filePath);
				return 1;
			}
			printf("D %s\n", ptr->filePath);
			sprintf(commitbuffer, "D\t%d\t%s\t%s\n", ptr->version+1, ptr->filePath, match->hash);
			write(cfd, commitbuffer, strlen(commitbuffer));
			break;
	}
	return 0; 
}
