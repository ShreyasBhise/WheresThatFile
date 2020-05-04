#include"client.h"
#include"readManifest.h"


int main(int argc, char** argv) {
	int toHash = open(argv[1], O_RDONLY);
	int size = lseek(toHash, 0, SEEK_END);
	lseek(toHash, 0, SEEK_SET);
		


}
