#include <stdio.h>

int main (int argc, char **argv) {
  printf("argc is: %d\n", argc);
  
  int i = 0;
  
  for (; i < argc; i++) {
    printf("arg %d is: %s\n", i + 1, argv[i]);
  }

  return 0;
}

