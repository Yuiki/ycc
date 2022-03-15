#ifndef _YCC_STDARG
#define _YCC_STDARG 1

typedef struct {
  int gp_offset;
  int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} _va_list;

typedef _va_list *va_list;

void __ycc_builtin_va_start(va_list ap);

#define va_start(ap, param) __ycc_builtin_va_start(ap)

#endif
