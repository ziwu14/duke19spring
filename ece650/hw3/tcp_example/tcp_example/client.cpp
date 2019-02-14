#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

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
  host_info.ai_family   = AF_UNSPEC;//indicates that getaddrinfo() should return socket addresss for any address family(e.g. either IPv4 or IPv6) that can be used with node and service
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

  //4.write message
  const char *message = "hi there!";
  send(socket_fd, message, strlen(message), 0);

  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
