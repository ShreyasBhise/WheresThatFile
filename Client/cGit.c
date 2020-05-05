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

int addFile(char* projName, char* fileName) {
	char filePath[256];
	char manifestPath[256];
	getPath(projName, ".Manifest", manifestPath);
	getPath(projName, fileName, filePath);
    if(projectExists(projName) == 0) {
		printf("Error: Project does not exist locally.\n");
		return -1;
	}
	if(fileExists(filePath) == -1) {
		printf("Error: File does not exist locally.\n");
		return -1;
	}
	  
    int manifest = open(manifestPath, O_RDWR | O_APPEND); //{
	int x = readNum(manifest);
	printf("In addFile: %d\t%c\n", x, x);
	node* mList = readManifest(manifest);
//	printf("created list\n");
	
	
	if(isFileAdded(mList, filePath)) {
		printf("Error: File is already in manifest\n");
		return -1;
	}
	int fileToAdd = open(filePath, O_RDONLY);
	char* fileContents = readFile(fileToAdd);
	close(fileToAdd);
	char* hash = getHash(fileContents);
	char toWrite[256];
	sprintf(toWrite, "A\t0\t%s\t%s\n", filePath, hash);
	
	int n = write(manifest, toWrite, strlen(toWrite)); 
	
	close(manifest); //}
	return 1;

}
int removeFile(char* projName, char* fileName) {
	char filePath[256]; 
	char manifestPath[256];
        getPath(projName, ".Manifest", manifestPath);
        getPath(projName, fileName, filePath);	
	
	int manifest = open(manifestPath, O_RDWR); //{
	int x = readNum(manifest);
	node* mList = readManifest(manifest);
	close(manifest); //}
	if(!isFileAdded(mList, filePath)) {
		printf("File is not in manifest, cannot be removed.\n");
		return 1;
	}
	char* manifestContents = readFile(manifest);
	char* fileLoc = strstr(manifestContents, filePath);
	*(fileLoc - 4) = 'D';
	lseek(manifest, 0, SEEK_SET);
	int n = write(manifest, manifestContents, strlen(manifestContents));
	return 0;
}
int destroyProject(int sfd, char* projName){
	int n = 0;
	char* command = "2 ";
	n = write(sfd, command, strlen(command));

	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);

	n = write(sfd, projSend, strlen(projSend));
	char c;
	while((n=read(sfd, &c, 1))==0)
	
	if(c == '1') printf("Project has been removed from the repository\n");
	else{
		char errorMsg[128] = "Error: Project dpesm't exist on Server\n";
		write(1, errorMsg, strlen(errorMsg));
	}
	return 0;
}

int createProject(int sfd, char* projName){
	int n = 0;
	char* command = "1 ";
	n = write(sfd, command, strlen(command));
	
	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);
	n = write(sfd, projSend, strlen(projSend));
	char c;
	while((n=read(sfd, &c, 1))==0){
	}
	if(c=='0'){ // project already exists
		char errorMsg[128] = "Error: Project already exists on Server\n";
		write(1, errorMsg, strlen(errorMsg));
		return 1;
	} else if (c!='1'){
		printf("Error\n");
		return 1;
	}
	char** buffer = (char**)malloc(sizeof(char*));
	int size = readFileFromServer(sfd, buffer);
	char* filebuffer = (char*)malloc(256);
	strcpy(filebuffer, projName);
	mkdir(filebuffer, 0700);
	strcat(filebuffer, "/.Manifest");
	int fd = open(filebuffer, O_RDWR | O_CREAT, 00600);
	write(fd, *buffer, size);
	return 0;
}
int currentVersion(int sfd, char* projName) {
	int n = 0;
	char* command = "5 ";
	n = write(sfd, command, 2);

	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);
	n = write(sfd, projSend, strlen(projSend));
	char c;
	n = read(sfd, &c, 1);
	if(c!='1'){
		printf("Error: Project doesn't exist on server\n");
		return 1;
	}
	int num = readNum(sfd);
	printf("Server Version: %d\n", num);
	int i;
	for(i = 0; i < num; i++) {
		int version = readNum(sfd);
		char* name = readStr(sfd);
		printf("%d\t%s\n", version, name);
	}
	return 0;
}
int checkout(int sfd, char* projName){
	if(projectExists(projName)){
		printf("Error: Project already exists on client\n");
		write(sfd, "0 ", 2);
		return 1;
	}
	int n = 0;
	char* command = "6 ";
	n = write(sfd, command, 2);
	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);
	n = write(sfd, projSend, strlen(projSend));

	char c;
	n = read(sfd, &c, 1);
	if(c!='1'){
		printf("Error: unable to get project from server\n");
		return 1;
	}
	int size = readNum(sfd);
	char newName[256];
	sprintf(newName, "%s.tar.gz", projName);
	printf("%s\n", newName);
	int fd = open(newName, O_RDWR | O_CREAT, 00600); //{
	char* file = (char*)malloc(size+1);
	readBytes(sfd, size, file);
	n = write(fd, file, size);
	close(fd); //}
	char sysCall[256];
	sprintf(sysCall, "tar -xzf %s", newName);
	system(sysCall);

	char rmCall[256];
	sprintf(rmCall, "rm %s", newName);
	system(rmCall);
	return 0;
}

