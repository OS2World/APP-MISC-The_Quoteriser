/*
 * qdb.c
 *
 * Functions for managing quote databases.
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
#include "quotes.h"


const char szQuoteExt[] = "*.qdb";

void QuoteNullifyDB(QUOTEDB *pqdb)
/*
 * Set an quote database structure to be empty.
 *
 * QUOTEDB *qdb	- the database structure to be nullified
 */
{
	pqdb->dbfInfo = NULL;
	pqdb->dbfText = NULL;
}


int QuoteOpenDB(char *pszFileName, QUOTEDB *pqdb, int iMode)
/*
 * Open an quote database for use.
 *
 * char *pszFileName	- the file name of the database to open (no extension)
 * QUOTEDB *pqdb	- the database structure to point to the file
 * int iMode		- the mode to open it in (S_IREAD, S_IWRITE, etc.)
 *
 * Returns:		- 1 on success
 *			  0 on failure
 */
{
	char szInfoFile[QUOTE_MAX_DBNAME + 5], szTextFile[QUOTE_MAX_DBNAME + 5];
	int iGDBM;

	if (strlen(pszFileName) > QUOTE_MAX_DBNAME)
		return (0);

	sprintf(szInfoFile, "%s.qdb", pszFileName);
	sprintf(szTextFile, "%s.tdb", pszFileName);

	if ((iMode & S_IWRITE) == S_IWRITE) {
		iGDBM = GDBM_WRCREAT;
	} else if ((iMode & S_IREAD) == S_IREAD) {
		iGDBM = GDBM_READER;
	} else {
		/* invalid value for iMode, so stop */
		return (0);
	}

	if ((pqdb->dbfInfo = gdbm_open(szInfoFile, 512, iGDBM, iMode, NULL)) == NULL) {
		QuoteNullifyDB(pqdb);
		return (0);
	}
	if ((pqdb->dbfText = gdbm_open(szTextFile, 512, iGDBM, iMode, NULL)) == NULL) {
		gdbm_close(pqdb->dbfInfo);
		QuoteNullifyDB(pqdb);
		return (0);
	}

	return (1);
}


int QuoteDBStat(char *pszFileName, QDBINFO *pqdbinfo)
/*
 * Retrieve information about a quote database.
 *
 * char *pszFileName	- the file name of the database to check (no extension)
 * QDBINFO *pqdbinfo	- the structure to hold database information (output)
 *
 * Returns:		- 1 if the database exists
 *			  0 if it does not
 */
{
	char szInfoFile[QUOTE_MAX_DBNAME + 5], szTextFile[QUOTE_MAX_DBNAME + 5];
	struct stat qstat, tstat;

	if (strlen(pszFileName) > QUOTE_MAX_DBNAME)
		return (0);

	sprintf(szInfoFile, "%s.qdb", pszFileName);
	sprintf(szTextFile, "%s.tdb", pszFileName);

	if ((stat(szInfoFile, &qstat) == -1) || (stat(szTextFile, &tstat) == -1))
		return (0);

	/* permissions are those that both files have only */
	pqdbinfo->flMode = (int)(qstat.st_mode & tstat.st_mode);

	/* the database size is the sum of the two files' sizes */
	pqdbinfo->lSize = (long)(qstat.st_size + tstat.st_size);

	/* the creation time is the earliest out of the two files */
	pqdbinfo->tCreation = (qstat.st_ctime < tstat.st_ctime)? qstat.st_ctime : tstat.st_ctime;

	/* the modification time is latest out of the two files */
	pqdbinfo->tModification = (qstat.st_mtime > tstat.st_mtime)? qstat.st_mtime : tstat.st_mtime;

	return (1);
}


void QuoteReorganiseDB(QUOTEDB *pqdb)
/*
 * Re-organise an quote database.
 *
 * QUOTEDB *pqdb	- the database structure to re-organise
 */
{
	gdbm_reorganize(pqdb->dbfInfo);
	gdbm_reorganize(pqdb->dbfText);
}


void QuoteCloseDB(QUOTEDB *pqdb)
/*
 * Close an quote database.
 *
 * QUOTEDB *pqdb	- the database structure to close
 */
{
	gdbm_close(pqdb->dbfInfo);
	gdbm_close(pqdb->dbfText);
	QuoteNullifyDB(pqdb);
}


void QuoteEmptyDB(QUOTEDB *pqdb)
/*
 * Remove all quotes from a database.
 *
 * QUOTEDB *padb	- the database to be emptied
 */
{
	QUOTESEARCH	qs;
	char		szDeleteCode[QUOTE_MAX_CODE + 1];
	int		bDone;

	bDone = !QuoteFullSearchInit(pqdb, &qs);
	while (!bDone) {
		strcpy(szDeleteCode, qs.szLastCode);
		bDone = !QuoteFullSearchNext(&qs);
		QuoteDeleteQuote(pqdb, szDeleteCode);
	}
	QuoteFullSearchEnd(&qs);
}
