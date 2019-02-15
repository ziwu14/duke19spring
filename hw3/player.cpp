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

void obtainMyAddr(int socket_fd, char *ipPtr, char*portPtr){
    
    struct sockaddr_in peerAddr;
    //bzero(&myAddress, sizeof(myAddress));
    socklen_t len = sizeof(peerAddr);
    getsockname(socket_fd,(struct sockaddr *) &peerAddr,&len);
    inet_ntop(AF_INET, &peerAddr.sin_addr, ipPtr, 16*sizeof(ipPtr));
    int temp = ntohs(peerAddr.sin_port);
    sprintf(portPtr,"%d",temp);
}


int main(int argc, char *argv[])
{
  
  if(argc != 2){
    perror("Usage: ./server [server ip]");
    exit(EXIT_FAILURE);
  }

  //1.create socket -- socket -> local
  int server_fd;
  struct sockaddr_in server_address;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd == -1){
    perror("socket() fails");
    exit(EXIT_FAILURE);
  }

  //2.connect to server via connect function which could return information of server socket address
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(argv[1]);
  server_address.sin_port = htons(4444);

  if ( connect(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1  ){
    perror("connect() fails");
    exit(EXIT_FAILURE);
  }
    
    char myIP[16]={0};
    char myPort[16]={0};
    obtainMyAddr(server_fd, myIP,myPort);
    cout<<"my ip: "<<myIP<<" my port: "<<myPort<<endl;
    
    char buffer[512];
    strcpy(buffer,temp.c_str());
    send(server_fd, buffer,strlen(buffer),0);
    
  
  close(server_fd);

  return 0;
}
