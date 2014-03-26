/*
 * sig.c
 *
 * The Quoteriser's random signature generator.
 *
 *      Created: 8th December 1997
 * Version 1.00: 21st December 1997
 * Version 1.01: 6th June 1998
 * Version 1.10: 27th October 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include "quotes.h"
#include "authors.h"
#include "html.h"
#include "general.h"
#include "typeset.h"


/* limits */
#define SIG_MAX_CELLS	80


/* output format flags */
#define QSIG_FLAUTHOR	0x00000001	/* print author's name */
#define QSIG_FLSOURCE	0x00000002	/* print quote source */
#define QSIG_FLFALLBACK	0x00000004	/* enable fall-back to other format */

/* internal function prototypes */
void	SigGenerate(QUOTEDB *, AUTHORDB *, char *, int, int, FILE *);
char	*SigGenerateRow(QUOTEDB *, AUTHORDB *, char *, int, int, FILE *);
int	SigQuoteTag(char *);
char	*SigRandomQuote(QUOTEDB *, AUTHORDB *, int, int);


int main(int argc, char **argv)
{
	FILE		*fTemplate;	/* template (input) file */
	FILE		*fSig;		/* signature (output) file */
	char		*pszQuoteDB;	/* name of quote database */
	char		*pszAuthorDB;	/* name of author database */
	char		*pszTemplate;	/* name of template file */
	QUOTEDB		qdb;		/* quote database */
	AUTHORDB	adb;		/* author database */
	int		iAuthor;	/* author output option */
	int		iSource;	/* source output option */
	int		iLineLimit;	/* output line limit */
	int		flMode;		/* output format flags */
	int		i;		/* counter */

	/* set defaults */
	iAuthor = -1;
	iSource = -1;
	iLineLimit = 0;

	/* parse command line */
	i = 1;
	while ((i < argc) && ((argv[i][0] == '-'))) {
		if (argv[i][1] == 'a') {
			if (argv[i][2] == '-') {
				iAuthor = 0;
			} else if (argv[i][2] == 's') {
				iAuthor = 1;
				iSource = (iSource == -1)? 0 : iSource;
			} else if (argv[i][2] == '+') {
				iAuthor = 2;
			} else {
				fprintf(stderr, "Unrecognised option %s.\n", argv[i]);
			}
		} else if (argv[i][1] == 'l') {
			iLineLimit = atoi(argv[i] + 2);
		} else if (argv[i][1] == 's') {
			if (argv[i][2] == '-') {
				iSource = 0;
			} else if (argv[i][2] == 'a') {
				iSource = 1;
				iAuthor = (iAuthor == -1)? 0 : iAuthor;
			} else if (argv[i][2] == '+') {
				iSource = 2;
			} else {
				fprintf(stderr, "Unrecognised option %s.\n", argv[i]);
			}
		} else if (argv[i][1] == '?') {
			i = argc;
		} else if (argv[i][1] == '-') {
			i++;
			break;
		} else {
			fprintf(stderr, "Unrecognised option %s.\n", argv[i]);
		}
		i++;
	}
	iAuthor = (iAuthor == -1)? 2 : iAuthor;
	iSource = (iSource == -1)? 2 : iSource;


	if (i >= argc) {
		/* print help screen */
		printf("The Quoteriser: Random Signature Generator 1.10\n");
		printf("(C) 1997-1998 Nicholas Paul Sheppard. This program is distributed\n");
		printf("under the GNU General Public License and comes with NO WARRANTY\n");
		printf("WHATSOEVER. See the file copying.txt for details.\n\n");
		printf("Usage: sig [options] <template> {<signature>}\n\n");
		printf("  where: <template>  is the name of the template (input) file\n");
		printf("         <signature> is the name of the signature (output) files\n\n");
		printf("Valid options:\n");
		printf("  -?   print this message\n");
		printf("  -a-  don't print author\n");
		printf("  -as  print author if available, otherwise source\n");
		printf("  -a+  print author (default)\n");
		printf("  -lN  print warning if output has more than N lines\n");
		printf("  -s-  don't print quote source\n");
		printf("  -sa  print source if available, otherwise author\n");
		printf("  -s+  print source (default)\n");
		printf("  --   no more arguments on the command line\n\n");
		printf("Environment: QUOTER_QDB - quote database (must be set)\n");
		printf("             QUOTER_ADB - author database (optional)\n");
		return (0);
	}

	/* determine the output format flags */
	flMode = 0;
	if ((iAuthor == 2) || (iAuthor == 1))
		flMode |= QSIG_FLAUTHOR;
	if ((iSource == 2) || (iSource == 1))
		flMode |= QSIG_FLSOURCE;
	if ((iAuthor == 1) || (iSource == 1))
		flMode |= QSIG_FLFALLBACK;
	if ((flMode & QSIG_FLAUTHOR) && (flMode & QSIG_FLSOURCE))
		flMode &= ~QSIG_FLFALLBACK;

	/* open quote database */
	if ((pszQuoteDB = getenv("QUOTER_QDB")) == NULL) {
		fprintf(stderr, "The QUOTER_QDB environment variable is not set.\n");
		return (1);
	}
	if (!QuoteOpenDB(pszQuoteDB, &qdb, S_IREAD)) {
		fprintf(stderr, "sig: unable to open %s for reading.\n", pszQuoteDB);
		return (1);
	}

	/* open author database (if necessary) */
	if (!(flMode & (QSIG_FLAUTHOR | QSIG_FLFALLBACK)) || ((pszAuthorDB = getenv("QUOTER_ADB")) == NULL)) {
		AuthorNullifyDB(&adb);
	} else if (!AuthorOpenDB(pszAuthorDB, &adb, S_IREAD)) {
		fprintf(stderr, "sig: unable to open %s for reading.\n", pszAuthorDB);
		return (1);
	}

	/* open template file */
	if ((fTemplate = fopen(argv[i], "r")) == NULL) {
		fprintf(stderr, "sig: %s: %s", argv[i], strerror(errno));
		return (1);
	}

	/* open signature file */
	if ((i + 1) >= argc) {
		/* no signature file specified; use stdout */
		fSig = stdout;
	} else {
		if ((fSig = fopen(argv[i + 1], "w")) == NULL) {
			fprintf(stderr, "sig: %s: %s", argv[i + 1], strerror(errno));
			return (1);
		}
	}

	/* read template into memory */
	if ((pszTemplate = strfromf(fTemplate)) == NULL) {
		fprintf(stderr, "sig: memory allocation failure.\n");
		return (1);
	}

	/* do it */
	srand(time(NULL));
	i++;
	if (i >= argc) {
		/* no signature file specified; use stdout */
		SigGenerate(&qdb, &adb, pszTemplate, flMode, iLineLimit, stdout);
	} else for (; i < argc; i++) {
		if ((fSig = fopen(argv[i], "w")) == NULL) {
			fprintf(stderr, "sig: %s: %s", argv[i], strerror(errno));
		} else {
			SigGenerate(&qdb, &adb, pszTemplate, flMode, iLineLimit, fSig);
			fclose(fSig);
		}
	}

	/* close files */
	QuoteCloseDB(&qdb);
	if (adb.dbfInfo != NULL)
		AuthorCloseDB(&adb);
	fclose(fTemplate);

	/* free memory */
	free(pszTemplate);

	return (0);
}