int commit(int sfd, char* projName){
	char conflict[256];
	char update[256];
	sprintf(conflict, "%s/.Conflict", projName);
	sprintf(update, "%s/.Upgrade", projName);
	if(fileExists(conflict) == 0) { 
		printf("Error: Conflicts exist\n");
		write(sfd, "0 ", 2);
		return 0;
	}
	if(fileExists(update) == 0) {
		struct stat st;
		stat(update, &st);
		if(st.st_size != 0) {
			printf("Error: Client is behind the server, upgrade before you commit");
			write(sfd, "0 ", 2);
			return 0;
		}
	}
	char fileName[256];
	sprintf(fileName, "%s/.Manifest", projName);
	int fd = open(fileName, O_RDONLY); //{
	if(fd==-1){
		printf("Error: unable to open client .Manifest");
		write(sfd, "0 ", 2);
		return 1;
	}
	int projVersion = readNum(fd);
	node* clientroot = readManifest(fd);
	close(fd); //}
	write(sfd, "7 ", 2);
	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);
	write(sfd, projSend, strlen(projSend));
	char c;
	read(sfd, &c, 1);
	if(c!='1'){
		printf("Error: project does not exist on server\n");
		return 1;
	}
	int size = readNum(sfd);
	printf("size: %d\n", size);
	int serverProjVersion = readNum(sfd);
	printf("Server Project Version: %d\n", serverProjVersion);
	node* serverroot = readManifest(sfd);
	printf("test3\n");
	printf("Client Manifest:\n");
	printManifest(clientroot);
	printf("Server Manifest:\n");
	printManifest(serverroot);
	if(serverProjVersion!=projVersion){
		printf("Server and Client project versions do not match, please update local project.\n");
	}
	char commitFile[256];
	sprintf(commitFile, "%s/.Commit", projName);
	int cfd = open(commitFile, O_RDWR | O_CREAT, 00600); //{
	node* ptr;
	printf("Changes:\n");
	
	for(ptr = clientroot; ptr != NULL; ptr = ptr->next) {
		writeCommitFileClient(cfd, ptr, serverroot);
	}
	lseek(cfd, 0, SEEK_SET);
