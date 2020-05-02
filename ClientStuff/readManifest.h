#ifndef READMANIFEST_H
#define READMANIFEST_H
int size;
typedef struct node{
	char status;
	struct node* next;
	int version;
	char* filePath;
	char* hash;
} node;

int readNum(int fd);
char* readStr(int fd);
node* readManifest(int fd);

#endif
