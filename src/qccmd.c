/*
 * qccmd.c
 *
 * Routines for parsing quoterc command files.
 *
 *      Created: 13th September, 1998
 * Version 1.00: 11th December, 1998
 *
 * (C) 1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <xtype.h>
#include "general.h"
#include "quoterc.h"


/* delimiters */
static const char *pszDelimSpace = " \t\r\n";		/* white space only */
static const char *pszDelimLeftBrace = " \t\r\n{";	/* white space plus left brace, for section header */
static const char *pszDelimSemiColon = " \t\r\n;";	/* white space plus semi-colon, for directives */


int QCProcessCommands(FILE *f, QCERROR *pqe)
/*
 * Process a command file.
 *
 * FILE *f	- the stream handle of the file to be processed
 * QCERROR *pqe	- an error structure (output)
 *
 * Return		- QC_E* code from quoterc.h
 */
{
	XSTR *		xstrLine;	/* one line of the input file */
	int		flMode;		/* flags */
	AUTHORDB	adb;		/* author database */
	QUOTEDB		qdb;		/* quote database */
	char *		pszToken;	/* pointer to next token in input */

	/* initialise variables */
	QuoteNullifyDB(&qdb);
	AuthorNullifyDB(&adb);
	pqe->iLine = 0;
	pqe->iError = QC_ENONE;

	/* allocate initial memory */
	if ((xstrLine = xstrnew(80, 20)) == NULL)
		pqe->iError = QC_ENOMEM;

	while ((pqe->iError == QC_ENONE) && !feof(f)) {
		/* read one line from the file */
		if (fgetxstr(xstrLine, f) == NULL) {
			if (feof(f))
				break;
			else if (ferror(f))
				pqe->iError = QC_EFILE;
			else
				pqe->iError = QC_ENOMEM;
		} else {
			pqe->iLine++;
		}

		flMode = 0;
		if ((pszToken = strtok(xstrcast(xstrLine), pszDelimSpace)) != NULL) {

			/* read "compile" or "decompile" directive */
			if (strcmpci(pszToken, "compile") == 0)
				flMode |= QC_FLCOMPILE;
			else if (strcmpci(pszToken, "decompile") == 0)
				flMode |= QC_FLDECOMPILE;
			else
				pqe->iError = QC_EBADCMD;

			/* read "quotes" or "authors" descriptor */
			if (pqe->iError == QC_ENONE) {
				if ((pszToken = strtok(NULL, pszDelimSpace)) == NULL)
					pqe->iError = QC_ENODBTYPE;
				else if (strcmpci(pszToken, "authors") == 0)
					flMode |= QC_FLAUTHOR;
				else if (strcmpci(pszToken, "quotes") == 0)
					flMode |= QC_FLQUOTE;
				else
					pqe->iError = QC_EBADDBTYPE;
			}

			/* read database name */
			if (pqe->iError == QC_ENONE) {
				if ((pszToken = strtok(NULL, pszDelimLeftBrace)) == NULL)
					pqe->iError = QC_ENOFILEN;
			}

			/* open database */
			if (pqe->iError == QC_ENONE) {
				if (flMode & QC_FLQUOTE) {
					if (!QuoteOpenDB(pszToken, &qdb, (flMode & QC_FLCOMPILE)? S_IWRITE : S_IREAD))
						pqe->iError = QC_EOPEN;
				} else {
					if (!AuthorOpenDB(pszToken, &adb, (flMode & QC_FLCOMPILE)? S_IWRITE : S_IREAD))
						pqe->iError = QC_EOPEN;
				}
			}

			/* process command block */
			if (pqe->iError == QC_ENONE) {
				if (flMode & QC_FLQUOTE) {
					QCProcessQuoteSection(f, &qdb, flMode, pqe, xstrLine);
				} else {
					QCProcessAuthorSection(f, &adb, flMode, pqe, xstrLine);
				}
			}

			/* close database */
			if (adb.dbfInfo != NULL)
				AuthorCloseDB(&adb);
			if (qdb.dbfInfo != NULL)
				QuoteCloseDB(&qdb);
		}
	}

	/* clean up */
	xstrfree(xstrLine);

	return (pqe->iError);
}


