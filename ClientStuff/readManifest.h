#ifndef READMANIFEST_H
#define READMANIFEST_H

typedef struct node{
	char status;
	struct node* next;
	int version;
	char* filePath;
	char* hash;
} node;

char* readStr(int fd);
char* readStrP(char* file, int* i);
int readNum(int fd);
int readNumP(char* file, int* i);
node* readManifest(int fd);
node* readManifestServer(char* file);

#endif
