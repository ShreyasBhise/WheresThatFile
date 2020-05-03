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
char* ipaddress;
int port;


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
int addFile(char* projName, char* fileName) {
	char filePath[256];
	char manifestPath[256];
	getPath(projName, ".Manifest", manifestPath);
	getPath(projName, fileName, filePath);
        
       	int manifest = open(manifestPath, O_RDWR | O_APPEND);
	/*TODO: Check if the file exists in the Manifest */
	int x = readNum(manifest);
	printf("In addFile: %d\t%c\n", x, x);
	node* mList = readManifest(manifest);
	printf("created list\n");
	
	
	if(isFileAdded(mList, filePath)) {
		printf("File is already in manifest\n");
		return 0;
	}
	int fileToAdd = open(filePath, O_RDONLY);
	char* fileContents = readFile(fileToAdd);

	char* hash = getHash(fileContents);

	char toWrite[256];
		
	sprintf(toWrite, "A\t0\t%s\t%s\n", filePath, hash);
	int n = write(manifest, toWrite, strlen(toWrite)); 
	
	return 1;

}
int removeFile(char* projName, char* fileName) {
	char filePath[256]; 
	char manifestPath[256];
        getPath(projName, ".Manifest", manifestPath);
        getPath(projName, fileName, filePath);	
	
	int manifest = open(manifestPath, O_RDWR);
	int x = readNum(manifest);
	node* mList = readManifest(manifest);
	
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
	n = write(sfd, projName, strlen(projName));
	char c;
	while((n=read(sfd, &c, 1))==0)
	
	if(c == '1') printf("Project has been removed from the repository\n");
	else printf("Error: Project does not exist in repository\n");
	return 0;
}

int createProject(int sfd, char* projName){
	int n = 0;
	char* command = "1 ";
	n = write(sfd, command, strlen(command));
	n = write(sfd, projName, strlen(projName));
//	printf("%s%s\n", command, str);
	char c;
	while((n=read(sfd, &c, 1))==0){
	}
	if(c=='0'){ // project already exists
		printf("Error: Project already exists\n");
		return 1;
	} else if (c!='1'){
		printf("Error\n");
		return 1;
	}
	char** buffer = (char**)malloc(sizeof(char*));
	int size = readFileFromServer(sfd, buffer);
//	printf("%s", *buffer);
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
	n = write(sfd, projName, strlen(projName));
	int num = readNum(sfd);
	printf("%d", num);
	int i;
	for(i = 0; i < num; i++) {
		printf("loop number: %d\n", i);
		int version = readNum(sfd);
		printf("%d\n", version);
		char* name = readStr(sfd);
		printf("%s\n", name);
		printf("%d\t%s\n", version, name);
	}
}
int checkout(int sfd, char* projName){
	if(projectExists(projName)){
		printf("Error: project already exits\n");
		write(sfd, "0 ", 2);
		return 1;
	}
	int n = 0;
	char* command = "6 ";
	n = write(sfd, command, 2);
	n = write(sfd, projName, strlen(projName));
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
	int fd = open(newName, O_RDWR | O_CREAT, 00600);
	char* file = (char*)malloc(size+1);
	readBytes(sfd, size, file);
	n = write(fd, file, size);
	char sysCall[256];
	char rmCall[256];
	sprintf(sysCall, "tar -xzf %s", newName);
	sprintf(rmCall, "rm %s", newName);
	system(sysCall);
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
		return 0;
	}
	if(fileExists(update) == 0) {
		struct stat st;
		stat(update, &st);
		if(st.st_size != 0) {
			printf("Error: Client is behind the server, upgrade before you commit");
			return 0;
		}
	}
	char fileName[256];
	sprintf(fileName, "%s/.Manifest", projName);
	int fd = open(fileName, O_RDONLY);
	if(fd==-1){
		printf("Error: unable to open client .Manifest");
		write(sfd, "0 ", 2);
		return 1;
	}
	int projVersion = readNum(fd);
	node* clientroot = readManifest(fd);
	write(sfd, "7 ", 2);
	write(sfd, projName, strlen(projName));
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
	int cfd = open(commitFile, O_RDWR | O_CREAT, 00600);
	node* ptr;
	printf("Changes:\n");
	for(ptr = clientroot; ptr != NULL; ptr = ptr->next) {
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
	}
	lseek(cfd, 0, SEEK_SET);
	system("cat p3/.Commit");
	sendFile(sfd, cfd);
	return 0;
}

