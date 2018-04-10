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
 }Header;

struct Header *head;
int getSize(struct Header *node);

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
	Mem_Free(ptr); //try to free the ptr again this should fail
	if(Mem_Free(ptr)!=-1){
		printf("Error: Mem_Free of same pointer should have failed!\n");
	}
	printf("Allocating 2 chunks of memory.\n");
	char *ptr2 = Mem_Alloc(4);
	strcpy(ptr2, "mhc");
	char *ptr3 = Mem_Alloc(4);
	strcpy(ptr3,"bos");
	Mem_Dump();
	printf("Should see 1 chunk\n");
	printf("Freeing first chunk and allocating a 3rd bigger chunk. \n");
	if(Mem_Free(ptr2) == -1){
		printf("ERROR:Mem_Free failed!\n");
	}
	char *ptr4 = Mem_Alloc(11);
	strcpy(ptr4, "0123456789");
	printf("Should see two chunks\n");
	printf("size of free headers is %d\n", getSize(head));
	Mem_Dump();
 
	printf("Reallocating from first chunk. \n");
	char *ptr5 = Mem_Alloc(4);
	strcpy(ptr5,"csc");
	Mem_Dump();
	//Verify that memory that was set and not freed has not changed
	if(strcmp(ptr3,"bos")){
		printf("ERROR:ptr changed to %s\n", ptr3);
	}
	if(strcmp(ptr4,"0123456789")){
		printf("ERROR: ptr4 changed to %s\n", ptr4);
	}
	if(strcmp(ptr5,"csc")){
		printf("ERROR: ptr5 changed to %s\n", ptr5);
	}
	printf("Allocated a big block.\n");
	void *ptr6 = Mem_Alloc(4000);
	printf("Allocated 4000 bytes at %p\n", ptr6);
	printf("Still one chink of free memory but much smaller");
	Mem_Dump();	
	return 0;
}

	
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
		printf("map failed");
		perror("mmap"); 
		exit(1);
	}
	//initialize head variable
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
	while(node!=NULL){
		/**big enough to fulfill request but small enough so that giving them the entire
		chunk is not wasteful**/
		struct Header *allocatedMemoryHeader;
		int nodeSize = node->size;
		void *nodeStartAddress = node->startAddress;
		struct Header *nodeNext = node->next;
		if(node->size >= sizeRequested && node->size <= sizeRequested + 24){
			printf("in mem_alloc if statement");
			allocatedMemoryHeader = nodeStartAddress;
			allocatedMemoryHeader->size = nodeSize;
			allocatedMemoryHeader->next = NULL;
			allocatedMemoryHeader->startAddress = nodeStartAddress;
			allocatedMemoryHeader->magic = 8;
			/**If the chunk we found was the head of the list allocating it may 	
			cause us to lose reference to head of list so we want to assign head of
			list to what's next in the list**/
			if(nodeStartAddress == head->startAddress){
				head = nodeNext;
			}
			return nodeStartAddress+sizeof(*allocatedMemoryHeader); //where free mem starts


		}
		/**the size of node is much bigger than the sizeRequested and it is worth creating another header for**/
		else if(node->size > sizeRequested+24){ 
			
			allocatedMemoryHeader = nodeStartAddress + sizeof(node)+nodeSize-sizeof(allocatedMemoryHeader)-sizeRequested;
			allocatedMemoryHeader->startAddress = nodeStartAddress + sizeof(node) +nodeSize -sizeof(allocatedMemoryHeader)-sizeRequested;
			allocatedMemoryHeader->size = sizeRequested;
			allocatedMemoryHeader->magic = 8;
			allocatedMemoryHeader->next = NULL;
			node->size = nodeSize-sizeof(*allocatedMemoryHeader) - sizeRequested;//minus size of header object
			node->magic = 0;
			return allocatedMemoryHeader->startAddress + sizeof(*allocatedMemoryHeader);
		}
		else{
			node=node->next;
		}
	}
	return NULL;
}

/**Given a pointer free the memory allocated**/
int Mem_Free(void *ptr){
	struct Header *freed = (struct Header *) (ptr-24);//calculate the address of the header
	if(ptr == NULL){
		return -1; 
	}
	else if(freed->magic == 8) { //if it's actually been allocated
		printf("freed.size is %d\n", freed->magic);
		freed->magic = 0;
		freed->next = head;
		head = freed; //add freed header to the head of the list
		return 0;	
	}
	else{
		return -1;
	}
}
/**For debugging**/

void Mem_Dump(){
	struct Header *node = head;
	printf("\n");
	printf("start of Mem_Dump--------------\n");
	printf("in Mem_Dump size of list is %d\n", getSize(head));
	while(node!=NULL){
		printf("address = %p\n", (void *)node);
		printf("size = %d\n", node->size);
		node = node->next;
	}
	printf("end of Mem_Dump----------------\n");
	printf("\n");
}
/**Gets size of the list of free mem blocks, for debugging**/
int getSize(struct Header *node){
	node = head;
	int size = 0 ;
	while(node!=NULL){
		size++;
		node=node->next;
	}
	return size;
}
/**Prints list of free mem blocks, for debugging**/
void printList(){
	struct Header *node = head;
	while(node!= NULL){
		printf("sizeOfNode %d\n", node->size);
		node = node->next;
	}
}
	
