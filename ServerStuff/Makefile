all: readManifest.o
	gcc -o WTFServer server.c readManifest.o -pthread
readManifest.o:
	gcc -I ../ClientStuff/ -c ../ClientStuff/readManifest.c -o readManifest.o
clean:
	rm WTTServer readManifest.o
