#ifndef READMANIFEST_H
#define READMANIFEST_H
int manifestSize;
typedef struct node{
	char status;
	struct node* next;
	int version;
	char* filePath;
	char* hash;
} node;
void printManifest(node* root);
char* readStr(int fd);
int readNum(int fd);
char* readStr(int fd);
node* readManifest(int fd);

#endif
