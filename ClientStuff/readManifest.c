#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include"readManifest.h"

int manifestVersion(int fd){
	int n;
	char* str = (char*)malloc(256);
	char c;
	int i = 0;
	while((n=read(fd, &c, 1))>=0){
		if(n==0) continue;
		if(c=='\n') break;
		str[i]=c;
		i++;
	}
	str[i]='\0';
	int x = atoi(str);
	return x;
}

node* readManifest(int fd){
	node* root = (node*)malloc(sizeof(node));
	
	return root;
}
