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
struct Header *head;

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
	printf("-------------------\n");
	printf("Allocating 2 chunks of memory.\n");
	char *ptr2 = Mem_Alloc(4);
	strcpy(ptr2, "mhc");
	char *ptr3 = Mem_Alloc(4);
	strcpy(ptr3,"bos");
	Mem_Dump();
	printf("Should see 1 chunk\n");
	printf("--------------------\n");
	printf("Freeing first chunk and allocating a 3rd bigger chunk. \n");
	if(Mem_Free(ptr2) == -1){
		printf("ERROR:Mem_Free failed!\n");
	}
	char *ptr4 = Mem_Alloc(11);
	strcpy(ptr4, "0123456789");
	printf("Should see two chunks\n");
	printf("size of free headers is %d\n", getSize(head));
	Mem_Dump();
	printf("--------------------\n");
 
	return 0;
}

//struct Header *head;
	
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
	printList();
	int isHead = 0;

	while(node!=NULL){
		printf("KAYA \n");
		if(node->size > sizeRequested+32){ 
			/**if(node->startAddress == head->startAddress){ //if the node we found is the head 
				isHead ==1;	
			}**/
			numNode ++;
			printf("node size %d\n", node->size);

			printf("numNode %d\n", numNode);
//			numNode++;	
//			printf("node size %d\n", node->size);
			printf("Before: in Mem_Alloc size of list is %d\n", getSize(head));
			//printf("Before nodeStartAddress %p\n", node->startAddress );	
			int nodeSize = node->size;
			void* nodeStartAddress = node->startAddress;
			struct Header *nodeNext = node->next;
			printf("nodeStartAddress %p\n",nodeStartAddress);
			printf("headStartAddress %p\n", head->startAddress);
			/**create the header for the allocated memory and init instance fields**/
			struct Header *allocatedMemoryHeader = nodeStartAddress;
			allocatedMemoryHeader->startAddress = nodeStartAddress;
			allocatedMemoryHeader->size = sizeRequested;
			allocatedMemoryHeader->magic = 8;
			allocatedMemoryHeader->next = NULL;
			/**
			printf("allocactedMemoryHeader->startAddress %p\n", allocatedMemoryHeader->startAddress);
			printf("allocatedMemoryHeader->size %d\n", allocatedMemoryHeader->size);
			printf("allocatedMemoryHeader->magic %d\n", allocatedMemoryHeader->magic);
			printf("allcoatedMemoryHeader object size is %d\n", sizeof(*allocatedMemoryHeader));
			**
			/**change instance fields of node**/
			node = nodeStartAddress + sizeof(*allocatedMemoryHeader) + sizeRequested;
			node->size = nodeSize-sizeof(*allocatedMemoryHeader) - sizeRequested;//minus size of header object
			node->magic = 0;
			node->startAddress = nodeStartAddress + sizeof(*allocatedMemoryHeader) + sizeRequested;
			node->next = nodeNext;
		
			printf("AAAAAAAAAAA");
			printf("nodeStartAddress: %p\n", nodeStartAddress);
			printf("head->startAddress: %p\n", head->startAddress);
		/**	if(nodeStartAddress == head->startAddress){
				head = node;
			} //otherwise just leave head where it is **/
			printf("Before if statement size of list is %d\n", getSize(head));
			if(numNode == 1 ){
				head = node;
			}
			printf("After: in Mem_Alloc size of list is %d\n",getSize(head));
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
	if(ptr == NULL){
		return -1; 
	}
	else if(freed->magic == 8) { //if it's actually been allocated
		printf("before adding to list size of list is %d\n", getSize(head));
		
		freed->magic = 0;
		freed->next = head;
		head = freed; //add freed header to the head of the list
		printf("after adding to list size of list is %d\n", getSize(head));
		/**	
		struct Header *tempNode = head;
		
	
		while(tempNode->next!=NULL){
			tempNode = tempNode->next;
		}
		tempNode->next = freed;
		freed->next = NULL;**/
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
	printf("in Mem_Dump size of list is %d\n", getSize(head));
	//printf("haha size is %d\n",size);
	int totalFreeSpace = head->size;
	//printf("head startAddress %p\n", head->startAddress);
	while(node!=NULL){
		printf("address = %p\n", (void *)node);
		printf("size = %d\n", node->size);
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
void printList(){
	struct Header *node = head;
	while(node!= NULL){
		printf("sizeOfNode %d\n", node->size);
		node = node->next;
	}
}
	
