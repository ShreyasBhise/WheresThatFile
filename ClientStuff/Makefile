all: readManifest.o
	gcc -o WTF client.c readManifest.o -lssl -lcrypto
readManifest.o:
	gcc -c readManifest.c
debug: readManifestDebug
	gcc -g -o WTF client.c readManifest.o -lssl -lcrypto
readManifestDebug:
	gcc -g -c readManifest.c
clean:
	rm WTF readManifest.o
