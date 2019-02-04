#include "my_malloc.h"
#include <sys/types.h>
#include <unistd.h>
#
#include <stdio.h>

void * first_block = NULL;//intialize the first block to NULL i.e. no block is allocated
void * first_free = NULL;

/////////////////////////Helper function///////////////////////////////
void print_linked_list(){
  meta_t b = first_block;
  printf("Blocks in my List:\n");
  while(b){
    printf("b: %p b.next: %p b.prev: %p b.free: %d b.size: %ld\n" , b, b->next, b->prev, b->free, b->dsize );
    b = b->next;
  }
}



/*
INPUT
last--keep track of last block to the new block
rqt_size--requested size
OUTPUT
pointer to the new block
 */
meta_t create_block(meta_t last, size_t rqt_size){
  meta_t nb;
  nb = sbrk(0);//get the start of the new block by pointing it at the current break pointer
  if(sbrk(BLOCK_SIZE + rqt_size) == (void *)-1){//increment break pointer
    return NULL;
  }
  //if allocation succeeds
  nb->dsize = rqt_size;
  nb->next = NULL;//intialize the next to NULL
  nb->free = 0;//not free
  nb->ptr = (meta_t) ((char *) nb + BLOCK_SIZE);
  nb->prev = last;//connect two adjacent block
  if(last != NULL){
    last->next = nb;
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
  nb = (meta_t)((char *)cb->ptr + rqt_size);//start of data block + requested size
  nb->dsize = cb->dsize - rqt_size - BLOCK_SIZE;
  nb->free = 1;
  nb->next = cb->next;
  nb->fnext = cb->fnext;
  nb->fprev = cb->fprev;
  if(nb->next){//corner case
    nb->next->prev = nb;
  }
  nb->prev = cb;
  nb->ptr = (char *) nb + BLOCK_SIZE;

  cb->dsize = rqt_size;
  cb->next = nb;
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
meta_t bf_find_block(meta_t *last, size_t rqt_size){
  meta_t cb = first_free;
  meta_t ans = NULL;//result
  size_t mim = 1000000000000;//keep track of the smallest result
  size_t diff;//keep track of smallest difference between requested size and block size
  
  while(cb != NULL){
    diff = cb->dsize - rqt_size;
    if(cb->free && (diff >= 0)){//if a fitting block is found, update
      if(diff < mim){
	mim = diff;
	ans = cb;
      }
    }
    *last = cb;
    cb = cb->fnext;
  }
 
  return ans;
}

void * bf_malloc(size_t size){
  size_t rqt_size = align8(size);//align address for 64-based system
  meta_t cb, last;

  if(first_free != NULL) {//if the list is not empty, search for a fitting block first
    last = first_free;
    cb = bf_find_block(&last, rqt_size);//the only difference from ff
    if(cb != NULL) {//if the fitting block is found
      
      if ((cb->dsize - rqt_size) >= ( BLOCK_SIZE + SPLIT_THR)){//block splitting
	split_block(cb, rqt_size);
      }
     
      cb->free = 0;
    } else {//if not found, create the block
      
      cb = create_block(last, rqt_size);
      if(cb == NULL)//the creation fails returns NULL
	return NULL;
      
    }
  } else { //if the list is empty, create the block directly
    cb = create_block(NULL, rqt_size);
    
    if(!cb)//if the creation fails
      return NULL;
    if(first_block == NULL){
      first_block = cb;
    }
  }
  return cb->ptr;
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
      return ptr == (get_block(ptr))->ptr;
    }
  }
  return 0;
}

/*
coalesce the current block and its next block
and return the pointer to the current block
 */
meta_t coalesce(meta_t cb){
  if (cb->next && cb->next->free) {//if next is not NULL and next block is free
    cb->dsize += (BLOCK_SIZE + cb->next->dsize);
    cb->next = cb->next->next;
    if(cb->next){//set the prev
      cb->next->prev = cb;
    }
  }
  return cb;
}

void locate_free(meta_t target){
  meta_t cb = first_free;
  meta_t last;
  if(cb == NULL){//if there is no free block
    first_free = target;
  }else{//traverse
    while(cb != NULL){
      if(cb > target){//if a fitting block is found, update
	if(cb->fprev){//if cb has last block
	  cb->fprev->fnext = target;
	}
	target->fprev = cb->fprev;
	target->fnext = cb;
	cb->fprev = cb;
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


void bf_free(void *p){
  meta_t b;
  if(valid_addr(p)) {//first check if the address is valid
    b = get_block(p);//if yes, get the start of the meta block
    b->free = 1;//set the block to free
    locate_free(b);
    
    //coalesce prev block
    if(b->prev && b->prev->free){
      b = coalesce(b->prev);
    }
    if(b->next){//if next is NULL
      coalesce(b);//if no, check if coalesce is available
    }else {//yes
      if(b->prev){//corner case: current block is the only block
	b->prev->next = NULL;
      }else{//both prev and next are NULL
	first_block = NULL;
	first_free = NULL;
      }
      if(brk(b) == 0){}
    }
  }
}

/////////////////////////////////////helper function to test fragmentation
//= entire heap memory
unsigned long get_data_segment_size(){
  return sbrk(0) - first_block;//the start of the first_block must be start of the heap
}
//actual usable free space + space occupied by metadata(unusable)
unsigned long get_data_segment_free_space_size(){
  meta_t ptr = first_block;
  unsigned long size = 0;
  while(ptr != NULL){
    if(ptr->free == 1){
      size += (ptr->dsize + BLOCK_SIZE);
    }
    ptr = ptr->next;
  }
  return size;
}
