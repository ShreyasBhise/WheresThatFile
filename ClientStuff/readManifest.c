#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include"readManifest.h"

int readNum(int fd){
	int n;
	char* str = (char*)malloc(256);
	char c;
	int i = 0;
	while((n=read(fd, &c, 1))>=0){
		if(n==0) continue;
		if(n<0){
			printf("Error: unable to read int");
			exit(1);
		}
		if(c=='\n' || c=='\t') break;
		str[i]=c;
		i++;
	}
	str[i]='\0';
	int x = atoi(str);
	return x;
}

char* readStr(int fd){
	int n;
	int length = 10;
	char* str = (char*)malloc(length);
	char c;
	int i = 0;
	while((n=read(fd, &c, 1))>=0){
		if(n==0) continue;
		if(n<0){
			printf("Error: unable to read string");
			exit(1);
		}
		if(c=='\t' || c=='\n'){
			str[i]='\0';
			return str;
		}
		if(i+1>=length){ // double length of char array
			length = length*2;
			char* temp = (char*)malloc(length);
			str[i]='\0';
			strcpy(temp, str);
			char* temp2 = str;
			str = temp;
			free(temp2);
		}
		str[i]=c;
		i++;
	}
	printf("Error: readStr not terminated by tab or newline");
	return str;
}

node* readManifest(int fd){
	node* root = (node*)malloc(sizeof(node);
	node* curr = root;
	node* prev = NULL;
	while(1){
		char c;
		int n;
		while((n=read(fd, &c, 1))>=0){
			if(n==0) continue;
		}
		if(n<0){
			break;
		}
		curr->status = c;
		curr->version = readNum(fd);
		curr->filePath = readStr(fd);
		curr->hash = readStr(fd);
		if(prev!=NULL){
			prev->next = curr;
		}
		prev = curr;
		curr = (node*)malloc(sizeof(node));
	}
	free(curr);
	if(prev==NULL) return NULL; // .manifest contains no entries
	return root;
}