void SigGenerate(QUOTEDB *pqdb, AUTHORDB *padb, char *pszTemplate, int flMode, int iLineLimit, FILE *f)
/*
 * Generate a signature file, inserting random quotes where indicated by the
 * template string.
 *
 * QUOTEDB *pqdb	- quote database to obtain quotes from
 * AUTHORDB *padb	- author database to contain authors from (may be empty)
 * char *pszTemplate	- template string - will be altered
 * int flMode		- output format flags
 * int iLineLimit	- output limit before printing a warning (0 for none)
 * FILE *f		- output file
 */
{
	char	*pszCurrent;		/* cursor */
	char	*pszNext;		/* look-ahead */
	char	szTag[HTML_MAX_TAG];	/* HTML tag */
	int	i;			/* counter */

	pszCurrent = pszTemplate;
	while (pszCurrent[0] != '\0') {

		/* search for next tag */
		while ((pszCurrent[0] != '\0') && (pszCurrent[0] != '<'))
			pszCurrent++;

		/* hopefully, it's a <TR> tag */
		if (pszCurrent[0] == '<') {
			i = HTMLGetNextChunk(pszCurrent, szTag, &pszNext);
			pszCurrent = pszNext;
			if ((i == HTML_TAG_MID) || (i == HTML_TAG_END)) {
				if (HTMLParseTag(szTag) == HTML_TABLE_ROW) {
					/* format the row */
					pszNext = SigGenerateRow(pqdb, padb, pszCurrent, flMode, iLineLimit, f);
					pszCurrent = pszNext;
				}
			}
		}
	}
}


