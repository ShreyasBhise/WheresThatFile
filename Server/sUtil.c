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

void displayMessage(char* function, char* pName) {
	char message[256];
	sprintf(message, "Client has requested to %s project %s.\n", function, pName);
	write(1, message, strlen(message));
}
int isChanged(char* fileName, node* root){
	node* ptr;
	for(ptr = root; ptr!=NULL; ptr=ptr->next){
		if(strcmp(fileName, ptr->filePath)==0){
			return 1;
		}
	}
	return 0;
}
char* removeProjName(char* filePath){
	char c;
	int i = 0;
	while(i<strlen(filePath)){
		c = filePath[i];
		if(c=='/'){
			char* newName = (char*)malloc(256);
			strcpy(newName, filePath+i);
			return newName;
		}
		i++;
	}
	return NULL;
}
void writeCommit(int fd, node* root){
	node* ptr;
	for(ptr = root; ptr!=NULL; ptr = ptr->next){
		if(ptr->status=='D') continue;
		char toWrite[256];
		sprintf(toWrite, "M\t%d\t%s\t%s\n", ptr->version, ptr->filePath, ptr->hash);
		int n = write(fd, toWrite, strlen(toWrite)); 
	}
	return;
}
void commitHistory(int fd, node* root) {
	node* ptr;
	for(ptr = root; ptr!=NULL; ptr = ptr->next){
		char toWrite[256];
		sprintf(toWrite, "%c\t%d\t%s\t%s\n", ptr->status, ptr->version, ptr->filePath, ptr->hash);
		int n = write(fd, toWrite, strlen(toWrite)); 
	}
	return;
}

projNode* searchPNode(char* pName) { //Return pointer to the projectNode matching the project name that is given.
	projNode* ptr;
	for(ptr = projRoot; ptr != NULL; ptr = ptr->next) {
		if(strcmp(pName, ptr->projName) == 0) {
			return ptr;
		}
	}
	return NULL;
}
void freeCommits(commitNode* root){
	commitNode* curr = root;
	commitNode* next;
	while(curr != NULL) {
		next = curr->next;
		free(curr);
		curr = next;
	}
	root = NULL;
	return;
}
char* getSize(int fd) {
	int fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	char* sizeStr = malloc(10 * sizeof(char)) ;
	sprintf(sizeStr, "%d", fileSize);
	return sizeStr;
}
void sendFile(int sockfd, int fd) { // sends the size of the file, a space, then the contents of the file
	int n;
	char* sizeStr = getSize(fd);
	int size = atoi(sizeStr);
	write(sockfd, strcat(sizeStr, "\t"), strlen(sizeStr) + 1);
//	printf("Sent back to client [%s ]\n", sizeStr);
	char* buffer = malloc(size * sizeof(char) + 1);
	n = read(fd, buffer, size);
	if (n <= 0) { 
		write(sockfd, "0", 1);
		return;
 	}

	n = write(sockfd, buffer, size);
//	printf("Sent back to client %d bytes [%s ]\n",size, buffer);
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
	if(n < 0) { 
		return NULL; 
	}
	buffer[n] = '\0';
	
	return buffer;
}
char* extractFileNameFromPath(char* path) {
	char* fileName = strrchr(path, '/');
	return (fileName + 1);
}

int getVersion(char* projName){
	char manifestPath[256];
	sprintf(manifestPath, "%s/.Manifest", projName);	
	int manfd = open(manifestPath, O_RDONLY);
	int x = readNum(manfd);
	close(manfd);
	return x;
}
void freeNodeList(node* root) {
	node* curr = root;
	node* next;
	while(curr != NULL) {
		next = curr->next;
		free(curr);
		curr = next;
	}
	root = NULL;	
}
int fileExists(char* filePath) {
	return access(filePath, F_OK);	
	//0 if file exists, -1 if not.
}
void cleanDirectory(char* projName) {
	char commit[256];
	sprintf(commit, "%s/.Commit", projName);
	char conflict[256];
	sprintf(conflict, "%s/.Conflict", projName);
	char update[256];
	sprintf(update, "%s/.Update", projName);
	if(0 == fileExists(commit)) 
		remove(commit);
	if( 0 == fileExists(conflict)) {
		remove(conflict);
	}

	if (0 == fileExists(update)) {
		remove(update);
	}
}
void saveToBackups(char* projDir, char* backupDir) {
	int fd = open("server.c", O_RDONLY);
	printf("fd at start of saveToBackups: %d\n", fd);
	close(fd);

	//Tar backupDir, move tar to projDir/.Backups, delete backupDir
	char sysCall[256];
	char backuptar[50];
	sprintf(backuptar, "%s.tar.gz", backupDir);
	sprintf(sysCall, "tar -czf %s ./%s", backuptar, backupDir);
	printf("Executing: %s\n", sysCall);
	system(sysCall);
	
	char cmd[128];	
	sprintf(cmd, "mv %s/.Backups %s", backupDir, projDir);
	printf("Executing: %s\n", cmd);
	system(cmd);
	
	char cmd2[128];
	sprintf(cmd2, "mv %s %s/.Backups/", backuptar, projDir);
	printf("Executing:  %s\n", cmd2);
	system(cmd2);
	
	char removeDir[256];
	sprintf(removeDir, "ls -lRa %s; rm -vrf %s", backupDir, backupDir);
	printf("Executing: %s\n", removeDir);
	system(removeDir);
}
