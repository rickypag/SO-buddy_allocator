#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "buddy_allocator.h"

// these are trivial helpers to support you in case you want
// to do a bitmap implementation
int levelIdx(size_t idx){
  return (int)floor(log2(idx)) + 1;
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
  
  int num_bits = (1 << num_levels) - 1;
  
  //I use the buffer to store both the BitMap struct and the BitMap buffer storing the bits
  //For this reason the first sizeof(BitMap) bytes will be used to store the struct
  alloc->tree = (BitMap*)buffer;
  
  
  char* aux_tree_buffer = (char*)(buffer + sizeof(BitMap) + BitMap_getBytes(num_bits));
  alloc->aux_tree = (BitMap*) aux_tree_buffer;
  
  assert (num_levels<MAX_LEVELS);
  // we need enough memory to handle internal structures
  assert (buffer_size>=BuddyAllocator_calcSize(num_levels));
 
  
  //init the bitmap
  //BitMap_init(alloc->tree,num_bits,(uint8_t*)(buffer + sizeof(BitMap)));
  BitMap_init(alloc->tree,num_bits,buffer + sizeof(BitMap));
  printf("[*] num_bits: %d\n", alloc->tree->num_bits);
  
  BitMap_init(alloc->aux_tree,num_bits,aux_tree_buffer + sizeof(BitMap));
  printf("[*] num_bits: %d\n", alloc->aux_tree->num_bits);
  
  //Only the first level is available
  int i;
  BitMap_setBit(alloc->tree,0,1);
  BitMap_setBit(alloc->aux_tree,0,0);
  for(i = 1; i < num_bits; i++){
	  BitMap_setBit(alloc->tree,i,0);
	  BitMap_setBit(alloc->aux_tree,i,0);
  }
  
};

int BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level){
	//printf("[*] Level: %d\n",level);
	if (level<1)
		return 0;
	assert(level <= alloc->num_levels);
	
	//I get the first index of the level
	int idx = 1 << (level - 1);
	//printf("[*] Idx: %d\n",idx);
	
	//and also the max number of buddies in this level
	int num_buddies = 1 << (level - 1);
	//printf("[*] num_buddies: %d\n",num_buddies);
	
	int i;	
	//First I inspect all the bits of tree at that level
	//I have to pay attention because in the bitmap the first bit has index 0
	for(i = 0; i < num_buddies; i++){
		//If the buddy is free I return its index and set the bit in the bitmap as unavailable
		if(BitMap_bit(alloc->tree, idx + i - 1)){
			BitMap_setBit(alloc->tree, idx + i - 1, 0);
			return idx + i;
		}
	}
	
	//printf("[*] parent_level: %d\n",levelIdx(parentIdx(idx)));
	
	//printf("[*] No buddy at this level, going upward.\n");
	//Otherwise I check if I can get a buddy from the parent level
	int ret_idx = BuddyAllocator_getBuddy(alloc, levelIdx(parentIdx(idx))) * 2;
	
	//If nothing is available I return 0
	if(!ret_idx) return 0;
	
	//Otherwise I set the buddy as available and return the index
	int buddy_idx = buddyIdx(ret_idx);
	BitMap_setBit(alloc->tree, buddy_idx - 1, 1);
	//-OSS.
	// I am sure that the buddy was previously not available, because if it hadn't been I would have found
	// immediately a free spot
	
	return ret_idx;
}


void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size){
	//I determine the level of the page
	int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;
	int  level=floor(log2(mem_size/size));
	//int  level=floor(log2(mem_size/(size)));

	
	// if the level is too small, we pad it to max
	if (level>alloc->num_levels)
		level=alloc->num_levels;

	printf("Requested: %d bytes, level %d \n",
			size, level);
			
	//I look for a free buddy
	int idx = BuddyAllocator_getBuddy(alloc,level);
	
	//If there is no buddy I return 0
	if(!idx) return 0;
	
	//printf("[*] Final index: %d\n", idx);
	
	//I determine the memory address associated with the index
	//In order to do that I simply get the first index of the level and subtract it from the returned index
	int offset = idx - (1 << (level - 1));
	int max_mem = alloc->min_bucket_size*(1 << (alloc->num_levels - 1));
	char* mem = alloc->memory + max_mem / (1 << (level - 1)) * offset;
	
	//And update the aux_tree stating that the node is used by a pointer
	BitMap_setBit(alloc->aux_tree,idx - 1,1);
	printf("[*] idx: %d\n",idx);
	
	return mem;	
}


// releases an allocated buddy, performing the necessary joins
// side effect on the internal structures
void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, int idx){
		
	//First I check if the buddy is free. If so I unite them setting the bit to 0
	//and notify the parent (if I am not at the first level)
	int buddy_idx = buddyIdx(idx);
	if(idx != 1 && BitMap_bit(alloc->tree, buddy_idx - 1)){
		printf("[*] Merging %d and %d\n", idx, buddy_idx);
		BitMap_setBit(alloc->tree, buddy_idx - 1, 0);
		BuddyAllocator_releaseBuddy(alloc, parentIdx(idx));
	}
	//Otherwise I just say that the buddy is free
	else{
		BitMap_setBit(alloc->tree, idx - 1, 1);
	}
}

//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem){
	//I retrieve the index of the buddy
	int max_mem = alloc->min_bucket_size*(1 << (alloc->num_levels - 1));
	printf("[*] max_mem = %d\n", max_mem);
	int idx = (max_mem + (mem - (void*)alloc->memory)) / alloc->min_bucket_size;
	printf("[*] mem = %ld\n", (mem - (void*)alloc->memory));
	printf("[*] idx = %d\n", idx);
	
	while(!BitMap_bit(alloc->aux_tree,idx - 1))	idx /= 2;
	BitMap_setBit(alloc->aux_tree,idx - 1,0);
	
	
	printf("Freeing %p with index %d\n", mem, idx);
	
	//and I inform the bitmap that a new block has been freed
	BuddyAllocator_releaseBuddy(alloc, idx);
}
