#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  pid_t pid;
  
  pid = fork();

  if (pid < 0) {
    printf("Error al crear el segundo proceso\n");
  }
  else if (pid == 0){
   //xeclp("p2", argv1, argv[4], NULL);
  printf("Webos\n");
  }
  else {
    while(1){
      printf("Padre\n");
      sleep(1);
    }
  return 0;
  }
}