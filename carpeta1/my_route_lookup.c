#include "io.h"
#include "utils.h"

enum flag{
  si = 1,
  no = 0
};

typedef struct prefijo{
  enum flag marker_flag;
  enum flag prefix_flag;
  uint32_t prefijo;// primer bit: 1-> prefix;
  short siguiente_salto;
}prefijo;

typedef struct nodo{
  char n;
  prefijo *tabla;
  int size_tabla;
  struct nodo *left;
  struct nodo *right;
}nodo;

nodo *crearNodo(nodo *raiz, char n){
  char n_aux = raiz->n;
  if(n_aux == n) return NULL;
  else{
    if(n < n_aux){
      if(raiz->left != NULL) return crearNodo(raiz->left,n);
      else{
        nodo *nodo_aux = (nodo*)malloc(sizeof(nodo));
        nodo_aux->n = n_aux/2;
        nodo_aux->tabla = (prefijo*)malloc(sizeof(prefijo)*2);
        nodo_aux->size_tabla = 2;
        nodo_aux->left = NULL;
        nodo_aux->right = NULL;
        raiz->left = nodo_aux;
        return nodo_aux;
      }
    }
    else{
      if(raiz->right != NULL) return crearNodo(raiz->right,n);
      else{
        nodo *nodo_aux = (nodo*)malloc(sizeof(nodo));
        nodo_aux->n = n_aux*2;
        nodo_aux->tabla = (prefijo*)malloc(sizeof(prefijo)*2);
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

  nodo *raiz = (nodo*)malloc(sizeof(nodo));
  raiz->n = 16;
  raiz->tabla = (prefijo*)malloc(sizeof(prefijo)*2);
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
    errno = readFIBLine(&prefix, &prefixLength, &outInterface);
    if(errno != OK && errno != REACHED_EOF){
      printIOExplanationError(errno);
      return -1;
    }
    else if(errno == OK){

      while(currentNode->n != prefixLength){
        if(currentNode->n > prefixLength){
          if(currentNode->left == NULL) currentNode->left = crearNodo(raiz,prefixLength);
          currentNode = currentNode->left;
        }
        else{
          if(currentNode->right == NULL) currentNode->right = crearNodo(raiz,prefixLength);
          currentNode = currentNode->right;
        }
      }
      printf("%i\n",currentNode->n);// hay una violacion de segmento por como esta definida la tabla (punteros...)
      currentNode->tabla[hash(prefix,currentNode->size_tabla)].prefijo = prefix;
      currentNode->tabla[hash(prefix,currentNode->size_tabla)].siguiente_salto = (short)outInterface;
    }
  }while(errno != REACHED_EOF);

}
