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
//	Mem_Dump();
	
	printf("Freeing the allocated chunk \n");
	if(Mem_Free(ptr) == -1){
		printf("ERROR: Mem_Free failed \n");
	}
	Mem_Dump();
	Mem_Free(ptr); //try to free the ptr again this should fail
	if(Mem_Free(ptr)!=-1){
		printf("Error: Mem_Free of same pointer should have failed!\n");
	}
	printf("-------------------");
	printf("Allocating 2 chunks of memory.\n");
	char *ptr2 = Mem_Alloc(4);
	strcpy(ptr2, "mhc");
	char *ptr3 = Mem_Alloc(4);
	strcpy(ptr3,"bos");
	Mem_Dump();
	printf("Should see 1 chunk\n");
	printf("--------------------");

	return 0;
}

struct Header *head;
	
/**Calls mmap to request sizeOfRegion bytes of memory to manage, subject to rounding up **/
void *Mem_Init(int sizeOfRegion){
//put first header in place
	int pageSize = getpagesize();
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
	
	close(fd);
	return ptr;
}

/**Given a number of bytes to allocate, allocate memory and returns a pointer to start of allocated memory**/
void *Mem_Alloc(int sizeRequested){
	struct Header *node;
	node = head;
	int numNode=0;
	while(node!=NULL){
		if(node->size > sizeRequested+32){ 
			numNode++;	
			printf("numNode %d\n", numNode);
			printf("Before nodeStartAddress %p\n", node->startAddress );	
			int nodeSize = node->size;
			void* nodeStartAddress = node->startAddress;
			struct Header *nodeNext = node->next;
			/**create the header for the allocated memory and init instance fields**/
			struct Header *allocatedMemoryHeader = nodeStartAddress;
			allocatedMemoryHeader->startAddress = nodeStartAddress;
			allocatedMemoryHeader->size = sizeRequested;
			allocatedMemoryHeader->magic = 8;
			allocatedMemoryHeader->next = NULL;
			printf("allocactedMemoryHeader->startAddress %p\n", allocatedMemoryHeader->startAddress);
			printf("allocatedMemoryHeader->size %d\n", allocatedMemoryHeader->size);
			printf("allocatedMemoryHeader->magic %d\n", allocatedMemoryHeader->magic);
			printf("allcoatedMemoryHeader object size is %d\n", sizeof(*allocatedMemoryHeader));
		
			/**change instance fields of node**/
			node = nodeStartAddress + sizeof(*allocatedMemoryHeader) + sizeRequested;
			node->size = nodeSize-sizeof(*allocatedMemoryHeader) - sizeRequested;//minus size of header object
			node->magic = 0;
			node->startAddress = nodeStartAddress + sizeof(*allocatedMemoryHeader) + sizeRequested;
			node->next = nodeNext;
			if(numNode == 1){
				head = node;
			}
		/*	printf("node->size %d\n", node->size);
			printf("node->magic %d\n", node->magic);
			printf("node->startAddress %p\n", node->startAddress);**/
			return allocatedMemoryHeader->startAddress + sizeof(*allocatedMemoryHeader);
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
/**
	printf("ptr address %p\n", ptr);
	printf("in free size of header object is %d\n", sizeof(struct Header));
	printf("in free size of header is %d\n", sizeof(struct Header *));
	printf("calculated header address %p\n", (struct Header *) (ptr-24));**/
	struct Header *freed = (struct Header *) (ptr-24);
	printf( "freed magic %d\n", freed->magic); 
	if(ptr == NULL){
		return -1; 
	}
	else if(freed->magic == 8) { //if it's actually been allocated
		freed->magic = 0;
		
	//	printf("size of freed space is %d\n", freed->size);
		struct Header *tempNode = head;
		while(tempNode->next!=NULL){
			tempNode = tempNode->next;
		}
	//	printf("before adding size of list is %d\n", getSize(head));
	//	printf("before adding head size is %d\n", head->size);
		//after breaking out of while loop we are at end up free list
		tempNode->next = freed;
		freed->next = NULL;
	//	printf("in mem_free size is %d\n", getSize(head));
		return 0;	
	}
	else{
	///	printf("in else statement\n");
		return -1;
	}
}
/**For debugging**/

void Mem_Dump(){
	struct Header *node = head;
	int size = getSize(node);
	//printf("haha size is %d\n",size);
	int totalFreeSpace = head->size;
	//printf("head startAddress %p\n", head->startAddress);
	while(node!=NULL){
		//printf("!!!!!!!!!!!!!!!!!!!!\n");
		printf("address = %p\n", (void *)node);
		printf("size = %d\n", node->size);
		//printf("!!!!!!!!!!!!!!!!!!!!\n");
		node = node->next;
	}
	//printf("size is %d\n", size);
}

int getSize(struct Header *node){
	node = head;
	int size = 0 ;
	while(node!=NULL){
		size++;
		node=node->next;
	}
	return size;
}
