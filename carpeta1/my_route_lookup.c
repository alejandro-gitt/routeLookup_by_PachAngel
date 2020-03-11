#include "io.h"
#include "utils.h"

enum flag{
  si = 1,
  no = 0
};

typedef struct
{
  enum flag marker_flag;
  enum flag prefix_flag;
  uint32_t prefijo;// primer bit: 1-> prefix;
  short siguiente_salto;
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
