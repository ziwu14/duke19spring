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

void obtainMyAddr(int socket_fd, char *myIP, char*myPort){

    
  char host[100]={0};
  size_t name_sz = sizeof(host);
  gethostname(host,name_sz);
  struct hostent *hp;
  hp = gethostbyname(host);
  char ip[16];
  struct sockaddr_in sock_addr;
  sock_addr.sin_addr = *((struct in_addr*) hp->h_addr_list[0]);
  inet_ntop(AF_INET, &sock_addr.sin_addr, ip, sizeof(ip));
  strcpy(myIP,ip);
  
  struct sockaddr_in myAddr;
  //bzero(&myAddress, sizeof(myAddress));
  socklen_t len = sizeof(myAddr);
  getsockname(socket_fd,(struct sockaddr *) &myAddr,&len);
  int temp = ntohs(myAddr.sin_port);
  sprintf(myPort,"%d",temp);
}

void obtainPeerAddr(int socket_fd, char *ipPtr, char*portPtr){
    
    struct sockaddr_in peerAddr;
    //bzero(&myAddress, sizeof(myAddress));
    socklen_t len = sizeof(peerAddr);
    getpeername(socket_fd,(struct sockaddr *) &peerAddr,&len);
    inet_ntop(AF_INET, &peerAddr.sin_addr, ipPtr, 16*sizeof(ipPtr));
    int temp = ntohs(peerAddr.sin_port);
    sprintf(portPtr,"%d",temp);

    cout<<"accept address "<<ipPtr<<":"<<portPtr<<endl;
}


int createListenSocket(){
  int listen_fd;

  //1. create a lisening socket -- defined by protocol, type
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);//family, type, anything
  if(listen_fd < 0){
    perror("socket() fails");
    exit(EXIT_FAILURE);
  }
  
    
  //2.listen
  if(listen(listen_fd,7) == -1){
    perror("listen() fails");
    exit(EXIT_FAILURE);
  }

  char myIP[16]={0};
  char myPort[16]={0};
  obtainMyAddr(listen_fd, myIP,myPort);
  cout<<"listen_fd: ip: "<<myIP<<" port: "<<myPort<<endl;

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

  cout<<"try to connect "<<myIP<<":"<<myPort<<endl;
  if ( connect(fd, (struct sockaddr*)&sock_address, sizeof(sock_address)) == -1  ){
    perror("connect() fails");
    exit(EXIT_FAILURE);
  }
  cout<<"connect successful"<<endl;
  return fd;
}


void acceptNeighbors(vector<int> &fd_List,struct timeval timeout,int listen_fd,const char *leftIP,const char *leftPort,const char *rightIP,const char *rightPort){

  fd_set reads;
  FD_ZERO(&reads);//00000000000000000
  FD_SET(listen_fd, &reads);//00000001000000000
  int fd_max = listen_fd;//
  
  int neighbor_fd;
  struct sockaddr_in neighbor_address;
  socklen_t address_size;

  //cout<<"listen_fd: "<<listen_fd<<endl;
  //cout<<"left neighbor "<<leftIP<<":"<<leftPort<<endl;
  //cout<<"right neighbor "<<rightIP<<":"<<rightPort<<endl;
  //char myIP[16]={0};
  //char myPort[16]={0};
  //obtainMyAddr(listen_fd, myIP,myPort);
  //cout<<"listen_fd: ip: "<<myIP<<" port: "<<myPort<<endl;
  
  
  int neighbor_count = 0;
  while(neighbor_count != 2){//only 2 neighbors
    
    fd_set temp = reads;//select will reset the fd_set so we need a temporary temp
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

	cout<<"something is ready" <<endl;
	if(fd == listen_fd){//connection request: add the client_fd to the fd_set
	  
	  address_size = sizeof(neighbor_fd);
	  neighbor_fd = accept(listen_fd, (struct sockaddr*)&neighbor_address,&address_size);

	  char nIP[16]="\0";
	  char nPort[16]="\0";
	  obtainPeerAddr(neighbor_fd,nIP,nPort);
	  cout<<"accept "<<nIP<<":"<<nPort<<endl;
	  
	  int a = strcmp(nIP,leftIP);
	  int b = strcmp(nPort,leftPort);
	  int c = strcmp(nIP,rightIP);
	  int d = strcmp(nPort,rightPort);
	  if( (a && b) || (c && d)){
	    cout<<"accept fd:"<<neighbor_fd<<endl;
	    FD_SET(neighbor_fd, &reads);
	    fd_List.push_back(neighbor_fd);
	 
	    //cout<<"client ip: "<<client_List[client_count].ip<<endl;
	    //cout<<"client port: "<<client_List[client_count].port<<endl;
	  
	    if(fd_max < neighbor_fd){//update fd_max when new socket is involved
	      fd_max = neighbor_fd;
	      cout<<"new client is connecting"<<endl;
	    }

	    neighbor_count++;
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

  //1.1.create listen socket
  int listen_fd = createListenSocket();

  char myIP[16]={0};
  char myPort[16]={0};
  obtainMyAddr(listen_fd, myIP,myPort);
  //cout<<"my ip: "<<myIP<<" my port: "<<myPort<<endl;
  //1.2.create socket -- socket -> local
  int server_fd = createConnectSocket(argv[1], PORT);
  send(server_fd,myIP,16,0);//send listen socket address to server
  send(server_fd,myPort,16,0);
  
  //2.1.recieve left and right neighbors info
  char id[16]="\0";//used to trace
  recv(server_fd,id,16,0);
  char left_ip[16]="\0";
  recv(server_fd,left_ip,16,0);
  char left_port[16]="\0";
  recv(server_fd,left_port,16,0);
  char right_ip[16]="\0";
  recv(server_fd,right_ip,16,0);
  char right_port[16]="\0";
  recv(server_fd,right_port,16,0);
  printf("my ID: %s\n",id);
  printf("left neighbor: %s:%s\n",left_ip,left_port);
  printf("right neighbor: %s:%s\n",right_ip,right_port);
  
  //2.2. connect to neighbor: passive & active
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 5000;
  vector<int> accept_fds;//passive accepted sockets
  int left_fd;
  int right_fd;
  if(atoi(id) %2 == 0){
    cout<<"acceptor first"<<endl;
    acceptNeighbors(accept_fds,timeout,listen_fd,left_ip,left_port,right_ip,right_port);
    left_fd = createConnectSocket(left_ip,left_port);
    right_fd = createConnectSocket(right_ip,right_port);
  }else{
    cout<<"connector first"<<endl;
    sleep(5);//since acceptor is a while loop
    left_fd = createConnectSocket(left_ip,left_port);
    right_fd = createConnectSocket(right_ip,right_port);
    acceptNeighbors(accept_fds,timeout,listen_fd,left_ip,left_port,right_ip,right_port);
  }
  
  for(size_t i = 0; i < accept_fds.size(); i++){
    cout<< accept_fds[i]<<endl;
  }
  //3. select() to check if server_fd and accept_fds
  
  
  //4. close all fds
  for(size_t i = 0; i < accept_fds.size(); i++){
    close(accept_fds[i]);
  }
  close(left_fd);
  close(right_fd);
  
  close(listen_fd);
  close(server_fd);

  return 0;
}
