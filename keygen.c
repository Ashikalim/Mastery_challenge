#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(int argc, char *argv[]) {
  
  // Check for correct number of arguments
  if (argc != 2) {
    printf("keygen usage:\n { keygen keylength }\n");
    exit(1);
  }
  
  else {
    srand(time(NULL));
    int length = atoi(argv[1]);
    char key[length];

    int i;
	char temp;
    // stores randomized uppercase characters and ' ' in a key array
	for (i=0; i<length; i++) {
      temp = rand() % 27;
	  
	  if (temp == 26)
		  key[i] = 32;
	  else
		  key[i] = temp + 65;
    }
    key[length] = '\0';
	
	// Outputs key to std out
    printf("%s\n", key);
  }

  return 0;
}