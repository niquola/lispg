
%{

#include "lispg_gram.h"
#include "mb/pg_wchar.h"


static YY_BUFFER_STATE scanbufhandle;
static char *scanbuf;
static int	scanbuflen;


static void lispg_scanner_init(const char *str, int slen);
void lispg_scanner_finish(void);

%}


%option noyywrap
%option 8bit
%option never-interactive
%option nodefault
%option noinput
%option nounput
%option prefix="lispg_yy"
%option warn
%option bison-bridge

%%

"(" { return '('; }
")" { return ')'; }
[0-9]+ { char *res = malloc(strlen(yytext)); strcpy(res, yytext); yylval->str = res; return NUMERIC; }
[a-zA-Z+-]+ { char *res = malloc(strlen(yytext)); strcpy(res, yytext); yylval->str = res; return SYMBOL;  }
\n { }
[ \t] { }
. { elog(INFO,"Mystery character %s\n", yytext); }

%%

void
yyerror(LValue **result, const char *message)
{
	if (*yytext == YY_END_OF_BUFFER_CHAR)
    {
      ereport(ERROR,
              (errcode(ERRCODE_SYNTAX_ERROR),
               errmsg("bad jsquery representation"),
               /* translator: %s is typically "syntax error" */
               errdetail("%s at end of input", message)));
    }
	else
    {
      ereport(ERROR,
              (errcode(ERRCODE_SYNTAX_ERROR),
               errmsg("bad jsquery representation"),
               /* translator: first %s is typically "syntax error" */
               errdetail("%s at or near \"%s\"", message, yytext)));
    }
}



static void
lispg_scanner_init(const char *str, int slen)
{
  if (slen <= 0)
    slen = strlen(str);

  /*
   * Might be left over after ereport()
   */
  if (YY_CURRENT_BUFFER)
    yy_delete_buffer(YY_CURRENT_BUFFER);

  /*
   * Make a scan buffer with special termination needed by 
   */

  scanbuflen = slen;
  scanbuf = malloc(slen + 2);
  memcpy(scanbuf, str, slen);
  scanbuf[slen] = scanbuf[slen + 1] = YY_END_OF_BUFFER_CHAR;
  scanbufhandle = yy_scan_buffer(scanbuf, slen + 2);
}

void
lispg_scanner_finish(void)
{
  yy_delete_buffer(scanbufhandle);
  free(scanbuf);
}


LValue* 
parselispg(const char *str, int len) {
  LValue		*parseresult;

  lispg_scanner_init(str, len);

  if (lispg_yyparse((void*)&parseresult) != 0)
    lispg_yyerror(NULL, "bugus input");

  lispg_scanner_finish();

  return parseresult;
}
