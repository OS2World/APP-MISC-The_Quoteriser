/*
 * qcdata.c
 *
 * Routines for parsing quoterc data files.
 *
 *      Created: 12th September, 1998
 * Version 1.00: 11th December, 1998
 *
 * (C) 1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xtype.h>
#include "authors.h"
#include "general.h"
#include "quoterc.h"
#include "quotes.h"


void QCFreeBlock(QCBLOCK *pqb)
/*
 * Reclaim the memory created by a call to QCReadBlock().
 *
 * QCBLOCK *pqb	- a QCBLOCK structure from QCReadBlock()
 */
{
	if (pqb != NULL) {
		free(pqb->pszAuthorCode);
		free(pqb->pszQuoteCode);
		free(pqb->pai);
		free(pqb->pqi);
		free(pqb->pszAuthorDesc);
		free(pqb->pszQuoteText);
	}
}


char *QCMatchString(XSTR *xstrInput, char *pszFormat, XSTR *xstrOutput)
/*
 * Try to match a string. The input string will be changed to be the first
 * sequence of symbols that could match the format, and the rest will be
 * appended to the output string.
 *
 * XSTR *xstrInput	- input string (will be altered)
 * char *pszFormat	- the format string to match
 * XSTR *xstrOutput	- output string
 *
 * Returns		- NULL, if there was a memory allocation failure
 *			  the part of pszFormat which we have matched up to, otherwise
 */
{
	char	*pszMatch;	/* return value */
	char	*pchInput;	/* counter in xstrInput */
	char	*pch;		/* working variable */
	int	bAbort;		/* abort the process? */

	/* initialise variables */
	pszMatch = pszFormat;
	pchInput = xstrcast(xstrInput);
	bAbort = 0;

	while (!bAbort && *pszMatch && *pchInput) {
		if ((pch = QCMatchSymbol(pszMatch, *pchInput)) == NULL) {
			/* no match; start searching for a new match */
			pszMatch = pszFormat;
			while (*pchInput && ((pch = QCMatchSymbol(pszMatch, *pchInput)) == NULL))
				pchInput++;
			if (pch != NULL)
				pszMatch = pch;

			/* move the unmatched section to the output */
			if (xstrncat(xstrOutput, xstrcast(xstrInput), pchInput - xstrcast(xstrInput) + 1) == NULL)
				bAbort = 1;
			else {
				xstrdel(xstrInput, 0, pchInput - xstrcast(xstrInput) - 1);
				pchInput = xstrcast(xstrInput);
			}
		} else {
			pszMatch = pch;
			pchInput++;
		}
	}

	return ((bAbort)? NULL : pszMatch);
}


char *QCMatchSymbol(char *pszFormat, char ch)
/*
 * See if ch matches a given format string.
 *
 * char *pszFormat	- the format string to match
 * char ch		- the character to match
 *
 * Returns		- NULL, if the ch does not match pszFormat
 *			  the point in pszFormat that we have matched up to, otherwise
 */
{
	char	*pszMatch;	/* return value */

	/* initialise variables */
	pszMatch = NULL;

	if (pszFormat[0] == '%') {
		/* special character */
		switch (pszFormat[1]) {
			case 'n':
				/* new line */
				if (ch == '\n')
					pszMatch = pszFormat + 2;
				break;

			case '_':
				/* white space */
				if (isspace(ch))
					pszMatch = pszFormat + 2;
				break;

			case ';':
				/* semi-colon */
				if (ch == ';')
					pszMatch = pszFormat + 2;
				break;

			case '>':
				/* tab */
				if (ch == '\t')
					pszMatch = pszFormat + 2;
				break;

			default:
				/* escaped character */
				if (pszFormat[1] && (ch == pszFormat[1]))
					pszMatch = pszFormat + 2;
				break;
		}
	} else {
		/* normal character */
		if (ch == pszFormat[0])
			pszMatch = pszFormat + 1;
	}

	return (pszMatch);
}