int connectToServer(){
	int sfd=-1;
	struct sockaddr_in serverAddressInfo;
	struct hostent *serverIPAddress;
	serverIPAddress = gethostbyname(ipaddress);
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd<0){
		printf("Error: creating socket failed\n");
		exit(1);
	}
	bzero((char*) &serverAddressInfo, sizeof(serverAddressInfo));
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_port = htons(port);
	bcopy((char*)serverIPAddress->h_addr, (char*)&serverAddressInfo.sin_addr.s_addr, serverIPAddress->h_length);
	if(connect(sfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo))<0){
		printf("Error connecting to server\n");
		exit(1);
	}
	printf("Connected to Server\n");
	return sfd;
}

void getconfig(){
	int fd = open("./.configure", O_RDONLY);
	if(fd<0){
		printf("Error: failed to open .configure file");
		exit(1);
	}
	int curr = 0;
	char c;
	ipaddress = (char*)malloc(256);
	char* portstr = (char*)malloc(10);
	int mode = 0;
	int n=0;
	while((n = read(fd, &c, 1))!=0){
		if(c=='\t'){
			mode = 1;
			ipaddress[curr]='\0';
			curr = 0;
			continue;
		}
		else if(c=='\n'){
			portstr[curr]='\0';
			port = atoi(portstr);
			free(portstr);
			break;
		} else if (mode==0){
			ipaddress[curr]=c;
		} else if (mode==1){
			portstr[curr]=c;
		}
		curr++;
	}
	return;
}

void config(char** argv){
	printf("configuring ip\n");
	int fd = open("./.configure", O_WRONLY | O_CREAT, 00600);
	if(fd<0){
		printf("Error: failed to create .configure file");
		exit(1);
	}
	write(fd, argv[2], strlen(argv[2]));
	write(fd, "\t", 1);
	write(fd, argv[3], strlen(argv[3]));
	write(fd, "\n", 1);
	close(fd);
	return;
}

int checkinput(int argc, char** argv){
	if(argc>=2){
		if(strcmp(argv[1], "configure")==0 && argc==4){
			config(argv);
			return 0;
		}
		else if(strcmp(argv[1], "create")==0 && argc==3){
			return 1;
		}
		else if(strcmp(argv[1], "destroy")==0 && argc==3){
			return 2;
		}
		else if(strcmp(argv[1], "add") == 0 && argc == 4) {
			return 3;
		}
		else if(strcmp(argv[1], "remove") == 0 && argc == 4) {
			return 4;
		}
		else if(strcmp(argv[1], "currentversion") == 0 && argc == 3) {
			return 5;
		}
		else if(strcmp(argv[1], "checkout") == 0 && argc == 3) {
			return 6;
		}
		else if(strcmp(argv[1], "commit") == 0 && argc == 3) {
			return 7;
		}
	}
	return -1;
}

int main(int argc, char** argv){
	int type = checkinput(argc, argv);
	if(type==0) return 0;
	else if(type==-1){
		printf("Error: Invalid Input");
		return 1;
	}
	if(type == 3) {
                int n = addFile(argv[2], argv[3]);
		return 0;
        } 
	else if(type == 4) {
		int n = removeFile(argv[2], argv[3]);
		return 0;
	}
	getconfig();
	int sfd = connectToServer();
	if(type==1){ // create called
		int n = createProject(sfd, argv[2]);	
	} else if (type==2){ // destroy called
		int n = destroyProject(sfd, argv[2]);
	} else if(type == 5) { 
		int n = currentVersion(sfd, argv[2]);
	} else if (type == 6) {
		int n = checkout(sfd, argv[2]);
	} else if (type == 7){ // commit called
		int n = commit(sfd, argv[2]);
	}
//	printf("%s\t%d\n", ipaddress, port);
	close(sfd);
	printf("Disconnected from Server\n");
	return 0;
}
