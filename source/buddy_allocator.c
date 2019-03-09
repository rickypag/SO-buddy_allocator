#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "buddy_allocator.h"

// these are trivial helpers to support you in case you want
// to do a bitmap implementation
int levelIdx(size_t idx){
  return (int)floor(log2(idx));
};

int buddyIdx(int idx){
  if (idx&0x1){
    return idx-1;
  }
  return idx+1;
}

int parentIdx(int idx){
  return idx/2;
}

int startIdx(int idx){
  return (idx-(1<<levelIdx(idx)));
}


// computes the size in bytes for the allocator
// I use methods offered by the bitmap
int BuddyAllocator_calcSize(int num_levels) {
  int num_bits = 1<<(num_levels) - 1; //Bit used in the bitmap
  return BitMap_getBytes(num_bits) + sizeof(BitMap);   //Bytes used in the bitmap
}

void BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         char* buffer,
                         int buffer_size,
                         char* memory,
                         int min_bucket_size){

  //num_levels includes also the zero level (e.g. if num_levels is 1 there is just the zero level)
  alloc->num_levels=num_levels;
  alloc->memory=memory;
  alloc->min_bucket_size=min_bucket_size;
  
  //I use the buffer to store both the BitMap struct and the BitMap buffer storing the bits
  //For this reason the first sizeof(BitMap) bytes will be used to store the struct
  alloc->tree = (BitMap*)buffer;
  assert (num_levels<MAX_LEVELS);
  // we need enough memory to handle internal structures
  assert (buffer_size>=BuddyAllocator_calcSize(num_levels));
  
  int num_bits = 1<<(num_levels) - 1; 
  //init the bitmap
  BitMap_init(alloc->tree,num_bits,(uint8_t*)(buffer + sizeof(BitMap))); 
  
};

void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size){
	//TO DO
	
	return NULL;	
}
