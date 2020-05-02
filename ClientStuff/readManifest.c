#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include"readManifest.h"

void printManifest(node* root) {
	node* ptr;
	for(ptr = root; ptr != NULL; ptr = ptr->next) {
		printf("%c %d %s %s\n", ptr->status, ptr->version, ptr->filePath, ptr->hash);
	}

}
int readNum(int fd){
	int n;
	char* str = (char*)malloc(256);
	char c;
	int i = 0;
	while((n=read(fd, &c, 1)) > 0){
		if(n==0) continue;
		if(n<0){
			printf("Error: unable to read int");
			exit(1);
		}
		if(c=='\n' || c=='\t' || c== ' ') break;
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
	while((n=read(fd, &c, 1)) > 0){
		if(n==0) continue;
		if(n<0){
			printf("Error: unable to read string");
			exit(1);
		}
		if(c=='\t' || c=='\n' || c==' '){
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
	printf("Error: readStr not terminated by tab or newline\n");
	return str;
}
int readNumP(char* file, int* i){
	char* str = (char*)malloc(256);
	char c;
	int count = 0;
	while(1){
		c = file[i[0]];
		if(c=='\n' || c=='\t' || c== ' ') break;
		str[count]=c;
		i[0]++;
		count++;
	}
	str[count++]='\0';
	int x = atoi(str);
	return x;
}

char* readStrP(char* file, int* i){
	int length = 10;
	char* str = (char*)malloc(length);
	char c;
	int count = 0;
	while(1){
		c = file[i[0]];
		if(c=='\t' || c=='\n' || c==' '){
			str[count]='\0';
			return str;
		}
		if(count+1>=length){ // double length of char array
			length = length*2;
			char* temp = (char*)malloc(length);
			str[count]='\0';
			strcpy(temp, str);
			char* temp2 = str;
			str = temp;
			free(temp2);
		}
		str[count]=c;
		i[0]++;
		count++;
	}
	printf("Error: readStr not terminated by tab or newline\n");
	return str;
}
node* readManifest(int fd){
	manifestSize = 0;
	node* root = (node*)malloc(sizeof(node));
	node* curr = root;
	node* prev = NULL;
	int count = 0, count2 = 0;
//	lseek(fd, 2, SEEK_SET);
	while(1){
		char c[3];
		int n;
		n=read(fd, &c, 2);
		if(n<=0){
			break;
		}
		if(c[0]=='\n') break;
		c[2] = '\0';
		curr->status = c[0];
		printf("Read status: %c.\n", curr->status);
		curr->version = readNum(fd);
		printf("Read version: %d.\n", curr->version);
		curr->filePath = readStr(fd);
		printf("Read filePath: %s.\n", curr->filePath);
		curr->hash = readStr(fd);
		printf("Read hash: %s.\n", curr->hash);
		if(prev!=NULL){
			prev->next = curr;
		}
		prev = curr;
		curr = (node*)malloc(sizeof(node));
		manifestSize++;
	}
	free(curr);
	if(prev==NULL) return NULL; // .manifest contains no entries
	prev->next = NULL;
	return root;
}

node* readManifestServer(char* file){
	node* root = (node*)malloc(sizeof(node));
	node* curr = root;
	node* prev = NULL;
	int* i = (int*)malloc(sizeof(int));
	i[0] = 0;
	int size = 0;
	while(1){
		char c = file[i[0]];
		if(c=='\n'){
			break;
		}
		i[0] = i[0]+2;
		curr->status = c;
		curr->version = readNumP(file, i);
		printf("Read version: %d.\n", curr->version);
		curr->filePath = readStrP(file, i);
		printf("Read filePath: %s.\n", curr->filePath);
		curr->hash = readStrP(file, i);
		printf("Read hash: %s.\n", curr->hash);
		if(prev!=NULL){
			prev->next = curr;
		}
		prev = curr;
		curr = (node*)malloc(sizeof(node));
		size++;
	}
	printf("%d\n", size);
	free(curr);
	if(prev==NULL) return NULL; // .manifest contains no entries
	prev->next = NULL;
	free(i);
	return root;
}
