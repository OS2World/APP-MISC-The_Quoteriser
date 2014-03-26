/*
 * authors.c
 *
 * Functions for manipulating individual authors.
 *
 *      Created: 23rd January, 1997
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
#include "authors.h"


void AuthorAddAuthor(AUTHORDB *padb, char *pszCode, AUTHORINFO *pai, char *pszDesc)
/*
 * Add an author to a database. If there is already an author with this code,
 * replace it.
 *
 * AUTHORDB *padb	- the database to add the author to
 * char *pszCode	- the code associated with this author
 * AUTHORINFO *pai	- the author information structure
 * char *pszDesc	- the author description
 */
{
	datum key, content;

	key.dptr = pszCode;
	key.dsize = strlen(pszCode) + 1;
	content.dptr = (char *)pai;
	content.dsize = sizeof(AUTHORINFO);
	gdbm_store(padb->dbfInfo, key, content, GDBM_REPLACE);

	if (pszDesc != NULL) {
		content.dptr = pszDesc;
		content.dsize = strlen(pszDesc) + 1;
		gdbm_store(padb->dbfDesc, key, content, GDBM_REPLACE);
	}
}


int AuthorGetAuthor(AUTHORDB *padb, char *pszCode, AUTHORINFO **ppai, char **ppszDesc)
/*
 * Get the author information and description from a database. Space for the
 * AUTHORINFO structure and the description string will be created.
 *
 * AUTHORDB *padb	- the database to get the author from
 * char *pszCode	- the code of the author to get
 * AUTHORINFO **ppai	- the author information structure (output)
 * char **ppszDesc	- the author description (output)
 *
 * Return: 0 - the author code is not in the database
 *         1 - the author has been retrieved
 */
{
	datum key, ret;

	key.dptr = pszCode;
	key.dsize = strlen(pszCode) + 1;

	if (ppai != NULL) {
		ret = gdbm_fetch(padb->dbfInfo, key);
		(*ppai) = (AUTHORINFO *)ret.dptr;
	}

	if (ppszDesc != NULL) {
		ret = gdbm_fetch(padb->dbfDesc, key);
		(*ppszDesc) = ret.dptr;
	}

	if (ppai != NULL)
		return ((*ppai) != NULL);
	else if (ppszDesc != NULL)
		return ((*ppszDesc) != NULL);
	else
		/* nothing to do! */
		return (0);
}


void AuthorDeleteAuthor(AUTHORDB *padb, char *pszCode)
/*
 * Delete an author from a database.
 *
 * AUTHORDB *padb	- the database to delete the author from
 * char *pszCode	- the code of the author to delete
 */
{
	datum key;

	key.dptr = pszCode;
	key.dsize = strlen(pszCode) + 1;

	gdbm_delete(padb->dbfInfo, key);
	gdbm_delete(padb->dbfDesc, key);
}


void AuthorFreeAuthor(AUTHORINFO **ppai, char **ppszDesc)
/*
 * Reclaim the memory used by an author and make the pointers to the
 * author NULL.
 *
 * AUTHORINFO **ppai	- the author information structure to by reclaimed.
 * char **ppszDesc	- the description string to be reclaimed.
 */
{
	if (ppai != NULL) {
		free(*ppai);
		(*ppai) = NULL;
	}

	if (ppszDesc != NULL) {
		free(*ppszDesc);
		(*ppszDesc) = NULL;
	}
}

int AuthorExists(AUTHORDB *padb, char *pszCode)
/*
 * Test for the existence of an author code in a database. Note: this function
 * will return 0 if the author exists in only one of the databases (i.e. they
 * are corrupt).
 *
 * QUOTEDB *pqdb	- the database to check in
 * char *pszCode	- the code of the author to check for
 *
 * Returns		- 1 if the author exists
 *			  0 otherwise
 */
{
	int ret;
	datum key;

	key.dptr = pszCode;
	key.dsize = strlen(pszCode) + 1;

	ret = gdbm_exists(padb->dbfInfo, key);
	ret = ret && gdbm_exists(padb->dbfDesc, key);

	return (ret);
}
