#include<stdlib.h>

#define BLOCK_SIZE 40 
//manually define BLOCK_SIZE since there is 
//8+8+8+4+4+8=40

void printList();

typedef struct _meta_t * meta_t;
struct _meta_t{
  size_t dsize;//size of data block
  meta_t prev;
  meta_t next;
  int free;//1 is free; 0 is not free
  int padding;//2 int  = 8 bytes, in order to make meta multiple of 8(no extra use)
  void *ptr;//point to data
};


//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);

//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

//in order to test fragmentation
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size();//in bytes

