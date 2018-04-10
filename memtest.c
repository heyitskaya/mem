#include "mem.h"
#include <stdio.h>
#include <string.h>

int main (int argc, char **argv) {
  // Get a big chunk of memory from the OS for the memory
  // allocator to manage.
  void *memory = Mem_Init (4096);
  printf ("Mem_Init returned %p\n\n", memory);

  // We should see one big chunk.
  Mem_Dump();

  // Allocate 4 bytes of memory and store "abc" in that address
  void *ptr = Mem_Alloc (4);
  printf ("Allocated 4 bytes at %p\n", ptr);
  strcpy(ptr, "abc");

  // Still one chunk of free memory but it is smaller
  Mem_Dump();

  // Free the block just allocated.
  printf ("Freeing the allocated chunk.\n");
  if (Mem_Free(ptr) == -1) {
    printf ("ERROR:  Mem_Free failed!\n");
  }


  // Should have 2 chunks of free memory:  one at the block from
  // the previous list and one at the start of the memory.
  Mem_Dump();

  // Try to free the pointer again.  This should fail.
  Mem_Free(ptr);
  if (Mem_Free(ptr) != -1) {
    printf ("ERROR:  Mem_Free of same pointer should have failed!\n");
  }

  // Allocate 2 chunks of memory
  printf ("Allocating 2 chunks of memory.\n");
  char *ptr2 = Mem_Alloc(4);
  strcpy (ptr2, "mhc");
  char *ptr3 = Mem_Alloc(4);
  strcpy (ptr3, "bos");

  // Should see 1 chunk
  Mem_Dump();

  // Freeing the first chunk and asking for memory that should come from the
  // second free chunk.
  printf ("Freeing first chunk and allocating a 3rd bigger chunk.\n");
  if (Mem_Free(ptr2) == -1) {
    printf ("ERROR:  Mem_Free failed!\n");
  }
  char *ptr4 = Mem_Alloc(11);
  strcpy (ptr4, "0123456789");

  // Should see 2 chunks
  Mem_Dump();

  // Allocate a chunk that should fit in the first free block
  printf ("Reallocating from first chunk.\n");
  char *ptr5 = Mem_Alloc(4);
  strcpy (ptr5, "csc");

  // Should see 1 chunk
  Mem_Dump();

  // Verify that memory that was set and not freed has not changed.
  if (strcmp (ptr3, "bos")) {
    printf ("ERROR: ptr3 changed to %s\n", ptr3);
  }
  
  if (strcmp (ptr4, "0123456789")) {
    printf ("ERROR: ptr4 changed to %s\n", ptr4);
  }
  
  if (strcmp (ptr5, "csc")) {
    printf ("ERROR: ptr5 changed to %s\n", ptr5);
  }
  
  // Allocate 4000 bytes
  printf ("Allocate a big blocks\n");
  void *ptr6 = Mem_Alloc (4000);
  printf ("Allocated 4000 bytes at %p\n", ptr6);

  // Still one chunk of free memory but much smaller
  Mem_Dump();

  // This allocation should fail
  void *ptr7 = Mem_Alloc (1000);
  printf ("Tried to allocate 1000 bytes.  Mem_Alloc returned %p\n", ptr7);
  Mem_Dump();

  printf ("Freeing a random address; it should fail\n");
  if (Mem_Free((void *) ptr3 + 4) != -1) {
    printf ("ERROR:  Mem_Free should have failed!\n");
  }
  
  Mem_Dump();
}

