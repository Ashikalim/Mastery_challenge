#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>


int main(int argc, char *argv[]) {
  
  // Check for correct number of arguments
  if (argc < 2) {
    fprintf(stderr, "otp_dec_d usage:\n { otp_dec_d listening port }\n");
    exit(1);
  }

  // Create new child process
  pid_t pid = fork();
  // Checks if forking failed
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  // Exits parent process
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  
  // Sets up the socket
  int port_num = atoi(argv[1]);
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (socket_fd < 0) { 	//exits if socket cannot be opened
    fprintf(stderr, "Error: Unable to open socket.\n");
    exit(1);
  }

  struct sockaddr_in server_add;
  struct sockaddr_in client_add;
  
  // Sets up the address struct
  bzero((char*)&server_add, sizeof(server_add));
  server_add.sin_family = AF_INET;
  server_add.sin_port = htons(port_num);
  server_add.sin_addr.s_addr = htonl(INADDR_ANY);

  // Enables socket to start listening
  if (bind(socket_fd, (struct sockaddr*)&server_add, sizeof(server_add)) < 0) {
    fprintf(stderr, "Error: Unable to bind server to socket.\n");
    exit(1);
  }

  // Allows five connections
  listen(socket_fd, 5); 
  socklen_t client_add_length = sizeof(client_add);
  int accept_fd;
  
  while (1) {
    
	// Accepts a connection
	accept_fd = accept(socket_fd, (struct sockaddr*)&client_add, &client_add_length);
    if (accept_fd < 0) {
      fprintf(stderr, "Error: Unable to accept.\n");
      exit(1);
    }
	
	// Sets up sending and receiving buffer
    int size = 80000;
    char send_buff[size];
    bzero(send_buff, size);
    char recv_buff[size];
    bzero(recv_buff, size);
    
	// Reads from file descriptor and into the receive buffer
	int val = read(accept_fd, recv_buff, size);
    if (val < 0) {
       fprintf(stderr, "Error: Unable to read from socket.\n");
       exit(1);
    }
	// Verifies if it is a valid connection by checking for a special character
    printf("%c",recv_buff[0]);
    if (recv_buff[0] != '*') {
      snprintf(send_buff, size, "%s", "invalid_conn");
      // Error sent, if invalid connection
	  val = write(accept_fd, send_buff, strlen(send_buff));
      if (val < 0) {
        fprintf(stderr, "Error: Unable to read from socket.\n");
        exit(1);
      }
      continue;
    }

    // Receive encrypted filename
    bzero(recv_buff, size);
    val = read(accept_fd, recv_buff, size);
    if (val < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }

    // Opens encrypted file, and give read permission
    FILE *f;
    f = fopen(recv_buff, "r");
    if (f == NULL) {
      fprintf(stderr, "Error: Unable to open ciphertext.\n");
      exit(1);
    }

    // Converts characters in encrypted file to uppercase letters
	// Stores characters in an array
    char ciphertext[size];
    fgets(ciphertext, size, f);
    int i;
    for (i=0; i<strlen(ciphertext); i++) {
      ciphertext[i] = toupper(ciphertext[i]);
    }
    ciphertext[(strlen(ciphertext))+1] = '\0';

    // Checks if file contains bad characters
    for (i=0; i<(strlen(ciphertext)-1); i++) {
      if ((ciphertext[i] < 65 && ciphertext[i] != 32) || ciphertext[i] > 90) {
        fprintf(stderr, "otp_dec_d error: input contains bad characters\n");
        exit(1);
      }
    }
    fclose(f);

    // Receive key filename
    bzero(recv_buff, size);
    val = read(accept_fd, recv_buff, size);
    if (val < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }

    // Opens key file, and gives read permission
    f = fopen(recv_buff, "r");
    if (f == NULL) {
      fprintf(stderr, "Error: Unable to open key.\n");
      exit(1);
    }

    // Stores key characters in an array
    char key[size];
    fgets(key, size, f);
    fclose(f);

    // Verifies if the length of key file is longer than that of receive file 
    if (strlen(key) < strlen(ciphertext)) {
      fprintf(stderr, "Error: key '%s' is too short.\n", recv_buff);
      exit(1);
    }

    // Decryption process
    char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int temp;
	int key_num;
	int ciphertext_num;
	char plaintext[strlen(ciphertext)];
  
	for(i=0; i < strlen(ciphertext); i++ ){
		if(key[i] == ' ')
			key_num = 26;
		else
			key_num = key[i] - 65;
	
		if (ciphertext[i] == ' ')
			ciphertext_num = 26;
		else
			ciphertext_num = ciphertext[i] - 65;
		
		temp = ciphertext_num - key_num;
		if ( temp < 0)
			temp = temp + 27;
		
		// Decrypted characters are stored in an array
		plaintext[i] = alphabet[temp];
		
	}
	
	plaintext[i + 1] = '\0';
	
    // Send decrypted array of characters back to client
    bzero(send_buff, size);
    snprintf(send_buff, size, "%s", plaintext);
    val = write(accept_fd, send_buff, strlen(send_buff));
    
	if (val < 0) {
      fprintf(stderr, "Error: Unable to write to socket.\n");
      exit(1);
    }
  
  }
  close(socket_fd);
  return 0;
}
