#ifndef MY_MALLOC_H
#define MY_MALLOC_H
#define BLOCK_SIZE sizeof(struct list_malloc) //size of our meta struct
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct list_malloc *meta;

struct list_malloc{
    size_t size;
    meta prev;
    meta next;
    int free;
  int padding;
};

size_t align(size_t size);

void print_linked_list();

void *ff_malloc(size_t size);

void myfree(void *ptr);

void ff_free(void *ptr);

void split(meta cur, size_t size);

unsigned long get_data_segment_size();

unsigned long get_data_segment_free_space_size();

#endif
