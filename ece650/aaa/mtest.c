#include <stdio.h>
#include <stdlib.h>
#include "my_malloc.h"
#include <sys/types.h>
#include <unistd.h>

/*
#define FREE ff_free
#define MALLOC ff_malloc
*/

int main(){
  
  char *ptr[20];


 for(int i = 0; i < 10; i++){

   ptr[i] = (char *) ff_malloc(16*sizeof(char));
  }
 
 //print_linked_list();
  ff_free(ptr[1]);
  //print_linked_list();
  ff_free(ptr[3]);
  //print_linked_list();
  ff_free(ptr[2]);
  //print_linked_list();
  ff_free(ptr[9]);
  //print_linked_list();
  ff_free(ptr[8]);
  //print_linked_list();
  ff_free(ptr[6]);
  //print_linked_list();
  //1.
  //print_linked_list();
  
  ptr[10] = (char *) ff_malloc(48*sizeof(char));
  print_linked_list();
  ptr[11] = (char *) ff_malloc(20*sizeof(char));
  print_linked_list();
  
  //2.
  print_linked_list();
  ff_free(ptr[4]);
  ff_free(ptr[5]);
  //3.
  print_linked_list();
  ptr[1] = (char *) ff_malloc(200*sizeof(char));
  print_linked_list();
  ptr[2] = (char *) ff_malloc(168*sizeof(char));
  print_linked_list();
  unsigned long a = get_data_segment_size();
  unsigned long b = get_data_segment_free_space_size();
  printf("result: %lf",(double) b/a);
 
  return 0;
}
