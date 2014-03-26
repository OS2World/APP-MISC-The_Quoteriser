/*
 * quoterc.h
 *
 * Header file for the Quoteriser database compiler.
 *
 *      Created: 12th September 1998
 * Version 1.00: 11th December 1998
 *
 * (C) 1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_QUOTERC_H
#define _QUOTERISER_QUOTERC_H

#include <stdio.h>
#include <xtype.h>
#include "authors.h"
#include "quotes.h"


/* error codes */
#define QC_ENONE	0	/* no error */
#define QC_ENOMEM	1	/* out of memory */
#define QC_EFILE	2	/* file error */
#define QC_EBADCMD	3	/* unrecognised command or keyword */
#define QC_ENODBTYPE	4	/* missing database type */
#define QC_EBADDBTYPE	5	/* unrecognised database type */
#define QC_ENOFILEN	6	/* missing file name */
#define QC_EOPEN	7	/* could not open file */
#define QC_ENOFORMAT	8	/* format string expected */
#define QC_EBADITEM	9	/* illegal input item */
#define QC_EPERM	10	/* permission denied */
#define QC_ENOARG	11	/* missing argument */
#define QC_EBAD1STITEM	12	/* format doesn't start with an input item */
#define QC_ENODELIM	13	/* undelimited input items */
#define QC_EUNKNOWN	14	/* unknown error */


/* mode flags */
#define QC_FLQUOTE	0x00000001	/* quotes */
#define QC_FLAUTHOR	0x00000002	/* authors */
#define QC_FLCOMPILE	0x00000004	/* compiling */
#define QC_FLDECOMPILE	0x00000008	/* de-compiling */
#define QC_FLCREATE	0x00000010	/* create/overwrite file */
#define QC_FLAPPEND	0x00000020	/* append to file */


typedef struct _QCERROR {
	int	iError;			/* error code */
	int	iLine;			/* line on which error occurred */
} QCERROR;


typedef struct _QCBLOCK {
	char		*pszAuthorCode;		/* author code */
	char		*pszQuoteCode;		/* quote code */
	AUTHORINFO	*pai;			/* author information */
	char		*pszAuthorDesc;		/* author description */
	QUOTEINFO	*pqi;			/* quote information */
	char		*pszQuoteText;		/* quote text */
} QCBLOCK;


/* functions in qccmd.c */
int	QCProcessCommands(FILE *, QCERROR *);					/* process command file */
int	QCProcessAuthorSection(FILE *, AUTHORDB *, int, QCERROR *, XSTR *);	/* process author section */
int	QCProcessQuoteSection(FILE *, QUOTEDB *, int, QCERROR *, XSTR *);	/* process quote section */
int	QCCompileAuthors(AUTHORDB *, char *, char *, char *, int);		/* compile author database */
int	QCDecompileAuthors(AUTHORDB *, char *, char *, int);			/* de-compile author database */
int	QCCompileQuotes(QUOTEDB *, char *, char *, char *, char *, char *, int);/* compile quote database */
int	QCDecompileQuotes(QUOTEDB *, AUTHORDB *, char *, char *, int);		/* de-compile quote database */
char *	QCErrorString(int);							/* get error string */


/* functions in qcdata.c */
void		QCFreeBlock(QCBLOCK *);						/* de-allocate QCBLOCK structure */
char *		QCMatchString(XSTR *, char *, XSTR *);				/* match a string */
char *		QCMatchSymbol(char *, char);					/* match a single symbol */
QCBLOCK *	QCReadBlock(FILE *, char *);					/* read a block of input */
XSTR *		QCReadItem(FILE *, char *, XSTR *);				/* read a unit of input */
int		QCVerifyFormat(char *, int);					/* test format string legality */
void		QCWriteBlock(FILE *, char *, QCBLOCK *);			/* write an author/quote block */

#endif
