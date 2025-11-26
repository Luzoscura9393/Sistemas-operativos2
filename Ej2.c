#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
int entero(char *input){
  if (*input == '\0') return 0;
  for(int i = 0; input[i] != '\0'; i++) {
    if(!isdigit(input[i])) return 0;
  }
  return 1;
}
void *binario(int num, char *bin, int bits) {
  for (int i = bits - 1; i >= 0; i--) {
    bin[bits - 1 - i] = ((num >> i) & 1) + '0';
  }
  bin[bits] = '\0';
  return bin;
}
int decimal(const char* bin){
  int num = 0;
  for(int i = 0; i < strlen(bin); i++){
    num += (bin[i] - '0') * pow(2,strlen(bin) - 1 - i);
  }
  return num;
}
int main(int argc, char *argv[]) {
  int salir = 1;
  while(salir){
    char input[100];
    printf("Ingrese direccion virtual: ");
    if (fgets(input, sizeof input, stdin) != NULL){
      input[strcspn(input, "\n")] = '\0';
      if (entero(input)) {
        int bits = sizeof(int) * 5;
        char bin[bits + 1];
        int number = atoi(input);
        binario(number, bin, bits);
        printf("%s\n", bin);
        printf("%d\n", decimal(bin));
        memset(bin, 0, sizeof(bin));
      } else {
        if (*input == 's') salir--;
        else (printf("Entrada invalidad\n"));
      }
    }
  }
  printf("Good bye!\n");
  return 0;
}
