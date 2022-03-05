#include "ycc.h"

int size_of(Type *type) {
  switch (type->ty) {
  case CHAR:
    return 1;
  case INT:
    return 4;
  case PTR:
    return 8;
  case ARRAY:
    return type->array_size * size_of(type->ptr_to);
  default:
    error("illegal type [TypeKind: %d]\n", type->ty);
    return -1; // not reachable
  }
}

int calc_stack_size(LVar *locals) {
  int size = 0;
  for (LVar *local = locals; local; local = local->next) {
    size += size_of(local->type);
  }
  return size + ((16 - (size % 16)) % 16);
}
