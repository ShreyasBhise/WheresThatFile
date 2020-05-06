#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Run TEST code
int main(int argc, char** argv){
	int pid = fork();
	
	if(pid == 0) {//Child Process -> Should be server.
		system("cd Server; ./WTFServer localhost 6782");
	} else { //Parent Process -> Should handle client.
		system("cd Client");
		int fd = open(".Commands", O_RDONLY);
		char* cmd = (char*)malloc(256);
		char c;
		int n;
		int i = 0;
		while((n = read(fd, &c, 1))>0){
			if(c=='\n'){
				cmd[i]='\0';
				system(cmd);
				i = 0;
				bzero(cmd, 255);
			} else {
				cmd[i] = c;
				i++;
			}
		}
		free(cmd);
	}
	return 0;
}
