/*
 * types.h
 *
 * Type definitions for use in the Quoteriser's GUI interface.
 *
 *      Created: 25th January, 1997
 * Version 1.00: 9th April, 1997
 * Version 2.00: 18th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_TYPES_H
#define _QUOTERISER_TYPES_H

#include <os2.h>
#include "quotes.h"
#include "authors.h"


/* modes the program can be in */
#define Q_NONE		0
#define Q_QUOTE		1
#define Q_AUTHOR	2
#define Q_SEARCHING	3


/* types of searches that can be performed */
#define Q_FULL		0
#define Q_KEY		1
#define Q_TEXT		2
#define Q_NAME		3


/* type to contain program settings */
typedef struct _PROFILE {
	char	szEditor[CCHMAXPATH];				/* text editor */
	char	szEditorParams[CCHMAXPATH];			/* parameters to pass to text editor */
	char	szDataPath[CCHMAXPATH];				/* path to data files */
	char	szDocPath[CCHMAXPATH];				/* path to documentation */
	char	szQuoteDB1[CCHMAXPATH];				/* last quote database used */
	char	szAuthorDB1[CCHMAXPATH];			/* last author database used */
	char	szBrowser[CCHMAXPATH];				/* HTML browser */
	char	szBrowserParams[CCHMAXPATH];			/* parameters to pass to HTML browser */
	FATTRS	fattrs;						/* font */
} PROFILE;


/* type to contain the current program state - basically, all the global variables */
typedef struct _PROGSTATE {
	int		iMode;					/* current mode of the program */
	AUTHORDB	adb;					/* currently open author database */
	char		szAuthorDB[CCHMAXPATH];			/* name of currently open author database */
	int		iAdbMode;				/* mode that adb is open in */
	char		szAuthorCode[AUTHOR_MAX_CODE + 1];	/* code of currently-selected author */
	AUTHORINFO	*pai;					/* information on currently-selected author */
	char		*pszDesc;				/* description of currently-sleected author */
	QUOTEDB		qdb;					/* currently open quote database */
	char		szQuoteDB[CCHMAXPATH];			/* name of the currently open quote database */
	int		iQdbMode;				/* mode that qdb is open in */
	char		szQuoteCode[QUOTE_MAX_CODE + 1];	/* code of currently-selected quote */
	QUOTEINFO	*pqi;					/* information on currently-selected quote */
	char		*pszText;				/* text of currently-selected quote */
} PROGSTATE;


/* type to contain search information */
typedef struct _SEARCHINFO {
	int		iType;					/* type of search to carry out */
	char		*pszString;				/* search data */
} SEARCHINFO;


/* type for passing information to GetTextBox() */
typedef struct _GETTEXT {
   	int		iMaxLength;				/* maximum length of string allowed */
	char		*pszString;				/* buffer to receive string */
	char		*pszTitle;				/* title of the get text box */
} GETTEXT;

#endif