QCBLOCK *QCReadBlock(FILE *f, char *pszFormat)
/*
 * Read a quote/author from a stream in a given format. The structure returned
 * from this function should be de-allocated with QCFreeBlock().
 *
 * FILE *f		- the stream handle to read from
 * char *pszFormat	- the format to read in
 *
 * Returns		- NULL, if there was an error (memory allocation if !feof() and !ferror())
 *			  (a pointer to) a new QCBLOCK containing the block, otherwise
 */
{
	QCBLOCK *	pqb;		/* return value */
	char *		pszSep;		/* string separtor one input makrer from the next */
	char *		pszCurrent;	/* current position in pszFormat */
	char *		pszNext;	/* place-holder in pszFormat */
	XSTR *		xstr;		/* read buffer */
	int		bDone;		/* have we found the next input marker yet? */
	int		bTrivial;	/* is the input trivial? */
	int		bStopped;	/* has the process been stopped (by end-of-file)? */
	int		bFatal;		/* has the process been fatally wounded? */

	/* initialise variables */
	pqb = NULL;
	pszSep = NULL;
	pszCurrent = pszFormat;
	bTrivial = 1;
	bStopped = 0;
	bFatal = 0;

	/* allocate memory */
	if ((pqb = (QCBLOCK *)malloc(sizeof(QCBLOCK))) == NULL)
		bFatal = 1;
	else {
		memset(pqb, 0, sizeof(QCBLOCK));
		if ((pqb->pai = (AUTHORINFO *)malloc(sizeof(AUTHORINFO))) == NULL)
			bFatal = 1;
		else if ((pqb->pqi = (QUOTEINFO *)malloc(sizeof(QUOTEINFO))) == NULL)
			bFatal = 1;
		else {
			memset(pqb->pai, 0, sizeof(AUTHORINFO));
			memset(pqb->pqi, 0, sizeof(QUOTEINFO));
		}
	}
	if (!bFatal) {
		if ((pszSep = (char *)malloc(strlen(pszFormat) + 1)) == NULL)
			bFatal = 1;
	}

	while (!bFatal && !bStopped && (*pszCurrent)) {
		/* obtain the separator */
		pszNext = pszCurrent + 2;
		bDone = 0;
		while (!bDone && *pszNext) {
			if (*pszNext == '%') {
				switch (pszNext[1]) {
					case 'a':
					case 'b':
					case 'd':
					case 'f':
					case 'g':
					case 'q':
					case 's':
					case 't':
					case 'x':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
						/* input item; stop */
						bDone = 1;
						break;

					default:
						/* some special character; skip it */
						pszNext += 2;
						break;
				}
			} else {
				/* ordinary character; continue */
				pszNext++;
			}
		}
		strtncpy(pszSep, pszCurrent + 2, pszNext - pszCurrent - 1);

		/* read the input item */
		if ((xstr = xstrnew(10, 10)) == NULL)
			bFatal = 1;
		else if (QCReadItem(f, pszSep, xstr) ==  NULL) {
			if (ferror(f) || !feof(f))
				bFatal = 1;
			else
				bStopped = 1;
		} else if (xstrlen(xstr) > 0) {
			bTrivial = 0;
		}
		if (!bFatal) {
			switch (pszCurrent[1]) {
				case 'a':
					xstrtrunc(xstr, AUTHOR_MAX_CODE);
					pqb->pszAuthorCode = xstrcvt(xstr);
					break;

				case 'b':
					strtncpy(pqb->pai->szBirthYear, xstrcast(xstr), AUTHOR_MAX_BIRTH);
					xstrfree(xstr);
					break;

				case 'd':
					pqb->pszAuthorDesc = xstrcvt(xstr);
					break;

				case 'f':
					strtncpy(pqb->pai->szGivenName, xstrcast(xstr), AUTHOR_MAX_GNAME);
					xstrfree(xstr);
					break;

				case 'g':
					strtncpy(pqb->pai->szSurname, xstrcast(xstr), AUTHOR_MAX_SURNAME);
					xstrfree(xstr);
					break;

				case 'q':
					xstrtrunc(xstr, QUOTE_MAX_CODE);
					pqb->pszQuoteCode = xstrcvt(xstr);
					break;

				case 's':
					strtncpy(pqb->pqi->szSource, xstrcast(xstr), QUOTE_MAX_SOURCE);
					xstrfree(xstr);
					break;

				case 't':
					pqb->pszQuoteText = xstrcvt(xstr);
					break;

				case 'x':
					strtncpy(pqb->pai->szDeathYear, xstrcast(xstr), AUTHOR_MAX_DEATH);
					xstrfree(xstr);
					break;

				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
					strtncpy(pqb->pqi->aszKeyword[pszCurrent[1] - '1'], xstrcast(xstr), QUOTE_MAX_KEYWORD);
					xstrfree(xstr);
					break;
			}
		}

		pszCurrent = pszNext;
	}

	/* clean up */
	free(pszSep);

	if (bStopped && *pszCurrent) {
		/* stopped before reading a whole block */
		bFatal = 1;
	}
	if (bTrivial && feof(f)) {
		/* garbage at end of file */
		bFatal = 1;
	}

	if (bFatal) {
		/* back out of memory allocation */
		QCFreeBlock(pqb);
		pqb = NULL;
	}

	return (pqb);
}


