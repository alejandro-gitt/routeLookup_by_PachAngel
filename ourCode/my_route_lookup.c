#include "io.h"
#include "utils.h"
#include <time.h>
#include <stdio.h>
#define TAMANO_INICIAL 3851
typedef struct entrada entrada;
typedef struct nodo nodo;

struct nodo{
  char n;
  entrada *tabla;
  int size_tabla;
  nodo *parentNode;
  nodo *left;
  nodo *right;
  nodo *nextToMark;
};

struct entrada{
  char marker_flag;
  char prefix_flag;
  uint32_t prefijo;
  short siguiente_salto;
  entrada *next;
};

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
          nodo_aux->nextToMark = NULL;
          raiz->left = nodo_aux;
          return nodo_aux;
        }
    }
    else{
      if(raiz->right != NULL) return crearNodo(raiz->right,n,param_nivel/2);
      else{
        nodo *nodo_aux = (nodo*)calloc(1,sizeof(nodo));
        nodo_aux->n = n_aux+param_nivel;
        nodo_aux->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada));
        nodo_aux->size_tabla = TAMANO_INICIAL;
        nodo_aux->left = NULL;
        nodo_aux->right = NULL;
        nodo_aux->nextToMark = NULL;
        if(nodo_aux->n == 30){//Construcción del los nodos por debajo del 30 (29, 31 y 32)
          //manualmente, ya que esta parte del arbol no sigue el mismo patrón.
          //Los nodos 29 y 32 serán descendientes del 30 y el 31 será descendiente del 32
          nodo_aux->right = (nodo*)calloc(1,sizeof(nodo));
          nodo_aux->right->n = 32;
          nodo_aux->right->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada));
          nodo_aux->right->size_tabla = TAMANO_INICIAL;
          nodo_aux->right->right = NULL;
          nodo_aux->right->nextToMark = NULL;

          nodo_aux->right->left = (nodo*)calloc(1,sizeof(nodo));
          nodo_aux->right->left->n = 31;
          nodo_aux->right->left->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada));
          nodo_aux->right->left->size_tabla = TAMANO_INICIAL;
          nodo_aux->right->left->left = NULL;
          nodo_aux->right->left->right = NULL;
          nodo_aux->right->left->nextToMark= NULL;

          nodo_aux->left = (nodo*)calloc(1,sizeof(nodo));
          nodo_aux->left->n = 29;
          nodo_aux->left->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada));
          nodo_aux->left->size_tabla = TAMANO_INICIAL;
          nodo_aux->left->left = NULL;
          nodo_aux->left->right = NULL;
          nodo_aux->left->nextToMark = NULL;

        }

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
  while (currentNode != NULL){
    currentItem = &currentNode->tabla[hash(prefix >> (32-currentNode->n),currentNode->size_tabla)];
    if(currentItem->prefix_flag != 0 || currentItem->marker_flag != 0){
      while(currentItem->prefijo != prefix){
        if(currentItem->next != NULL){
          currentItem = currentItem->next;
          *numberOfTableAccesses += 1;
        }else break;
      }
    }
    if(currentItem->prefijo == prefix && (currentItem->marker_flag != 0 || currentItem->prefix_flag != 0)){
      if(currentItem->siguiente_salto != 0) next_hop = currentItem->siguiente_salto;
      if(currentItem->marker_flag != 0){
        currentNode = currentNode->right;
      }
      else break;
    }else{
      currentNode = currentNode->left;
    }
    if(currentNode != NULL){
      getNetmask(currentNode->n,&netmask);
    }else{
      break;
    }
    prefix = dir & netmask;
    *numberOfTableAccesses += 1;
  }//end of while (currentNode != NULL)
  return next_hop;
}

void addMarker(uint32_t dir,int prefixLength, short defaultInterface, nodo *firstInList,int *numberOfTableAccesses){
  entrada *currentItem = NULL;
  nodo *currentNode = firstInList;
  int netmask;
  uint32_t prefix;



  while(currentNode != NULL){//recorre la lista de nodos en los que hay que añadir markers
    getNetmask(currentNode->n,&netmask);
    prefix = dir & (uint32_t)netmask;
    currentItem = &currentNode->tabla[hash(prefix >> (32-currentNode->n),currentNode->size_tabla)];
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
      if(currentNode->left != NULL)
        currentItem->siguiente_salto = calc_next_hop(currentNode->left, prefix, defaultInterface, numberOfTableAccesses);
    }
    currentNode = currentNode->nextToMark;
  }
}

