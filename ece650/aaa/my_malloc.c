#include "my_malloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
meta head = NULL;
meta last = NULL;
void* first_address = NULL;
char * tail = NULL;

size_t align(size_t size){
    return (((size - 1) >> 3 << 3) + 8);
}

void print_linked_list(){
  meta cb = head;
  printf("BLOCK LIST\n:");
  while(cb != NULL){
    printf("block address: %p, block size: %lu, prev: %p, next: %p, free: %d\n ",cb,cb->size,cb->prev,cb->next,cb->free);
    cb = cb->next;
  }
}

unsigned long get_data_segment_size(){
  return 0;
}
unsigned long get_data_segment_free_space_size(){
  return 0;
}

void *ff_malloc(size_t size) {  
    if (!first_address) {
	first_address = sbrk(0);
    }
    size_t asize = align(size);
    size_t total = asize + BLOCK_SIZE;
    if (head == NULL) {
      // Total size
      meta block = sbrk(0);
      if (sbrk(total) == (void*) -1) {
	return NULL;
      }	    
      block->size = total;
      block->next = NULL;
      block->prev = NULL;
      block->free = 0;
      head = block;
      last = block;
      //printf("The address is: %p\n", ((char*)block + BLOCK_SIZE));
      return (char*)block + BLOCK_SIZE;
    } else {
	meta curr = (meta)head;
	// First fit. Break immediately when found a block.
	while (curr) {
	  // printf("A\n");
	    if (curr->free && curr->size >= total){
		break;
	    }
	    curr = curr->next;
	}
	// Found a block.
	if (curr != NULL) {
	    // Data segment at least 8 bytes.
	    if(curr->size - total >= BLOCK_SIZE + 8) {
		split(curr, total);
	    }
	    //printf("The address is: %p\n", ((char*)curr + BLOCK_SIZE));
	    return (char*)curr + BLOCK_SIZE;
	} else {
	    meta block;
	    // Last block may be busy or free. We still sbrk(total) size.
	    block = sbrk(0);
	    if(sbrk(total) == (void *) -1){
	      return NULL;
	    }
	    block->size = total;
	    block->next = NULL;
	    block->prev = last;
	    block->free = 0;
	    last->next = block;
	    // New last block.
	    last = block;
	    //printf("The address is: %p\n", ((char*)block + BLOCK_SIZE));
	    return (char*)block + BLOCK_SIZE;
	}
    }
}

void split(meta curr, size_t size){
    meta next;
    next = (meta)((char*)curr + size);
    next->size = curr->size - size - BLOCK_SIZE;
    next->prev = curr;
    next->next = curr->next;
    if(next->next){
      next->next->prev = next;
    }
    curr->size = size;
    curr->next = next;
    next->free = 1;
    curr->free = 0;
    
}

meta combine(meta block){//prior block
  block->size = block->size + block->next->size;
  block->next = block->next->next;
  if(block->next){
    block->next->prev = block;
  }
  return block;
}
void myfree(void *ptr){
  //printf("Enter free\n");
  if (head == NULL) {
    return;
  }
    // Check if the ptr is in range.
  if (ptr > sbrk(0) || ptr < first_address) {
    return;
  }

  char * temp = ((char*) ptr) - BLOCK_SIZE;
  meta block = (meta) temp;
  if(block->free != 1){
    return;
  }
  block->free = 1;
  if (block->prev){
    if( block->prev->free) {
      block = combine(block->prev);
    }
  }
  if( block->next){
    if( block->next->free){
      combine(block);
    }
  }
}

void ff_free(void *ptr){
    myfree(ptr);
}
