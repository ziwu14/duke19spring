#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <vector>

#define PORT 4444
#define NUM_PLAYER 2

using namespace std;

typedef struct _addrPack{
  int fd;
  char ip[16];
  char listenPort[16];
  char leftPort[16];
  char rightPort[16];
}addrPack;

int createListenSocket(){
  struct sockaddr_in server_address;
  int listen_fd;

  //1. create a lisening socket -- defined by protocol, type
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);//family, type, anything
  if(listen_fd < 0){
    perror("socket() fails");
    exit(EXIT_FAILURE);
  }
  
  //2. bind() to an address -- defined by ip, port, protocol-> server_address
  //otherwise, it will be automatically allocated a port, clients have no idea what it is
  memset(&server_address, 0, sizeof(server_address));//clear server_address
  server_address.sin_family = AF_INET;//address family
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);//ip address
  server_address.sin_port = htons(PORT);//port

  socklen_t address_size = sizeof(server_address);
  if(bind(listen_fd, (struct sockaddr*)&server_address, address_size) == -1){//return socket address
    perror("bind() fails");
    exit(EXIT_FAILURE);
  }
  //3.listen
  if(listen(listen_fd,7) == -1){
    perror("listen() fails");
    close(listen_fd);
    exit(EXIT_FAILURE);
  }
  return listen_fd;
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

void Server_to_Clients(vector<addrPack> &client_List,struct timeval timeout,int listen_fd,int num_player){

  fd_set reads;//fd_set including server and all clients
  int fd_max;//the largest fd number
  FD_ZERO(&reads);//00000000000000000
  FD_SET(listen_fd, &reads);//00000001000000000
  fd_max = listen_fd;//???????
  
  int client_fd;
  struct sockaddr_in client_address;
  socklen_t address_size;

  int client_count = 0;
  while(client_count != num_player){
    
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
	  char client_listenPort[16]="\0";
	  char client_leftPort[16]="\0";
	  char client_rightPort[16]="\0";
	  recv(client_fd,clientIP,16,0);
	  recv(client_fd,client_listenPort,16,0);
	  recv(client_fd,client_leftPort,16,0);
	  recv(client_fd,client_rightPort,16,0);
	  //obtainPeerAddr(client_fd,clientIP,clientPort);
	  //cout<< "client ip: "<<clientIP<<" client port: "<<clientPort<<endl;
	  
	  FD_SET(client_fd, &reads);

	  addrPack temp;
	  temp.fd = client_fd;//comunication file
	  strcpy(temp.ip,clientIP);//tell neighbors' listen address
	  strcpy(temp.listenPort, client_listenPort);
	  strcpy(temp.leftPort, client_leftPort);
	  strcpy(temp.rightPort, client_rightPort);
	  client_List.push_back(temp);
	 
	  //cout<<"client ip: "<<client_List[client_count].ip<<endl;
	  //cout<<"client port: "<<client_List[client_count].port<<endl;
	  
	  if(fd_max < client_fd){//update fd_max when new socket is involved
	    fd_max = client_fd;
	
	  }
	  cout<<"new client is connecting with"<<endl;

	  client_count++;
	  
	}

	
      }
      
    }
    
  }
}

void Clients_to_Clients(const vector<addrPack> &List){
  size_t n = List.size();
  cout<<"n: "<<n<<endl;
  for(size_t i = 0; i < n; i++){
    
    size_t left = i - 1;
    size_t right = i + 1;
    if(i == 0){
      left = n - 1; 
    }
    if(i == n-1){
      right = 0;
    }
    char id[16] = "\0";
    sprintf(id,"%lu",i);
    //use fixed size to send/recv message, don't use sizeof() 
    send(List[i].fd, id,16,0);
    
    send(List[i].fd, List[left].ip,16,0);
    send(List[i].fd, List[left].listenPort,16,0);
    send(List[i].fd, List[left].rightPort,16,0);

    send(List[i].fd, List[right].ip,16,0);
    send(List[i].fd, List[right].listenPort,16,0);
    send(List[i].fd, List[right].leftPort,16,0);
  }
}

int main(int argc, char *argv[])
{
  //0. check argc
  if(argc != 4){
    perror("Usage: ./ringmaster <port> <num_player> <num_hop>");
    exit(EXIT_FAILURE);
  }
  //1.setupthe listening socket
  int listen_fd = createListenSocket();
  
  //2.connect server to all clients via select() & accept() 
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 5000;
  vector<addrPack> clientList;//record order of connection, to close them at the end

  Server_to_Clients(clientList,timeout,listen_fd,atoi(argv[2]));

  //3.connect clients to their neighbors
  Clients_to_Clients(clientList);
  /*
  //4.1.start the game
  srand((unsigned)time(NULL));
  int start = rand() % atoi(argv[2]);
  cout << "Ready to start the game, sending potato to player " << start <<endl;
  send(clientList[start].fd,argv[3],16,0);
  //4.2.wait
  fd_set reads;
  FD_ZERO(&reads);//00000000000000000
  int fd_max = 0;
  for(size_t i = 0; i < clientList.size(); i++){
    if(fd_max < clientList[i].fd){
      fd_max = clientList[i].fd;
    }
    FD_SET(clientList[i].fd, &reads);
  }
  int sigEND = 0;
  while (sigEND == 0){
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

	
	  char trace[1024]={0};
	  recv(fd,trace,1024,0);
	  cout<<"Trace of potato"<<endl;
	  cout<<trace<<endl;
	  
	  for(size_t i = 0; i < clientList.size(); i++){
	    send(clientList[i].fd,"stop",16,0);
	    send(clientList[i].fd,trace,1024,0);
	  }
	
      }
    }
  }

  */
  //close all fds
  for(size_t i = 0; i < clientList.size(); i++){
    close(clientList[i].fd);
  }
  close(listen_fd);
  return 0;
}
