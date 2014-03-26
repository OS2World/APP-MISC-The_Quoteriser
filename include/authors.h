/*
 * authors.h
 *
 * Header file for author functions for the Quoteriser.
 *
 *      Created: 18th January, 1997
 * Version 1.00: 2nd March, 1997
 * Version 2.00: 24th November, 1997
 * Version 2.10: 1st December, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_AUTHORS_H
#define _QUOTERISER_AUTHORS_H

#include <gnu/gdbm.h>
#ifndef GENERIC
# include <gnu/rxall.h>
# include <gnu/rxposix.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

/* limits */
#define AUTHOR_MAX_DBNAME	256	/* maximum length of database name */
#define AUTHOR_MAX_CODE		19	/* maximum length of author code */
#define AUTHOR_MAX_GNAME	49	/* maximum length of given name */
#define AUTHOR_MAX_SURNAME	49	/* maximum length of surname */
#define AUTHOR_MAX_BIRTH	9	/* maximum length of year of birth */
#define AUTHOR_MAX_DEATH	9	/* maximum length of year of death */
#define AUTHOR_MAX_SEARCH	199	/* maximum length of search string */

/* author database structure */
typedef struct _AUTHORDB {
	GDBM_FILE	dbfInfo;		/* information database */
	GDBM_FILE	dbfDesc;		/* description database */
} AUTHORDB;

/* quote database information structure */
typedef struct _ADBINFO {
	int		flMode;			/* S_IREAD, S_IWRITE, etc. */
	long		lSize;			/* database size in bytes */
	time_t		tCreation;		/* time of creation */
	time_t		tModification;		/* time of last modification */
} ADBINFO;

/* author information structure */
typedef struct _AUTHORINFO {
	char		szGivenName[AUTHOR_MAX_GNAME + 1];	/* given name */
	char		szSurname[AUTHOR_MAX_SURNAME + 1];	/* surname */
	char		szBirthYear[AUTHOR_MAX_BIRTH + 1];	/* year of birth */
	char		szDeathYear[AUTHOR_MAX_DEATH + 1];	/* year of death */
} AUTHORINFO;

/* author search structure */
typedef struct _AUTHORSEARCH {
	AUTHORDB	*padb;					/* database to search */
	char		szLastCode[AUTHOR_MAX_CODE + 1];	/* last code found */
	char		szSearch[AUTHOR_MAX_SEARCH + 1];	/* search string */
#ifndef GENERIC
	regex_t		rxSearch;				/* compiled search string */
#endif
} AUTHORSEARCH;

extern const char szAuthorExt[];

/* functions in adb.c */
void	AuthorNullifyDB(AUTHORDB *);
int	AuthorOpenDB(char *, AUTHORDB *, int);
int	AuthorDBStat(char *, ADBINFO *);
void	AuthorReorganiseDB(AUTHORDB *);
void	AuthorCloseDB(AUTHORDB *);
void	AuthorEmptyDB(AUTHORDB *);

/* functions in authors.c */
void	AuthorAddAuthor(AUTHORDB *, char *, AUTHORINFO *, char *);
int	AuthorGetAuthor(AUTHORDB *, char *, AUTHORINFO **, char **);
void	AuthorDeleteAuthor(AUTHORDB *, char *);
void	AuthorFreeAuthor(AUTHORINFO **, char **);
int	AuthorExists(AUTHORDB *, char *);

/* functions in asearch.c */
int	AuthorFullSearchInit(AUTHORDB *, AUTHORSEARCH *);
int	AuthorFullSearchNext(AUTHORSEARCH *);
void	AuthorFullSearchEnd(AUTHORSEARCH *);
int	AuthorDescSearchInit(AUTHORDB *, char *, AUTHORSEARCH *);
int	AuthorDescSearchNext(AUTHORSEARCH *);
void	AuthorDescSearchEnd(AUTHORSEARCH *);
int	AuthorNameSearchInit(AUTHORDB *, char *, AUTHORSEARCH *);
int	AuthorNameSearchNext(AUTHORSEARCH *);
void	AuthorNameSearchEnd(AUTHORSEARCH *);

#endif
