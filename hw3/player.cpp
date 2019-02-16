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

void obtain_fdPort(int socket_fd, char*myPort){

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

    //cout<<"accept address "<<ipPtr<<":"<<portPtr<<endl;
}


int createBindedSocket(char * ip, int & port){
  //1.create socket
  int fd = socket(AF_INET, SOCK_STREAM, 0);//family, type, anything
  if(fd < 0){
    perror("socket() fails");
    exit(EXIT_FAILURE);
  }
  //2. bind() to an address -- defined by ip, port, protocol-> server_address
  //otherwise, it will be automatically allocated a port, clients have no idea what it is
  struct sockaddr_in fd_address;
  memset(&fd_address, 0, sizeof(fd_address));//clear server_address
  fd_address.sin_family = AF_INET;//address family
  fd_address.sin_addr.s_addr = inet_addr(ip);//ip address
  fd_address.sin_port = htons(port);//port
  socklen_t address_size = sizeof(fd_address);
  int status;
  while((status= bind(fd, (struct sockaddr*)&fd_address, address_size)) != 0){
    port++;
    fd_address.sin_port = htons(port);
  }
  return fd;
}

void connectSocket(int fd,const char *target_ip,const char *target_port){
  struct sockaddr_in sock_address;

  memset(&sock_address, 0, sizeof(sock_address));
  sock_address.sin_family = AF_INET;
  sock_address.sin_addr.s_addr = inet_addr(target_ip);
  sock_address.sin_port = htons(atoi(target_port));

  cout<<endl;
  cout<<"try to connect "<<target_ip<<":"<<target_port<<endl;
  if ( connect(fd, (struct sockaddr*)&sock_address, sizeof(sock_address)) == -1  ){
    perror("connect() fails");
    exit(EXIT_FAILURE);
  }
  cout<<"connect successful"<<endl;
  cout<<endl;
}


void acceptNeighbors(vector<int> &fd_List,struct timeval timeout,int listen_fd,const char *leftIP,const char *left_rightPort,const char *rightIP,const char *right_leftPort,int&fd_max){

  fd_set reads;
  FD_ZERO(&reads);//00000000000000000
  FD_SET(listen_fd, &reads);//00000001000000000
  fd_max = listen_fd;//
  
  int neighbor_fd;
  struct sockaddr_in neighbor_address;
  socklen_t address_size;
  /*
  cout<<endl;
  cout<<"listen_fd: "<<listen_fd<<endl;
  cout<<"left neighbor's right connector: "<<leftIP<<":"<<left_rightPort<<endl;
  cout<<"right neighbor's left connector: "<<rightIP<<":"<<right_leftPort<<endl;
  cout<<endl;
  */
  int neighbor_count = 0;
  while(neighbor_count < 2){//only 2 neighbors
    
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

	if(fd == listen_fd){//connection request: add the client_fd to the fd_set
	  
	  address_size = sizeof(neighbor_fd);
	  neighbor_fd = accept(listen_fd, (struct sockaddr*)&neighbor_address,&address_size);

	  char nIP[16]="\0";
	  char nPort[16]="\0";
	  obtainPeerAddr(neighbor_fd,nIP,nPort);

	  cout<<endl;
	  cout<<"accept "<<nIP<<":"<<nPort<<endl;
	  cout<<endl;
	  
	  int a = strcmp(nIP,leftIP);
	  int b = strcmp(nPort,left_rightPort);
	  int c = strcmp(nIP,rightIP);
	  int d = strcmp(nPort,right_leftPort);
	  if( ( (a == 0) && (b == 0) ) || ( (c == 0) && (d == 0) )){//only allows two neighbors to connect to the client
	    //cout<<"accept fd:"<<neighbor_fd<<endl;
	    FD_SET(neighbor_fd, &reads);
	    fd_List.push_back(neighbor_fd);
	 
	    //cout<<"client ip: "<<client_List[client_count].ip<<endl;
	    //cout<<"client port: "<<client_List[client_count].port<<endl;
	  
	    if(fd_max < neighbor_fd){//update fd_max when new socket is involved
	      fd_max = neighbor_fd;
	      //cout<<"new client is connecting"<<endl;
	    }

	    neighbor_count++;
	  }
	}

	
      }
      
    }
    
  }
}