int QCProcessAuthorSection(FILE *fCmd, AUTHORDB *padb, int flMode, QCERROR *pqe, XSTR *xstrLine)
/*
 * Process the commands inside an author database section.
 *
 * FILE *fCmd		- stream handle of command file
 * AUTHORDB *padb	- the auther database we're using
 * int flMode		- mode (QC_FLCOMPILE, QCFL_DECOMPILE)
 * QCERROR *pqe		- error structure (output)
 * XSTR *xstrLine	- line buffer
 *
 * Returns		- error code from quoterc.h
 */
{
	char *	pszKeyword;			/* keyword */
	char *	pszArg1;			/* file name or code stem */
	char *	pszFormat;			/* format string */
	int	bStem;				/* are we changing the code stem? */
	char	szStem[AUTHOR_MAX_CODE + 1];	/* author code stem */
	int	bDone;				/* loop invariant */

	/* initialise variables */
	pszArg1 = NULL;
	pszFormat = NULL;
	szStem[0] = '\0';
	pqe->iError = QC_ENONE;
	bDone = 0;

	while (!bDone && (pqe->iError == QC_ENONE)) {
		/* initialise flags for this line */
		flMode &= ~(QC_FLCREATE | QC_FLAPPEND);
		bStem = 0;

		/* read one line from the file */
		if (fgetxstr(xstrLine, fCmd) == NULL) {
			if (feof(fCmd))
				break;
			else if (ferror(fCmd))
				pqe->iError = QC_EFILE;
			else
				pqe->iError = QC_ENOMEM;
		} else {
			pqe->iLine++;
		}

		if ((pszKeyword = stresctok(xstrcast(xstrLine), pszDelimSemiColon, '%')) != NULL) {
			/* read directive */
			if (strcmpci(pszKeyword, "create") == 0)
				flMode |= QC_FLCREATE;
			else if (strcmpci(pszKeyword, "append") == 0)
				flMode |= QC_FLAPPEND;
			else if (strcmpci(pszKeyword, "stem") == 0)
				bStem = 1;
			else if (pszKeyword[0] == '}')
				bDone = 1;
			else
				pqe->iError = QC_EBADCMD;

			/* read first argument */
			if (!bDone && (pqe->iError == QC_ENONE)) {
				if ((pszArg1 = stresctok(NULL, pszDelimSemiColon, '%')) == NULL)
					pqe->iError = QC_ENOARG;
				else
					strrmchr(pszArg1, '%');
			}

			/* read format string, if required */
			if (!bDone && (pqe->iError == QC_ENONE) && ((flMode & QC_FLCREATE) || (flMode & QC_FLAPPEND))) {
				if ((pszFormat = stresctok(NULL, pszDelimSemiColon, '%')) == NULL)
					pqe->iError = QC_ENOFORMAT;
			}

			if (!bDone && (pqe->iError == QC_ENONE) && bStem) {
				/* change automatic author code stem */
				strtncpy(szStem, pszArg1, AUTHOR_MAX_CODE);
			}

			if (!bDone && (pqe->iError == QC_ENONE) && ((flMode & QC_FLCREATE) || (flMode & QC_FLAPPEND))) {
				/* compile or de-compile */
				if (!bDone && (pqe->iError == QC_ENONE)) {
					if (flMode & QC_FLCOMPILE)
						pqe->iError = QCCompileAuthors(padb, pszArg1, pszFormat, szStem, flMode);
					else
						pqe->iError = QCDecompileAuthors(padb, pszArg1, pszFormat, flMode);
				}
			}
		}
	}

	return (pqe->iError);
}


