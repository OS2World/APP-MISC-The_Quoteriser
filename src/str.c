/*
 * str.c
 *
 * Miscellaneous string-handling functions.
 *
 *      Created: 25th November, 1997 (was part of general.c)
 * Verison 2.00: 15th December, 1997
 * Version 2.10: 10th December, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xtype.h>
#include "general.h"


int cgetc(FILE *f)
/*
 * Hack to make getc() return '\n' for all end-of-line regimes (that this
 * author has heard of).
 *
 * FILE *f	- stream to read from
 *
 * Returns	- EOF on error
 *		  the character read, otherwise
 */
{
	static int	bCR = 0;	/* was the last character read '\r'? */
	int		ch;		/* return value */

	if ((ch = getc(f)) == '\r') {
		bCR = 1;
		ch = '\n';
	} else if (bCR) {
		bCR = 0;
		if (ch == '\n')
			ch = cgetc(f);
	}

	return (ch);
}


char *strabbrev(char *pszString, char *pszAbbrev, int iMaxLength)
/*
 * Truncate a string to iMaxLength characters, breaking at word boundaries only,
 * and appending an ellipsis to the truncated string if necessary.
 *
 * char *pszString	- the string to be abbreviated
 * char *pszAbbrev	- the abbreviated version of the string (output)
 * int iMaxLength	- the maximum length to abbreviate to (including any ellipses)
 *
 * Returns:		- pszAbbrev
 */
{
	char *pchSave, *pchProbe;
	int bDone, bEllipsis;

	/* start at the beginning of the string */
	pchProbe = pchSave = pszString;

	/* go through the string word-by-word until we go as far as we can */
	bDone = 0;
	bEllipsis = 1;
	while (!bDone) {

		/* find the end of the next word */
		while (!isspace(*pchProbe) && !ispunct(*pchProbe) && ((*pchProbe) != '\0'))
			pchProbe++;

		if (((*pchProbe) == '\0') && ((pchProbe - pszString) < iMaxLength)) {
			/* pszString has ended; stop and don't use an ellipsis */
			pchSave = pchProbe;
			bDone = 1;
			bEllipsis = 0;
		} if ((pchProbe - pszString) < (iMaxLength - 3)) {
			/* still inside limit; continue */
			pchSave = pchProbe++;
		} else {
			/* gone past the limit; stop and use an ellipsis */
			bDone = 1;
			bEllipsis = 1;
		}
	}

	/* form abbreviated string */
	strncpy(pszAbbrev, pszString, pchSave - pszString);
	pszAbbrev[pchSave - pszString] = '\0';
	if (bEllipsis)
		strcat(pszAbbrev, "...");

	return (pszAbbrev);
}


int strboxf(FILE *f, char **papszBox, int *aiWidth, int n)
/*
 * Print a series of strings to a file in the form of a series of boxes.
 * The first string in papsz will be in the leftmost box, and the last (nth)
 * string in the right-most box. Strings in excess of the given width for
 * their box will be truncated to fit.
 *
 * FILE *f		- the file to print to
 * char **papszBox	- the array of strings to put in the boxes
 * int *aiWidth		- array of widths for each box
 * int n		- the number of boxes
 *
 * Returns		- the number of lines written, if success
 *                        -1, if failure (memory allocation)
 */
{
	char *pszLine, **apchCurrent, *pch;
	int i, iCount;
	int iLines;
	int bDone;

	/* initialise variables */
	iLines = 0;

	/* stop now if papszBox is trivial */
	if (n < 1)
		return (0);

	/* allocate memory to hold a line (plus two bytes for EOL stuff) */
	iCount = 2;
	for (i = 0; i < n; i++)
		iCount += aiWidth[i];
	if ((pszLine = (char *)malloc(iCount)) == NULL)
		return (-1);

	/* initialise cursor array */
	if ((apchCurrent = (char **)malloc(n)) == NULL) {
		free(pszLine);
		return (-1);
	}
	for (i = 0; i < n; i++)
		apchCurrent[i] = papszBox[i];

	do {
		/* build up a line */
		pszLine[0] = '\0';
		bDone = 1;
		for (i = 0; i < n; i++) {
			if ((*apchCurrent[i]) != '\0') {

				/* still have some to go, so set done to false */
				bDone = 0;

				/* determine what goes in this box for this line */
				if ((pch = strchr(apchCurrent[i], '\n')) == NULL)
					pch = strchr(apchCurrent[i], '\0');
				iCount = pch - apchCurrent[i];
				if (iCount > aiWidth[i])
					iCount = aiWidth[i];

				/* append it to the line we're working on */
				strncat(pszLine, apchCurrent[i], iCount);

				/* update cursor for this box */
				apchCurrent[i] = pch;
				if ((*pch) == '\n')
					apchCurrent[i]++;

				/* pad with spaces out to the width of the box */
				pch = strchr(pszLine, '\0');
				for (iCount = aiWidth[i] - iCount; iCount > 0; iCount--) {
					(*pch) = ' ';
					pch++;
				}
				(*pch) = '\0';
			} else {
				pch = strchr(pszLine, '\0');
				for (iCount = aiWidth[i]; iCount > 0; iCount--) {
					(*pch) = ' ';
					pch++;
				}
				(*pch) = '\0';
			}
		}

		if (!bDone) {
			/* add the line to the file */
			iLines++;
			strcat(pszLine, "\n");
			fputs(pszLine, f);
		}
	} while (!bDone);

	/* clean up */
	free(pszLine);
	free(apchCurrent);

	return (iLines);
}