//	system("cat p3/.Commit");
	sendFile(sfd, cfd);
	close(cfd); //}
	return 0;
}
int pushCommit(int sfd, char* projName){
	char commitName[256];
	sprintf(commitName, "%s/.Commit", projName);
	int fd = open(commitName, O_RDONLY); //{
	if(fd==-1){
		printf("Error: unable to open client .Commit\n");
		write(sfd, "0 ", 2);
		return 1;
	}
	write(sfd, "8 ", 2);
	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);
	write(sfd, projSend, strlen(projSend));
	char c;
	read(sfd, &c, 1);
	if(c!='1'){
		printf("Error: project does not exist on server\n");
		return 1;
	}
	sendFile(sfd, fd);
	read(sfd, &c, 1);
	if(c!='1'){
		printf("Error: Push failed. Please commit again.\n");
		return 1;
	}
	lseek(fd, 0, SEEK_SET);
	node* commitRoot = readManifest(fd); // commit is now contained in linked list
	close(fd); //}
	printManifest(commitRoot);
	node* ptr;
	int elements = 0;
	for(ptr = commitRoot; ptr!=NULL; ptr=ptr->next){
		if(ptr->status=='M' || ptr->status=='A'){
			elements++;
		}
	}
	//need to handle 0 elements case separately.
	char* fileNames = (char*)malloc(128*elements);
	strcat(fileNames, "tar -czf push.tar.gz ");
	for(ptr = commitRoot; ptr!=NULL; ptr=ptr->next){
		if(ptr->status=='M' || ptr->status=='A'){
			strcat(fileNames, ptr->filePath);
			strcat(fileNames, " ");
		}
	}
	printf("%s\n", fileNames);
	system(fileNames); // creates tar file push.tar.gz
	int tarfd = open("push.tar.gz", O_RDONLY); //{
	sendFile(sfd, tarfd);
	close(tarfd); //}
	char cmd[64] = "rm push.tar.gz";
	system(cmd);
	char cmd2[128];
	sprintf(cmd2, "rm %s/.Manifest", projName);
	system(cmd2);
	int size = readNum(sfd);
	char* newMan = (char*)malloc(size);
	char* manPath = (char*)malloc(128);
	sprintf(manPath, "%s/.Manifest", projName);
	int manfd = open(manPath, O_RDWR | O_CREAT, 00600);
	readBytes(sfd, size, newMan);
	write(manfd, newMan, size);
	return 0;
}
int update(int sfd, char* projName) {
	char fileName[256];
	sprintf(fileName, "%s/.Manifest", projName);
	int fd = open(fileName, O_RDONLY); //{
	if(fd==-1){
		printf("Error: unable to open client .Manifest");
		write(sfd, "0 ", 2);
		return 1;
	}
	int projVersion = readNum(fd);
	node* clientroot = readManifest(fd);
	close(fd); //}
	write(sfd, "9 ", 2);
	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);
	write(sfd, projSend, strlen(projSend));
	char c;
	read(sfd, &c, 1);
	if(c!='1'){
		printf("Error: project does not exist on server\n");
		return 1;
	}
	int size = readNum(sfd);
	printf("size: %d\n", size);
	int serverProjVersion = readNum(sfd);
	printf("Server Project Version: %d\n", serverProjVersion);
	node* serverroot = readManifest(sfd);
	char updateName[128];
	char conflictName[128];
	sprintf(updateName, "%s/.Update", projName);
	sprintf(conflictName, "%s/.Conflict", projName);
	char rmcmd[128];
	sprintf(rmcmd, "rm %s", conflictName);
	if(fileExists(conflictName)==0){
		system(rmcmd);
	}
	int ufd = open(updateName, O_RDWR | O_CREAT, 00600); //{
	if(serverProjVersion==projVersion){
		write(1, "Everything Up to date.\n", strlen("Everything up to date.\n"));
		return 0;
	}
	node* ptr;
	for(ptr = serverroot; ptr!=NULL; ptr = ptr->next){
		node* match = getMatch(ptr->filePath, clientroot);
		char updatebuffer[256+MD5_DIGEST_LENGTH];
		if(match==NULL){ // entry exists on server but not client
			printf("A %s\n", ptr->filePath);
			sprintf(updatebuffer, "A\t%d\t%s\t%s\n", ptr->version, ptr->filePath, ptr->hash);
			write(ufd, updatebuffer, strlen(updatebuffer));
		} else { // entry exists on both client and server
			int tempfd = open(match->filePath, O_RDONLY); //{
			char* fileContents = readFile(tempfd);
//			printf("FILECONTENTS:%s\n", fileContents);
			char* hash = getHash(fileContents);
//			printf("LIVE HASH: %s\n SERVER HASH: %s\n", hash, ptr->hash);
			if(strcmp(hash, ptr->hash)==0){ // no change with server, does not need updating
				close(tempfd);
				continue;
			}
			if(strcmp(hash, match->hash)!=0){ // conflict exists
				int cfd; //FD for conflict file. closed on line 350.
				if(fileExists(conflictName)!=0){
					cfd = open(conflictName, O_RDWR | O_CREAT, 00600);
					printf("Conflicts Exist. They must be resolved before upgrading.\n");
				} else {
					cfd = open(conflictName, O_RDWR);
					lseek(cfd, 0, SEEK_END);
				}
				printf("C %s\n", ptr->filePath);
				sprintf(updatebuffer, "C\t%d\t%s\t%s\n", ptr->version, ptr->filePath, hash);
				write(cfd, updatebuffer, strlen(updatebuffer));
			} else {
				printf("M %s\n", ptr->filePath);
				sprintf(updatebuffer, "M\t%d\t%s\t%s\n", ptr->version, ptr->filePath, ptr->hash);
				write(ufd, updatebuffer, strlen(updatebuffer));
			}
			close(tempfd); //}
		}
	}
	for(ptr = clientroot; ptr!=NULL; ptr = ptr->next){
		node* match = getMatch(ptr->filePath, serverroot);
		char updatebuffer[256+MD5_DIGEST_LENGTH];
		if(match==NULL){
			printf("D %s\n", ptr->filePath);
			sprintf(updatebuffer, "D\t%d\t%s\t%s\n", ptr->version, ptr->filePath, ptr->hash);
			write(ufd, updatebuffer, strlen(updatebuffer));
		}
	}

	close(ufd); //opened on line 380.

	return 0;
}
int upgrade(int sfd, char* projName){
	char conflict[256];
	char update[256];
	sprintf(conflict, "%s/.Conflict", projName);
	sprintf(update, "%s/.Update", projName);
	if(fileExists(conflict) == 0) { 
		char errorMsg[128] = "Error: Conflicts Exist. They must be resolved before upgrading the project.\n";
		write(1, errorMsg, strlen(errorMsg));
		write(sfd, "0 ", 2);
		return -1;
	}
	int size;
	if(fileExists(update) != 0) {
		printf("Error: Client does not have .Update file, run update before upgrading.\n");
		write(sfd, "0 ", 2);
		return -1;
	} else {
		struct stat st;
		stat(update, &st);
		if(st.st_size == 0) {
			printf("Project is up to date!\n");
			return 0;
		}
	} 
	write(sfd, "10 ", 3);
	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);
	write(sfd, projSend, strlen(projSend));
	char c;
	read(sfd, &c, 1);
	if(c!='1'){
		printf("Error: project does not exist on server\n");
		return -1;
	}
	int ufd = open(update, O_RDONLY); //{
	node* updateRoot = readManifest(ufd);
	close(ufd); //}
	printManifest(updateRoot);
	node* ptr;
	char cmd[256];
	//For anything in the .Update that should be deleted, delete the file.
	for(ptr = updateRoot; ptr != NULL; ptr = ptr->next) {
		if(ptr->status == 'D' || ptr->status == 'M') {
			sprintf(cmd, "rm %s", ptr->filePath);
			system(cmd);
			bzero(cmd, 256);
		}
	}
	char rmMan[128];
	sprintf(rmMan, "rm %s/.Manifest", projName);
	system(rmMan);
	lseek(ufd, 0, SEEK_SET);
	sendFile(sfd, ufd);
	write(sfd, "\n\n", 2);

	int tarSize = readNum(sfd);
	char* tarFile = (char*)malloc(tarSize+1);
	printf("%d\n", tarSize);
	read(sfd, tarFile, tarSize);
	int tarfd = open("upgrade.tar.gz", O_RDWR | O_CREAT, 00600); //{
	write(tarfd, tarFile, tarSize);
	close(tarfd); //}

	char cmd2[128];
	sprintf(cmd2, "tar -xzf upgrade.tar.gz %s", projName);
	system(cmd2);
	system("rm upgrade.tar.gz");
	return 0;
}
int history(int sfd, char* projName) { 
	write(sfd, "11 ", 3);
	char* projSend = (char*) malloc(strlen(projName) + 2);
	sprintf(projSend, "%s\t", projName);
	write(sfd, projSend, strlen(projSend));
	char c;
	read(sfd, &c, 1);
	if(c!='1'){
		printf("Error: project does not exist on server\n");
		return -1;
	}
	int size = readNum(sfd);
	char* historyFile = (char*)malloc(size);
	read(sfd, historyFile, size);
	write(1, historyFile, size);
	free(historyFile);
	return 0;
} 

int rollback(int sfd, char* projName, char* version) {
	write(sfd, "12 ", 3);
	char* arguments = (char*)malloc(strlen(projName) + strlen(version) + 15);
	sprintf(arguments, "%s\t%s\n", projName, version);
	write(sfd, arguments, strlen(arguments));
//	printf("%s", arguments);
	char c;
	read(sfd, &c, 1);
	if('0' == c) {
		printf("Error: Project does not exist on server.\n");
		return -1;
	}
	
	read(sfd, &c, 1);
	if('0' == c) {
		printf("Error: Specified version does not exist on server.\n");
		return -1;
	}
	printf("Project %s has been rolled back to version %s.\n", projName, version);
	return 0;
}
