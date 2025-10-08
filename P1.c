#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
  pid_t pid;
  
  pid = fork();

  if (pid < 0) {
    printf("Error al crear el segundo proceso\n");
  }
  else if (pid == 0){
   //execlp("p2", argv[1], argv[4], NULL);
  printf("Webos\n");
  }
  else {
    int N = atoi(argv[1]);
    int a1 = atoi(argv[2]);
    int a2 = atoi(argv[3]);
    printf("%d\n", a1);
    printf("%d\n", a2);
    for(int i = 0; i < N - 2; i ++){
      int j = a2;
      a2 = a1 + a2;
      printf("%d\n", a2);
      a1 = j;
    }
  return -1;
  }
}
