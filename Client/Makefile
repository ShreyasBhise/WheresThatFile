all: readManifest.o cGit.o cUtil.o
	gcc -o WTF client.c readManifest.o cGit.o cUtil.o -lssl -lcrypto
readManifest.o:
	gcc -c readManifest.c
cGit.o:
	gcc -c cGit.c
cUtil.o:
	gcc -c cUtil.c
debug: clean readManifestDebug cGitDebug cUtilDebug
	gcc -g -o WTF client.c readManifest.o cGit.o cUtil.o -lssl -lcrypto
readManifestDebug:
	gcc -g -c readManifest.c
cGitDebug:
	gcc -g -c cGit.c
cUtilDebug:
	gcc -g -c cUtil.c
clean:
	rm WTF readManifest.o cGit.o cUtil.o
