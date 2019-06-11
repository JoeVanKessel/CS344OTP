#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char *argv[]){
  srand(time(NULL));
  int lengthOfKey;
  char alfaArr[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

  if (argv[1] != NULL)
  {
    lengthOfKey = atoi(argv[1]);
  }
  else
  {
    printf("Unspecified Length of Key\n");
    exit(0);
  }

  int i;
  for (i = 0; i < lengthOfKey; i++)
  {
    int randNum = rand()%27;
    printf("%c", alfaArr[randNum]);
  }

  printf("\n");

  return 0;

}
