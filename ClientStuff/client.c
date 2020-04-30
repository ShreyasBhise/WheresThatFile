#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<openssl/md5.h>
#include"readManifest.h"
char* ipaddress;
int port;

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
		if(c==' ') break;
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
void getHash(char* toHash, unsigned char* fileHash) {
	MD5_CTX c;
        MD5_Init(&c);
        MD5_Update(&c, toHash, strlen(toHash));
        MD5_Final(fileHash, &c);
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
int addFile(char* projName, char* fileName) {
	char manifestPath[100];
	char filePath[100];
	sprintf(manifestPath, "%s/.Manifest", projName); 
	sprintf(filePath, "%s/%s", projName, fileName);
        
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

	unsigned char fileHash[MD5_DIGEST_LENGTH];
	getHash(fileContents, fileHash);

	int i;
	char toWrite[256];
	char hash[33];
        
	for(i = 0; i < MD5_DIGEST_LENGTH; i++) 
		sprintf(&hash[i * 2], "%02x", (unsigned int) fileHash[i]);
		
	sprintf(toWrite, "A\t0\t%s\t%s\n", filePath, hash);
	int n = write(manifest, toWrite, strlen(toWrite)); 
	
	return 1;

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
	write(fd, "\n", 1);
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
	printf("configuring ip");
	int fd = open("./.configure", O_WRONLY | O_CREAT, 00600);
	if(fd<0){
		printf("Error: failed to create .configure file");
		exit(1);
	}
	printf(argv[2]);
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
	getconfig();
	int sfd = connectToServer();
	if(type==1){ // create called
		int n = createProject(sfd, argv[2]);	
	} else if (type==2){ // destroy called
		int n = destroyProject(sfd, argv[2]);
	} else if(type == 3) {
		int n = addFile(argv[2], argv[3]);
	}
//	printf("%s\t%d\n", ipaddress, port);
	close(sfd);
	printf("Disconnected from Server\n");
	return 0;
}
