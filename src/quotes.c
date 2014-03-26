/*
 * quotes.c
 *
 * Functions for manipulating individual quotes.
 *
 *      Created: 22nd January, 1997
 * Version 1.00: 19th March, 1997
 * Version 2.00: 21st November, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <gnu/gdbm.h>
#include <string.h>
#include <stdlib.h>
#include "quotes.h"


void QuoteAddQuote(QUOTEDB *pqdb, char *pszCode, QUOTEINFO *pqi, char *pszText)
/*
 * Add a quote to a database. If there is already a quote with this code,
 * replace it.
 *
 * QUOTEDB *pqdb	- the database to add the quote to
 * char *pszCode	- the code associated with this quote
 * QUOTEINFO *pqi	- the quote information structure
 * char *pszText	- the quote text
 */
{
	datum key, content;

	key.dptr = pszCode;
	key.dsize = strlen(pszCode) + 1;
	content.dptr = (char *)pqi;
	content.dsize = sizeof(QUOTEINFO);
	gdbm_store(pqdb->dbfInfo, key, content, GDBM_REPLACE);

	if (pszText != NULL) {
		content.dptr = pszText;
		content.dsize = strlen(pszText) + 1;
		gdbm_store(pqdb->dbfText, key, content, GDBM_REPLACE);
	}
}


int QuoteGetQuote(QUOTEDB *pqdb, char *pszCode, QUOTEINFO **ppqi, char **ppszText)
/*
 * Get the quote information and text from a database. Space for the the
 * QUOTEINFO structure and the text string will be created.
 *
 * QUOTEDB *pqdb	- the database to get the quote from
 * char *pszCode	- the code of the quote to get
 * QUOTEINFO **ppqi	- the quote information structure (output)
 * char **ppszText	- the quote text (output)
 *
 * Return: 0 - the quote code is not in the database
 *         1 - the quote has been retrieved
 */
{
	datum key, ret;

	key.dptr = pszCode;
	key.dsize = strlen(pszCode) + 1;

	if (ppqi != NULL) {
		ret = gdbm_fetch(pqdb->dbfInfo, key);
		(*ppqi) = (QUOTEINFO *)ret.dptr;
	}

	if (ppszText != NULL) {
		ret = gdbm_fetch(pqdb->dbfText, key);
		(*ppszText) = ret.dptr;
	}

	if (ppqi != NULL)
		return ((*ppqi) != NULL);
	else if (ppszText != NULL)
		return ((*ppszText) != NULL);
	else
		/* nothing to do! */
		return (0);
}


void QuoteDeleteQuote(QUOTEDB *pqdb, char *pszCode)
/*
 * Delete a quote from a database.
 *
 * QUOTEDB *pqdb	- the database to delete the quote from
 * char *pszCode	- the code of the quote to delete
 */
{
	datum key;

	key.dptr = pszCode;
	key.dsize = strlen(pszCode) + 1;

	gdbm_delete(pqdb->dbfInfo, key);
	gdbm_delete(pqdb->dbfText, key);
}


void QuoteFreeQuote(QUOTEINFO **ppqi, char **ppszText)
/*
 * Reclaim the memory used by a quote and make the pointers to the
 * quote NULL.
 *
 * QUOTEINFO **ppqi	- the quote information structure to by reclaimed.
 * char **ppszText	- the text string to be reclaimed.
 */
{
	if (ppqi != NULL) {
		free(*ppqi);
		(*ppqi) = NULL;
	}

	if (ppszText != NULL) {
		free(*ppszText);
		(*ppszText) = NULL;
	}
}


int QuoteExists(QUOTEDB *pqdb, char *pszCode)
/*
 * Test for the existence of a quote code in a database. Note: this function
 * will return 0 if the quote exists in only one of the databases (i.e. they
 * are corrupt).
 *
 * QUOTEDB *pqdb	- the database to check in
 * char *pszCode	- the code of the quote to check for
 *
 * Returns		- 1 if the quote exists
 *			  0 otherwise
 */
{
	int ret;
	datum key;

	key.dptr = pszCode;
	key.dsize = strlen(pszCode) + 1;

	ret = gdbm_exists(pqdb->dbfInfo, key);
	ret = ret && gdbm_exists(pqdb->dbfText, key);

	return (ret);
}
