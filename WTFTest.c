#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//Run TEST code
int main(int argc, char** argv){
	int fd = open(".Commands", RD_ONLY);
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
	return 0;
}