int QCProcessQuoteSection(FILE *fCmd, QUOTEDB *pqdb, int flMode, QCERROR *pqe, XSTR *xstrLine)
/*
 * Process the commands inside a quote database section.
 *
 * FILE *fCmd		- stream handle of command file
 * QUOTEDB *pqdb	- the quote database we're using
 * int flMode		- mode (QC_FLCOMPILE, QC_FLDECOMPILE)
 * QCERROR *pqe		- error structure (output)
 * XSTR *xstrLine	- line buffer
 *
 * Returns		- error code from quoterc.h
 */
{
	char *		pszKeyword;			/* keyword */
	char *		pszArg1;			/* code or file name */
	char *		pszFormat;			/* format string */
	int		bAuthorDB;			/* are we opening a new author database? */
	int		bAuthor;			/* are we changing the current author code? */
	int		bSource;			/* are we changing the current source? */
	int		bStem;				/* are we changing the code stem? */
	AUTHORDB	adb;				/* author database */
	char		szAuthor[AUTHOR_MAX_CODE + 1];	/* author code */
	char		szSource[QUOTE_MAX_SOURCE + 1];	/* quote source */
	char		szStem[QUOTE_MAX_CODE + 1];	/* quote code stem */
	int		bDone;				/* loop invariant */

	/* initialise variables */
	pszArg1 = NULL;
	pszFormat = NULL;
	AuthorNullifyDB(&adb);
	szAuthor[0] = '\0';
	szSource[0] = '\0';
	szStem[0] = '\0';
	pqe->iError = QC_ENONE;
	bDone = 0;

	while (!bDone && (pqe->iError == QC_ENONE)) {
		/* initialise flags for this line */
		flMode &= ~(QC_FLCREATE | QC_FLAPPEND);
		bAuthorDB = 0;
		bAuthor = 0;
		bSource = 0;
		bStem = 0;

		/* read one line from the file */
		if (fgetxstr(xstrLine, fCmd) == NULL) {
			if (feof(fCmd))
				break;
			else if (ferror(fCmd))
				pqe->iError = QC_EFILE;
			else
				pqe->iError = QC_ENOMEM;
		} else {
			pqe->iLine++;
		}

		if ((pszKeyword = stresctok(xstrcast(xstrLine), pszDelimSemiColon, '%')) != NULL) {
			/* read directive */
			if (strcmpci(pszKeyword, "create") == 0)
				flMode |= QC_FLCREATE;
			else if (strcmpci(pszKeyword, "append") == 0)
				flMode |= QC_FLAPPEND;
			else if (strcmpci(pszKeyword, "authors") == 0)
				bAuthorDB = 1;
			else if (strcmpci(pszKeyword, "author") == 0)
				bAuthor = 1;
			else if (strcmpci(pszKeyword, "source") == 0)
				bSource = 1;
			else if (strcmpci(pszKeyword, "stem") == 0)
				bStem = 1;
			else if (pszKeyword[0] == '}')
				bDone = 1;
			else
				pqe->iError = QC_EBADCMD;

			/* read first argument */
			if (!bDone && (pqe->iError == QC_ENONE)) {
				if ((pszArg1 = stresctok(NULL, pszDelimSemiColon, '%')) == NULL)
					pqe->iError = QC_ENOARG;
				else
					strrmchr(pszArg1, '%');
			}

			/* read format string, if required */
			if (!bDone && (pqe->iError == QC_ENONE) && ((flMode & QC_FLCREATE) || (flMode & QC_FLAPPEND))) {
				if ((pszFormat = stresctok(NULL, pszDelimSemiColon, '%')) == NULL)
					pqe->iError = QC_ENOFORMAT;
			}

			if (!bDone && (pqe->iError == QC_ENONE) && bAuthorDB) {
				/* open a new author database */
				if (adb.dbfInfo != NULL)
					AuthorCloseDB(&adb);
				if (!AuthorOpenDB(pszArg1, &adb, (flMode & QC_FLCOMPILE)? S_IWRITE : S_IREAD))
					pqe->iError = QC_EOPEN;
			}

			if (!bDone && (pqe->iError == QC_ENONE) && bAuthor) {
				/* change current author */
				strtncpy(szAuthor, pszArg1, AUTHOR_MAX_CODE);
			}

			if (!bDone && (pqe->iError == QC_ENONE) && bSource) {
				/* change current source */
				strtncpy(szSource, pszArg1, QUOTE_MAX_SOURCE);
			}

			if (!bDone && (pqe->iError == QC_ENONE) && bStem) {
				/* change automatic quote code stem */
				strtncpy(szStem, pszArg1, QUOTE_MAX_CODE);
			}

			if (!bDone && (pqe->iError == QC_ENONE) && ((flMode & QC_FLCREATE) || (flMode & QC_FLAPPEND))) {
				/* compile or de-compile */
				if (flMode & QC_FLCOMPILE)
					pqe->iError = QCCompileQuotes(pqdb, pszArg1, pszFormat, szStem, szSource, szAuthor, flMode);
				else
					pqe->iError = QCDecompileQuotes(pqdb, &adb, pszArg1, pszFormat, flMode);
			}
		}
	}

	/* clean up */
	if (adb.dbfInfo != NULL)
		AuthorCloseDB(&adb);

	return (pqe->iError);
}


