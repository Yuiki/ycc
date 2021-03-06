#include "ycc.h"
#include <stdio.h>

int size_of(Type *type) {
  switch (type->kind) {
  case CHAR:
    return 1;
  case SHORT:
    return 2;
  case INT:
    return 4;
  case VOID:
    return 0;
  case PTR:
    return 8;
  case ENUM:
    return 4;
  case ARRAY:
    return type->array_size * size_of(type->ptr_to);
  case STRUCT: {
    int max_size = 0;
    int size = 0;
    for (StructMember *member = type->member; member; member = member->next) {
      int s = size_of(member->type);
      if (s > max_size) {
        max_size = s;
      }

      size = member->offset + s;
    }

    if (max_size == 0) {
      return 0;
    }

    int padd = (max_size - (size % max_size)) % max_size;
    return size + padd;
  }
  default:
    error("illegal type [TypeKind: %d]\n", type->kind);
    return -1; // not reachable
  }
}

int offset_of(int curr_size, Type *type) {
  int size = size_of(type);
  int padd = (size - (curr_size % size)) % size;
  return padd + curr_size;
}
