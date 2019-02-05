#include<stdlib.h>

//manually define BLOCK_SIZE since there is 
//8(size_t)+8(meta_t)+8(meta_t)+4(int)+4(int)+8(void *)=40
#define BLOCK_SIZE sizeof(struct _meta_t) 
//manually defined threshold for block splitting
#define SPLIT_THR 8

//8+4+4+8+8+8=40
typedef struct _meta_t * meta_t;
struct _meta_t{
  size_t dsize;//size of data block
  meta_t fprev;
  meta_t fnext;
  void * list_head;
  void * freelist_head;
};



//locking version
void * bf_malloc(size_t size);
void bf_free(void *ptr);
//non-locking version
void * ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);


//used to test my own test case, and debug
void print_linked_list();


unsigned long get_data_segment_size();

//actual usable free space + space occupied by metadata(unusable)
unsigned long get_data_segment_free_space_size();
