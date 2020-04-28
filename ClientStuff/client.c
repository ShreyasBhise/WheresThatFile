#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

char* ipaddress;
int port;

void readBytes(int sfd, int x, char* buffer){
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

int readFile(int sfd, char** buffer){
	int n = 0;
	char c;
	char* sizeStr = (char *)malloc(256);
	int curr = 0;
	while((n=read(sfd, &c, 1))>=0){
		if(c==' ') break;
		sizeStr[curr++]=c;
	}
	sizeStr[curr]='\0';
	int size = atoi(sizeStr);
	*buffer = (char*)malloc(size);
	readBytes(sfd, size, *buffer);
	printf("%d bytes\n", size);
	return size;
}

int createProject(int sfd, char* str){
	int n = 0;
	char* command = "1 ";
	n = write(sfd, command, strlen(command));
	n = write(sfd, str, strlen(str));
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
	int size = readFile(sfd, buffer);
	printf("char 1: %c\n", buffer[0][0]);
	printf("char 2: %c\n", buffer[0][1]);	
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
	getconfig();
	int sfd = connectToServer();
	if(type==1){ // create called
		int n = createProject(sfd, argv[2]);	
	}
//	printf("%s\t%d\n", ipaddress, port);
	close(sfd);
	printf("Disconnected from Server\n");
	return 0;
}
