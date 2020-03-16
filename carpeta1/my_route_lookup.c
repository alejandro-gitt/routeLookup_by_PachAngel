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

int main(void){
  nodo *raiz = (nodo*)malloc(sizeof(nodo));
  raiz->n = 16;
  raiz->tabla = (prefijo*)malloc(sizeof(prefijo)*2);
  raiz->left = NULL;
  raiz->right = NULL;
  crearNodo(raiz,6);
  free_tree(raiz);
  return 0;
}
