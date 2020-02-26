#include "io.h"
#include "utils.h"

enum flag{
  si = 1,
  no = 0
};

typedef struct
{
  enum flag marker_flag;
  char prefijo[4];
  char siguiente_salto[2];
  enum flag prefix_flag;
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
