/*
 * asearch.c
 *
 * Functions for searching an author database. Note that the use of the
 * GENERIC macro here is not because the #ifndef'd sections won't compile
 * in the generic build; it's so that the GNU Rx library isn't required
 * for the generic build, since the generic build doesn't use these
 * functions.
 *
 *      Created: 27th January, 1997
 * Version 1.00: 19th March, 1997
 * Version 2.00: 24th November, 1997
 * Version 2.10: 7th September, 1998
 *
 * (C) 1997 Nicholas Paul Sheppard
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
#include "authors.h"
#include "html.h"


int AuthorFullSearchInit(AUTHORDB *padb, AUTHORSEARCH *pas)
/*
 * Find the first author in a database, and initialise a structure to be used
 * in finding the rest. The first key is returned in the szLastCode field of
 * the AUTHORSEARCH structure.
 *
 * AUTHORDB *padb	- the author database to search
 * AUTHORSEARCH *pas	- the structure to be initialised for further searching
 *
 * Returns:		- 0 if the database is empty
 *			  1 otherwise
 */
{
	datum first;

	first = gdbm_firstkey(padb->dbfInfo);
	if (first.dptr == NULL)
		return (0);

	pas->padb = padb;
	strcpy(pas->szLastCode, first.dptr);
	free(first.dptr);

	return (1);
}


int AuthorFullSearchNext(AUTHORSEARCH *pas)
/*
 * Find the next author in a database and update the structure to be used for
 * finding the rest. The next key is returned in the szLastCode field of the
 * AUTHORSEARCH structure.
 *
 * AUTHORSEARCH *as	- the search structure
 *
 * Returns:		- 0 if there are no more authors in the database
 *			  1 otherwise
 */
{
	datum current, next;

	current.dptr = pas->szLastCode;
	current.dsize = strlen(pas->szLastCode) + 1;
	next = gdbm_nextkey(pas->padb->dbfInfo, current);
	if (next.dptr == NULL)
		return (0);

	strcpy(pas->szLastCode, next.dptr);
	free(next.dptr);

	return (1);
}

void AuthorFullSearchEnd(AUTHORSEARCH *pas)
/*
 * Clean up after finishing a full search of the database. Currently, this
 * function does nothing and only exists to make a full search look the
 * same as other searches.
 */
{
}

#ifndef GENERIC
int AuthorNameSearchInit(AUTHORDB *padb, char *pszName, AUTHORSEARCH *pas)
/*
 * Find the first author in a database with the specified name (whether it
 * is their first or last name), and initialise a structure to be used for
 * finding the rest. The first key is returned in the szLastCode field of
 * the AUTHORSEARCH structure.
 *
 * AUTHORDB *padb	- the quote database to search
 * char *pszName	- the name to search for
 * AUTHORSEARCH *pas	- the structure to be initialised for further searching
 *
 * Returns:		- -1 if the regular expression is invalid
 *			   0 if there are no matching quotes in the database
 *			   1 otherwise
 */
{
	AUTHORINFO *pai;
	int bDone, bFound;

	/* fill the AUTHORSEARCH structure */
	strcpy(pas->szSearch, pszName);
	pas->padb = padb;
	if (regcomp(&(pas->rxSearch), pszName, REG_NOSUB | REG_EXTENDED | REG_ICASE))
		return (-1);

	/* find the first quote with a matching keyword */
	bFound = 0;
	bDone = !AuthorFullSearchInit(padb, pas);
	while (!bDone) {
		AuthorGetAuthor(padb, pas->szLastCode, &pai, NULL);
		if (!regexec(&(pas->rxSearch), pai->szGivenName, 0, NULL, 0))
			bFound = 1;
		if (!regexec(&(pas->rxSearch), pai->szSurname, 0, NULL, 0))
			bFound = 1;
		AuthorFreeAuthor(&pai, NULL);
		if (bFound)
			bDone = 1;
		else
			bDone = !AuthorFullSearchNext(pas);
	}

	return (bFound);
}
#endif