int strcmpci(const char *psz1, const char *psz2)
/*
 * Compare strings case-insensitively. Some compilers have stricmp() or
 * strcasecmp() for this, but we put it here so that everyone can have it.
 *
 * char *psz1	- the first string to compare
 * char *psz2	- the second string to compare
 *
 * Returns:	-1, if psz1 is lexicographically before psz2
 *		0, if psz1 and psz2 are equal
 *		1, if psz1 is lexicographically after psz2
 */
{
	const char *	pch1;	/* counter in psz1 */
	const char *	pch2;	/* counter in psz2 */

	pch1 = psz1;
	pch2 = psz2;
	while (toupper(*pch1) == toupper(*pch2) && (*pch1) && (*pch2)) {
		pch1++;
		pch2++;
	}

	if (toupper(*pch1) < toupper(*pch2))
		return (-1);
	else if (toupper(*pch1) > toupper(*pch2))
		return (1);

	return (0);
}


char *strencl(const char *pszString, char *pszDelimited, char chLeftDelim, char chRightDelim)
/*
 * Find text in a substring enclosed by given delimiters. The last set of delimiters will
 * be used.
 *
 * char *pszString	- the string to search in
 * char *pszDelimited	- the delimited substring (output)
 * char chLeftDelim	- the left-hand delimiter
 * char chRightDelim	- the right-hand delimiter
 *
 * Returns:		- pszDelimited if a pair of delimiters are found
 *			  NULL otherwise
 */
{
	char *pchLeft, *pchRight;

	if ((pchLeft = strrchr(pszString, chLeftDelim)) != NULL)
		if ((pchRight = strchr(pchLeft + 1, chRightDelim)) != NULL) {
			strncpy(pszDelimited, pchLeft + 1, pchRight - pchLeft - 1);
			pszDelimited[pchRight - pchLeft - 1] = '\0';
			return (pszDelimited);
		}

	return (NULL);
}


char *stresctok(char *pszString, const char *pszDelimiters, char chEscape)
/*
 * strtok() with an escape character.
 *
 * char *pszString	- the string to search in for tokens
 * char *pszDelimiters	- the token delimiters
 * char chEscape	- the escape character
 *
 * Returns:		- as strtok(), but escaped delimiters are ignored
 */
{
	static char	*pszCurrent;	/* the current token */
	char		*pszToken;	/* return value */

	if (pszString) {
		/* we have a new string to tokenise */
		pszCurrent = pszString;
	} else if (!pszCurrent) {
		/* no more tokens */
		return (NULL);
	}

	/* search for start of next token */
	pszToken = pszCurrent;
	while (*pszToken && strchr(pszDelimiters, *pszToken)) {
		pszToken++;
	}
	if (!(*pszToken)) {
		/* no more tokens */
		pszToken = NULL;
	}

	/* search for end of token */
	pszCurrent = pszToken;
	if (pszToken) {
		pszCurrent = pszCurrent;
		while (*pszCurrent && !strchr(pszDelimiters, *pszCurrent)) {
			if (*pszCurrent == chEscape)
				pszCurrent++;
			if (*pszCurrent)
				pszCurrent++;
		}
		if (*pszCurrent) {
			*pszCurrent = '\0';
			pszCurrent++;
		} else {
			pszCurrent = NULL;
		}
	}

	return (pszToken);
}


