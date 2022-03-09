#include "ycc.h"

int size_of(Type *type) {
  switch (type->kind) {
  case CHAR:
    return 1;
  case INT:
    return 4;
  case VOID:
    return 0;
  case PTR:
    return 8;
  case ARRAY:
    return type->array_size * size_of(type->ptr_to);
  default:
    error("illegal type [TypeKind: %d]\n", type->kind);
    return -1; // not reachable
  }
}
