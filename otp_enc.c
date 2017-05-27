#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>


int main(int argc, char *argv[]) {
  
  // Check for correct number of arguments
  if (argc < 4) {
    fprintf(stderr, "otp_enc usage:\n { otp_enc plaintext key port }\n");
    exit(1);
  }

  // Sets up the socket
  int port_num = atoi(argv[3]);
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    fprintf(stderr, "Error: Unable to open socket.\n");
    exit(1);
  }

  struct sockaddr_in server_add;
  
  // Sets up the address struct
  bzero((char*)&server_add, sizeof(server_add));
  server_add.sin_family = AF_INET;
  server_add.sin_port = htons(port_num);
  server_add.sin_addr.s_addr = htonl(INADDR_ANY);

  // Connects to the port
  if (connect(socket_fd, (struct sockaddr*)&server_add, sizeof(server_add)) < 0) {
    fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", port_num);
    exit(2);
  }

  // Sets up the buffer
  int size = 80000;
  char buff[size];
  char temp[size];
  temp[1] = '^';
  snprintf(buff, size, "%c", temp[1]);
  int val = write(socket_fd, buff, strlen(buff));
  if (val < 0) {
    fprintf(stderr, "Error: Unable to write plaintext filename to socket.\n");
    exit(1);
  }
  sleep(1);

  // Sends plaintext filename server
  snprintf(buff, size, "%s", argv[1]);
  val = write(socket_fd, buff, strlen(buff));
  if (val < 0) {
    fprintf(stderr, "Error: Unable to write plaintext filename to socket.\n");
    exit(1);
  }
  sleep(1);

  // Sends key filename to server
  snprintf(buff, size, "%s", argv[2]);
  val = write(socket_fd, buff, strlen(buff));
  if (val < 0) {
    fprintf(stderr, "Error: Unable to write key filename to socket.\n");
    exit(1);
  }

  // Reads in response from server
  bzero(buff, size);
  val = read(socket_fd, buff, size);
  if (val < 0) {
    fprintf(stderr, "Error: could not contact otp_enc_d on port %d", port_num);
    exit(2);
  }
  
  // Checks for error
  if (strcmp(buff,"short_key") == 0) {
    close(STDOUT_FILENO);
    fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
    exit(1);
  }
  else if (strcmp(buff,"bad_char") == 0) {
    close(STDOUT_FILENO);
    fprintf(stderr, "otp_enc_d error: input contains bad characters\n");
    exit(1);
  }
  else if (strcmp(buff,"invalid_conn") == 0) {
    close(STDOUT_FILENO);
    fprintf(stderr, "otp_enc error: unable to connect to otp_enc_d on port %d\n", port_num);
  }
  else {
    printf("%s\n", buff);
  }

  // Close socket
  close(socket_fd);

  return 0;
}