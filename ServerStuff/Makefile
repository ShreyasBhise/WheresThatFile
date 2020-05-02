all: ../ClientStuff/readManifest.o
	gcc -o WTFServer server.c ../ClientStuff/readManifest.o -pthread
readManifest.o:
	gcc -c ../ClientStuff/readManifest.c
clean:
	rm WTFServer ../ClientStuff/readManifest.o