#ifndef GENERIC
int AuthorNameSearchNext(AUTHORSEARCH *pas)
/*
 * Find the next author in a database having the specified name.
 *
 * AUTHORSEARCH *pas	- the structure containing the search information
 *
 * Returns:		- 0 if there are no more matching authors in the database
 *			  1 otherwise
 */
{
	AUTHORINFO *pai;
	int bDone, bFound;

	bFound = 0;
	bDone = !AuthorFullSearchNext(pas);
	while (!bDone) {
		AuthorGetAuthor(pas->padb, pas->szLastCode, &pai, NULL);
		if (!regexec(&(pas->rxSearch), pai->szGivenName, 0, NULL, 0))
			bFound = 1;
		if (!regexec(&(pas->rxSearch), pai->szSurname, 0, NULL, 0))
			bFound = 1;
		AuthorFreeAuthor(&pai, NULL);
		if (bFound)
			bDone = 1;
		else
			bDone = !AuthorFullSearchNext(pas);
	}

	return (bFound);
}
#endif


#ifndef GENERIC
void AuthorNameSearchEnd(AUTHORSEARCH *pas)
/*
 * Clean up after completing a description search.
 */
{
	regfree(&(pas->rxSearch));
}
#endif


#ifndef GENERIC
int AuthorDescSearchInit(AUTHORDB *padb, char *pszDesc, AUTHORSEARCH *pas)
/*
 * Find the first author in a database containing the specified text in
 * their description, and initialise a structure to be used for finding
 * the rest. The first key is returned in the szLastCode field of the
 * AUTHORSEARCH structure.
 *
 * AUTHORDB *padb	- the quote database to search
 * char *pszDesc	- the text to search for
 * AUTHORSEARCH *pas	- the structure to be initialised for further searching
 *
 * Returns:		- -1 if the regular expression is invalid
 *			   0 if there are no matching quotes in the database
 *			   1 otherwise
 */
{
	char *pszAuthorDesc;
	int bDone, bFound;

	/* fill the AUTHORSEARCH structure */
	strcpy(pas->szSearch, pszDesc);
	pas->padb = padb;
	if (regcomp(&(pas->rxSearch), pszDesc, REG_NOSUB | REG_EXTENDED | REG_ICASE))
		return (-1);

	/* find the first quote with a matching keyword */
	bFound = 0;
	bDone = !AuthorFullSearchInit(padb, pas);
	while (!bDone) {
		AuthorGetAuthor(padb, pas->szLastCode, NULL, &pszAuthorDesc);
		HTMLMakePlain(pszAuthorDesc);
		if (!regexec(&(pas->rxSearch), pszAuthorDesc, 0, NULL, 0))
			bFound = 1;
		AuthorFreeAuthor(NULL, &pszAuthorDesc);
		if (bFound)
			bDone = 1;
		else
			bDone = !AuthorFullSearchNext(pas);
	}

	return (bFound);
}
#endif


#ifndef GENERIC
int AuthorDescSearchNext(AUTHORSEARCH *pas)
/*
 * Find the next author in a database containing the specified text in their
 * description.
 *
 * AUTHORSEARCH *pas	- the structure containing the search information
 *
 * Returns:		- 0 if there are no more matching authors in the database
 *			  1 otherwise
 */
{
	char *pszAuthorDesc;
	int bDone, bFound;

	bFound = 0;
	bDone = !AuthorFullSearchNext(pas);
	while (!bDone) {
		AuthorGetAuthor(pas->padb, pas->szLastCode, NULL, &pszAuthorDesc);
		HTMLMakePlain(pszAuthorDesc);
		if (!regexec(&(pas->rxSearch), pszAuthorDesc, 0, NULL, 0))
			bFound = 1;
		AuthorFreeAuthor(NULL, &pszAuthorDesc);
		if (bFound)
			bDone = 1;
		else
			bDone = !AuthorFullSearchNext(pas);
	}

	return (bFound);
}
#endif


#ifndef GENERIC
void AuthorDescSearchEnd(AUTHORSEARCH *pas)
/*
 * Clean up after completing a description search.
 */
{
	regfree(&(pas->rxSearch));
}
#endif