int QCCompileAuthors(AUTHORDB *padb, char *pszFileName, char *pszFormat, char *pszStem, int flMode)
/*
 * Compile an author database from a text file.
 *
 * AUTHORDB *padb	- author database to compile into
 * char *pszFileName	- name of file to compile
 * char *pszFormat	- format string
 * char *pszStem	- stem for automatic author codes
 * int flMode		- flags (QC_FLCREATE, QC_FLAPPEND)
 *
 * Returns		- error code from quoterc.h
 */
{
	FILE *		f;					/* stream handle of input file */
	QCBLOCK *	pqb;					/* block read from input */
	int		iAuthorCode;				/* counter for generating author codes */
	int		iAutoNumWidth;				/* width of automatic number */
	char		szAuthorCode[QUOTE_MAX_CODE + 1];	/* generated author codes */
	int		iError;					/* return value */

	/* initialise variables */
	f = NULL;
	iAuthorCode = 0;
	iAutoNumWidth = AUTHOR_MAX_CODE - strlen(pszStem);
	iError = QC_ENONE;

	/* verify format string */
	iError = QCVerifyFormat(pszFormat, flMode);

	if (iError == QC_ENONE) {
		if (flMode & QC_FLCREATE) {
			AuthorEmptyDB(padb);
		}
	}

	/* open the input file */
	if (iError == QC_ENONE) {
		if ((f = fopen(pszFileName, "r")) == NULL)
			iError = QC_EOPEN;
	}

	/* choose the initial quote code */
	if (iError == QC_ENONE) {
		do {
			iAuthorCode++;
			sprintf(szAuthorCode, "%s%0*d", pszStem, iAutoNumWidth, iAuthorCode);
		} while (AuthorExists(padb, szAuthorCode));
	}

	/* read quotes */
	while ((iError == QC_ENONE) && !feof(f)) {
		if ((pqb = QCReadBlock(f, pszFormat)) != NULL) {
			AuthorAddAuthor(padb, (pqb->pszAuthorCode)? pqb->pszAuthorCode : szAuthorCode, pqb->pai, pqb->pszAuthorDesc);
			if (!pqb->pszAuthorCode) {
				do {
					iAuthorCode++;
					sprintf(szAuthorCode, "%s%0*d", pszStem, iAutoNumWidth, iAuthorCode);
				} while (AuthorExists(padb, szAuthorCode));
			}
			QCFreeBlock(pqb);
		} else if (ferror(f)) {
			iError = QC_EFILE;
		} else if (!feof(f)) {
			iError = QC_ENOMEM;
		}
	}

	/* clean up */
	if (f != NULL)
		fclose(f);

	return (iError);
}


