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
void mover(int lista[], int n) {
  int posicion = -1;
  for (int i = 0; i < 5; i++){
    if (lista[i] == n){
      posicion = i;
      break;
    }
  }
  int temp = lista[posicion];
  for (int i = posicion; i < 4; i++) {
    lista[i] = lista[i + 1];
  }
  lista[4] = temp;
}
int main(int argc, char *argv[]) {
  int salir = 1;
  void *TLB = malloc(300); 
  int orden[5] = {0, 1, 2, 3, 4};
  while(salir){
    int indice = -1;
    char input[100];
    printf("Ingrese direccion virtual: ");
    if (fgets(input, sizeof input, stdin) != NULL){
      input[strcspn(input, "\n")] = '\0';
      if (entero(input)) {
        printf("TLB desde 0x0%x hasta 0x0%x\n", TLB, TLB + 300);
        //printf("%x\n",TLB);
        //printf("%x\n", TLB + 300);
        
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
          mover(orden, esta_en - 1);
          printf("TLB Hit\n");
        } else {
          printf("TLB Miss\n");
          int lleno = 1;
          for (int i = 0; i < 270; i+=54) {
            if (*(int*)((char*)TLB + i) == 0) {
              lleno = 0;
	      memcpy(TLB + i, &numero, sizeof(int));
	      memcpy(TLB + i + sizeof(int), &pagina, sizeof(int));
	      memcpy(TLB + i + sizeof(int) * 2, &desp, sizeof(int));
	      memcpy(TLB + i + sizeof(int) * 3, &pagbin, 21);
	      memcpy(TLB + i + sizeof(int) * 3 + 21, &desbin, 21);
              break;
            }
          }
          if (lleno) {
            indice = orden[0];
            mover(orden, indice);
            int irem = 54 * indice;
            memcpy(TLB + irem, &numero, sizeof(int));
	    memcpy(TLB + irem + sizeof(int), &pagina, sizeof(int));
	    memcpy(TLB + irem + sizeof(int) * 2, &desp, sizeof(int));
	    memcpy(TLB + irem + sizeof(int) * 3, &pagbin, 21);
            memcpy(TLB + irem + sizeof(int) * 3 + 21, &desbin, 21);
          }
        }
        binario(numero, bin, bits);
        printf("Pagina: %d\n", pagina);
        printf("Desplazamiento: %d\n", desp);
        printf("Pagina en binario: %s\n", pagbin);
        printf("Desplazamiento en binario: %s\n", desbin);
        printf("Politica de reemplazo: ");
        if (indice >= 0) printf("0x0%x\n", TLB + 54 * indice);
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
  printf("Good bye!\n");
  return 0;
}
