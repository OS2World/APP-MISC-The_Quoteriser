/*
 * adb.c
 *
 * Functions for managing author databases.
 *
 *      Created: 18th January, 1997
 * Version 1.00: 18th January, 1997
 * Version 2.00: 24th November, 1997
 * Version 2.10: 1st December, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdio.h>
#include <string.h>
#include <gnu/gdbm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "authors.h"


const char szAuthorExt[] = "*.adb";


void AuthorNullifyDB(AUTHORDB *padb)
/*
 * Set an author database structure to be empty.
 *
 * AUTHORDB *padb	- the database structure to be nullified
 */
{
	padb->dbfInfo = NULL;
	padb->dbfDesc = NULL;
}


int AuthorOpenDB(char *pszFileName, AUTHORDB *padb, int iMode)
/*
 * Open an author database for use.
 *
 * char *pszFileName	- the file name of the database to open (no extension)
 * AUTHORDB *padb	- the database structure to point to the file
 * int iMode		- the mode to open it in (S_IREAD, S_IWRITE, etc.)
 *
 * Returns:		- 1 on success
 *			  0 on failure
 */
{
	char szInfoFile[AUTHOR_MAX_DBNAME + 5], szDescFile[AUTHOR_MAX_DBNAME + 5];
	int iGDBM;

	if (strlen(pszFileName) > AUTHOR_MAX_DBNAME)
		return (0);

	sprintf(szInfoFile, "%s.adb", pszFileName);
	sprintf(szDescFile, "%s.ddb", pszFileName);

	if ((iMode & S_IWRITE) == S_IWRITE) {
		iGDBM = GDBM_WRCREAT;
	} else if ((iMode & S_IREAD) == S_IREAD) {
		iGDBM = GDBM_READER;
	} else {
		/* invalid value for iMode, so stop */
		return (0);
	}

	if ((padb->dbfInfo = gdbm_open(szInfoFile, 512, iGDBM, iMode, NULL)) == NULL) {
		AuthorNullifyDB(padb);
		return (0);
	}
	if ((padb->dbfDesc = gdbm_open(szDescFile, 512, iGDBM, iMode, NULL)) == NULL) {
		gdbm_close(padb->dbfInfo);
    		AuthorNullifyDB(padb);
		return (0);
	}

	return (1);
}


int AuthorDBStat(char *pszFileName, ADBINFO *padbinfo)
/*
 * Retrieve information about an author database.
 *
 * char *pszFileName	- the file name of the database to check (no extension)
 * ADBINFO *padbinfo	- the structure to hold database information (output)
 *
 * Returns:		- 1 if the database exists
 *			  0 if it does not
 */
{
	char szInfoFile[AUTHOR_MAX_DBNAME + 5], szDescFile[AUTHOR_MAX_DBNAME + 5];
	struct stat astat, dstat;

	if (strlen(pszFileName) > AUTHOR_MAX_DBNAME)
		return (0);

	sprintf(szInfoFile, "%s.adb", pszFileName);
	sprintf(szDescFile, "%s.ddb", pszFileName);

	if ((stat(szInfoFile, &astat) == -1) || (stat(szDescFile, &dstat) == -1))
		return (0);

	/* permissions are those that both files have only */
	padbinfo->flMode = (int)(astat.st_mode & dstat.st_mode);

	/* the database size is the sum of the two files' sizes */
	padbinfo->lSize = (long)(astat.st_size + dstat.st_size);

	/* the creation time is the earliest out of the two files */
	padbinfo->tCreation = (astat.st_ctime < dstat.st_ctime)? astat.st_ctime : dstat.st_ctime;

	/* the modification time is latest out of the two files */
	padbinfo->tModification = (astat.st_mtime > dstat.st_mtime)? astat.st_mtime : dstat.st_mtime;

	return (1);
}


void AuthorReorganiseDB(AUTHORDB *padb)
/*
 * Re-organise an author database.
 *
 * AUTHORDB *padb	- the database structure to re-organise
 */
{
	gdbm_reorganize(padb->dbfInfo);
	gdbm_reorganize(padb->dbfDesc);
}


void AuthorCloseDB(AUTHORDB *padb)
/*
 * Close an author database.
 *
 * AUTHORDB *padb	- the database structure to close
 */
{
	gdbm_close(padb->dbfInfo);
	gdbm_close(padb->dbfDesc);
	AuthorNullifyDB(padb);
}


void AuthorEmptyDB(AUTHORDB *padb)
/*
 * Remove all authors from a database.
 *
 * AUTHORDB *padb	- the database to be emptied
 */
{
	AUTHORSEARCH	as;
	char		szDeleteCode[AUTHOR_MAX_CODE + 1];
	int		bDone;

	bDone = !AuthorFullSearchInit(padb, &as);
	while (!bDone) {
		strcpy(szDeleteCode, as.szLastCode);
		bDone = !AuthorFullSearchNext(&as);
		AuthorDeleteAuthor(padb, szDeleteCode);
	}
	AuthorFullSearchEnd(&as);
}
