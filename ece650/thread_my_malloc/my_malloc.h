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
  int free;//1 is free; 0 is not free
  int padding;//2 int  = 8 bytes, in order to make meta multiple of 8(no extra use)
  void *ptr;//point to data
  meta_t fprev;
  meta_t fnext;
};



//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

//in order to test fragmentation
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size();//in bytes

//used to test my own test case, and debug
void print_linked_list();
