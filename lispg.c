#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "funcapi.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "lib/stringinfo.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#include "utils/jsonb.h"
#include "utils/builtins.h"
#include "lispg.h"

#include "lispg_operations.c"

typedef struct
{
	int32	vl_len_;	/* varlena header (do not touch directly!) */
} Lispg;

#define DatumGetLispgP(d)	((Lispg*)DatumGetPointer(PG_DETOAST_DATUM(d)))
#define PG_GETARG_LISPG(x)	DatumGetLispgP(PG_GETARG_DATUM(x))
#define PG_RETURN_LISPG(p)	PG_RETURN_POINTER(p)
#define NUMSYM 1000

static HTAB *symbolsHash = NULL;

typedef struct symbolEnt
{
	char   name[NAMEDATALEN];
	struct LSymbol *value;
} symbolEnt;

LSymbol snumeric_add = { "+", lnumeric_add };
LSymbol snumeric_sub = { "-", lnumeric_sub };

void inline addsymbol(LSymbol *lsym) {
	bool found;
	symbolEnt *hentry;
	hentry= (symbolEnt *) hash_search(symbolsHash, lsym->name, HASH_ENTER, &found);
	hentry->value = lsym;
}

void
createSymbolsHash(void)
{
	HASHCTL		ctl;

	if(symbolsHash != NULL) return;

	ctl.keysize = NAMEDATALEN;
	ctl.entrysize = sizeof(symbolEnt);

	symbolsHash = hash_create("Remote symbol hash", NUMSYM, &ctl, HASH_ELEM);

	addsymbol(&snumeric_add);
	addsymbol(&snumeric_sub);

}

LSymbol *lookup_symbol(const char *key){
	bool found;
	symbolEnt *hentry = (symbolEnt *) hash_search(symbolsHash, key, HASH_ENTER, &found);

	if(found){
		return hentry->value;
	} else {
		return NULL;
	}
}



PG_FUNCTION_INFO_V1(lispg_in);
Datum
lispg_in(PG_FUNCTION_ARGS)
{
	char *in = PG_GETARG_CSTRING(0);
	StringInfoData		buf;
	Lispg        		*res;


	elog(INFO, "Hello world, %s", in);

	initStringInfo(&buf);
	appendStringInfoSpaces(&buf, VARHDRSZ);
	appendBinaryStringInfo(&buf, in, strlen(in) + 1);

	res = (Lispg*)buf.data;
	SET_VARSIZE(res, buf.len);
	PG_RETURN_LISPG(res);
}


PG_FUNCTION_INFO_V1(lispg_out);
Datum
lispg_out(PG_FUNCTION_ARGS)
{
	Lispg *in = PG_GETARG_LISPG(0);
	char *res;

	res = palloc(VARSIZE(in));
	memcpy(res, VARDATA(in), VARSIZE(in) - VARHDRSZ);

	PG_RETURN_CSTRING(res);
}


PG_FUNCTION_INFO_V1(lispg_debug);
Datum
lispg_debug(PG_FUNCTION_ARGS)
{
	char *in = PG_GETARG_CSTRING(0);

	LValue *prog = parselispg(in, strlen(in));
	LValue *res = lispg_eval(prog);

	PG_RETURN_TEXT_P(LispgToText(res));
}
