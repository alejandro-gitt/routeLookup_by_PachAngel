#include "io.h"
#include "utils.h"
#include <time.h>
#include <stdio.h>
#define COEFICIENTE 1
#define TAMANO_INICIAL 1049

typedef struct
{
  char marker_flag;
  char prefix_flag;
  uint32_t prefijo;
  short siguiente_salto;
  struct entrada *next;
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

nodo *crearNodo(nodo *raiz, char n, char param_nivel){
  char n_aux = raiz->n;
  if(n_aux == n) return NULL;
  else{
    if(n < n_aux){
      if(raiz->left != NULL) return crearNodo(raiz->left,n,param_nivel/2);
      else{
        nodo *nodo_aux = (nodo*)calloc(1,sizeof(nodo));
        nodo_aux->n = n_aux-param_nivel;
        nodo_aux->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada));
        nodo_aux->size_tabla = TAMANO_INICIAL;
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
        nodo_aux->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada ));

        nodo_aux->size_tabla = TAMANO_INICIAL;
        nodo_aux->left = NULL;
        nodo_aux->right = NULL;
        raiz->right = nodo_aux;
        return nodo_aux;
      }
    }
  }
  return NULL;
}

short calc_next_hop(nodo *raiz, uint32_t dir, short defaultInterface, int *numberOfTableAccesses){
  int netmask = 0;
  getNetmask(raiz->n,&netmask);
  uint32_t prefix = dir & (uint32_t)netmask;
  short next_hop = defaultInterface;
  nodo *currentNode = raiz;
  entrada *currentItem = NULL;
  *numberOfTableAccesses += 1;
  while (currentNode->tabla[hash(prefix >> (32-currentNode->n),currentNode->size_tabla)].prefix_flag != 0 || currentNode->tabla[hash(prefix >> (32-currentNode->n),currentNode->size_tabla)].marker_flag != 0){
    currentItem = &currentNode->tabla[hash(prefix >> (32-currentNode->n),currentNode->size_tabla)];
    //printf("Prefijo calculado a partir de netmask: %u\n",prefix);
    //printf("Prefijo en el nodo: %u\n",currentItem->prefijo);
    if(currentItem->prefix_flag != 0 || currentItem->marker_flag != 0){
      while(currentItem->prefijo != prefix){
        if(currentItem->next != NULL){
          currentItem = currentItem->next;
          //printf("Prefijo en el nodo: %u\n",currentItem->prefijo);
          *numberOfTableAccesses += 1;
        }else break;
      }
    }
    if(currentItem->prefijo == prefix){
      printf("%s\n", "Existe match");
      next_hop = currentItem->siguiente_salto;
      if(currentItem->marker_flag == 1){
      currentNode = currentNode->right;
    }else break;
      if(currentNode != NULL){
        getNetmask(currentNode->n,&netmask);
      }else break;
    }else break;
    prefix = dir & netmask;
    *numberOfTableAccesses += 1;
    if(currentNode->left != NULL) currentNode = currentNode->left;
    else break;
  }//
  return next_hop;
}

void addMarker(uint32_t prefix,int prefixLength, short defaultInterface, nodo *parentNode,int *numberOfTableAccesses){
  entrada *currentItem = NULL;
  if(parentNode != NULL){
    currentItem = &parentNode->tabla[hash(prefix >> (32-parentNode->n),parentNode->size_tabla)];
    if(currentItem->prefix_flag != 0 || currentItem->marker_flag != 0){
      while(currentItem->prefijo != prefix){
        if(currentItem->next != NULL) currentItem = currentItem->next;
        else{
          currentItem->next = calloc(1,sizeof(entrada));
          currentItem = currentItem->next;
          break;
        }
      }
    }
    currentItem->prefijo = prefix;
    currentItem->marker_flag = 1;
    if(currentItem->siguiente_salto == 0){
      if(parentNode->left != NULL)
        currentItem->siguiente_salto = calc_next_hop(parentNode->left, prefix, defaultInterface, numberOfTableAccesses);
    }
  }
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
  uint32_t dir = 0;
  int netmask = 0;
  int prefixLength = 0;
  int outInterface = 0;
  short defaultInterface = 0;
  int numberOfTableAccesses = 0;

  nodo *currentNode = NULL;
  nodo* parentNode = NULL;
  entrada *currentItem = NULL;
  int counter = 0;

  nodo *raiz = (nodo*)calloc(1,sizeof(nodo));
  raiz->n = 16;
  raiz->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada));
  raiz->size_tabla = TAMANO_INICIAL;
  raiz->left = NULL;
  raiz->right = NULL;

  currentNode = raiz;

  errno = initializeIO(argv[1],argv[2]);
  if(errno != OK){
    printIOExplanationError(errno);
    return -1;
  }

  do{
    //printf(" val del counter: %i\n",counter);
    errno = readFIBLine(&dir, &prefixLength, &outInterface);
    if(dir == 0){
      defaultInterface = outInterface;
      continue;
    }
    getNetmask(prefixLength,&netmask);
    prefix = dir & (uint32_t)netmask;
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
      currentItem = &currentNode->tabla[hash(prefix >> (32-prefixLength),currentNode->size_tabla)];
      if(currentItem->prefix_flag != 0 || currentItem->marker_flag != 0){
        while(currentItem->prefijo != prefix){
          if(currentItem->next != NULL) currentItem = currentItem->next;
          else{
            currentItem->next = calloc(1,sizeof(entrada));
            currentItem = currentItem->next;
            break;
          }
        }
      }
      currentItem->prefijo = prefix;
      currentItem->prefix_flag = 1;
      if(currentNode == parentNode->right){
        addMarker(prefix, prefixLength, defaultInterface, parentNode, &numberOfTableAccesses);
      }
      currentItem->siguiente_salto = (short)outInterface;
      counter += 1;
    }
  }while(errno != REACHED_EOF);
  errno = 0;

   //printf("%d\n",calc_next_hop(raiz,3512205399));
  dir = 0;
  short siguiente_salto = 0;
  struct timespec initialTime;
  struct timespec finalTime;
  double searchingTime = 0;
  double  TotalTime = 0;
  numberOfTableAccesses = 0;
  int totalTableAccesses = 0;
  counter = 0;
  do{
    errno = readInputPacketFileLine(&dir);
    clock_gettime(CLOCK_MONOTONIC_RAW, &initialTime);
    siguiente_salto = calc_next_hop(raiz,dir, defaultInterface, &numberOfTableAccesses);
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
  printf("--------------------------\n\n");
  //int i = 0;
  // for(i=0;i<raiz->size_tabla;i++){
  //  printf("%u\n",raiz->tabla[i].siguiente_salto);
  // }
  free_tree(raiz);
  freeIO();
}