char *strfchr(char *pszString, char *pchMarker, char ch)
/*
 * See if a character appears in a string before a given position.
 *
 * char *pszString	- the string to search in
 * char *pchMarker	- the current position in the string
 * char ch		- the character to search for
 *
 * Returns:		- a pointer to ch in the string, if it occurs before pchMarker
 *			  pchMarker otherwise
 */
{
	char *pch;

	pch = strchr(pszString, ch);
	if ((pchMarker == NULL) || ((pch != NULL) && (pch < pchMarker)))
		return (pch);
	else
		return (pchMarker);
}


char *strfromf(FILE *f)
/*
 * Place the contents of a file into a null-terminated string. Space for the string will
 * be allocated.
 *
 * FILE *f		- the file to be read into the buffer
 *
 * Returns		- NULL if there is a memory allocation failure
 *			  a pointer to the allocated string otherwise
 */
{
	XSTR *xstrRet;
	char *pszRet;
	XSTR *xstrLine;
	int bAbort;

	/* initialise variables */
	bAbort = 0;
	xstrRet = NULL;
	xstrLine = NULL;

	/* allocate some initial memory */
	if ((xstrRet = xstrnew(80, 80)) == NULL)
		bAbort = 1;
	else if ((xstrLine = xstrnew(80, 10)) == NULL)
		bAbort = 1;

	/* compile the string */
	while (!feof(f) && !bAbort) {
		if (fgetxstr(xstrLine, f) == NULL) {
			if (!feof(f))
				bAbort = 1;
		} else if (xstrcat(xstrRet, xstrcast(xstrLine)) == NULL) {
			bAbort = 1;
		}
	}

	/* clean up */
	xstrfree(xstrLine);

	/* convert the extensible string into a normal one */
	if (!bAbort) {
		pszRet = xstrcvt(xstrRet);
	} else {
		xstrfree(xstrRet);
		pszRet = NULL;
	}

	return (pszRet);
}


char *strtncpy(char *psz1, char *psz2, int n)
/*
 * Copy strings up to n characters, ensuring that there is a terminating
 * null.
 *
 * char *psz1	- destination
 * char *psz2	- source
 * int n	- maximum number of character to copy
 *
 * Returns	- psz1
 */
{
	char *pch1, *pch2;
	int i;

	i = 0;
	pch1 = psz1;
	pch2 = psz2;
	while ((++i < n) && *pch2) {
		*pch1 = *pch2;
		pch1++;
		pch2++;
	}
	*pch1 = '\0';

	return (psz1);
}

char *strpre(char *pszString, char *pszInsert)
/*
 * Pre-pend one string to another.
 *
 * char *pszString	- the string to be pre-pended to
 * char *pszInsert	- the string to be pre-pended
 *
 * Returns		- pszString
 */
{
	char	*pch;
	int	n;

	/* move pszString forward to make space */
	n = strlen(pszInsert);
	for (pch = strchr(pszString, '\0'); pch >= pszString; pch--)
		*(pch + n) = *pch;

	/* copy pszInsert into the space we've created */
	strncpy(pszString, pszInsert, n);

	return (pszString);
}


char *strreplace(char *pszTemplate, const char *pszMarker, const char *pszReplace, char *pszOutput)
/*
 * Replace every occurrence of a marker in a string with a value.
 *
 * char *pszTemplate	- the template string containing markers
 * char *pszMarker	- the marker to be replaced
 * char *pszReplace	- the string to replace the marker with
 * char *pszOutput	- a string to hold the resulting string (output)
 *
 * Returns		- pszOutput
 */
{
	char	*pchStart, *pchEnd;

	pszOutput[0] = '\0';
	pchStart = pszTemplate;
	while ((pchEnd = strstr(pchStart, pszMarker)) != NULL) {
		strncat(pszOutput, pchStart, pchEnd - pchStart);
		strcat(pszOutput, pszReplace);
		pchStart += strlen(pszMarker);
	}
	strcat(pszOutput, pchStart);

	return (pszOutput);
}


char *strrmchr(char *pszString, char ch)
/*
 * Remove a character from a string.
 *
 * char *pszString	- the string to have the character removed
 * char ch		- the character to be removed
 *
 * Returns		- pszString
 */
{
	char	*pch1, *pch2;	/* counters */

	pch1 = pch2 = pszString;
	while (*pch2) {
		if (*pch2 == ch)
			pch2++;
		*pch1 = *pch2;
		pch1++;
		pch2++;
	}
	*pch1 = '\0';

	return (pszString);
}
