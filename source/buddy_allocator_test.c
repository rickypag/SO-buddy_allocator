#include "buddy_allocator.h"
#include <stdio.h>

#define BUFFER_SIZE 256
#define BUDDY_LEVELS 3
#define MEMORY_SIZE 256
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
  
  void* p1 = BuddyAllocator_malloc(&alloc, 60);
  bitmap_test(alloc.tree);
  void* p2 = BuddyAllocator_malloc(&alloc, 240);
  bitmap_test(alloc.tree);
  //void* p3 = BuddyAllocator_malloc(&alloc, 60);
  //void* p4 = BuddyAllocator_malloc(&alloc, 60);
  
  //BuddyAllocator_free(&alloc,p2);
  bitmap_test(alloc.tree);
  BuddyAllocator_free(&alloc,p1);
  bitmap_test(alloc.tree);
  
}
