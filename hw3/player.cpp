#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[])
{
  
  if(argc != 2){
    perror("Usage: ./server [server ip]");
    exit(EXIT_FAILURE);
  }

  //1.create socket
  int server_fd;
  struct sockaddr_in server_address;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd == -1){
    perror("socket() fails");
    exit(EXIT_FAILURE);
  }
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(argv[1]);
  server_address.sin_port = htons(4444);

  //2.connect
  if ( connect(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1  ){
    perror("connect() fails");
    exit(EXIT_FAILURE);
  }

  while(1){
    string temp;
    cout<<"input sth"<<endl;
    cin >> temp;
    char buffer[512];
    strcpy(buffer,temp.c_str());
    send(server_fd, buffer,strlen(buffer),0);
    if(strcmp(buffer,"q") == 0){
      break;
    }
  }
  
  close(server_fd);

  return 0;
}
