all: readManifest.o cGit.o cUtil.o
	gcc -o WTF client.c readManifest.o cGit.o cUtil.o -lssl -lcrypto
readManifest.o:
	gcc -c readManifest.c
cGit.o:
	gcc -c cGit.c
cUtil.o:
	gcc -c cUtil.c
debug: readManifestDebug
	gcc -g -o WTF client.c readManifest.o -lssl -lcrypto
readManifestDebug:
	gcc -g -c readManifest.c
clean:
	rm WTF readManifest.o cGit.o cUtil.o
