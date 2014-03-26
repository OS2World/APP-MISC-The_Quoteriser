/*
 * quotes.h
 *
 * Header file for quotes functions for the Quoteriser.
 *
 *      Created: 18th January, 1997
 * Version 1.00: 20th March, 1997
 * Version 2.00: 24th November, 1997
 * Version 2.10: 1st December, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_QUOTES_H
#define _QUOTERISER_QUOTES_H

#include <gnu/gdbm.h>
#ifndef GENERIC
# include <gnu/rxall.h>
# include <gnu/rxposix.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "authors.h"

/* limits */
#define QUOTE_MAX_DBNAME	256	/* maximum length of database name */
#define QUOTE_MAX_CODE		19	/* maximum length of quote code */
#define QUOTE_MAX_SOURCE	99	/* maximum length of quote source */
#define QUOTE_MAX_KEYWORD	19	/* maximum length of a keyword */
#define	QUOTE_NUM_KEYWORD	5	/* maximum number of keywords */
#define QUOTE_MAX_SEARCH	199	/* maximum length of search string */

/* quote database structure */
typedef struct _QUOTEDB {
	GDBM_FILE	dbfInfo;		/* information database */
	GDBM_FILE	dbfText;		/* text database */
} QUOTEDB;

/* quote database information structure */
typedef struct _QDBINFO {
	int		flMode;			/* S_IREAD, S_IWRITE, etc. */
	long		lSize;			/* database size in bytes */
	time_t		tCreation;		/* time of creation */
	time_t		tModification;		/* time of last modification */
} QDBINFO;

/* quote information structure */
typedef struct _QUOTEINFO {
	char		szAuthorCode[AUTHOR_MAX_CODE + 1];			/* author's code */
	char		szSource[QUOTE_MAX_SOURCE + 1];				/* source */
	char		aszKeyword[QUOTE_NUM_KEYWORD][QUOTE_MAX_KEYWORD + 1];	/* keywords */
} QUOTEINFO;

/* quote search structure */
typedef struct _QUOTESEARCH {
	QUOTEDB		*pqdb;				/* database to search in */
	char		szLastCode[QUOTE_MAX_CODE + 1];	/* last code found */
	char		szSearch[QUOTE_MAX_SEARCH];	/* search string */
#ifndef GENERIC
	regex_t		rxSearch;			/* compiled search string */
#endif
} QUOTESEARCH;

extern const char szQuoteExt[];

/* functions contained in qdb.c */
void	QuoteNullifyDB(QUOTEDB *);
int	QuoteOpenDB(char *, QUOTEDB *, int);
int	QuoteDBStat(char *, QDBINFO *);
void	QuoteReorganiseDB(QUOTEDB *);
void	QuoteCloseDB(QUOTEDB *);
void	QuoteEmptyDB(QUOTEDB *);

/* functions contained in quotes.c */
void	QuoteAddQuote(QUOTEDB *, char *, QUOTEINFO *, char *);
int	QuoteGetQuote(QUOTEDB *, char *, QUOTEINFO **, char **);
void	QuoteDeleteQuote(QUOTEDB *, char *);
void	QuoteFreeQuote(QUOTEINFO **, char **);
int	QuoteExists(QUOTEDB *, char *);

/* functions contained in qsearch.c */
int	QuoteFullSearchInit(QUOTEDB *, QUOTESEARCH *);
int	QuoteFullSearchNext(QUOTESEARCH *);
void	QuoteFullSearchEnd(QUOTESEARCH *);
int	QuoteKeySearchInit(QUOTEDB *, char *, QUOTESEARCH *);
int	QuoteKeySearchNext(QUOTESEARCH *);
void	QuoteKeySearchEnd(QUOTESEARCH *);
int	QuoteTextSearchInit(QUOTEDB *, char *, QUOTESEARCH *);
int	QuoteTextSearchNext(QUOTESEARCH *);
void	QuoteTextSearchEnd(QUOTESEARCH *);
int	QuoteAuthorSearchInit(QUOTEDB *, char *, QUOTESEARCH *);
int	QuoteAuthorSearchNext(QUOTESEARCH *);
void	QuoteAuthorSearchEnd(QUOTESEARCH *);

#endif
