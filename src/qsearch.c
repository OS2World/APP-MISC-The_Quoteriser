/*
 * qsearch.c
 *
 * Functions for searching a quote database. Note that the use of the
 * GENERIC macro here is not because the #ifndef'd sections won't compile
 * in the generic build; it's so that the GNU Rx library isn't required
 * for the generic build, since the generic build doesn't use these
 * functions.
 *
 *      Created: 27th January, 1997
 * Version 1.00: 20th March, 1997
 * Version 2.00: 24th November, 1997
 * Version 2.10: 7th September, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdlib.h>
#include <string.h>
#include <gnu/gdbm.h>
#ifndef GENERIC
# include <gnu/rxposix.h>
# include <gnu/rxall.h>
#endif
#include "quotes.h"
#include "html.h"


int QuoteFullSearchInit(QUOTEDB *pqdb, QUOTESEARCH *pqs)
/*
 * Find the first quote in a database, and initialise a structure to be used
 * in finding the rest. The first key is returned in the szLastCode field of
 * the QUOTESEARCH structure.
 *
 * QUOTEDB *pqdb	- the quote database to search
 * QUOTESEARCH *pqs	- the structure to be initialised for further searching
 *
 * Returns:		- 0 if the database is empty
 *			  1 otherwise
 */
{
	datum first;

	first = gdbm_firstkey(pqdb->dbfInfo);
	if (first.dptr == NULL)
		return (0);

	pqs->pqdb = pqdb;
	strcpy(pqs->szLastCode, first.dptr);
	free(first.dptr);

	return (1);
}


int QuoteFullSearchNext(QUOTESEARCH *pqs)
/*
 * Find the next quote in a database and update the structure to be used for
 * finding the rest. The next key is returned in the szLastCode field of the
 * QUOTESEARCH structure.
 *
 * QUOTESEARCH *qs	- the search structure
 *
 * Returns:		- 0 if there are no more quotes in the database
 *			  1 otherwise
 */
{
	datum current, next;

	current.dptr = pqs->szLastCode;
	current.dsize = strlen(pqs->szLastCode) + 1;
	next = gdbm_nextkey(pqs->pqdb->dbfInfo, current);
	if (next.dptr == NULL)
		return (0);

	strcpy(pqs->szLastCode, next.dptr);
	free(next.dptr);

	return (1);
}


void QuoteFullSearchEnd(QUOTESEARCH *pqs)
/*
 * Clean up after finishing a full search of the database. Currently, this
 * function does nothing and only exists to make a full search look the
 * same as other searches.
 */
{
}

#ifndef GENERIC
int QuoteKeySearchInit(QUOTEDB *pqdb, char *pszKeyword, QUOTESEARCH *pqs)
/*
 * Find the first quote in a database with a given keyword, and initialise
 * a structure to be used for finding the rest. The first key is returned
 * in the szLastCode field of the QUOTESEARCH structure.
 *
 * QUOTEDB *pqdb	- the quote database to search
 * char *pszKeyword	- the keyword to search for
 * QUOTESEARCH *pqs	- the structure to be initialised for further searching
 *
 * Returns:		- -1 if the regular expression is invalid
 *			   0 if there are no matching quotes in the database
 *			   1 otherwise
 */
{
	QUOTEINFO *pqi;
	int bDone, bFound;
	int i;

	/* fill the QUOTESEARCH structure */
	strcpy(pqs->szSearch, pszKeyword);
	pqs->pqdb = pqdb;
	if (regcomp(&(pqs->rxSearch), pszKeyword, REG_NOSUB | REG_EXTENDED | REG_ICASE))
		return (-1);

	/* find the first quote with a matching keyword */
	bFound = 0;
	bDone = !QuoteFullSearchInit(pqdb, pqs);
	while (!bDone) {
		QuoteGetQuote(pqdb, pqs->szLastCode, &pqi, NULL);
		for (i = 0; i < 5; i++)
			if (!regexec(&(pqs->rxSearch), pqi->aszKeyword[i], 0, NULL, 0))
				bFound = 1;
		QuoteFreeQuote(&pqi, NULL);
		if (bFound)
			bDone = 1;
		else
			bDone = !QuoteFullSearchNext(pqs);
	}

	return (bFound);
}
#endif


#ifndef GENERIC
int QuoteKeySearchNext(QUOTESEARCH *pqs)
/*
 * Find the next quote in a database with a specified keyword.
 *
 * QUOTESEARCH *pqs	- the structure containing the search information
 *
 * Returns:		- 0 if there are no more matching quotes in the database
 *			  1 otherwise
 */
{
	QUOTEINFO *pqi;
	int bDone, bFound;
	int i;

	bFound = 0;
	bDone = !QuoteFullSearchNext(pqs);
	while (!bDone) {
		QuoteGetQuote(pqs->pqdb, pqs->szLastCode, &pqi, NULL);
		for (i = 0; i < 5; i++)
			if (!regexec(&(pqs->rxSearch), pqi->aszKeyword[i], 0, NULL, 0))
				bFound = 1;
		QuoteFreeQuote(&pqi, NULL);
		if (bFound)
			bDone = 1;
		else
			bDone = !QuoteFullSearchNext(pqs);
	}

	return (bFound);
}
#endif


#ifndef GENERIC
void QuoteKeySearchEnd(QUOTESEARCH *pqs)
/*
 * Clean up after completing a keyword search.
 */
{
	regfree(&(pqs->rxSearch));
}
#endif


