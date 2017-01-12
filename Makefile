# contrib/lispg/Makefile

MODULE_big = lispg
OBJS = lispg.o lispg_gram.o

EXTENSION = lispg
DATA = lispg--1.0.sql

ENCODING = UTF8

REGRESS = lispg

EXTRA_CLEAN = y.tab.c y.tab.h lispg_gram.c lispg_lex.c lispg_gram.h


ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/lispg
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

lispg_gram.o: lispg_lex.c

lispg_gram.c: BISONFLAGS += -d

distprep: lispg_gram.c lispg_lex.c

maintainer-clean:
	rm -f lispg_gram.c lispg_lex.c lispg_gram.h
