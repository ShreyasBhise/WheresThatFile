all: readManifest.o
	gcc -o WTF client.c readManifest.o -lssl -lcrypto
readManifest.o:
	gcc -c readManifest.c
clean:
	rm WTF readManifest.o
