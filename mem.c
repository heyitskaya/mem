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
	
	printf("Freeing the allocated chunk \n");
	if(Mem_Free(ptr) == -1){
		printf("ERROR: Mem_Free failed \n");
	}
	Mem_Dump();
	
	return 0;
}

struct Header *head;
	
/**Calls mmap to request sizeOfRegion bytes of memory to manage, subject to rounding up **/
void *Mem_Init(int sizeOfRegion){
//put first header in place
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
	(*head).size = sizeOfRegion-sizeof(*head);
	head->startAddress = ptr;
	head->magic = 0;
	head->next = NULL;
	printf("size of head in init is %d\n",sizeof(head));
	close(fd);
	return ptr;
}

/**Given a number of bytes to allocate, allocate memory and returns a pointer to start of allocated memory**/
void *Mem_Alloc(int sizeRequested){
	struct Header *node;
	node = head;
	while(node!=NULL){
		if(node->size > sizeRequested+32){ //does not need to add additional 32 because the node already has header
			//allocate the memory at the first n bytes of free space
			void *nodeAddress = node->startAddress;
			int sizeOfNode = node->size;
			int sizeofNodeHeader = sizeof(node);	
			struct Header *allocatedMemory = node->startAddress;
			printf("size of node %d\n", sizeOfNode);
			allocatedMemory->magic = 20;
			allocatedMemory->size = sizeRequested;	
			node = allocatedMemory->startAddress + sizeof(allocatedMemory) + sizeRequested;

			node->size = sizeOfNode-sizeRequested-sizeof(allocatedMemory);
			printf("node->size %d\n", node->size);	
			node->startAddress = node->startAddress + sizeRequested + sizeof(allocatedMemory); //change starting address
			printf("allocatedMemory.startAddress is %p\n",allocatedMemory->startAddress);
			node->next = allocatedMemory->next;
			allocatedMemory->next = NULL;
			printf("sizeof(allocatedMemoryHeader): %d\n", sizeof(allocatedMemory));
			printf("allocatedMemory.magic is %d\n", allocatedMemory->magic);
			return allocatedMemory->startAddress+sizeof(allocatedMemory);
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
	printf("%s\n", "------------------");
	printf("ptr address %p\n", ptr);
	printf("in free size of header is %d\n", sizeof(struct Header *));
	printf("header address %p\n", (struct Header *) (ptr-8));
	struct Header *freed = (struct Header *) (ptr-8);
	printf( "freed magic %d\n", freed->magic); 
	if(ptr == NULL){
		printf("%s\n", "in if statement");
		return -1; //unsuccessful
	}
	else if (freed->magic == 32){ //if it's actually been allocated
		printf("%s\n", "in else if statement");
		freed->magic = 0;
		//add to beginning of free list
		freed->next = head;
		head = freed;
		return 0;	
	}
	else{
		printf("%s\n", "in else ");
		return -1;
	}
}
/**For debugging**/

void Mem_Dump(){
/**	struct Header *node = head;
	while(node!=NULL){
		printf("%s\n", "Free memory:");
		printf("address = %p\n", (void *)node);
		printf("size = %d\n", node->size);
		node = node->next;
	}
	printf("----------------\n");**/
}


