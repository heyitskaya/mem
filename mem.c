#include "mem.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
typedef struct Header {
           void *startAddress; //includes header
           int size; //amount of free space not including head
           int magic;
           struct Header* next;
 };

typedef struct List {
         struct Header head;
};

int main(int argc, char **argv){
	 Mem_Init(4096); //virtual address
	Mem_Dump();
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
//	head->size = sizeOfRegion-sizeof(head);
	int pageSize = getpagesize();
	printf("pageSize %d\n", pageSize);
	if(sizeOfRegion%pageSize!=0){ //need to round up
		int num = sizeOfRegion/pageSize;
		sizeOfRegion = pageSize*(num+1);
	}
	
	int fd = open("/dev/zero", O_RDWR);
	void *ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if(ptr == MAP_FAILED){
		printf("map failed");
		perror("mmap"); 
		exit(1);
	}
	head = (struct Header *)ptr;
	printf("sizeOfRegion %d\n", sizeOfRegion);
	printf("size of head %d\n", sizeof(*head));
	(*head).size = sizeOfRegion-sizeof(*head);
	printf("head.size %d\n", (*head).size);
	head->startAddress = ptr;
	head->magic = 0;
	head->next = NULL;
	close(fd);
	return ptr;
}

/**Given a number of bytes to allocate, allocate memory and returns a pointer to start of allocated memory**/
void *Mem_Alloc(int sizeRequested){
	struct Header *node;
	node = head;
//	printf("size of node %d\n", head->size);
	while(node!=NULL){
	//node->size
		if(node->size > sizeRequested){ //does not need to add additional 32 because the node already has header
			//allocate the memory at the first n bytes of free space
printf("in if statement");
			void *nodeAddress = node->startAddress;
			int sizeOfNode = node->size;
			int sizeofNodeHeader = sizeof(node);	
			struct Header *allocatedMemory = node->startAddress;
			printf("size of node %d\n", sizeOfNode);

allocatedMemory->size = sizeRequested; //amount of free memory
			allocatedMemory->magic = 8;
			allocatedMemory->startAddress = node->startAddress;
			
			node->size = sizeOfNode-sizeRequested-sizeof(allocatedMemory);
			printf("sizeof(allocatedMemory): %d\n", sizeof(allocatedMemory));			
			printf("node->size %d\n", node->size);	
	//	int sizeRequested = 4096;
			node->startAddress = node->startAddress + sizeRequested + sizeof(allocatedMemory); //change starting address
			return allocatedMemory->startAddress;
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
	struct Header *freed = (struct Header *) ptr;
	if(ptr == NULL){
		return -1; //unsuccessful
	}
	else if (freed->magic == 8){ //if it's actually been allocated
		freed->magic == 0;
		//add to beginning of free list
		freed->next = head;
		head = freed;
		return 0;	
	}
	else{
		return -1;
	}
}
/**For debugging**/

void Mem_Dump(){
	struct Header *node = head;
	while(node!=NULL){
		printf("%s\n", "Free memory");
		printf("address = %p\n", (void *)node);
		printf("size = %d\n", node->size);
		node = node->next;
	}
	printf("----------------\n");
}