char *SigGenerateRow(QUOTEDB *pqdb, AUTHORDB *padb, char *pszTemplate, int flMode, int iLineLimit, FILE *f)
/*
 * Typeset one row of cells.
 *
 * QUOTEDB *pqdb	- quote database to obtain quotes from
 * AUTHORDB *padb	- author database to contain authors from (may be empty)
 * char *pszTemplate	- template string
 * int flMode		- output format flags
 * int iLineLimit	- output limit before printing a warning (0 for none)
 * FILE *f		- output file
 *
 * Returns		- pointer to the position in pszTemplate from which to carry on
 */
{
	char		*apszCells[SIG_MAX_CELLS];	/* cell contents */
	char		szTag[HTML_MAX_TAG];		/* HTML tag */
	char		*pszBuffer;			/* copy of pszTemplate */
	char		*pszCurrent;			/* cursor */
	char		*pszNext;			/* look-ahead */
	int		aiWidth[SIG_MAX_CELLS];		/* cell widths */
	int		iCount;				/* number of cells */
	int		i;				/* counter */
	HTML_TDATTR	tda;				/* attribute from TD elements */
	int		iLines;				/* number of lines in output */
	int		bDone;				/* loop invariant */
	int		bMove;				/* update pszCurrent? */
	int		abAllocated[SIG_MAX_CELLS];	/* did we allocate cells? */

	/* start with a copy of the template string */
	if ((pszBuffer = (char *)malloc(strlen(pszTemplate) + 1)) == NULL) {
		fprintf(stderr, "sig: memory allocation failure.\n");
		exit(1);
	}
	strcpy(pszBuffer, pszTemplate);

	/* divide the input string up into cells */
	pszCurrent = pszBuffer;
	iCount = 0;
	bDone = 0;
	memset(abAllocated, 0, SIG_MAX_CELLS * sizeof(int));
	do {
		/* search for next tag */
		while ((pszCurrent[0] != '\0') && (pszCurrent[0] != '<'))
			pszCurrent++;

		if (pszCurrent[0] == '<') {
			bMove = 1;
			switch (HTMLGetNextChunk(pszCurrent, szTag, &pszNext)) {
				case HTML_TAG_MID:
				case HTML_TAG_END:
					switch (HTMLParseTag(szTag)) {
						case HTML_TABLE_ROW:
							bDone = 1;
							bMove = 0;
							break;

						case HTML_TABLE_DATA:
							HTMLAttrTD(szTag, &tda);
							apszCells[iCount] = pszNext;
							aiWidth[iCount] = tda.iColSpan;
							if (++iCount > SIG_MAX_CELLS)
								bDone = 1;
							break;

						case HTML_UNKNOWN:
							/* hopefully, a <quote> tag */
							if ((aiWidth[iCount] = SigQuoteTag(szTag)) > 0) {
								apszCells[iCount] = SigRandomQuote(pqdb, padb, aiWidth[iCount], flMode);
								abAllocated[iCount] = 1;
								if (++iCount > SIG_MAX_CELLS)
									bDone = 1;
							}
							break;
					}
					break;

				case HTML_END:
					bDone = 1;
					break;
			}
			*(pszCurrent - 1) = '\0';
			if (bMove)
				pszCurrent = pszNext;
		} else
			bDone = 1;
	} while (!bDone);

	/* write the row cells out to the file */
	if (iCount > 0) {
		if ((iLines = strboxf(f, apszCells, aiWidth, iCount)) == -1) {
			fprintf(stderr, "sig: memory allocation failure.\n");
			exit(1);
		} else if ((iLineLimit > 0) && (iLines > iLineLimit)) {
			fprintf(stderr, "sig: warning: output has %d lines.\n", iLines);
		}
	}

	/* de-allocate any memory we allocated */
	free(pszBuffer);
	for (i = 0; i < iCount; i++)
		if (abAllocated[i])
			free(apszCells[i]);

	return (pszCurrent);
}


int SigQuoteTag(char *pszTag)
/*
 * Read information from a <quote> tag. This is *not* a real HTML tag;
 * this is used for Quoteriser purposes only.
 *
 * char *pszTag		- the tag
 *
 * Returns		- the value of the COLSPAN attribute
 *			  0 if there is a problem
 */
{
	char	*pch;			/* cursor */
	char	szString[HTML_MAX_TAG];	/* working buffer */
	char	szAttr[8];		/* attribute name */
	int	i;			/* counter */

	/* find tag identifier */
	pch = pszTag + 1;
	while (isspace(*pch))
		pch++;

	/* check that this *is* a quote tag */
	i = 0;
	while (!isspace(*pch) && ((*pch) != '>')) {
		szString[i++] = (*pch);
		pch++;
	}
	szString[i] = '\0';
	if (strcmpci(szString, "QUOTE") != 0)
		/* not a <quote> tag */
		return (0);

	/* look for COLSPAN attribute */
	i = 0;
	pch = HTMLAttrFirst(pszTag, szAttr, sizeof(szAttr), szString, sizeof(szString));
	while (pch != NULL) {
		if (strcmpci(szAttr, "COLSPAN") == 0)
			i = atoi(szString);
		pch = HTMLAttrNext(pch, szAttr, sizeof(szAttr), szString, sizeof(szString));
	}

	return (i);
}


