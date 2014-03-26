/*
 * htmlattr.c
 *
 * Routines for extracting HTML tag attributes.
 *
 *      Created: 5th December, 1997
 * Version 2.00: 10th December, 1997
 * Version 2.10: 8th September, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "general.h"
#include "html.h"

/* internal function prototypes */
char *HTMLAttrParse(char *, char *, int, char *, int);
int HTMLAttrAlign(char *);


void HTMLAttrTD(char *pszTag, HTML_TDATTR *ptd)
/*
 * Exract the attributes from a TD flag.
 *
 * char *pszTag		- the TD tag
 * HTML_TDATTR *ptd	- structure to contain the attributes
 */
{
	char szAttr[8], szValue[11];
	char *pszCurrent;

	/* initialise attribute structure to defaults */
	ptd->bNoWrap = 0;
	ptd->iRowSpan = 1;
	ptd->iColSpan = 1;
	ptd->iAlign = HTML_ALIGN_LEFT;
	ptd->iVAlign = HTML_ALIGN_TOP;

	pszCurrent = HTMLAttrFirst(pszTag, szAttr, sizeof(szAttr), szValue, sizeof(szValue));
	while (pszCurrent != NULL) {
		if (strcmpci(szAttr, "NOWRAP") == 0)
			ptd->bNoWrap = 1;
		else if (strcmpci(szAttr, "ROWSPAN") == 0)
			ptd->iRowSpan = atoi(szValue);
		else if (strcmpci(szAttr, "COLSPAN") == 0)
			ptd->iColSpan = atoi(szValue);
		else if (strcmpci(szAttr, "ALIGN") == 0)
			ptd->iAlign = HTMLAttrAlign(szValue);
		else if (strcmpci(szAttr, "VALIGN") == 0)
			ptd->iVAlign = HTMLAttrAlign(szValue);
		pszCurrent = HTMLAttrNext(pszCurrent, szAttr, sizeof(szAttr), szValue, sizeof(szValue));
	}
}


char *HTMLAttrFirst(char *pszTag, char *pszAttr, int iMaxAttr, char *pszValue, int iMaxValue)
/*
 * Retrieve the first attribute string from a tag.
 *
 * char *pszTag		- the tag
 * char *pszAttr	- string to receive attribute name
 * int iMaxAttr		- maximum length of attribute name
 * char *pszValue	- string to receive value
 * int iMaxValue	- maximum value
 *
 * Returns		- pointer to the end of the attribute string
 *			  NULL if there are no attribtue strings
 */
{
	char *pch;

	/* start after opening angle bracket */
	pch = pszTag + 1;

	/* skip white space preceding tag identifier */
	while (isspace(*pch))
		pch++;

	/* skip tag identifier */
	while (!isspace(*pch) && ((*pch) != '>'))
		pch++;

	/* skip white space after tag identifier */
	while (isspace(*pch))
		pch++;

	if ((*pch) == '>')
		/* no attributes */
		return (NULL);
	else
		/* get the attribute string */
		return (HTMLAttrParse(pch, pszAttr, iMaxAttr, pszValue, iMaxValue));
}


char *HTMLAttrNext(char *pszCursor, char *pszAttr, int iMaxAttr, char *pszValue, int iMaxValue)
/*
 * Retrieve the next attribute string from a tag.
 *
 * char *pszCursor	- pointer to where we're up to
 * char *pszAttr	- string to receive attribute name
 * int iMaxAttr		- maximum length of attribute name
 * char *pszValue	- string to receive value
 * int iMaxValue	- maximum value
 *
 * Returns		- pointer to the end of the attribute string
 *			  NULL if there are no more attribtue strings
 */
{
	char *pch;

	/* skip white space up to next tag */
	pch = pszCursor;
	while (isspace(*pch))
		pch++;

	if ((*pch) == '>')
		/* no more attributes */
		return (NULL);
	else
		/* get the attribute string */
		return (HTMLAttrParse(pch, pszAttr, iMaxAttr, pszValue, iMaxValue));
}


char *HTMLAttrParse(char *pszCursor, char *pszAttr, int iMaxAttr, char *pszValue, int iMaxValue)
/*
 * Parse the attribute at cursor position.
 *
 * char *pszCursor	- pointer to where we're up to
 * char *pszAttr	- string to receive attribute name
 * int iMaxAttr		- maximum length of attribute name
 * char *pszValue	- string to receive value
 * int iMaxValue	- maximum length of value
 *
 * Returns		- pointer to end of attribute string
 */
{
	char *pch1, *pch2;
	int i;

	/* get attribute name */
	i = 0;
	pch1 = pszCursor;
	pch2 = pszAttr;
	while (((*pch1) != '=') && !isspace(*pch1) && ((*pch1) != '>')) {
		if (i < (iMaxAttr - 1)) {
			(*pch2) = (*pch1);
			pch2++;
		}
		i++;
		pch1++;
	}
	(*pch2) = '\0';

	pch2 = pszValue;
	if ((*pch1) == '=') {
		/* get value */
		pch1++;
		i = 0;
		if ((*pch1) == '\"') {
			pch1++;
			while (((*pch1) != '\"') && ((*pch1) != '\0')) {
				if (i < (iMaxValue - 1)) {
					(*pch2) = (*pch1);
					pch2++;
				}
				i++;
				pch1++;
			}
			if ((*pch1) == '\"')
				pch1++;
		} else {
			while (((*pch1) != '>') && !isspace(*pch1) && ((*pch1) != '\0')) {
				if (i < (iMaxValue - 1)) {
					(*pch2) = (*pch1);
					pch2++;
				}
				i++;
				pch1++;
			}
		}
	}
	(*pch2) = '\0';

	return (pch1);
}


int HTMLAttrAlign(char *pszAlign)
/*
 * Return the number corresponding to the given alignment value.
 *
 * char *pszAlign	- the string name of an alignment
 *
 * Returns		- HTML_ALIGN_* as appropriate
 */
{
	if (strcmpci(pszAlign, "LEFT") == 0)
		return (HTML_ALIGN_LEFT);
	if (strcmpci(pszAlign, "RIGHT") == 0)
		return (HTML_ALIGN_RIGHT);
	if (strcmpci(pszAlign, "TOP") == 0)
		return (HTML_ALIGN_TOP);
	if (strcmpci(pszAlign, "BOTTOM") == 0)
		return (HTML_ALIGN_BOTTOM);
	if (strcmpci(pszAlign, "MIDDLE") == 0)
		return (HTML_ALIGN_MIDDLE);
	if (strcmpci(pszAlign, "CENTER") == 0)
		return (HTML_ALIGN_MIDDLE);
	if (strcmpci(pszAlign, "CENTRE") == 0)
		return (HTML_ALIGN_MIDDLE);

	/* oops! not a valid alignment */
	return (HTML_ALIGN_INVALID);
}
