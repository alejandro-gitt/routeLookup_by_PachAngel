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

typedef struct nodo{
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
nodo *crearNodo(nodo *raiz, char n, char param_nivel){
  char n_aux = raiz->n;
  if(n_aux == n) return NULL;
  else{
    if(n < n_aux){
      if(raiz->left != NULL) return crearNodo(raiz->left,n,param_nivel/2);
      else{
        nodo *nodo_aux = (nodo*)calloc(1,sizeof(nodo));
        nodo_aux->n = n_aux-param_nivel;
        nodo_aux->tabla = (entrada*)calloc(2,sizeof(entrada));
        nodo_aux->size_tabla = 2;
        nodo_aux->left = NULL;
        nodo_aux->right = NULL;
        raiz->left = nodo_aux;
        return nodo_aux;
      }
    }
    else{
      if(raiz->right != NULL) return crearNodo(raiz->right,n,param_nivel/2);
      else{
        nodo *nodo_aux = (nodo*)calloc(1,sizeof(nodo));
        nodo_aux->n = n_aux+param_nivel;
        nodo_aux->tabla = (entrada*)calloc(2,sizeof(entrada ));
        nodo_aux->size_tabla = 2;
        nodo_aux->left = NULL;
        nodo_aux->right = NULL;
        raiz->right = nodo_aux;
        return nodo_aux;
      }
    }
  }
  return NULL;
}

void free_tree(nodo *raiz){
  if(raiz->left != NULL) free_tree(raiz->left);
  if(raiz->right != NULL) free_tree(raiz->right);
  if(raiz->tabla != NULL) free(raiz->tabla);
  free(raiz);
  return;
}

int main(int argc, char *argv[]){

  if(argc != 3) return -1;

  int errno = 0;
  uint32_t prefix;
  int prefixLength;
  int outInterface;

  nodo *currentNode;

  int counter = 0;

  nodo *raiz = (nodo*)calloc(1,sizeof(nodo));
  raiz->n = 16;
  raiz->tabla = (entrada*)calloc(2,sizeof(entrada));
  raiz->size_tabla = 2;
  raiz->left = NULL;
  raiz->right = NULL;

  currentNode = raiz;


  errno = initializeIO(argv[1],argv[2]);
  if(errno != OK){
    printIOExplanationError(errno);
    return -1;
  }

  do{
    printf("%i\n",counter);
    errno = readFIBLine(&prefix, &prefixLength, &outInterface);
    currentNode = raiz;
    if(errno != OK && errno != REACHED_EOF){
      printIOExplanationError(errno);
      return -1;
    }
    else if(errno == OK){

      while(currentNode->n != prefixLength){
        if(currentNode->n > prefixLength){
          //printf("mayor");
          if(currentNode->left == NULL) crearNodo(raiz,prefixLength,raiz->n/2);
          currentNode = currentNode->left;
        }
        else{
          //printf("menor");
          if(currentNode->right == NULL) crearNodo(raiz,prefixLength,raiz->n/2);
          currentNode = currentNode->right;
        }
      }

      while (currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefix_flag == 1 || currentNode->tabla[hash(prefix,currentNode->size_tabla)].marker_flag == 1){
        if(currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefijo == prefix){// no debe hacer redimensiona
          break;
        }else{
        currentNode->tabla = redimensiona(currentNode->tabla,currentNode->size_tabla);
        }
      }
      if(currentNode->tabla[hash(prefix,currentNode->size_tabla)].marker_flag == 1){
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefix_flag = 1;
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].siguiente_salto = (short)outInterface;

      }else{
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefijo = prefix;
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefix_flag = 1;
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].siguiente_salto = (short)outInterface;
        }
      }
       counter += 1;
     }while(errno != REACHED_EOF);

}
