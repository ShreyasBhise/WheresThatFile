all: readManifest.o sUtil.o sGit.o
	gcc -I ../ClientStuff/ -o WTFServer server.c readManifest.o sUtil.o sGit.o -pthread
readManifest.o:
	gcc -I ../ClientStuff/ -c ../ClientStuff/readManifest.c -o readManifest.o
sUtil.o: 
	gcc -I ../ClientStuff/ -c sUtil.c
sGit.o: 
	gcc -I ../ClientStuff/ -c sGit.c
clean:
	rm WTFServer readManifest.o sUtil.o sGit.o
