#include "buddy_allocator.h"
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 5120
#define BUDDY_LEVELS 9
#define MEMORY_SIZE 1024*128
#define MIN_BUCKET_SIZE (MEMORY_SIZE>>(BUDDY_LEVELS - 1))

char buffer[BUFFER_SIZE]; // 100 Kb buffer to handle memory should be enough
char memory[MEMORY_SIZE];

BuddyAllocator alloc;

static void bitmap_test(BitMap* tree){
	int i;
	printf("[*] bitmap: (");
	for(i = 0; i < tree->num_bits; i++)
		printf(" %d,", BitMap_bit(tree, i));
	printf(")\n");
}

int main(int argc, char** argv) {
  printf("[*] BUFFER_SIZE: %d\n", BUFFER_SIZE);
  printf("[*] BUDDY_LEVELS: %d\n", BUDDY_LEVELS);
  printf("[*] MIN_BUCKET_SIZE: %d\n", MIN_BUCKET_SIZE);

  //1 we see if we have enough memory for the buffers
  int req_size=BuddyAllocator_calcSize(BUDDY_LEVELS);
  printf("size requested for initialization: %d/BUFFER_SIZE\n", req_size);

  //2 we initialize the allocator
  printf("init... ");
  BuddyAllocator_init(&alloc, BUDDY_LEVELS,
                      buffer,
                      BUFFER_SIZE,
                      memory,
                      MIN_BUCKET_SIZE);
  printf("DONE\n");
  
  bitmap_test(alloc.tree);
  bitmap_test(alloc.aux_tree);
  
  void* p1 = BuddyAllocator_malloc(&alloc, 1024*128 );
  printf("Indirizzo %p\n",p1);
  if(!p1){
	  printf("Memory not available\n");
	  exit(1);
  }
  BuddyAllocator_free(&alloc,p1);
  
  
  void* p2 = BuddyAllocator_malloc(&alloc, 32*256);
  printf("Indirizzo %p\n",p2);
  if(!p2){
	  printf("Memory not available\n");
	  exit(1);
  }

  BuddyAllocator_free(&alloc,p2);


  void* p3 = BuddyAllocator_malloc(&alloc, 64*64);
  printf("Indirizzo %p\n",p3);
  if(!p3){
	  printf("Memory not available\n");
	  exit(1);
  }
  BuddyAllocator_free(&alloc,p3);
  
  void* p4 = BuddyAllocator_malloc(&alloc, 28);
  if(!p4){
	  printf("Memory not available\n");
	  exit(1);
  }
  BuddyAllocator_free(&alloc,p3);
  
  bitmap_test(alloc.tree);
  bitmap_test(alloc.aux_tree);
  
}
