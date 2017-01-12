
-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION lispg" to load this file. \quit

CREATE TYPE lispg;

CREATE FUNCTION lispg_in(cstring)
RETURNS lispg
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE FUNCTION lispg_out(lispg)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE lispg (
INTERNALLENGTH = -1,
INPUT = lispg_in,
OUTPUT = lispg_out,
STORAGE = extended
);

CREATE FUNCTION lispg_debug(cstring)
RETURNS text
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;
