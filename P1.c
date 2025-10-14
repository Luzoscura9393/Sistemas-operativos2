#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
  pid_t pid;
  int N = atoi(argv[1]);
  int a1 = atoi(argv[2]);
  int a2 = atoi(argv[3]);
  int a3 = atoi(argv[4]);
  
  pid = fork();

  if (pid < 0) {
    printf("Error al crear el segundo proceso\n");
  }
  else if (pid == 0){
    for(int i = a3; i < N; i++) {
      printf("%.0f\n", pow(2, i));
    }
  printf("P2 termina\n");
  return -2;
  }
  else {
    printf("%d\n", a1);
    printf("%d\n", a2);
    for(int i = 0; i < N - 2; i ++){
      int j = a2;
      a2 = a1 + a2;
      printf("%d\n", a2);
      a1 = j;
    }
  printf("P1 terminado\n");
  return -1;
  }
}
