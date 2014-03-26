/*
 * xstr.c
 *
 * Extensible string routines.
 *
 *      Created: 21st January 1998
 * Version 1.00: 10th December 1998
 *
 * (C) 1998 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xtype.h"


/* internal function prototypes */
int	_cgetc(FILE *);			/* hack for getc() */


XSTR *fgetxstr(XSTR *xstr, FILE *f)
/*
 * Read a line from a file, as fgets().
 *
 * XSTR *xstr	- the extensible string to read the line into
 * FILE *f	- the file to read from
 *
 * Returns	- xstr, if the line was read successfully
 *		  NULL, if there was an error (feof() or ferror(), otherwise memory allocation)
 */
{
	int	ch;	/* read buffer */

	/* start with an empty string */
	xstr->pszString[0] = '\0';

	if ((ch = _cgetc(f)) == EOF)
		return (NULL);
	if (xstrcatc(xstr, (char)ch) == NULL)
		return (NULL);
	while ((ch != '\n') && (ch != EOF)) {
		if ((ch = _cgetc(f)) != EOF) {
			if (xstrcatc(xstr, (char)ch) == NULL)
				return (NULL);
		}
	}

	if ((ch == EOF) && !feof(f))
		return (NULL);

	return (xstr);
}


XSTR *xstrcat(XSTR *xstr, char *psz)
/*
 * Concatenate a string to the end of an extensible string.
 *
 * XSTR *xstr	- the extensible string to be concatenated to
 * char *psz	- the string to concatenate to xstr
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xstr, otherwise
 */
{
	int	iReq;		/* space required for new string */
	int	iNewSize;	/* new size of xstr */
	char *	pch;		/* temporary pointer */

	/* ensure that xstr is long enough to hold the concatenation */
	iReq = xstrlen(xstr) + strlen(psz) + 1;
	if (xstr->iSize <= iReq) {
		iNewSize = xstr->iSize;
		while (iNewSize <= iReq)
			iNewSize += xstr->iBlock;
		if ((pch = (char *)realloc(xstr->pszString, iNewSize)) == NULL)
			return (NULL);
		else {
			xstr->pszString = pch;
			xstr->iSize = iNewSize;
		}
	}

	/* concatenate */
	strcat(xstrcast(xstr), psz);

	return (xstr);
}


XSTR *xstrcatc(XSTR *xstr, char ch)
/*
 * Concatenate a character to the end of an extensible string.
 *
 * XSTR *xstr	- the extensible string to be concatenated to
 * char ch	- the character to concatenate to xstr
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xstr, otherwise
 */
{
	char *	pch;		/* temporary pointer */

	/* ensure that xstr is long enough to hold the concatenation */
	if (xstr->iSize <= (xstrlen(xstr) + 1)) {
		if ((pch = (char *)realloc(xstr->pszString, xstr->iSize + xstr->iBlock)) == NULL)
			return (NULL);
		else {
			xstr->pszString = pch;
			xstr->iSize += xstr->iBlock;
		}
	}

	/* concatenate */
	pch = strchr(xstr->pszString, '\0');
	*pch = ch;
	*(pch + 1) = '\0';

	return (xstr);
}


XSTR *xstrcpy(XSTR *xstr, char *psz)
/*
 * Copy a string into an extensible string.
 *
 * XSTR *xstr	- destination
 * char *psz	- source
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xstr, otherwise
 */
{
	int	iReq;		/* space required for new string */
	int	iNewSize;	/* new size of xstr */
	char *	pch;		/* temporary pointer */

	/* ensure that xstr is long enough to hold psz */
	iReq = strlen(psz) + 1;
	if (xstr->iSize <= iReq) {
		iNewSize = xstr->iSize;
		while (iNewSize <= iReq)
			iNewSize += xstr->iBlock;
		if ((pch = (char *)realloc(xstr->pszString, iNewSize)) == NULL)
			return (NULL);
		else {
			xstr->pszString = pch;
			xstr->iSize = iNewSize;
		}
	}

	/* copy */
	strcpy(xstrcast(xstr), psz);

	return (xstr);
}


char *xstrcvt(XSTR *xstr)
/*
 * Convert an extensible string into a normal string (i.e. get rid of all the
 * extraneous stuff extenesible strings use). Do not use xstr after calling
 * this function.
 *
 * XSTR *xstr	- the extensible string to be converted.
 *
 * Returns	- (a pointer to) the null-terminated string represented by xstr
 */
{
	char *	pszString;	/* return value */

	if (xstr != NULL) {
		pszString = xstr->pszString;
		free(xstr);
	} else {
		pszString = NULL;
	}

	return (pszString);
}