XSTR *QCReadItem(FILE *f, char *pszFormat, XSTR *xstr)
/*
 * Read a file into a string until a particular sequence of symbols is found.
 *
 * FILE *f		- the stream handle to read from
 * char *pszFormat	- the format string terminating the input
 * XSTR *xstr		- the string read (output)
 *
 * Returns		- NULL, if there is an error (memory allocation if !feof() and !ferror())
 *			  xstr, otherwise
 */
{
	int	ch;		/* read buffer */
	XSTR	*xstrBuffer;	/* match buffer */
	char	*pszMatch;	/* current position in pszFormat */
	char	*pch;		/* working variable */
	int	bAbort;		/* abort the process? */

	/* initialise variables */
	xstrBuffer = NULL;
	xstrcpy(xstr, "");
	pszMatch = pszFormat;
	bAbort = 0;

	/* allocate some initial memory */
	if ((xstrBuffer = xstrnew(10, 10)) == NULL)
		bAbort = 1;

	while (!bAbort && *pszMatch) {
		if ((ch = cgetc(f)) == EOF) {
			/* end-of-file or read error */
			bAbort = 1;
		} else if ((pch = QCMatchSymbol(pszMatch, ch)) == NULL) {
			/* not a match; append buffer to output */
			if (xstrcatc(xstrBuffer, (char)ch) == NULL)
				bAbort = 1;
			else if ((pszMatch = QCMatchString(xstrBuffer, pszFormat, xstr)) == NULL)
				bAbort = 1;
		} else if (xstrcatc(xstrBuffer, (char)ch) == NULL) {
			bAbort = 1;
		} else {
			pszMatch = pch;
		}
	}

	if (feof(f)) {
		/* append the remainder of the buffer to the output */
		if (xstrcat(xstr, xstrcast(xstrBuffer)) == NULL)
			bAbort = 1;
	}

	/* clean up */
	xstrfree(xstrBuffer);

	return ((bAbort)? NULL : xstr);
}


