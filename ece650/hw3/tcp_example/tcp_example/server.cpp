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
  struct addrinfo host_info;//to avoid warning
  struct addrinfo *host_info_list;//result is stored here
  const char *hostname = NULL;//IP will be set to loopback
  const char *port     = "4444";

  //1. get the host_info by getaddrinfo, which returns host_info_list
  memset(&host_info, 0, sizeof(host_info));//clear the warning remover

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;//for wildcard IP address, which means "any" and can only be used for bind operations i.e. 0.0.0.0 

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  //2. create socket, which returns the socket file descriptor
  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  //3. set 
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  cout << "Waiting for connection on port " << port << endl;
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return -1;
  } //if

  char buffer[512];
  recv(client_connection_fd, buffer, 9, 0);
  buffer[9] = 0;

  cout << "Server received: " << buffer << endl;

  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
