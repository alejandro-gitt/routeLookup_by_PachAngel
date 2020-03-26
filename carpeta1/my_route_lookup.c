#include "io.h"
#include "utils.h"
#define COEFICIENTE 2

typedef struct
{
  char marker_flag;
  char prefix_flag;
  uint32_t prefijo;
  short siguiente_salto;
}entrada;

typedef struct {
  char n;
  entrada *tabla;
  int size_tabla;
  struct nodo *left;
  struct nodo *right;
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

entrada *redimensiona(entrada *tabla_in,int size_tabla){// Puede que debamos añadir una segunda entrada con el nuevo tamaño, de momento es un valor fijo(COEFICIENTE)
    entrada *nueva_tabla = calloc(COEFICIENTE*size_tabla,sizeof(entrada));

    if(nueva_tabla == NULL){
      printf("%s\n","Asignación fallida");
      return 0;
    }else{
      //Asignación conseguida

      /*Recorremos toda la tabla antigua, extrayendo los prefijos y añadiendolos en la nueva*/
      int i;
      for (i = 0; i < size_tabla; i++){
        if(tabla_in[i].marker_flag == 0 && tabla_in[i].prefix_flag == 0){
          //es una posición vacía
        }else{
          nueva_tabla[hash(tabla_in[i].prefijo,COEFICIENTE*size_tabla)] = tabla_in[i];
        }
      }
      //Hemos recorrido toda la tabla antigua, la desechamos
      free(tabla_in);
      return nueva_tabla;
    }
}

int main(void)
{
entrada *tabla = calloc(2,sizeof(entrada));
        tabla = redimensiona(tabla , 2);

free(tabla);
return 0;
}
