#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
  int N = atoi(argv[1]);
  int a3 = atoi(argv[2]);
  for(int i = a3; i < N; i++) {
    printf("%.0f\n", pow(2, i));
  }
  printf("PN termina\n");
  return -2;
}