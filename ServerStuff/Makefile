all: readManifest.o sUtil.o
	gcc -I ../ClientStuff/ -o WTFServer server.c readManifest.o sUtil.o -pthread
readManifest.o:
	gcc -I ../ClientStuff/ -c ../ClientStuff/readManifest.c -o readManifest.o
sUtil.o:
	gcc -I ../ClientStuff/ -c sUtil.c
clean:
	rm WTTServer readManifest.o
