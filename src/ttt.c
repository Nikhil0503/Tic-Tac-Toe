#define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

// int read_data(){

// }

int sendData(int socket, char *buffer, int size) {
    char *ptr = buffer;
    int len = 0;

    while (len < size) {
        int sent = send(socket, ptr + len, size - len, 0);
        if (sent == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
        len += sent;
    }
    return 0;
}


int connect_inet(char *host, char *service) {
  struct addrinfo hints, *info_list, *info;
  int sock, error;
  // look up remote host
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // in practice, this means give us IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // indicate we want a streaming socket
  error = getaddrinfo(host, service, &hints, &info_list);
  if (error) {
    fprintf(stderr, "error looking up %s:%s: %s\n", host, service,
            gai_strerror(error));
    return -1;
  }
  for (info = info_list; info != NULL; info = info->ai_next) {
    sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (sock < 0)
      continue;
    error = connect(sock, info->ai_addr, info->ai_addrlen);
    if (error) {
      close(sock);
      continue;
    }
    break;
  }

  freeaddrinfo(info_list);
  if (info == NULL) {
    fprintf(stderr, "Unable to connect to %s:%s\n", host, service);
    return -1;
  }
  return sock;
}

#define BUFLEN 256

int main(int argc, char **argv) {
  int sock, bytes;
  char buf[BUFLEN];
  if (argc < 3) {
    printf("Specify host and service\n");
    exit(EXIT_FAILURE);
  } else if(argc > 3){
    printf("Too many arguments.\n");
    exit(EXIT_FAILURE);
  }
  
  sock = connect_inet(argv[1], argv[2]);
  if (sock < 0) exit(EXIT_FAILURE);
  while ((bytes = read(STDIN_FILENO, buf, BUFLEN)) > 0) {
    int success = send(sock, buf, bytes, 0);
    // FIXME: should check whether the write succeeded!
    if(success == -1) printf("Can't send message to server.");
    //Receive the response from the server
    char response[150];
    int receive = recv(sock, response, 150, 0);
    //Check to see if you have read all of the bytes.
    if(receive == -1) printf("Couldn't receive a response back from the server.");
    printf("Message sent back: %s\n", response);
    fflush(stdout);
  }
  
  close(sock);
  return EXIT_SUCCESS;
}