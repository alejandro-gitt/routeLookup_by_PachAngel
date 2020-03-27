#include "io.h"
#include "utils.h"

typedef struct prefijo{
  char marker_flag;
  char prefix_flag;
  uint32_t prefijo;
  short siguiente_salto;
}prefijo;

typedef struct nodo{
  char n;
  prefijo *tabla;
  int size_tabla;
  struct nodo *left;
  struct nodo *right;
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
        nodo_aux->tabla = (prefijo*)calloc(2,sizeof(prefijo));
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
        nodo_aux->tabla = (prefijo*)calloc(2,sizeof(prefijo));
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
  raiz->tabla = (prefijo*)calloc(2,sizeof(prefijo));
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
      currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefijo = prefix;
      currentNode->tabla[hash(prefix,currentNode->size_tabla)].siguiente_salto
      = (short)outInterface;
    }
    counter += 1;
  }while(errno != REACHED_EOF);

}