#ifndef GENERIC
int QuoteTextSearchInit(QUOTEDB *pqdb, char *pszText, QUOTESEARCH *pqs)
/*
 * Find the first quote in a database containing the specified text,
 * and initialise a structure to be used for finding the rest. The
 * first key is returned in the szLastCode field of the QUOTESEARCH
 * structure.
 *
 * QUOTEDB *pqdb	- the quote database to search
 * char *pszText	- the text to search for (regular expression)
 * QUOTESEARCH *pqs	- the structure to be initialised for further searching
 *
 * Returns:		- -1 if the regular expression is invalid
 *			   0 if there are no matching quotes in the database
 *			   1 otherwise
 */
{
	char *pszQuoteText;
	int bDone, bFound;

	/* fill the QUOTESEARCH structure */
	strcpy(pqs->szSearch, pszText);
	pqs->pqdb = pqdb;
	if (regcomp(&(pqs->rxSearch), pszText, REG_NOSUB | REG_EXTENDED | REG_ICASE))
		return (-1);

	/* find the first quote with a matching keyword */
	bFound = 0;
	bDone = !QuoteFullSearchInit(pqdb, pqs);
	while (!bDone) {
		QuoteGetQuote(pqdb, pqs->szLastCode, NULL, &pszQuoteText);
		HTMLMakePlain(pszQuoteText);
		if (!regexec(&(pqs->rxSearch), pszQuoteText, 0, NULL, 0))
			bFound = 1;
		QuoteFreeQuote(NULL, &pszQuoteText);
		if (bFound)
			bDone = 1;
		else
			bDone = !QuoteFullSearchNext(pqs);
	}

	return (bFound);
}
#endif


#ifndef GENERIC
int QuoteTextSearchNext(QUOTESEARCH *pqs)
/*
 * Find the next quote in a database containing the specified text.
 *
 * QUOTESEARCH *pqs	- the structure containing the search information
 *
 * Returns:		- 0 if there are no more matching quotes in the database
 *			  1 otherwise
 */
{
	char *pszQuoteText;
	int bDone, bFound;

	bFound = 0;
	bDone = !QuoteFullSearchNext(pqs);
	while (!bDone) {
		QuoteGetQuote(pqs->pqdb, pqs->szLastCode, NULL, &pszQuoteText);
		HTMLMakePlain(pszQuoteText);
		if (!regexec(&(pqs->rxSearch), pszQuoteText, 0, NULL, 0))
			bFound = 1;
		QuoteFreeQuote(NULL, &pszQuoteText);
		if (bFound)
			bDone = 1;
		else
			bDone = !QuoteFullSearchNext(pqs);
	}

	return (bFound);
}
#endif


#ifndef GENERIC
void QuoteTextSearchEnd(QUOTESEARCH *pqs)
/*
 * Clean up after completing a search of specified text.
 */
{
	regfree(&(pqs->rxSearch));
}
#endif


int QuoteAuthorSearchInit(QUOTEDB *pqdb, char *pszAuthor, QUOTESEARCH *pqs)
/*
 * Find the first quote in a database from a given author, and initialise a
 * structure to be used for finding the rest. The first key is returned in
 * the szLastCode field of the QUOTESEARCH structure.
 *
 * QUOTEDB *pqdb	- the quote database to search
 * char *pszAuthor	- the author code to search for
 * QUOTESEARCH *pqs	- the structure to be initialised for further searching
 * Returns:		- 0 if there are no matching quotes in the database
 *			  1 otherwise
 */
{
	QUOTEINFO *pqi;
	int bDone, bFound;

	/* fill the QUOTESEARCH structure */
	strcpy(pqs->szSearch, pszAuthor);
	pqs->pqdb = pqdb;

	/* find the first quote with a matching keyword */
	bFound = 0;
	bDone = !QuoteFullSearchInit(pqdb, pqs);
	while (!bDone) {
		QuoteGetQuote(pqdb, pqs->szLastCode, &pqi, NULL);
		if (strcmp(pszAuthor, pqi->szAuthorCode) == 0)
			bFound = 1;
		QuoteFreeQuote(&pqi, NULL);
		if (bFound)
			bDone = 1;
		else
			bDone = !QuoteFullSearchNext(pqs);
	}

	return (bFound);
}


int QuoteAuthorSearchNext(QUOTESEARCH *pqs)
/*
 * Find the next quote in a database from a given author.
 *
 * QUOTESEARCH *pqs	- the structure containing the search information
 *
 * Returns:		- 0 if there are no more matching quotes in the database
 *			  1 otherwise
 */
{
	QUOTEINFO *pqi;
	int bDone, bFound;

	bFound = 0;
	bDone = !QuoteFullSearchNext(pqs);
	while (!bDone) {
		QuoteGetQuote(pqs->pqdb, pqs->szLastCode, &pqi, NULL);
		if (strcmp(pqs->szSearch, pqi->szAuthorCode) == 0)
			bFound = 1;
		QuoteFreeQuote(&pqi, NULL);
		if (bFound)
			bDone = 1;
		else
			bDone = !QuoteFullSearchNext(pqs);
	}

	return (bFound);
}


void QuoteAuthorSearchEnd(QUOTESEARCH *pqs)
/*
 * Clean up after finishing an author search of the database. Currently,
 * this function does nothing and only exists to make a full search look
 * the same as other searches.
 */
{
}
