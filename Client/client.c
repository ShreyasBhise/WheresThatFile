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

char* ipaddress;
int port;

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
	while(connect(sfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo))<0){
		printf("Connection Failed. Trying again in 3 seconds\n");
		sleep(3);
	}
	printf("Connected to Server\n");
	return sfd;
}
void getconfig(){
	int fd = open("./.Configure", O_RDONLY);
	if(fd<0){
		printf("Error: failed to open .Configure file");
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
	int fd = open("./.Configure", O_WRONLY | O_CREAT, 00600);
	if(fd<0){
		printf("Error: failed to create .Configure file");
		exit(1);
	}
	char* configStr = (char*)malloc(128);
	sprintf(configStr, "Configured to %s:%s\n", argv[2], argv[3]);
	write(1, configStr, strlen(configStr));
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
		else if(strcmp(argv[1], "push") == 0 && argc == 3){
			return 8;
		} 
		else if (strcmp(argv[1], "update") == 0 && argc == 3) {
			return 9;
		}
		else if (strcmp(argv[1], "upgrade") == 0 && argc == 3) {
			return 10;
		}
		else if(strcmp(argv[1], "history") == 0 && argc == 3) {
			return 11;
		}
		else if(strcmp(argv[1], "rollback") == 0 && argc == 4) {
			return 12;
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
	int n;
	switch(type) {
		case 1: //create
			n = createProject(sfd, argv[2]);	
			break;
		case 2: //destroy
			n = destroyProject(sfd, argv[2]);
			break;
		case 5: //currentversion
			n = currentVersion(sfd, argv[2]);
			break;
		case 6: //checkout
			n = checkout(sfd, argv[2]);
			break;
		case 7: //commit
			n = commit(sfd, argv[2]);
			break;
		case 8: //push
			n = pushCommit(sfd, argv[2]);
			break;
		case 9: //update
			n = update(sfd, argv[2]);
			break;
		case 10: //upgrade
			n = upgrade(sfd, argv[2]);
			break;
		case 11: // histroy
			n = history(sfd, argv[2]);
			break;
		case 12: // rollback
			n = rollback(sfd, argv[2], argv[3]);
			break;
	}
	close(sfd);
	printf("Disconnected from Server\n");
	return 0;
}
