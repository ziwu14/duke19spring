#include "my_malloc.h"
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

meta_t first_free = NULL;
void * first_block = NULL;
int empty_heap = 1;
char * heap_end = NULL;

/////////////////////////Helper function///////////////////////////////
void print_linked_list(){
  meta_t b = first_free;
  printf("Blocks in my List:\n");
  while(b != NULL){
    printf("b: %p b.next: %p b.prev: %p b.size: %ld\n" , b, b->fnext, b->fprev, b->dsize );
    b = b->fnext;
  }
}



/*
INPUT
last--keep track of last block to the new block
rqt_size--requested size
OUTPUT
pointer to the new block
 */
meta_t create_block(size_t rqt_size){
  meta_t nb;
  nb = sbrk(0);//get the start of the new block by pointing it at the current break pointer
  if((heap_end = sbrk(BLOCK_SIZE + rqt_size)) == (void *)-1){//increment break pointer
    return NULL;
  }
  heap_end = heap_end + BLOCK_SIZE + rqt_size;
  //if allocation succeeds
  nb->dsize = rqt_size;

  if(empty_heap == 1){
    empty_heap = 0;
    first_block = nb;
  }
  return nb;
}
/*
INPUT
cb--current block which is large enough for the requested size
rqt_size--requested size
DESCRIPTION
split the data block of the current block into a new data block of requested size and a new block with complete meta and data block
 */
void split_block(meta_t cb,size_t rqt_size){

  meta_t nb;
  nb = (meta_t)((char *)cb + BLOCK_SIZE + rqt_size);//start of data block + requested size
  nb->dsize = cb->dsize - rqt_size - BLOCK_SIZE;
  nb->fnext = cb->fnext;
  nb->fprev = cb->fprev;

  if(cb->fprev)
    cb->fprev->fnext = nb;
  if(cb->fnext)
    cb->fnext->fprev = nb;
  
  cb->dsize = rqt_size;

  if(first_free == cb){//corner case
    first_free = nb;
  }
}

/*
DESCRIPTION
used to align the requested size to be multiple of 8, fitting the 64-bit based system 
*/
//run command uname -a, we get this is a 64-bit based system -> 8 bytes
//align the requested size to multiple of 8
size_t align8(size_t size){
  if((size & 0x7) == 0){//multiple of 8 should give least significant 3 bits to be 0
    return size;
  }
  return ((size >> 3) + 1) << 3;
}


/////////////////////////////////BF_malloc///////////////////////////////
meta_t bf_find_block(size_t rqt_size){
  meta_t cb = first_free;
  meta_t ans = NULL;//result
  int mim = 1000000000;//keep track of the smallest result
  int diff;//keep track of smallest difference between requested size and block size
  
  while(cb != NULL){
    diff = cb->dsize - rqt_size;
    if(diff == 0){
      ans = cb;
      break;
    }else if(diff > 0){//if a fitting block is found, update
      if(diff < mim){
	mim = diff;
	ans = cb;
      }
    }
    cb = cb->fnext;
  }
 
  return ans;
}

void * bf_malloc(size_t size){
  size_t rqt_size = align8(size);//align address for 64-based system
  meta_t cb;

  if(first_free != NULL) {//if the list is not empty, search for a fitting block first
    cb = bf_find_block(rqt_size);//the only difference from ff
    if(cb != NULL) {//if the fitting block is found, we should kick it out of the list
      
      if ((cb->dsize - rqt_size) >= ( BLOCK_SIZE + SPLIT_THR)){//block splitting
	split_block(cb, rqt_size);
      }else{
	if(cb->fprev)
	  cb->fprev->fnext = cb->fnext;
	if(cb->fnext)
	  cb->fnext->fprev = cb->fprev;
	if(first_free == cb){//corner case: free only existing block
	  first_free = NULL;
	}
      }
     
    } else {//if not found, create the block
      
      cb = create_block(rqt_size);
      if(cb == NULL)//the creation fails returns NULL
	return NULL;
      
    }
  } else { //if the list is empty, create the block directly
    cb = create_block(rqt_size);
    
    if(cb == NULL)//if the creation fails
      return NULL;
  }
  return (char *)cb + BLOCK_SIZE;
}
///////////////////////////////////free////////////////////////////////
/*
DESCRIPTION
to obtain the start of the meta block by minus fixed bias from the start of the data block
 */
meta_t get_block(void *ptr) {
  char *tmp;
  tmp = ptr;//char type offset i.e. size in bytes
  return (ptr = tmp -= BLOCK_SIZE);//start of the block; reverse to creation of a block
}

int valid_addr(void *ptr) {
  if(first_block != NULL) {
    if(ptr >= first_block && ptr < sbrk(0)) {//the requested address is located between first block and end of the heap
      return ptr == ((char*)(get_block(ptr)) + BLOCK_SIZE);
    }
  }
  return 0;
}

void locate_free(meta_t target){
  meta_t cb = first_free;
  meta_t last;
  if(cb == NULL){//if there is no free block
    first_free = target;
    target->fprev = NULL;
    target->fnext = NULL;
  }else{//traverse
    while(cb != NULL){
      if(cb > target){//if a fitting block is found, update
	if(cb->fprev){//if cb has last block
	  cb->fprev->fnext = target;
	}
	target->fprev = cb->fprev;
	target->fnext = cb;
	cb->fprev = target;
	break;
      }
      last = cb;
      cb = cb->fnext;
    }
    if(cb == NULL){
      last->fnext = target;
      target->fprev = last;
      target->fnext = NULL;
    }
  }
}


/*
coalesce the current block and its next block
and return the pointer to the current block
 */
meta_t coalesce(meta_t cb){
  cb->dsize += (BLOCK_SIZE + cb->fnext->dsize);
  cb->fnext = cb->fnext->fnext;
  if(cb->fnext){//set the prev
    cb->fnext->fprev = cb;
  }
  
  return cb;
}



void bf_free(void *p){
  meta_t b;
  if(valid_addr(p)) {//first check if the address is valid

    //insert the block in the list
    b = get_block(p);//if yes, get the start of the meta block
    locate_free(b);
    //coalesce prev block
    if(b->fprev && ((char *)b->fprev + BLOCK_SIZE + b->fprev->dsize == (char *)b )){
      if(first_free == b){
	first_free = b->fprev;
      }
      b = coalesce(b->fprev);
    }
    //coalesce next block
    if(b->fnext && ((char *)b + BLOCK_SIZE + b->dsize == (char *)b->fnext )){//if next is NULL
      if(first_free == b->fnext){
	first_free = b;
      }
      coalesce(b);//if no, check if coalesce is available
    }
   
  }
}

/////////////////////////////////////helper function to test fragmentation
//= entire heap memory
unsigned long get_data_segment_size(){
  return heap_end - (char *)first_block;//the start of the first_block must be start of the heap
}
//actual usable free space + space occupied by metadata(unusable)
unsigned long get_data_segment_free_space_size(){
  meta_t ptr = first_free;
  unsigned long size = 0;
  while(ptr != NULL){
    size += (ptr->dsize + BLOCK_SIZE);
    ptr = ptr->fnext;
  }
  return size;
}
