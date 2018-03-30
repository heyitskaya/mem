#include "mem.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
int main(int argc, char **argv){
	return 0;
}

typedef struct header {
	int size;
	int magic;
};

/**node will have a header and next **/

/**Calls mmap to request sizeOfRegion bytes of memory to manage, subject to rounding up **/
void *Mem_Init(int sizeOfRegion){
	int pageSize = getpagesize();
	if(sizeOfRegion%pageSize!=0){ //need to round up
		int num = sizeOfRegion/pageSize;
		sizeOfRegion = pageSize*(num+1);
	}
	
	int fd = open("/dev/zero", O_RDWR);
	void *ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if(ptr == MAP_FAILED){
		perror("mmap"); 
		exit(1);
	}
	close(fd);
	return ptr;
}

/**Given a number of bytes to allocate, allocate memory and returns a pointer to start of allocated memory**/
void *Mem_Alloc(int size){

}

/**Given a pointer free the memory allocated**/
int Mem_Free(void *ptr){
	return -1;
}
/**For debugging**/
void MemDump(){

}


