#include "io.h"
#include "utils.h"
#include <time.h>
#include <stdio.h>
#define COEFICIENTE 1
#define TAMANO_INICIAL 3617

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
  struct nodo *parentNode;
  struct nodo *left;
  struct nodo *right;
  struct nodo *nextToMark;
}nodo;

nodo *crearNodo(nodo *raiz, char n, char param_nivel){
  char n_aux = raiz->n;
  if(n_aux == n) return NULL;
  else{
    if(n < n_aux){
        if(raiz->left != NULL) return crearNodo(raiz->left,n,param_nivel/2);
        else{
            nodo *nodo_aux = (nodo*)calloc(1,sizeof(nodo));
            nodo_aux->n = n_aux-param_nivel;
            printf("%s= %d\n","nodo a punto de crear",nodo_aux->n );

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

        if(nodo_aux->n == 31) nodo_aux->n = 32;
        nodo_aux->tabla = (entrada*)calloc(TAMANO_INICIAL,sizeof(entrada ));

        nodo_aux->size_tabla = TAMANO_INICIAL;
        nodo_aux->left = NULL;
        nodo_aux->right = NULL;
        nodo_aux->nextToMark = NULL;
        raiz->right = nodo_aux;
        return nodo_aux;
      }
    }
  }
  return NULL;
}

short calc_next_hop(nodo *raiz, uint32_t dir, short defaultInterface, int *numberOfTableAccesses){
  printf("%s====%u\n","defaultencalc",defaultInterface );
  int netmask = 0;
  getNetmask(raiz->n,&netmask);
  uint32_t prefix = dir & (uint32_t)netmask;
  short next_hop = defaultInterface;
  nodo *currentNode = raiz;
  //printf("En el nodo 16\n");
  entrada *currentItem = NULL;
  *numberOfTableAccesses += 1;
  while (currentNode != NULL){
  //  printf("En el nodo %u\n",currentNode->n);
    currentItem = &currentNode->tabla[hash(prefix >> (32-currentNode->n),currentNode->size_tabla)];
    // printf("Prefijo calculado a partir de netmask: %u\n",prefix);
    // printf("Prefijo en el nodo: %u\n",currentItem->prefijo);
    if(currentItem->prefix_flag != 0 || currentItem->marker_flag != 0){
      while(currentItem->prefijo != prefix){
        if(currentItem->next != NULL){
          currentItem = currentItem->next;
          //printf("Prefijo en el nodo: %u\n",currentItem->prefijo);
          *numberOfTableAccesses += 1;
        }else break;
      }
    }
    if(currentItem->prefijo == prefix && (currentItem->marker_flag != 0 || currentItem->prefix_flag != 0)){
    //  printf("%s\n", "Existe match");
      next_hop = currentItem->siguiente_salto;
      if(currentItem->marker_flag != 0){
        currentNode = currentNode->right;
      //  printf("%s\n", "Me voy a la derecha");
      }
      else break;
    }else{
      currentNode = currentNode->left;
    //  printf("%s\n", "Me voy a la izquierda");
    }
    if(currentNode != NULL){
      getNetmask(currentNode->n,&netmask);
    }else{
    //  printf("%s\n", "NO hay nodo");
      break;
    }
    prefix = dir & netmask;
    *numberOfTableAccesses += 1;
  }//end of while grande
  return next_hop;
}

void addMarker(uint32_t dir,int prefixLength, short defaultInterface, nodo *firstInList,int *numberOfTableAccesses){
  entrada *currentItem = NULL;
  nodo *currentNode = firstInList;
  int netmask;
  uint32_t prefix;
  //printf("%s\n", "no estoy loco");



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
    //printf(" val del counter: %i\n",counter);
    errno = readFIBLine(&dir, &prefixLength, &outInterface);
    if(prefixLength == 0){
      defaultInterface = outInterface;
      printf("%s= %u\n","default interface",defaultInterface );
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
      //    parentNode = currentNode;
          currentNode = currentNode->left;
        }
        else{
          //printf("menor");
          if(currentNode->right == NULL) crearNodo(raiz,prefixLength,raiz->n/2);
          /*Añadiendo currentNode a la lista para añadir marker (mark_list)*/
          if(headNode == NULL){//mark_list estaba vacía, añadimos el primer elemento
            headNode = currentNode;
            tailNode = headNode;
          }else{//mark_list no estaba vacía, añadimos al final:
            currentNode->nextToMark = NULL;
            tailNode->nextToMark = currentNode;
            tailNode = currentNode;
          }
          /*Añadido a la lista*/
          //parentNode = currentNode;
          currentNode = currentNode->right;
        }
      }
      //printf("Rellenando prefijo en nodo %u\n", currentNode->n);
      currentLista = headNode; // importante borrar la lista en cada iteración (head = null)
      printf("Lista: ");
      while(currentLista != NULL){
        printf("%u ", currentLista->n);
        currentLista = currentLista->nextToMark;
      }
      printf("\n");
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
    printf("siguiente_salto = %u\n", siguiente_salto);
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
  imprimirPost(raiz);
  free_tree(raiz);
  freeIO();
}
