#include "io.h"
#include "utils.h"
#include <time.h>
#define COEFICIENTE 1

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

entrada *redimensiona(entrada *tabla_in,int *tam_tabla){
    int size_tabla=*tam_tabla;
    entrada *nueva_tabla = calloc(COEFICIENTE+size_tabla,sizeof(entrada));

    if(nueva_tabla == NULL){
      printf("%s\n","Asignación fallida");
      return 0;
    }else{
      //Asignación conseguida
      printf("%s\n", "Asignación realizada");
      /*Recorremos toda la tabla antigua, extrayendo los prefijos y añadiendolos en la nueva*/
      int i;
      for (i = 0; i < size_tabla; i++){
        if(tabla_in[i].marker_flag == 0 && tabla_in[i].prefix_flag == 0){
          //es una posición vacía
        }else{
          nueva_tabla[hash(tabla_in[i].prefijo,COEFICIENTE+size_tabla)] = tabla_in[i];
        }
      }
      free(tabla_in);
      *tam_tabla = size_tabla+COEFICIENTE;
      return nueva_tabla;
    }
}
void addMarker(uint32_t prefix,int prefixLength, nodo *currentNode,nodo *parentNode){
  int netmask;
  getNetmask(prefixLength, &netmask);
  uint32_t marker_to_add = prefix & netmask;
  if(parentNode != NULL){
    if(parentNode->tabla[hash(marker_to_add,parentNode->size_tabla)].prefijo == marker_to_add && parentNode->tabla[hash(marker_to_add,parentNode->size_tabla)].marker_flag == 1){
      // Ya está el marker, no hace falta añadirlo
    }else{
      parentNode->tabla[hash(marker_to_add,parentNode->size_tabla)].prefijo = marker_to_add;
      parentNode->tabla[hash(marker_to_add,parentNode->size_tabla)].marker_flag = 1;
    }
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

        if(nodo_aux->n == 31) nodo_aux->n = 32;
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

short calc_next_hop(nodo *raiz, uint32_t dir, int *numberOfTableAccesses){
  int netmask = 0;
  getNetmask(raiz->n,&netmask);
  uint32_t prefix = dir & netmask;
  short next_hop = 0;
  nodo *currentNode = raiz;
  while (currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefix_flag == 1 ||
  currentNode->tabla[hash(prefix,currentNode->size_tabla)].marker_flag == 1){

    next_hop = currentNode->tabla[hash(prefix,currentNode->size_tabla)].siguiente_salto;
    currentNode = currentNode->right;
    getNetmask(currentNode->n,&netmask);
    prefix = dir & netmask;
    numberOfTableAccesses += 1;
  }
  return next_hop;
}

void free_tree(nodo *raiz){
  if(raiz->left != NULL) free_tree(raiz->left);
  if(raiz->right != NULL) free_tree(raiz->right);
  if(raiz->tabla != NULL) free(raiz->tabla);
  free(raiz);
  return;
}

int main(int argc, char *argv[]){

  if(argc != 3) {
    printf("%s\n","not enough arguments");
    return -1;
  }

  int errno = 0;
  uint32_t prefix = 0;
  int prefixLength = 0;
  int outInterface = 0;

  nodo *currentNode = NULL;
  nodo* parentNode = NULL;
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
    printf(" val del counter: %i\n",counter);
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
          parentNode = currentNode;
          currentNode = currentNode->left;
        }
        else{
          //printf("menor");
          if(currentNode->right == NULL) crearNodo(raiz,prefixLength,raiz->n/2);
          parentNode = currentNode;
          currentNode = currentNode->right;
        }
      }

      while (currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefix_flag == 1 || currentNode->tabla[hash(prefix,currentNode->size_tabla)].marker_flag == 1){
        //    printf("En el while de flags tratando el nodo %d\n",currentNode->n);
        //printf("en el prefijo: %d\n",prefix);
        //printf("tamaño OG: %d\n",currentNode->size_tabla);
        //printf("Valor del hash: %d\n",hash(prefix,currentNode->size_tabla));

        if(currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefijo == prefix){// no debe hacer redimensiona
          break;
        }else{

        currentNode->tabla = redimensiona(currentNode->tabla,&currentNode->size_tabla);

        //printf("nuevo tamaño= %d \n ",currentNode->size_tabla);
        //printf("nuevo HASH= %d \n ",hash(prefix,currentNode->size_tabla));
        }
      }
      if(currentNode->tabla[hash(prefix,currentNode->size_tabla)].marker_flag == 1){
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefix_flag = 1;
        addMarker(prefix,prefixLength,currentNode,parentNode);
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].siguiente_salto = (short)outInterface;

      }else{
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefijo = prefix;
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefix_flag = 1;
        addMarker(prefix,prefixLength,currentNode,parentNode);
        currentNode->tabla[hash(prefix,currentNode->size_tabla)].siguiente_salto = (short)outInterface;
        }
      }
       counter += 1;
     }while(errno != REACHED_EOF);
     errno = 0;

     //printf("%d\n",calc_next_hop(raiz,3512205399));
     uint32_t dir = 0;
     short siguiente_salto = 0;
     struct timespec initialTime;
     struct timespec finalTime;
     double searchingTime = 0;
     double  TotalTime = 0;
     int numberOfTableAccesses = 0;
     int totalTableAccesses = 0;
     counter = 0;
     do{
       errno = readInputPacketFileLine(&dir);
       clock_gettime(CLOCK_MONOTONIC_RAW, &initialTime);
       siguiente_salto = calc_next_hop(raiz,dir,&numberOfTableAccesses);
       clock_gettime(CLOCK_MONOTONIC_RAW, &finalTime);
       printOutputLine(dir, (int)siguiente_salto, &initialTime, &finalTime, &searchingTime, numberOfTableAccesses);
       TotalTime += searchingTime;
       totalTableAccesses += numberOfTableAccesses;
       searchingTime = 0;
       numberOfTableAccesses = 0;
       counter += 1;
     }while(errno != REACHED_EOF);

     printSummary(counter, totalTableAccesses/counter, TotalTime/counter);
     printMemoryTimeUsage();
     free_tree(raiz);
     freeIO();
}
