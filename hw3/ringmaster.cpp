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
#include <utility>

using namespace std;

void obtainPeerAddr(int socket_fd, char *ipPtr, char*portPtr){
    
    struct sockaddr_in peerAddr;
    //bzero(&myAddress, sizeof(myAddress));
    socklen_t len = sizeof(peerAddr);
    getpeername(socket_fd,(struct sockaddr *) &peerAddr,&len);
    inet_ntop(AF_INET, &peerAddr.sin_addr, ipPtr, 16*sizeof(ipPtr));
    int temp = ntohs(peerAddr.sin_port);
    sprintf(portPtr,"%d",temp);
}

void Server_to_Clients(fd_set &reads, int &fd_max, vector<int> &client_fdList,vector<pair<char*,char*> > &client_addrList,struct timeval timeout,int listen_fd){

  FD_ZERO(&reads);//00000000000000000
  FD_SET(listen_fd, &reads);//00000001000000000
  fd_max = listen_fd;//???????
  
  int client_fd;
  struct sockaddr_in client_address;
  socklen_t address_size;

  int client_count = 0;
  while(client_count != 1){
    
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

	  
	  char clientIP[16]={0};
	  char clientPort[16]={0};
	  obtainPeerAddr(client_fd,clientIP,clientPort);
	  cout<< "client ip: "<<clientIP<<"client port: "<<clientPort<<endl;
	  
	  FD_SET(client_fd, &reads);
	  client_fdList.push_back(client_fd);
	  client_addrList.push_back(pair<char*,char*>(clientIP,clientPort));
	  //cout<<"client ip: "<<client_addrList[client_count].first<<endl;
	  //cout<<"client port: "<<client_addrList[client_count].second<<endl;
	  
	  if(fd_max < client_fd){//update fd_max when new socket is involved
	    fd_max = client_fd;
	    cout<<"new client is connecting"<<endl;
	  }

	  client_count++;
	  
	} else{//receive message from client_fd i.e. client_fd is ready to be read

	  cout<<"changed fd is "<<fd<<endl;
	  char buffer[512]={0};
	  
	  recv(fd,buffer,sizeof(buffer),0);
	  cout<<fd<<endl;
	  
	  if(strcmp(buffer,"q") == 0){//exit signal
	    
	    break;
	  }
	  
	}

      }
    }
    
  }
}

void Clients_to_Clients(const vector<int> &client_fdList,struct timeval timeout){
  for(size_t i = 0; i < client_fdList.size(); i++){
    
  }
}

int main(int argc, char *argv[])
{
  int listen_fd;
  struct sockaddr_in server_address;
  

  //1. socket create -- defined by protocol, type
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
  server_address.sin_port = htons(4444);//port

  socklen_t address_size = sizeof(server_address);
  if(bind(listen_fd, (struct sockaddr*)&server_address, address_size) == -1){//return socket address
    perror("bind() fails");
    exit(EXIT_FAILURE);
  }
  //3.listen
  if(listen(listen_fd,10) == -1){
    perror("listen() fails");
    exit(EXIT_FAILURE);
  }

  //4.1.connect server to all clients via select() & accept() 
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 5000;
  vector<int> client_fdList;//record order of connection, to close them at the end
  vector<pair<char*,char*> > client_addrList;
  fd_set reads;//fd_set including server and all clients
  int fd_max;//the largest fd number

  Server_to_Clients(reads, fd_max, client_fdList,client_addrList,timeout,listen_fd);
  
  //4.2.connect clients to their neighbors
  Clients_to_Clients(client_fdList,timeout);
  
  

  for(size_t i = 0; i < client_fdList.size(); i++){
    close(client_fdList[i]);
  }
  close(listen_fd);
  return 0;
}
