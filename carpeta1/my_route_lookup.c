#include "io.h"
#include "utils.h"

enum marker{
  si = 1,
  no = 0
};

typedef struct
{
  enum marker marker_flag;
  char prefijo[4]; //primer byte es flag de marker (1 es marker)
  char siguiente_salto[2];
}prefijo;

typedef struct {
  char n;
  prefijo *tabla;
  nodo *left;
  nodo *right;
}nodo;

int main(void)
{
prefijo *tabla = malloc(sizeof(prefijo)*2);
free(tabla);
return 0;
}
