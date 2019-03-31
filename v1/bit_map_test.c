#include <stdio.h>
#include <stdlib.h>
#include "bit_map.h"

int main(){
	uint8_t* buffer = malloc(sizeof(uint8_t));
	BitMap b;// = malloc(sizeof(BitMap));
	
	BitMap_init(&b,8,buffer);
	
	BitMap_setBit(&b,0,0);
	BitMap_setBit(&b,1,1);
	BitMap_setBit(&b,2,1);
	BitMap_setBit(&b,3,0);
	BitMap_setBit(&b,4,0);
	BitMap_setBit(&b,5,1);
	BitMap_setBit(&b,6,1);
	BitMap_setBit(&b,7,1);
	
	int i;
	for(i=0;i<8;i++){
		printf("%d\n",BitMap_bit(&b,i));
	}
}