int QCDecompileAuthors(AUTHORDB *padb, char *pszFileName, char *pszFormat, int flMode)
/*
 * De-compile an author database to a text file.
 *
 * AUTHORDB *padb	- author database to de-compile
 * char *pszFileName	- name of file to de-compile to
 * char *pszFormat	- format string
 * int flMode		- flags (QC_FLCREATE, QC_FLAPPEND)
 *
 * Returns		- error code from quoterc.h
 */
{
	FILE *		f;	/* stream handle for output */
	AUTHORSEARCH	as;	/* search structure */
	QCBLOCK		qb;	/* output block */
	int		bDone;	/* loop invariant */
	int		iError;	/* return value */

	/* initialise variables */
	f = NULL;
	iError = QC_ENONE;

	/* validate format string and open output file */
	if ((iError = QCVerifyFormat(pszFormat, flMode)) == QC_ENONE) {
		if ((f = fopen(pszFileName, (flMode & QC_FLCREATE)? "w" : "a")) == NULL)
			iError = QC_EOPEN;
	}

	/* traverse database */
	bDone = !AuthorFullSearchInit(padb, &as);
	while ((iError == QC_ENONE) && !bDone) {
		/* get next author */
		qb.pszAuthorCode = as.szLastCode;
		AuthorGetAuthor(padb, as.szLastCode, &qb.pai, &qb.pszAuthorDesc);

		/* output this author */
		QCWriteBlock(f, pszFormat, &qb);

		/* move on to next author */
		AuthorFreeAuthor(&qb.pai, &qb.pszAuthorDesc);
		bDone = !AuthorFullSearchNext(&as);
	}

	/* clean up */
	AuthorFullSearchEnd(&as);
	if (f != NULL)
		fclose(f);

	return (iError);
}


int QCCompileQuotes(QUOTEDB *pqdb, char *pszFileName, char *pszFormat, char *pszStem, char *pszSource, char *pszAuthor, int flMode)
/*
 * Compile a quote database from a text file.
 *
 * QUOTEDB *pqdb	- the quote database to be compiled into
 * char *pszFileName	- the name of the file to compile
 * char *pszFormat	- format string
 * char *pszStem	- stem for automatic quote codes
 * char *pszSource	- source
 * char *pszAuthor	- author code
 * int flMode		- flags (QC_FLAPPEND, QC_FLCREATE)
 *
 * Returns		- error code from quoterc.h
 */
{
	FILE *		f;					/* stream handle of input file */
	QCBLOCK *	pqb;					/* block read from input */
	int		iQuoteCode;				/* counter for generating quote codes */
	int		iAutoNumWidth;				/* width of automatic number */
	char		szQuoteCode[QUOTE_MAX_CODE + 1];	/* generated quote codes */
	int		iError;					/* return value */

	/* initialise variables */
	f = NULL;
	iQuoteCode = 0;
	iAutoNumWidth = QUOTE_MAX_CODE - strlen(pszStem);
	iError = QC_ENONE;

	/* verify format string */
	iError = QCVerifyFormat(pszFormat, flMode);

	if (iError == QC_ENONE) {
		if (flMode & QC_FLCREATE) {
			QuoteEmptyDB(pqdb);
		}
	}

	/* open the input file */
	if (iError == QC_ENONE) {
		if ((f = fopen(pszFileName, "r")) == NULL)
			iError = QC_EOPEN;
	}

	/* choose the initial quote code */
	if (iError == QC_ENONE) {
		do {
			iQuoteCode++;
			sprintf(szQuoteCode, "%s%0*d", pszStem, iAutoNumWidth, iQuoteCode);
		} while (QuoteExists(pqdb, szQuoteCode));
	}

	/* read quotes */
	while ((iError == QC_ENONE) && !feof(f)) {
		if ((pqb = QCReadBlock(f, pszFormat)) != NULL) {
			/* sort out source author details */
			if (pqb->pszAuthorCode && (pqb->pszAuthorCode[0] != '\0'))
				strtncpy(pqb->pqi->szAuthorCode, pqb->pszAuthorCode, AUTHOR_MAX_CODE);
			else
				strcpy(pqb->pqi->szAuthorCode, pszAuthor);
			if (pqb->pqi->szSource[0] == '\0')
				strcpy(pqb->pqi->szSource, pszSource);

			/* add quote to database */
			QuoteAddQuote(pqdb, (pqb->pszQuoteCode)? pqb->pszQuoteCode : szQuoteCode, pqb->pqi, pqb->pszQuoteText);
			if (!pqb->pszQuoteCode) {
				do {
					iQuoteCode++;
					sprintf(szQuoteCode, "%s%0*d", pszStem, iAutoNumWidth, iQuoteCode);
				} while (QuoteExists(pqdb, szQuoteCode));
			}
			QCFreeBlock(pqb);
		} else if (ferror(f)) {
			iError = QC_EFILE;
		} else if (!feof(f)) {
			iError = QC_ENOMEM;
		}
	}

	/* clean up */
	if (f != NULL)
		fclose(f);

	return (iError);
}