XSTR *xstrdel(XSTR *xstr, int iStart, int iEnd)
/*
 * Delete a portion of an extensible string.
 *
 * XSTR *xstr	- the extensible string
 * int iStart	- the index of the first character to be removed
 * int iEnd	- the index of the last character to be removed
 *
 * Returns	- xstr
 */
{
	memmove(xstr->pszString + iStart, xstr->pszString + iEnd + 1, xstrlen(xstr) - iEnd);

	return (xstr);
}


void xstrfree(XSTR *xstr)
/*
 * Free the memory used by an extensible string.
 *
 * XSTR *xstr	- pointer to the memory to be freed.
 */
{
	if (xstr != NULL) {
		free(xstr->pszString);
		free(xstr);
	}
}


XSTR *xstrncat(XSTR *xstr, char *psz, int n)
/*
 * Concatenate a string to the end of an extensible string up to a given
 * length.
 *
 * XSTR *xstr	- the extensible string to be concatenated to
 * char *psz	- the string to concatenate to xstr
 * int n	- the maximum number of characters to concatenate
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xstr, otherwise
 */
{
	int	iReq;		/* space required for new string */
	int	iNewSize;	/* new size of xstr */
	char *	pch;		/* temporary pointer */

	/* ensure that xstr is long enough to hold the concatenation */
	if ((iReq = strlen(psz)) > n)
		iReq = n;
	iReq += xstrlen(xstr);
	if (xstr->iSize <= iReq) {
		iNewSize = xstr->iSize;
		while (iNewSize <= iReq)
			iNewSize += xstr->iBlock;
		if ((pch = (char *)realloc(xstr->pszString, iNewSize)) == NULL)
			return (NULL);
		else {
			xstr->pszString = pch;
			xstr->iSize = iNewSize;
		}
	}

	/* concatenate */
	strncat(xstrcast(xstr), psz, n);

	return (xstr);
}


XSTR *xstrncpy(XSTR *xstr, char *psz, int n)
/*
 * Copy a string into an extensible string up a given length.
 *
 * XSTR *xstr	- destination
 * char *psz	- source
 * int n	- the maximum number of characters to copy
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xstr, otherwise
 */
{
	int	iReq;		/* space required for new string */
	int	iNewSize;	/* new size of xstr */
	char *	pch;		/* temporary pointer */

	/* ensure that xstr is long enough to hold psz */
	if ((iReq = strlen(psz)) > (n + 1))
		iReq = n + 1;
	if (xstr->iSize <= iReq) {
		iNewSize = xstr->iSize;
		while (iNewSize <= iReq)
			iNewSize += xstr->iBlock;
		if ((pch = (char *)realloc(xstr->pszString, iNewSize)) == NULL)
			return (NULL);
		else {
			xstr->pszString = pch;
			xstr->iSize = iNewSize;
		}
	}

	/* copy */
	strncpy(xstrcast(xstr), psz, n);
	xstr->pszString[n] = '\0';

	return (xstr);
}


XSTR *xstrnew(int iSize, int iBlock)
/*
 * Create a new extensible string.
 *
 * int iSize	- the initial size of the string
 * int iBlock	- the size the string should grow by
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  a pointer to a new extensible string, otherwise
 */
{
	XSTR *	xstr;	/* return value */

	if ((xstr = (XSTR *)malloc(sizeof(XSTR))) != NULL) {
		xstr->iSize = iSize;
		xstr->iBlock = iBlock;
		if ((xstr->pszString = (char *)malloc(iSize)) == NULL) {
			free(xstr);
			xstr = NULL;
		} else {
			xstr->pszString[0] = '\0';
		}
	}

	return (xstr);
}


XSTR *xstrtrunc(XSTR *xstr, int n)
/*
 * Truncate an extensible string.
 *
 * XSTR *xstr	- the string to be truncated
 * int n	- the number of characters to truncate to
 *
 * Returns	- xstr
 */
{
	if (n < xstr->iSize)
		xstr->pszString[n] = '\0';

	return (xstr);
}


int _cgetc(FILE *f)
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
			ch = _cgetc(f);
	}

	return (ch);
}
