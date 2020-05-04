#ifndef serverFunc
#define serverFunc
typedef struct commitNode{
	char* file;
	int size;
	struct commitNode* next;
} commitNode;

typedef struct projNode {
	char* projName;
	struct commitNode* commitListRoot;
	struct projNode* next;
} projNode;

projNode* projRoot;

void error (char *msg);
int isChanged (char *fileName, node *root);
char *removeProjName (char *filePath);
void writeCommit (int fd, node *root);
projNode *searchPNode (char *pName);
char *getSize (int fd);
void sendFile (int sockfd, int fd);
int projectExists (char *projectName);
void freeNodeList(node* ptr);

char *getProjName (int sockfd);
int destroyProject (int sockfd);
int createProject (int sockfd);
char *extractFileNameFromPath (char *path);
int currentVersion (int sockfd);
int getVersion (char *projName);
int checkout (int sockfd);
int commit (int sockfd);
int push (int sockfd);

void *clientConnect (void *clientSockfd);
int main (int argc, char **argv);
#endif //serverFunc
