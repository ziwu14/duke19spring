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
#include <time.h>

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
  while((status= bind(fd, (struct sockaddr*)&fd_address, address_size)) != 0){//update port,until find a usable one
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
    perror("try again");
  }
  //cout<<"connect successful"<<endl;
  cout<<endl;
}


void acceptNeighbors(vector<int> &fd_List,struct timeval timeout,int listen_fd,const char *leftIP,const char *left_rightPort,const char *rightIP,const char *right_leftPort,int&fd_max,int num_player,int id){

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
  while(((neighbor_count !=2)&&((id !=0)&&(id!=(num_player-1)))  ) || ( neighbor_count != 1 && ((id == 0)||(id == num_player - 1))  ) ){//only 2 neighbors
    
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

int max(int a, int b){
  if(a > b){
    return a;
  }else{
    return b;
  }
}

void start_game(const vector<int> &fd_List,struct timeval timeout,int server_fd,int left_fd, int right_fd, char *id){

  fd_set reads;
  FD_ZERO(&reads);
  //monitor server end, and two acceptor side

  FD_SET(server_fd, &reads);//connected to server
  FD_SET(fd_List[0], &reads);//these two accept left & right neighbors
  FD_SET(fd_List[1], &reads);

  int fd_max = max(server_fd,fd_List[0]);
  fd_max = max(fd_max,fd_List[1]);
  //cout <<"fd_max: "<<fd_max<<endl;

  srand((unsigned)time(NULL)+ atoi(id));
  int count = 0;
  while(count < 1){
    
    fd_set temp = reads;
    int status = select(fd_max + 1, &temp,0, 0, &timeout);
    if(status == -1){//function fails
      perror("select() fails");
    }
    else if(status == 0){//timeout
      //cout<< "timeout"<<endl;
      continue;
    }
    //////////////////////////////////////////////
    
    int random;
    
    if(FD_ISSET(server_fd,&temp)){
      
      char buffer[16]={0};
      recv(server_fd,buffer,16,0);
      if(strlen(buffer) != 0){
	//cout << buffer <<endl;
	//cout << "receive from server: "<<endl;
	//count++;
	if(strcmp(buffer,"stop") == 0){//1
	  count ++;
	}else{//2
	  cout<<"You are the start"<<endl;
	  int hops = atoi(buffer);
	  string trace(id);
	  
	  if(hops == 0){//2.1
	    send(server_fd, id,512,0);
	  }else{//2.2
	    
	    random = rand() % 10;
	    cout <<"random: "<<random<<endl;
	    hops--;
	    char hops_str[16];
	    sprintf(hops_str,"%d",hops);
	    if(random < 5){
	      send(left_fd, id,512,0);
	      send(left_fd, hops_str,16,0);
	    }else{
	      send(right_fd, id,512,0);
	      send(right_fd, hops_str,16,0);
	    }
	  }
	}
      }else{
	cout<<"Oops"<<endl;
      }
      //FD_CLR(fd_List[0], &temp);
    }
    
    if(FD_ISSET(fd_List[0],&temp)){
      char _trace[512]={0};
      char _hops[16]={0};
      recv(fd_List[0],_trace,512,0);
      recv(fd_List[0],_hops,16,0);
      if(strlen(_trace) != 0){//1
	//cout << buffer <<endl;
	//cout << "receive from fd0: "<<endl;
	int hops = atoi(_hops);
	string trace(_trace);
	trace = trace +"->"+string(id);
	cout << trace;
	if(hops == 0){//1.1
	  send(server_fd,trace.c_str(),512,0);
	}else{//1.2
	  
	  random = rand() % 10;
	  cout <<"random: "<<random<<endl;
	  hops--;
	  char hops_str[16];
	  sprintf(hops_str,"%d",hops);
	  if(random < 5){
	    cout<<"to left"<<endl;
	    send(left_fd, trace.c_str(),512,0);
	    send(left_fd, hops_str,16,0);
	  }else{
	    cout<<"to right"<<endl;
	    send(right_fd, trace.c_str(),512,0);
	    send(right_fd, hops_str,16,0);
	  }
	}
      }else{
	cout<<"Oops"<<endl;
      }
      //FD_CLR(fd_List[0], &temp);
    }

    if(FD_ISSET(fd_List[1],&temp)){
      char _trace[512]={0};
      recv(fd_List[1],_trace,512,0);
      char _hops[16];
      recv(fd_List[1],_hops,16,0);
      if(strlen(_trace) != 0){
	//cout << buffer <<endl;
	//cout << "receive from fd1: "<<endl;
	int hops = atoi(_hops);
	string trace(_trace);
	trace = trace +"->"+string(id);
	cout<<trace<<endl;
	if(hops == 0){//1.1
	  send(server_fd,trace.c_str(),512,0);
	}else{//1.2
	  
	  random = rand() % 10;
	  cout <<"random: "<<random<<endl;
	  hops--;
	  char hops_str[16];
	  sprintf(hops_str,"%d",hops);
	  if(random < 5){
	    cout<<"to left"<<endl;
	    send(left_fd, trace.c_str(),512,0);
	    send(left_fd, hops_str,16,0);
	  }else{
	    cout<<"to right"<<endl;
	    send(right_fd, trace.c_str(),512,0);
	    send(right_fd, hops_str,16,0);
	  }
	}
      }else{
	cout<<"Oops"<<endl;
      }
      //FD_CLR(fd_List[1], &temp);
    }
    
    //////////////////////////////////////////////
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

  char _num_player[16];//used to handle special case when # of players is odd
  recv(server_fd,_num_player,16,0);
  int num_player = atoi(_num_player);
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
  if(atoi(id) %2 == 0){//speicial case: num_player is odd number
    //cout<<"acceptor first"<<endl;
    acceptNeighbors(accept_fds,timeout,listen_fd,left_ip,left_rightPort,right_ip,right_leftPort,fd_max,num_player,atoi(id));

    connectSocket(left_fd, left_ip,left_listenPort);
    connectSocket(right_fd,right_ip,right_listenPort);
  }else{
    //cout<<"connector first"<<endl;
    sleep(2);//since acceptor is a while loop
    
    connectSocket(left_fd, left_ip,left_listenPort);
    connectSocket(right_fd,right_ip,right_listenPort);
    acceptNeighbors(accept_fds,timeout,listen_fd,left_ip,left_rightPort,right_ip,right_leftPort,fd_max,num_player,atoi(id));
  }
  if(num_player % 2 == 1){//special case
    if(atoi(id) == 0){
      acceptNeighbors(accept_fds,timeout,listen_fd,left_ip,left_rightPort,right_ip,right_leftPort,fd_max,num_player,atoi(id));
      connectSocket(left_fd, left_ip,left_listenPort);
    }else if(atoi(id) == (num_player -1)){
      sleep(2);
      connectSocket(right_fd,right_ip,right_listenPort);
      acceptNeighbors(accept_fds,timeout,listen_fd,left_ip,left_rightPort,right_ip,right_leftPort,fd_max,num_player,atoi(id));
    }
  }
  cout<<"client-to-client connection succeeds"<<endl<<endl;
  cout <<"left_fd: "<<left_fd<<" right_fd: "<<right_fd<<endl;
  /*
  for(size_t i = 0; i < accept_fds.size(); i++){
    cout<< accept_fds[i]<<endl;
  }
  */
  /*
  if(strcmp(id,"0") == 0){
    send(accept_fds[0],"0-0",16,0);
    send(accept_fds[1],"0-1",16,0);
    //send(server_fd,id,16,0);
  }
  if(strcmp(id,"1") == 0){
    send(accept_fds[0],"1-0",16,0);
    send(accept_fds[1],"1-1",16,0);
    //send(server_fd,id,16,0);
  }
  if(strcmp(id,"2") == 0){
    send(accept_fds[0],"2-0",16,0);
    send(accept_fds[1],"2-1",16,0);
    //send(server_fd,id,16,0);
  }
  //send(accept_fds[0],id,16,0);
  //send(accept_fds[1],id,16,0);
  char l1[16]={0};
  char l2[16]={0};
  recv(left_fd,l1,16,0);
  recv(right_fd,l2,16,0);
  cout<<"l1: "<<l1<<" l2: "<<l2<<endl<<endl;
  */

  /*
  //5. start the game
  
  start_game(accept_fds,timeout,server_fd,left_fd,right_fd, id);
  
  char trace[512]={0};
  recv(server_fd,trace,512,0);
  cout << "Trace of Potato:"<<endl;
  cout << trace << endl;
 
  //send(server_fd,id,512,0);
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
