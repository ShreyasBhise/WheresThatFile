aall: readManifest.o sUtil.o sGit.o
	gcc -I ../Client/ -o WTFServer server.c readManifest.o sUtil.o sGit.o -pthread
readManifest.o:
	gcc -I ../Client/ -c ../Client/readManifest.c -o readManifest.o
sUtil.o: 
	gcc -I ../Client/ -c sUtil.c
sGit.o: 
	gcc -I ../Client/ -c sGit.c
debug: readManifestDebug sUtilDebug sGitDebug
	gcc -g -I ../Client/ -o WTFServer server.c readManifest.o sUtil.o sGit.o -pthread
readManifestDebug:
	gcc -g -I ../Client/ -c ../Client/readManifest.c -o readManifest.o
sUtilDebug: 
	gcc -g -I ../Client/ -c sUtil.c
sGitDebug: 
	gcc -g -I ../Client/ -c sGit.c
clean:
	rm WTFServer readManifest.o sUtil.o sGit.o
