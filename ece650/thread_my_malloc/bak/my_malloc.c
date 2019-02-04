#include "my_malloc.h"
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

void * first_block = NULL;//intialize the first block to NULL i.e. no block is allocated

/////////////////////////Helper function///////////////////////////////
void printList(){
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
  nb->prev = last;
  nb->ptr = (meta_t) ((char *) nb + BLOCK_SIZE);
  if(last != NULL){//connect two adjacent block
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
  nb = (meta_t)((char *)cb->ptr + rqt_size);
  nb->dsize = cb->dsize - rqt_size - BLOCK_SIZE;
  nb->free = 1;
  nb->next = cb->next;
  nb->next->prev = nb;
  nb->prev = cb;
  nb->ptr = (char *) nb + BLOCK_SIZE;
  
  cb->dsize = rqt_size;
  cb->next = nb;
}
//run command uname -a, we get this is a 64-bit based system -> 8 bytes
//align the requested size to multiple of 8
size_t align8(size_t size){
  if((size & 0x7) == 0){//multiple of 8 should give least significant 3 bits to be 0
    return size;
  }
  return ((size >> 3) + 1) << 3;
}

////////////////////First Fit malloc function///////////////////////////
/*
INPUT 
last--last block
rqt_size--requested size
OUTPUT 
block pointer--if fail to find a block, return NULL
 */
meta_t ff_find_block(meta_t *last, size_t rqt_size){
  //last is used to keep track of the last block to current block; it will be used when we  can't find a block that has sufficient size
  meta_t cb = first_block;//cb is current block

  while(cb != NULL){
    if(cb->free && (cb->dsize >= rqt_size) ){
      break;
    }
    *last = cb;
    cb = cb->next;
  }
  return cb;//NB: every block's next is initialized with NULL if block is not found, it will return NULL 
}

void *ff_malloc(size_t size){
  size_t rqt_size = align8(size);//align address for 64-based system
  meta_t cb, last;

  if(first_block != NULL) {//if some allocation are done previously
    
    last = first_block;
    cb = ff_find_block(&last, rqt_size);//try to find the fitting block
    if(cb != NULL) {//if found

      /*
	if the data block >= requested size + meta block size + 8
	where 8 means at least 8 bytes data block for the new block
      */
      if ((cb->dsize - rqt_size) >= ( BLOCK_SIZE + 8)){
	split_block(cb, rqt_size);
      }
      
      cb->free = 0;//set the fitting block to not free
      
    } else {//if not found
      
      cb = create_block(last, rqt_size);
      if(cb == NULL){//if the creation fails
	return NULL;
      }
    }
  } else {//if no allocation has been done 
    cb = create_block(NULL, rqt_size);
    if(!cb){
      return NULL;
    }
    first_block = cb;//set the first_block
  }
  return cb->ptr;//return the start of data block
}
/////////////////////////////////BF_malloc///////////////////////////////
meta_t bf_find_block(meta_t *last, size_t rqt_size){
  meta_t cb = first_block;
  meta_t ans = NULL;//result
  size_t mim = rqt_size;//keep track of the smallest result
  size_t diff;
  
  while(cb != NULL){
    diff = cb->dsize - rqt_size;
    if(cb->free && (diff >= 0)){//if a fitting block is found
      if(diff < mim){
	mim = diff;
	ans = cb;
      }
    }
    *last = cb;
    cb = cb->next;
  }

  return ans;
}

void * bf_malloc(size_t size){
  size_t rqt_size = align8(size);//align address for 64-based system
  meta_t cb, last;

  if(first_block != NULL) {
    last = first_block;
    cb = bf_find_block(&last, rqt_size);//the only difference from ff
    if(cb != NULL) {
      if ((cb->dsize - rqt_size) >= ( BLOCK_SIZE + 8)){
	split_block(cb, rqt_size);
      }
      cb->free = 0;
    } else {
      cb = create_block(last, rqt_size);
      if(cb == NULL){
	return NULL;
      }
    }
  } else { 
    cb = create_block(NULL, rqt_size);
    if(!cb)
      return NULL;
    first_block = cb;
  }
  return cb->ptr;
}
///////////////////////////////////free////////////////////////////////
meta_t get_block(void *ptr) {
  char *tmp;//to tell data type of p
  tmp = ptr;//char type offset
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

void ff_free(void *p){
  meta_t b;
  if(valid_addr(p)) {//first check if the address is valid
    b = get_block(p);//if yes, get the start of the meta block
    b->free = 1;//set the block to free

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
      }
      if(brk(b) == 0){}
    }
  }
}

void bf_free(void *ptr){
  ff_free(ptr);
}
/////////////////////////////////////helper function to test fragmentation
unsigned long get_data_segment_size(){
  return sbrk(0) - first_block;//the start of the first_block must be start of the heap
}
unsigned long get_data_segment_free_space_size(){
  meta_t ptr = first_block;
  unsigned long size = 0;
  while(ptr != NULL){
    if(ptr->free == 1){
      size += ptr->dsize;
    }
    ptr = ptr->next;
  }
  return size;
}