int QCDecompileQuotes(QUOTEDB *pqdb, AUTHORDB *padb, char *pszFileName, char *pszFormat, int flMode)
/*
 * De-compile a quote database into a text file.
 *
 * QUOTEDB *pqdb	- the quote database to be de-compiled
 * AUTHORDB *padb	- an author database to reference
 * char *pszFileName	- the name of the file to de-compile into
 * char *pszFormat	- format string
 * int flMode		- flags
 *
 * Returns		- error code from quoterc.h
 */
{
	FILE *		f;	/* stream handle for output */
	QUOTESEARCH	qs;	/* search structure */
	QCBLOCK		qb;	/* output block */
	int		bDone;	/* loop invariant */
	int		iError;	/* return value */

	/* initialise variables */
	f = NULL;
	iError = QC_ENONE;

	/* validate format string and open output file */
	if (padb->dbfInfo != NULL)
		flMode |= QC_FLAUTHOR;
	if ((iError = QCVerifyFormat(pszFormat, flMode)) == QC_ENONE) {
		if ((f = fopen(pszFileName, (flMode & QC_FLCREATE)? "w" : "a")) == NULL)
			iError = QC_EOPEN;
	}

	/* traverse database */
	bDone = !QuoteFullSearchInit(pqdb, &qs);
	while ((iError == QC_ENONE) && !bDone) {
		/* get next quote */
		qb.pszQuoteCode = qs.szLastCode;
		QuoteGetQuote(pqdb, qs.szLastCode, &qb.pqi, &qb.pszQuoteText);
		qb.pszAuthorCode = qb.pqi->szAuthorCode;

		/* get the associated author */
		qb.pai = NULL;
		qb.pszAuthorDesc = NULL;
		if ((qb.pqi->szAuthorCode[0] != '\0') && (padb->dbfInfo != NULL))
			AuthorGetAuthor(padb, qb.pqi->szAuthorCode, &qb.pai, &qb.pszAuthorDesc);

		/* output this quote */
		QCWriteBlock(f, pszFormat, &qb);

		/* move on to next author */
		QuoteFreeQuote(&qb.pqi, &qb.pszQuoteText);
		AuthorFreeAuthor(&qb.pai, &qb.pszAuthorDesc);
		bDone = !QuoteFullSearchNext(&qs);
	}

	/* clean up */
	QuoteFullSearchEnd(&qs);
	if (f != NULL)
		fclose(f);

	return (iError);
}


/* error strings */
char *aszQCError[] = {
	"no error",					/* QC_ENONE */
	"out of memory",				/* QC_ENOMEM */
	"file error",					/* QC_EFILE */
	"unrecognised command keyword",			/* QC_EBADCMD */
	"database type missing",			/* QC_ENODBTYPE */
	"unrecognised database type",			/* QC_EBADDBTYPE */
	"missing file name",				/* QC_ENOFILEN */
	"could not open file",				/* QC_EOPEN */
	"missing format string",			/* QC_ENOFORMAT */
	"invalid data item",				/* QC_EBADITEM */
	"permission denied",				/* QC_EPERM */
	"missing argument",				/* QC_ENOARG */
	"format doesn't start with a data item",	/* QC_EBAD1STITEM */
	"undelimited data items",			/* QC_ENODELIM */
	"unknown error"					/* QC_EUNKNOWN */
};


char *QCErrorString(int iError)
/*
 * Get a pointer to a string describing an error (like strerror()).
 *
 * int iError	- the error code (QC_E*)
 *
 * Returns	- a pointer a string describing error iError
 */
{
	if ((iError < 0) || (iError > QC_EUNKNOWN))
		return (aszQCError[QC_EUNKNOWN]);
	else
		return (aszQCError[iError]);
}
