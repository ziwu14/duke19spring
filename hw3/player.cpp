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
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = argv[1];//IP address=host name e.g. 127.0.0.1
  const char *port     = "4444";//port#
  
  if (argc < 2) {
      cout << "Syntax: client <hostname>\n" << endl;
      return 1;
  }

  //1. get the host information by getaddrinfo function, result is stored in host_info_list
  memset(&host_info, 0, sizeof(host_info));//clear host_info struct
  host_info.ai_family   = AF_INET;//indicates that getaddrinfo() should return socket addresss for any address family(e.g. either IPv4 or IPv6) that can be used with node and service
  host_info.ai_socktype = SOCK_STREAM;//one of the connection based socket

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);//the result is stored in host_info_list, host_info is used to get rid of warning
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  
  //2.get the socket file descriptor -> socket is specific to combination of address familty + socket type + protocol
  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  
  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

  //3.connect, return addr and addrlen
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  
  char clientIP[16];
  unsigned int clientPort;
  struct sockaddr_in client_addr;
  //bzero(&client_addr, sizeof(client_addr));
  socklen_t len = sizeof(client_addr);
  getsockname(socket_fd,(struct sockaddr *) &client_addr,&len);
  inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, sizeof(clientIP));
  clientPort = ntohs(client_addr.sin_port);
  printf("ip: %s  , port:  %u\n",clientIP,clientPort);
  

  //send(socket_fd, "hi there!", 9, 0);
  //char buffer[512];
  char buffer[512];
  int end = recv(socket_fd, buffer, sizeof(buffer), 0);
  buffer[end] = 0;
  cout<< "client received: " << buffer<<endl;
  
  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
