/* simplest version of calculator */
%{

#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "lispg.h"

#define YYMALLOC palloc
#define YYFREE   pfree

/* Avoid exit() on fatal scanner errors (a bit ugly -- see yy_fatal_error) */
#undef fprintf
#define fprintf(file, fmt, msg)  fprintf_to_ereport(fmt, msg)

	static void
		fprintf_to_ereport(const char *fmt, const char *msg)
	{
		ereport(ERROR, (errmsg_internal("%s", msg)));
	}



  LValue * mksymbol(char *name);
  LValue * mknumeric(char *s);

  LValue * mksymbol(char *name) {
    LValue *val = malloc(sizeof(LValue));
    val->type = LSYMBOL;
    val->val.name = name;
    return val;
  }

  LValue * mknumeric(char *s) {
	  LValue *val = malloc(sizeof(LValue));
	  val->type = LNUMERIC;
	  val->val.numeric = DatumGetNumeric(DirectFunctionCall3(numeric_in, CStringGetDatum(s), 0, -1));
	  return val;
  }

#include "lispg_gram.h"

%}
/* declare tokens */

%union {
   char* str;
   long bigint;
   LValue *val;
}

%pure-parser
%expect 0
%error-verbose
%name-prefix="lispg_yy"
%parse-param {LValue **result}

%token  <str> SYMBOL
%token  <str> NUMERIC
%type  <val>  expr
%type  <val>  term
%type  <val>  list


%%

prog: expr { *result = $1; } ;

expr: '(' list ')' { $$ = $2; } ;

list: { $$ = mklist(NULL, NULL); }
  | term list { $$ = mklist($1, $2); }
  | expr list { $$ = mklist($1, $2); }
  ;

term:
  NUMERIC { $$ = mknumeric($1) }
  | SYMBOL {$$ = mksymbol($1); }
  ;
  
%%

#include "lispg_lex.c"