void start_game(vector<int> &fd_List,struct timeval timeout,int server_fd,int left_fd, int right_fd, int fd_max, char *id){

  fd_set reads;
  FD_ZERO(&reads);
  //monitor server end, and two acceptor side
  FD_SET(server_fd, &reads);
  FD_SET(fd_List[0], &reads);
  FD_SET(fd_List[1], &reads);

  int sigEND = 0;
  while(sigEND != 1){
    
    fd_set temp = reads;
    int status = select(fd_max + 1, &temp, 0, 0, &timeout);
    if(status == -1){//function fails
      perror("select() fails");
    }
    else if(status == 0){//timeout
      //cout<< "timeout"<<endl;
      continue;
    }

    for(int fd = 0; fd < fd_max + 1; fd++){
      if(FD_ISSET(fd, &temp)){
	
	if(fd == server_fd){//from server side
	  
	  char buffer[16]={0};
	  recv(fd, buffer, 16, 0);
	  
	  if(strcmp(buffer,"stop") == 0){//stop the game
	    
	    sigEND = 1;
	    break;
	    
	  }else{//i.e. you are the start
	    
	    int hops = atoi(buffer);
	    
	    if(hops == 0){//unlucky
	      send(server_fd, id, 1024, 0);
	    }else{//randomly choose one side to pass the potato

	      hops--;
	      char hops_str[16]={0};
	      sprintf(hops_str,"%d",hops);
	      
	      srand((unsigned)time(NULL));
	      int random = rand() % 100;
	      
	      if(random < 50){
		send(left_fd,id,1024,0);
		send(left_fd,hops_str,16,0);
	      }else{
		send(right_fd,id,1024,0);
		send(right_fd,hops_str,16,0);
	      }
	    }
	  }
	}
	if( (fd == fd_List[0]) || (fd == fd_List[1])){

	  char trace[1024]={0};
	  char hops_str[16]={0};
	  recv(fd,trace,1024,0);
	  recv(fd,hops_str,16,0);

	  int hops = atoi(hops_str);
	  hops--;
	  char hops_str_new[16]={0};
	  
	  strcat(trace," ");
	  strcat(trace,id);
	  sprintf(hops_str_new,"%d",hops);

	  if(hops == 0){
	    send(server_fd,trace,1024,0);
	  }else{
	    srand((unsigned)time(NULL));
	    int random = rand() % 100;
	      
	    if(random < 50){
	      send(left_fd,id,1024,0);
	      send(left_fd,hops_str,16,0);
	    }else{
	      send(right_fd,id,1024,0);
	      send(right_fd,hops_str,16,0);
	    }
	  }
	}
      }      
    }
  }

  
}


