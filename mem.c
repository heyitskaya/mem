#include "mem.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
//struct Header *head;
typedef struct Header {
           void *startAddress;
           int size; //amount of free space not including head
           int magic;
           struct Header* next;
 };

typedef struct List {
         struct Header head;
};

int main(int argc, char **argv){
	printf("in main function");
	int sizeRequested = 4096;
	void *memory = Mem_Init(4096); //virtual address
	void *ptr = Mem_Alloc(4);
	printf("Allocated 4 bytes at %p\n", ptr);
	strcpy(ptr,"abc");
	Mem_Dump();
	return 0;
}

struct Header *head;
	
/**Calls mmap to request sizeOfRegion bytes of memory to manage, subject to rounding up **/
void *Mem_Init(int sizeOfRegion){
//put first header in place
	head->size = sizeOfRegion-sizeof(head);
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
	head = (struct Header *)ptr;
	(*head).size = sizeOfRegion-sizeof(*head);
	head->startAddress = ptr;
	head->magic = 8;
	head->next = NULL;
	close(fd);
	return ptr;
}

/**Given a number of bytes to allocate, allocate memory and returns a pointer to start of allocated memory**/
void *Mem_Alloc(int sizeRequested){
	struct Header *node;
	node = head;
	while(node!=NULL){
	//node->size
		if(node->size>sizeRequested){ //does not need to add additional 32 because the node already has header
			node->size=node->size-sizeRequested; //change size
			node->startAddress = node->startAddress + sizeRequested; //change starting address
			return node->startAddress;
		}
		else{
			node=node->next;
		}
	}
	//if no such node exists
	return NULL;
}

/**Given a pointer free the memory allocated**/
int Mem_Free(void *ptr){
	return -1;
}
/**For debugging**/

void MemDump(){
	struct Header *node = head;
	while(node!=NULL){
		printf("%s\n", "Free memory");
		printf("address = %p\n", (void *)node);
		printf("size = %d\n", node->size);
		node = node->next;
	}

}


