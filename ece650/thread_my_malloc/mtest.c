#include <stdio.h>
#include <stdlib.h>
#include "my_malloc.h"
#include <sys/types.h>
#include <unistd.h>


int main(){
  
  char *ptr[5];
  
  //printf("debug\n");
  ptr[0] = (char *) bf_malloc(80*sizeof(char));
  //printf("size of int: %ld\n", sizeof(int));
  //*ptr = 10;
  //*(ptr+1) = 11;
  //printf("%p,\nvalue at ptr[0]: %d\nvalue at ptr[1]: %d\n" , ptr, *ptr, *(ptr+1));
  //printf("ptr has address %p, and meta_data address %p\n",ptr[0],((char *)ptr[0]-BLOCK_SIZE));

  //printf("debug2\n");
  //char *ptr2 = (char *) ff_malloc(1000*sizeof(char));
  ptr[1] = (char *) bf_malloc(40*sizeof(char));
  ptr[2] = (char *) bf_malloc(24*sizeof(char));
  ptr[3] = (char *) bf_malloc(120*sizeof(char));
  print_linked_list();
  bf_free(ptr[1]);
  bf_free(ptr[0]);
  print_linked_list();
  ptr[4] = (char *) bf_malloc(104*sizeof(char));
  for(int i = 2; i < 5; i++){
    bf_free(ptr[i]);
    //printf("i=%d has no problem\n",i);
    print_linked_list();
  }
  return 0;
}