void free_hash_list(entrada *head){
  if(head != NULL){
    if(head->next != NULL){
      free_hash_list(head->next);
    }
    free(head);
  }
}

void free_tree(nodo *raiz){
  if(raiz->left != NULL) free_tree(raiz->left);
  if(raiz->right != NULL) free_tree(raiz->right);
  int i = 0;
  if(raiz->tabla != NULL){
    for(i=0;i<raiz->size_tabla;i++){
      free_hash_list(raiz->tabla[i].next);
    }
    free(raiz->tabla);
  }
  free(raiz);
  return;
}

void imprimirPost(nodo *raiz)
{
    if (raiz != NULL)
    {
        imprimirPost(raiz->left);
        imprimirPost(raiz->right);
        printf("%i-",raiz->n);
    }
}

int main(int argc, char *argv[]){

  if(argc != 3) {
    printf("%s\n","Incorrect number of arguments");
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
  nodo* headNode = NULL;
  nodo* tailNode = NULL;
  nodo *currentLista = NULL;
  entrada *currentItem = NULL;
  int counter = 0;

  nodo *raiz = (nodo*)calloc(1,sizeof(nodo));
  raiz->n = 16;
  raiz->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada));
  raiz->size_tabla = TAMANO_INICIAL;
  raiz->left = NULL;
  raiz->right = NULL;
  raiz->nextToMark = NULL;

  currentNode = raiz;

  errno = initializeIO(argv[1],argv[2]);
  if(errno != OK){
    printIOExplanationError(errno);
    return -1;
  }

  do{
    errno = readFIBLine(&dir, &prefixLength, &outInterface);
    if(prefixLength == 0){
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
          if(currentNode->left == NULL) crearNodo(raiz,prefixLength,raiz->n/2);
          currentNode = currentNode->left;
        }
        else{
          if(currentNode->right == NULL) crearNodo(raiz,prefixLength,raiz->n/2);
          /*Añadiendo currentNode a la lista para añadir marker*/
          if(headNode == NULL){
            headNode = currentNode;
            tailNode = headNode;
          }else{
            currentNode->nextToMark = NULL;
            tailNode->nextToMark = currentNode;
            tailNode = currentNode;
          }
          currentNode = currentNode->right;
        }
      }
      currentLista = headNode;
      while(currentLista != NULL){
        currentLista = currentLista->nextToMark;
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
      currentItem->siguiente_salto = (short)outInterface;
      if(headNode != NULL){//hay que añadir markers
        addMarker(dir, prefixLength, defaultInterface, headNode, &numberOfTableAccesses);
      }
      counter += 1;
      headNode = NULL;
      currentLista = NULL;
      tailNode = NULL;
    }
  }while(errno != REACHED_EOF);
  errno = 0;
  dir = 0;
  short siguiente_salto = 0;
  struct timespec initialTime;
  struct timespec finalTime;
  double searchingTime = 0;
  double  TotalTime = 0;
  numberOfTableAccesses = 0;
  int totalTableAccesses = 0;
  counter = 0;
  while(1){
    errno = readInputPacketFileLine(&dir);
    if(errno == REACHED_EOF) break;
    clock_gettime(CLOCK_MONOTONIC_RAW, &initialTime);
    siguiente_salto = calc_next_hop(raiz,dir, defaultInterface, &numberOfTableAccesses);
    clock_gettime(CLOCK_MONOTONIC_RAW, &finalTime);
    printOutputLine(dir, (int)siguiente_salto, &initialTime, &finalTime, &searchingTime, numberOfTableAccesses);
    TotalTime += searchingTime;
    totalTableAccesses += numberOfTableAccesses;
    searchingTime = 0;
    numberOfTableAccesses = 0;
    counter += 1;
  }

  printSummary(counter, totalTableAccesses/counter, TotalTime/counter);
  free_tree(raiz);
  freeIO();
}
