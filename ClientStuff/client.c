#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

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
	}
}

int main(int argc, char** argv){
	checkinput(argc, argv);
	return 0;
}
