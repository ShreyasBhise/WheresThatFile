#ifndef funcPrototype
#define funcPrototype
int projectExists (char *projectName);
int fileExists (char *filePath);
char *getSize (int fd);
void sendFile (int sockfd, int fd);
void readBytes (int sfd, int x, void *buffer);
char *readFile (int fd);
int readFileFromServer (int sfd, char **buffer);
char *getHash (char *toHash);
int isFileAdded (node *root, char *filePath);
node *getMatch (char *fileName, node *root);
void getPath (char *pName, char *fName, char *buffer);
int writeCommitFileClient(int fd, node* ptr, node* serverroot);

int addFile (char *projName, char *fileName);
int removeFile (char *projName, char *fileName);
int destroyProject (int sfd, char *projName);
int createProject (int sfd, char *projName);
int currentVersion (int sfd, char *projName);
int checkout (int sfd, char *projName);
int commit (int sfd, char *projName);
int pushCommit (int sfd, char *projName);
int update (int sfd, char *projName);
int history(int sfd, char* projName);
int rollback(int sfd, char* projName, char* version);

int connectToServer (void);
void getconfig (void);
void config (char **argv);
int checkinput (int argc, char **argv);
#endif //funcPrototype
