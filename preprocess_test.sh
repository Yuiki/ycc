cd src

cpp codegen.c -I ../include -P > ../tmp/codegen.c
cpp main.c -I ../include -P > ../tmp/main.c
cpp file.c -I ../include -P > ../tmp/file.c
cpp error.c -I ../include -P > ../tmp/error.c
cpp parse.c -I ../include -P > ../tmp/parse.c
cpp tokenize.c -I ../include -P > ../tmp/tokenize.c
cpp file.c -I ../include -P > ../tmp/file.c
