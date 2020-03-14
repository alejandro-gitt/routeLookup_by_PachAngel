#include "io.h"
#include "utils.h"
#define COEFICIENTE 2

enum flag{
  si = 1,
  no = 0
};

typedef struct
{
  enum flag marker_flag;
  enum flag prefix_flag;
  uint32_t prefijo;// primer bit: 1-> prefix;
  short siguiente_salto;
}prefijo;

typedef struct {
  char n;
  prefijo *tabla;
  nodo *left;
  nodo *right;
}nodo;

/*
método que recibe una tabla con una memoria insuficiente, se le adjudica más memoria,
de momento una cantidad fija (mediante realloc)
entradas:
  - Puntero a la tabla
  - Puntero a tabla nueva
salidas:z
  - nuevo puntero con mayor espacio reservado (COEFICIENTE*tamaño_anterior)
*/

int redimensiona(void *tabla_in){// Puede que debamos añadir una segunda entrada con el nuevo tamaño, de momento es un valor fijo(COEFICIENTE)
    void *pnt_tmp = realloc(tabla_in , COEFICIENTE*sizeof(tabla_in));

    if(pnt_tmp == NULL){
      printf("%s\n","Asignación fallida");
      return 0;
    }else{
      //Asignación conseguida
      tabla_in = pnt_tmp;
      return 1;
    }
}

int main(void)
{
prefijo *tabla = malloc(sizeof(prefijo)*2);
free(tabla);
return 0;
}