int main(int argc, char *argv[])
{
  //0. check argc & get host ip
  if(argc != 3){
    perror("Usage: ./server [server ip] [port]");
    exit(EXIT_FAILURE);
  }

  char host[100]={0};
  size_t name_sz = sizeof(host);
  gethostname(host,name_sz);
  struct hostent *hp;
  hp = gethostbyname(host);
  char ip[16];//local ip address
  struct sockaddr_in sock_addr;
  sock_addr.sin_addr = *((struct in_addr*) hp->h_addr_list[0]);
  inet_ntop(AF_INET, &sock_addr.sin_addr, ip, sizeof(ip));
  
  //1.create all 4 sockets we will use: to server, two connectors, and the listner
  int port = atoi(argv[2]);
  int server_fd = createBindedSocket(ip,port);
  int listen_fd = createBindedSocket(ip,port);
  if(listen(listen_fd,7) == -1){
    perror("listen() fails");
    close(listen_fd);
    exit(EXIT_FAILURE);
  }
  int left_fd = createBindedSocket(ip,port);
  int right_fd = createBindedSocket(ip,port);

  char listen_fdPort[16] = {0};
  char left_fdPort[16] = {0};
  char right_fdPort[16] ={0};
  obtain_fdPort(listen_fd, listen_fdPort);
  obtain_fdPort(left_fd, left_fdPort);
  obtain_fdPort(right_fd, right_fdPort);
  //2.connet to server and send address of 1 listener and 2 connectors
  connectSocket(server_fd,argv[1],argv[2]);//connect to server
  
  send(server_fd,ip,16,0);//send 
  send(server_fd,listen_fdPort,16,0);
  send(server_fd,left_fdPort,16,0);
  send(server_fd,right_fdPort,16,0);
  cout<<" ip: "<<ip<<" listenPort: "<<listen_fdPort<<" leftPort: "<<left_fdPort<<" rightPort: "<<right_fdPort<<endl;
  //3.recieve info about client's id, how to connect to its neighbor's listener, and check by left and right port (limitation on two communication channels)

  char id[16]="\0";//used to trace the result
  recv(server_fd,id,16,0);
  
  char left_ip[16]="\0";
  recv(server_fd,left_ip,16,0);
  char left_listenPort[16]="\0";
  recv(server_fd,left_listenPort,16,0);//connector will try to connecto via listenPort
  char left_rightPort[16]="\0";
  recv(server_fd,left_rightPort,16,0);//after accept(), will check if this is the correct port
  cout<<"You ID: "<<id<<endl;
  cout<<"left neighbor-- ip: "<< left_ip<<" listenPort: "<<left_listenPort<<" rightPort: "<<left_rightPort <<endl<<endl;;
  
  char right_ip[16]="\0";
  recv(server_fd,right_ip,16,0);
  char right_listenPort[16]="\0";//connect()
  recv(server_fd,right_listenPort,16,0);
  char right_leftPort[16]="\0";//after accept()
  recv(server_fd,right_leftPort,16,0);
  cout<<"right neighbor-- ip: "<< right_ip<<" listenPort: "<<right_listenPort<<" leftPort: "<<right_leftPort <<endl;
  
  //printf("my ID: %s\n",id);
  //printf("left neighbor: %s:%s\n",left_ip,left_port);
  //printf("right neighbor: %s:%s\n",right_ip,right_port);
  
  //4. connect to neighbors: passive & active
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 5000;
  vector<int> accept_fds;//passive accepted sockets
  int fd_max;
  if(atoi(id) %2 == 0){
    //cout<<"acceptor first"<<endl;
    acceptNeighbors(accept_fds,timeout,listen_fd,left_ip,left_rightPort,right_ip,right_leftPort,fd_max);

    connectSocket(left_fd, left_ip,left_listenPort);
    connectSocket(right_fd,right_ip,right_listenPort);
  }else{
    //cout<<"connector first"<<endl;
    sleep(5);//since acceptor is a while loop
    
    connectSocket(left_fd, left_ip,left_listenPort);
    connectSocket(right_fd,right_ip,right_listenPort);
    acceptNeighbors(accept_fds,timeout,listen_fd,left_ip,left_rightPort,right_ip,right_leftPort,fd_max);
  }
  cout<<"client-to-client connection succeeds"<<endl;
  /*
  for(size_t i = 0; i < accept_fds.size(); i++){
    cout<< accept_fds[i]<<endl;
  }
  */
  /*
  //5. start the game
  start_game(accept_fds,timeout,server_fd,left_fd,right_fd, fd_max, id);
  char trace[1024];
  recv(server_fd,trace,1024,0);
  cout << "Trace of Potato:"<<endl;
  cout << trace << endl;
  */
  //6. close all fds
  for(size_t i = 0; i < accept_fds.size(); i++){
    close(accept_fds[i]);
  }
  
  close(left_fd);
  close(right_fd);
  
  close(listen_fd);
  close(server_fd);

  return 0;
}
