#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define PAGE_SIZE 4096
#define PAGE_COUNT (1ULL << 20)
#define OFFSET_BITS 12

int entero(char *input){
  if (*input == '\0') return 0;
  for(int i = 0; input[i] != '\0'; i++) {
    if(!isdigit(input[i])) return 0;
  }
  return 1;
}
void binario(int num, char *bin, int bits) {
  for (int i = bits - 1; i >= 0; i--) {
    bin[bits - 1 - i] = ((num >> i) & 1) + '0';
  }
  bin[bits] = '\0';
}
int decimal(const char* bin){
  int num = 0;
  for(int i = 0; i < strlen(bin); i++){
    num += (bin[i] - '0') * pow(2,strlen(bin) - 1 - i);
  }
  return num;
}
int hit(int *TLB, int direccion) {
  for (int i = 0; i < 270; i+=54) {
    if (*(TLB + i) == direccion){
      return i + 1;
    }
  }
  return 0;
}
void re_hit(void *TLB, void *aux1, void *aux2, int n){
  int indice = 270;
  for (int i = 0; i < 270; i += 54){
    if (*(int*)((char*)TLB + i) == 0) {
      indice = i;
      memcpy(aux1, TLB + i, 54);
      break;
    }
  }
  for (int i = n * 54; i < indice; i += 54) {
    if (i == indice - 54) {
      memcpy(TLB + i, aux1, 54);
    } else {
      memcpy(TLB + i, TLB + i + 54, 54);
    }
  }
}
int re_miss(void *TLB, void *aux1) {
  int indice = -1;
  for (int i = 0; i < 270; i+=54){
    if (*(int*)((char*)TLB + i) == 0) {
      indice = i;
      break;
    }
  }
  if (indice >= 0) {
    memcpy(TLB + indice, aux1, 54);
    return 0;
  } else {
    for (int i = 0; i < 162; i+=54){
      memcpy(TLB + i, TLB + i + 54, 54);
    }
    memcpy(TLB + 216, aux1, 54);
    return 1;
  }
}
int main(int argc, char *argv[]) {
  int salir = 1;
  void *TLB = malloc(300);
  void *aux1 = malloc(54);
  void *aux2 = malloc(54); 
  while(salir){
    char input[100];
    int reemplazo = 0;
    printf("Ingrese direccion virtual: ");
    if (fgets(input, sizeof input, stdin) != NULL){
      input[strcspn(input, "\n")] = '\0';
      if (entero(input)) {
        printf("TLB desde 0x0%x hasta 0x0%x\n", TLB, TLB + 300);
        int bits = sizeof(int) * 5;
        char bin[bits + 1];
        int numero = atoi(input);
        int pagina = numero / 4096;
        int desp = numero % 4096;
        char pagbin[bits + 1];
        char desbin[bits + 1];
        binario(pagina, pagbin, bits);
        binario(desp, desbin, bits);
        int esta_en = hit(TLB, numero);
        if (esta_en > 0) {
          re_hit(TLB, aux1, aux2, esta_en - 1);
          printf("TLB Hit\n");
        } else {
          printf("TLB Miss\n");
          memcpy(aux1, &numero, sizeof(int));
          memcpy(aux1 + sizeof(int), &pagina, sizeof(int));
          memcpy(aux1 + sizeof(int) * 2, &desp, sizeof(int));
          memcpy(aux1 + sizeof(int) * 3, &pagbin, 21);
          memcpy(aux1 + sizeof(int) * 3 + 21, &desbin, 21);
          reemplazo = re_miss(TLB, aux1);
        }
        binario(numero, bin, bits);
        printf("Pagina: %d\n", pagina);
        printf("Desplazamiento: %d\n", desp);
        printf("Pagina en binario: %s\n", pagbin);
        printf("Desplazamiento en binario: %s\n", desbin);
        printf("Politica de reemplazo: ");
        if (reemplazo) printf("0x0%x\n", TLB);
        else printf("0x0\n");
        printf("\n");
        memset(bin, 0, sizeof(bin));
      }else {
        if (*input == 's') salir--;
        else (printf("Entrada invalidad\n"));
      }
    }
  }
  free(TLB);
  free(aux1);
  free(aux2);
  printf("Good bye!\n");
  return 0;
}
