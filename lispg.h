
#ifndef __LISPG_H__
#define __LISPG_H__

#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "funcapi.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "lib/stringinfo.h"
#include "utils/jsonb.h"
#include "utils/builtins.h"
#include "utils/numeric.h"

typedef enum LType {
	LSYMBOL,
	LCONS,
	LNUMERIC,
	LERROR
} LType;

typedef struct LValue {
	LType type;
	union {
		char *name;
		Numeric		numeric;

		struct {
			struct LValue *value;
			struct LValue *next;
		} cons;
	} val;
} LValue;



LValue* parselispg(const char *str, int len);

LValue * mklist(LValue *head, LValue *list);
LValue *list_next(LValue *list);
LValue *list_value(LValue *list);
LValue *ensure_numeric(LValue *num);
LValue *mkerror(char *msg);
LValue *lnumeric_add(LValue *list);
LValue *lnumeric_sub(LValue *list);
void _lispg_to_text(StringInfo buf, LValue *val);
LValue * lispg_eval_args(LValue *list);
LValue * lispg_eval(LValue *val);
text *LispgToText(LValue *prog);

int lispg_yylex();
void lispg_yyerror(LValue **result, const char *message);

void createSymbolsHash(void);

typedef LValue *(*loperation)(LValue *);

typedef struct LSymbol
{
	char   *name;
	loperation fn;
} LSymbol;

LSymbol *lookup_symbol(const char *key);

#endif
