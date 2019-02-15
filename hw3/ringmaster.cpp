#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
  int server_fd;
  int client_fd;
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;
  socklen_t address_size;


  //1. socket setup
  server_fd = socket(AF_INET, SOCK_STREAM, 0);//family, type, anything
  memset(&server_address, 0, sizeof(server_address));//clear server_address
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);//ip address
  server_address.sin_port = htons(4444);//port

  //2. bind() to an address
  address_size = sizeof(server_address);
  if(bind(server_fd, (struct sockaddr*)&server_address, address_size) == -1){//return socket address
    perror("bind() fails");
    exit(EXIT_FAILURE);
  }
  //3.listen
  if(listen(server_fd,10) == -1){
    perror("listen() fails");
    exit(EXIT_FAILURE);
  }

  //4.select() & accept()
  struct timeval timeout;
  fd_set reads, temp;//select() function will only leave fd to change so we use a temp to pass into select
  int fd_max;
  vector<int> client_fdList;
  
  FD_ZERO(&reads);//00000000000000000
  FD_SET(server_fd, &reads);//00000001000000000
  fd_max = server_fd;//???????
  timeout.tv_sec = 5;
  timeout.tv_usec = 5000;

  int game_over = 0;
  while(game_over != 1){
    
    temp = reads;
    int status= select(fd_max + 1, &temp, 0, 0, &timeout);
    if(status == -1){
      perror("select() fails");
    }
    else if(status == 0){
      //cout<< "timeout"<<endl;
      continue;
    }

    for(int fd = 0; fd < fd_max + 1; fd++){
      if(FD_ISSET(fd, &temp)){//check which file descriptor changes
	
	if(fd == server_fd){//connection request
	  
	  address_size = sizeof(client_fd);
	  client_fd = accept(server_fd, (struct sockaddr*)&client_address,&address_size);

	  FD_SET(client_fd, &reads);
	  client_fdList.push_back(client_fd);
	  if(fd_max < client_fd){//update fd_max when new socket is involved
	    fd_max = client_fd;
	    cout<<"new client is connecting"<<endl;
	  }
	  
	} else{//receive message from client_fd i.e. client_fd is ready to be read

	  cout<<"changed fd is "<<fd<<endl;
	  char buffer[512]={0};
	  recv(fd,buffer,sizeof(buffer),0);
	  cout<<fd<<endl;
	  if(strcmp(buffer,"q") == 0){
	    game_over = 1;
	    break;
	  }
	  
	}

      }
    }
    
  }
  

  for(size_t i = 0; i < client_fdList.size(); i++){
    close(client_fdList[i]);
  }
  close(server_fd);
  return 0;
}
