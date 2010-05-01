#ifndef EXPR_EXPR_H
#define EXPR_EXPR_H

#include "global.hp"

#define NSYMS (20)

struct symtab
{
  char* name;
  long (*funcptr)(...);
  double value;
};

struct symtab* symlook( char* s );
int my_yyinput( char* buf, int max_size );



#endif