char *SigRandomQuote(QUOTEDB *pqdb, AUTHORDB *padb, int iWidth, int flMode)
/*
 * Choose and typeset a random quote. Memory for the quote will be
 * allocated.
 *
 * QUOTEDB *pqdb	- quote database to get quote from
 * AUTHORDB *padb	- author database to get author from
 * int flMode		- output format flags
 * int iWidth		- width to typeset quote in
 *
 * Returns	- a pointer to the quote
 *		  NULL if there is a problem
 */
{
	QUOTESEARCH	qs;			/* for searching the database */
	QUOTEINFO	*pqi;			/* quote to print */
	AUTHORINFO	*pai;			/* author to print */
	int		iQuote;			/* number of quote to choose */
	int		bPrintAuthor;		/* print author's name? */
	int		bPrintSource;		/* print quote source? */
	char		*pszText;		/* quote text */
	char		*apszTypeset[3];	/* strings to typeset */
	char		szString[210];		/* working buffer */
	int		i;			/* counter */
	char		*pszRet;		/* return value */
	static int	iCount = -1;		/* number of quotes in the database */

	if (iCount == -1) {
	   	/* count the number of quotes in the database */
		iCount = 0;
		if (QuoteFullSearchInit(pqdb, &qs)) {
			iCount++;
			while (QuoteFullSearchNext(&qs))
				iCount++;
		}
		QuoteFullSearchEnd(&qs);
	}

	if (iCount == 0) {
		/* empty database */
		return (NULL);
	}

	/* choose a random quote */
	if (iCount > 1) {
		iQuote = rand() % iCount;
	} else
		iQuote = 0;
	for (QuoteFullSearchInit(pqdb, &qs), i = 0; i < iQuote; QuoteFullSearchNext(&qs), i++);
	QuoteFullSearchEnd(&qs);

	/* initialise the typeset array to be empty */
	for (i = 0; i < 3; i++)
		apszTypeset[i] = NULL;

	/* decide on what to print */
	bPrintAuthor = 0;
	bPrintSource = 0;
	if (!QuoteGetQuote(pqdb, qs.szLastCode, &pqi, &pszText)) {
		fprintf(stderr, "sig: memory allocation failure\n");
		exit(1);
	}
	if (flMode & QSIG_FLAUTHOR) {
		if (padb->dbfInfo != NULL)
			if (AuthorGetAuthor(padb, pqi->szAuthorCode, &pai, NULL))
				bPrintAuthor = 1;
		if (!bPrintAuthor && (flMode & QSIG_FLFALLBACK))
				bPrintSource = strlen(pqi->szSource) > 0;
	}
	if (flMode & QSIG_FLSOURCE) {
		bPrintSource = strlen(pqi->szSource) > 0;
		if (!bPrintSource && (flMode & QSIG_FLFALLBACK))
			if (padb->dbfInfo != NULL)
				if (AuthorGetAuthor(padb, pqi->szAuthorCode, &pai, NULL))
					bPrintAuthor = 1;
	}
	if (pszText != NULL)
		apszTypeset[0] = pszText;

	/* typeset the quote */
	if (bPrintAuthor || bPrintSource)  {
		strcpy(szString, "<p>- ");
		if (bPrintAuthor) {
			strcat(szString, pai->szGivenName);
			if ((strlen(pai->szSurname) > 0) && (strlen(pai->szGivenName) > 0))
				strcat(szString, " ");
			strcat(szString, pai->szSurname);
		}
		if (bPrintAuthor && bPrintSource)
			strcat(szString, ", ");
		if (bPrintSource) {
			strcat(szString, "<i>");
			strcat(szString, pqi->szSource);
			strcat(szString, "</i>");
		}
		apszTypeset[1] = szString;
	}
	pszRet = TypesetASCII(apszTypeset, iWidth);

	/* de-allocate memory */
	QuoteFreeQuote(&pqi, &pszText);
	if (bPrintAuthor)
		AuthorFreeAuthor(&pai, NULL);

	return (pszRet);
}
