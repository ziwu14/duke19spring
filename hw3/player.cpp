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

#define PORT "4444"

using namespace std;

int createListenSocket(const char *myIP, const char * myPort){
  struct sockaddr_in my_address;
  int listen_fd;

  //1. create a lisening socket -- defined by protocol, type
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);//family, type, anything
  if(listen_fd < 0){
    perror("socket() fails");
    exit(EXIT_FAILURE);
  }
  
  //2. bind() to an address -- defined by ip, port, protocol-> my_address
  //otherwise, it will be automatically allocated a port, clients have no idea what it is
  memset(&my_address, 0, sizeof(my_address));//clear my_address
  my_address.sin_family = AF_INET;//address family
  my_address.sin_addr.s_addr = inet_addr(myIP);//ip address
  my_address.sin_port = htons(atoi(myPort));//port

  socklen_t address_size = sizeof(my_address);
  if(bind(listen_fd, (struct sockaddr*)&my_address, address_size) == -1){//return socket address
    perror("bind() fails");
    exit(EXIT_FAILURE);
  }
  //3.listen
  if(listen(listen_fd,7) == -1){
    perror("listen() fails");
    exit(EXIT_FAILURE);
  }
  return listen_fd;
}

int createConnectSocket(const char *myIP,const char *myPort){
  struct sockaddr_in sock_address;
  int fd;

  //1. create a lisening socket -- defined by protocol, type
  fd = socket(AF_INET, SOCK_STREAM, 0);//family, type, anything
  if(fd < 0){
    perror("socket() fails");
    exit(EXIT_FAILURE);
  }
  //2.connect to server via connect function which could return information of server socket address
  memset(&sock_address, 0, sizeof(sock_address));
  sock_address.sin_family = AF_INET;
  sock_address.sin_addr.s_addr = inet_addr(myIP);
  sock_address.sin_port = htons(atoi(myPort));

  if ( connect(fd, (struct sockaddr*)&sock_address, sizeof(sock_address)) == -1  ){
    perror("connect() fails");
    exit(EXIT_FAILURE);
  }
  
  return fd;
}



void obtainMyAddr(int socket_fd, char *ipPtr, char*portPtr){
    
    struct sockaddr_in peerAddr;
    //bzero(&myAddress, sizeof(myAddress));
    socklen_t len = sizeof(peerAddr);
    getsockname(socket_fd,(struct sockaddr *) &peerAddr,&len);
    inet_ntop(AF_INET, &peerAddr.sin_addr, ipPtr, 16*sizeof(ipPtr));
    int temp = ntohs(peerAddr.sin_port);
    sprintf(portPtr,"%d",temp);
}

void obtainPeerAddr(int socket_fd, char *ipPtr, char*portPtr){
    
    struct sockaddr_in peerAddr;
    //bzero(&myAddress, sizeof(myAddress));
    socklen_t len = sizeof(peerAddr);
    getpeername(socket_fd,(struct sockaddr *) &peerAddr,&len);
    inet_ntop(AF_INET, &peerAddr.sin_addr, ipPtr, 16*sizeof(ipPtr));
    int temp = ntohs(peerAddr.sin_port);
    sprintf(portPtr,"%d",temp);
}


void connectToNeighbors(vector<int> &fd_List,struct timeval timeout,int listen_fd,const char *leftIP,const char *leftPort,const char *rightIP,const char *rightPort){

  fd_set reads;
  int fd_max;
  FD_ZERO(&reads);//00000000000000000
  FD_SET(listen_fd, &reads);//00000001000000000
  fd_max = listen_fd;//???????
  
  int client_fd;
  struct sockaddr_in client_address;
  socklen_t address_size;

  int client_count = 0;
  while(client_count != 2){//only 2 neighbors
    
    fd_set temp = reads;
    int status = select(fd_max + 1, &temp, 0, 0, &timeout);//polling all the fds
    if(status == -1){//function fails
      perror("select() fails");
    }
    else if(status == 0){//timeout
      //cout<< "timeout"<<endl;
      continue;
    }

    for(int fd = 0; fd < fd_max + 1; fd++){
      if(FD_ISSET(fd, &temp)){//check which file descriptor changes
	
	if(fd == listen_fd){//connection request: add the client_fd to the fd_set
	  
	  address_size = sizeof(client_fd);
	  client_fd = accept(listen_fd, (struct sockaddr*)&client_address,&address_size);

	  
	  char clientIP[16]="\0";
	  char clientPort[16]="\0";
	  obtainPeerAddr(client_fd,clientIP,clientPort);
	  cout<< "client ip: "<<clientIP<<"client port: "<<clientPort<<endl;

	  int a = strcmp(clientIP,leftIP);
	  int b = strcmp(clientPort,leftPort);
	  int c = strcmp(clientIP,rightIP);
	  int d = strcmp(clientPort,rightPort);
	  if( (a && b) || (c && d)){
	    FD_SET(client_fd, &reads);
	    fd_List.push_back(client_fd);
	 
	    //cout<<"client ip: "<<client_List[client_count].ip<<endl;
	    //cout<<"client port: "<<client_List[client_count].port<<endl;
	  
	    if(fd_max < client_fd){//update fd_max when new socket is involved
	      fd_max = client_fd;
	      cout<<"new client is connecting"<<endl;
	    }

	    client_count++;
	  }
	}

	
      }
      
    }
    
  }
}


int main(int argc, char *argv[])
{
  
  if(argc != 2){
    perror("Usage: ./server [server ip]");
    exit(EXIT_FAILURE);
  }

  //1.create socket -- socket -> local
  int server_fd = createConnectSocket(argv[1], PORT);
 
    
  char myIP[16]={0};
  char myPort[16]={0};
  obtainMyAddr(server_fd, myIP,myPort);
  cout<<"my ip: "<<myIP<<" my port: "<<myPort<<endl;
  
  //2.1.recieve left and right neighbors info
  char left_ip[16]="\0";
  recv(server_fd,left_ip,16,0);
  char left_port[16]="\0";
  recv(server_fd,left_port,16,0);
  char right_ip[16]="\0";
  recv(server_fd,right_ip,16,0);
  char right_port[16]="\0";
  recv(server_fd,right_port,16,0);
  printf("left neighbor: %s:%s\n",left_ip,left_port);
  printf("right neighbor: %s:%s\n",right_ip,right_port);
  /*
  //2.2.create listen socket
  int listen_fd = createListenSocket(myIP, myPort);
  //2.3. connect to neighbor: passive
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 5000;
  vector<int> fdList;//record order of connection, to close them at the end
  connectToNeighbors(fdList,timeout,listen_fd,left_ip,left_port,right_ip,right_port);
  //3.4. connect to neighbor: active
  int left_fd = createConnectSocket(left_ip,left_port);
  int right_fd = createConnectSocket(right_ip,right_port);;
  
  //close all fds
  for(size_t i = 0; i < fdList.size(); i++){
    close(fdList[i]);
  }
  */
  close(server_fd);

  return 0;
}