int QCVerifyFormat(char *pszFormat, int flMode)
/*
 * Verify the correctness of a format string.
 *
 * char *pszFormat	- the format string to verify
 * int flMode		- flags (QC_FLAUTHOR, QC_FLQUOTE)
 *
 * Returns		- error code from quoterc.h (QC_E*)
 */
{
	char	*pch;		/* counter in pszFormat */
	int	bDelimiter;	/* was the last character a delimiter? */
	int	bFirst;		/* are we on the first symbol? */
	int	iError;		/* return value */

	/* initialise variables */
	pch = pszFormat;
	bFirst = 1;
	bDelimiter = 1;
	iError = QC_ENONE;

	/* search the string */
	while (*pch && (iError == QC_ENONE)) {
		if (*pch == '%') {
			pch++;
			switch (*pch) {
				case 'n':
				case '%':
				case '_':
				case ';':
				case ' ':
				case '\t':
				case '>':
					/* legal for all uses, except as first symbol */
					bDelimiter = 1;
					if (bFirst)
						iError = QC_EBAD1STITEM;
					break;

				case 'b':
				case 'd':
				case 'f':
				case 'g':
				case 'x':
					/* legal only if we have authors */
					if (!(flMode & QC_FLAUTHOR))
						iError = QC_EBADITEM;
					else if (!bDelimiter)
						iError = QC_ENODELIM;
					bDelimiter = 1;
					break;

				case 'q':
				case 's':
				case 't':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
					/* legal only if we have quotes */
					if (!(flMode & QC_FLQUOTE))
						iError = QC_EBADITEM;
					else if (!bDelimiter)
						iError = QC_ENODELIM;
					bDelimiter = 0;
					break;

				case 'a':
					/* legal if we have either authors or quotes */
					if (!(flMode & (QC_FLAUTHOR | QC_FLQUOTE)))
						iError = QC_EBADITEM;
					else if (!bDelimiter)
						iError = QC_ENODELIM;
					bDelimiter = 0;
					break;
			}
		} else {
			bDelimiter = 1;
			if (bFirst) {
				/* first character is not an input item */
				iError = QC_EBAD1STITEM;
			}
		}
		bFirst = 0;
		pch++;
	}

	if (!bDelimiter) {
		/* no delimiter at the end of the block */
		iError = QC_ENODELIM;
	}

	return (iError);
}


void QCWriteBlock(FILE *f, char *pszFormat, QCBLOCK *pqb)
/*
 * Write an quote/author to a stream in a specified format.
 *
 * FILE *f		- the stream to write to
 * char *pszFormat	- the format string
 * QCBLOCK *pqb		- the quote and author information to write
 */
{
	char	*pch;	/* counter in pszFormat */

	/* initialise variables */
	pch = pszFormat;

	/* write out the block */
	while (*pch) {
		if (*pch == '%') {
			pch++;
			if (*pch == 'a') {
				if (pqb->pszAuthorCode)
					fprintf(f, "%s", pqb->pszAuthorCode);
			} else if (*pch == 'b') {
				if (pqb->pai)
					fprintf(f, "%s", pqb->pai->szBirthYear);
			} else if (*pch == 'd') {
				if (pqb->pszAuthorDesc)
					fprintf(f, "%s", pqb->pszAuthorDesc);
			} else if (*pch == 'f') {
				if (pqb->pai)
					fprintf(f, "%s", pqb->pai->szGivenName);
			} else if (*pch == 'g') {
				if (pqb->pai)
					fprintf(f, "%s", pqb->pai->szSurname);
			} else if (*pch == 'n') {
				fprintf(f, "\n");
			} else if (*pch == 'q') {
				if (pqb->pszQuoteCode)
					fprintf(f, "%s", pqb->pszQuoteCode);
			} else if (*pch == 's') {
				if (pqb->pqi)
					fprintf(f, "%s", pqb->pqi->szSource);
			} else if (*pch == 't') {
				if (pqb->pszQuoteText)
					fprintf(f, "%s", pqb->pszQuoteText);
			} else if (*pch == 'x') {
				if (pqb->pai)
					fprintf(f, "%s", pqb->pai->szDeathYear);
			} else if (*pch == '1') {
				if (pqb->pqi)
					fprintf(f, "%s", pqb->pqi->aszKeyword[0]);
			} else if (*pch == '2') {
				if (pqb->pqi)
					fprintf(f, "%s", pqb->pqi->aszKeyword[1]);
			} else if (*pch == '3') {
				if (pqb->pqi)
					fprintf(f, "%s", pqb->pqi->aszKeyword[2]);
			} else if (*pch == '4') {
				if (pqb->pqi)
					fprintf(f, "%s", pqb->pqi->aszKeyword[3]);
			} else if (*pch == '5') {
				if (pqb->pqi)
					fprintf(f, "%s", pqb->pqi->aszKeyword[4]);
			} else if (*pch == '_') {
				putc(' ', f);
			} else if (*pch == '>') {
				putc('\t', f);
			} else if (*pch) {
				putc(*pch, f);
			}
		} else {
			putc((int)(*pch), f);
		}
		pch++;
	}
}
