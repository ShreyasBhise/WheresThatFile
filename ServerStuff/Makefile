all: clean readManifest.o sUtil.o sGit.o
	gcc -I ../ClientStuff/ -o WTFServer server.c readManifest.o sUtil.o sGit.o -pthread
readManifest.o:
	gcc -I ../ClientStuff/ -c ../ClientStuff/readManifest.c -o readManifest.o
sUtil.o: 
	gcc -I ../ClientStuff/ -c sUtil.c
sGit.o: 
	gcc -I ../ClientStuff/ -c sGit.c
debug: readManifestDebug sUtilDebug sGitDebug
	gcc -g -I ../ClientStuff/ -o WTFServer server.c readManifest.o sUtil.o sGit.o -pthread
readManifestDebug:
	gcc -g -I ../ClientStuff/ -c ../ClientStuff/readManifest.c -o readManifest.o
sUtilDebug: 
	gcc -g -I ../ClientStuff/ -c sUtil.c
sGitDebug: 
	gcc -g -I ../ClientStuff/ -c sGit.c
clean:
	rm WTFServer readManifest.o sUtil.o sGit.o
