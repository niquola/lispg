#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "funcapi.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "lib/stringinfo.h"
#include "utils/jsonb.h"
#include "utils/builtins.h"
#include "lispg.h"


LValue * mklist(LValue *head, LValue *list) {
    LValue *val = malloc(sizeof(LValue));
    val->type = LCONS;
    val->val.cons.value = head;
    val->val.cons.next = list;
    return val;
}

LValue *list_next(LValue *list) {
	return list->val.cons.next;
}

LValue *list_value(LValue *list) {
	return list->val.cons.value;
}

LValue *mkerror(char *msg){
	LValue *res = palloc(sizeof(LValue));
	res->type = LERROR;
	res->val.name = pstrdup(msg);
	return res;
}

LValue *ensure_numeric(LValue *num) {
	if(num->type == LNUMERIC){
		return num;
	} else {
		return mkerror("Expected number");
	}
}


LValue *lnumeric_add(LValue *list) {
	LValue *la, *lb;
	Numeric a, b, nres;
	LValue *valid;

	la = list_value(list);

	valid = ensure_numeric(la);
	if(valid->type == LERROR) { return valid; }
	a= la->val.numeric;

	lb = list_value(list_next(list));

	valid = ensure_numeric(lb);
	if(valid->type == LERROR) { return valid; }

	b = lb->val.numeric;

	nres = DatumGetNumeric(DirectFunctionCall2(numeric_add, PointerGetDatum(a), PointerGetDatum(b)));

	LValue *res = palloc(sizeof(LValue));
	res->type = LNUMERIC;
	res->val.numeric = nres;
	return res;
}

LValue *lnumeric_sub(LValue *list) {

	LValue *la, *lb;
	Numeric a, b, nres;
	LValue *valid;

	la = list_value(list);

	valid = ensure_numeric(la);
	if(valid->type == LERROR) { return valid; }
	a= la->val.numeric;

	lb = list_value(list_next(list));

	valid = ensure_numeric(lb);
	if(valid->type == LERROR) { return valid; }

	b = lb->val.numeric;

	nres = DatumGetNumeric(DirectFunctionCall2(numeric_sub, PointerGetDatum(a), PointerGetDatum(b)));

	LValue *res = palloc(sizeof(LValue));
	res->type = LNUMERIC;
	res->val.numeric = nres;
	return res;
}


void _lispg_to_text(StringInfo buf, LValue *val){
	if(val == NULL) return;
	LValue *cur;
	switch(val->type) {
	case  LSYMBOL:
		appendStringInfoString(buf, val->val.name);
		break;
	case  LERROR:
		appendStringInfoString(buf, "ERROR:");
		appendStringInfoString(buf, val->val.name);
		break;
	case  LNUMERIC:
		appendStringInfoString(buf,
							   DatumGetCString(
								   DirectFunctionCall1(numeric_out, PointerGetDatum(val->val.numeric))));
		break;
	case  LCONS:
		cur = val;
		appendStringInfoString(buf, "(");
		_lispg_to_text(buf, val->val.cons.value);
		while((cur = cur->val.cons.next) != NULL){
			appendStringInfoString(buf, " ");
			_lispg_to_text(buf, cur->val.cons.value);
		}
		appendStringInfoString(buf, ")");
		break;
	default:
		elog(INFO,"Unknown type %d", val->type);
	}
}

text *LispgToText(LValue *prog){
	StringInfoData		buf;
	initStringInfo(&buf);
	appendStringInfoSpaces(&buf, VARHDRSZ);
	_lispg_to_text(&buf, prog);
	SET_VARSIZE(buf.data, buf.len);
	return (text *)buf.data;
}

void ldebug(char *mess, LValue *x) {
	elog(INFO, "%s %s", mess, text_to_cstring(LispgToText(x)));
}

LValue * lispg_eval_args(LValue *list) {
	LValue *cur = list;
	while(cur != NULL){
		if(cur->val.cons.value != NULL) {
			cur->val.cons.value = lispg_eval(list_value(cur));
		}
		cur = list_next(cur);
	}
	return list;
}

LValue * lispg_eval(LValue *val) {
	createSymbolsHash();

	if(val == NULL) return NULL;

	LValue *cur;

	switch(val->type) {
	case  LSYMBOL:
	case  LNUMERIC:
		return val;
		break;
	case  LCONS:
		cur = val->val.cons.value;
		if(cur != NULL && cur->val.name != NULL) {
			LSymbol *sym = lookup_symbol(cur->val.name);
			if(sym != NULL){
				LValue *args = lispg_eval_args(list_next(val));
				return (sym->fn)(args);
			} else {
				elog(INFO, "sym %s not found", cur->val.name);
			}
		} else {
			elog(INFO, "empty symbol");
		}
		return val;
		break;
	default:
		elog(INFO,"Eval: unknown type %d", val->type);
	}
	return mkerror("Ups :(");
}
