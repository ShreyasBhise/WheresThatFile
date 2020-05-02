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

char* readStr(int fd);
char* readStrP(char* file, int* i);
int readNum(int fd);
<<<<<<< HEAD
char* readStr(int fd);
=======
int readNumP(char* file, int* i);
>>>>>>> 1dcc1c7c7fdd0a8924d9f62efcbddcb4fb2042df
node* readManifest(int fd);
node* readManifestServer(char* file);

#endif
