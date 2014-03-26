/*
 * qdlg.c
 *
 * Routines for quote menu options and dialogue boxes.
 *
 *      Created: 16th July, 1997 (split from quoter.c)
 * Version 2.00: 21st December, 1997
 * Version 2.10: 3rd November, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#define INCL_DOSPROCESS
#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "quoter.h"
#include "quotes.h"
#include "threads.h"
#include "types.h"
#include "html.h"
#include "general.h"
#include "pmutil.h"

#define QDLG_MAX_QUOTE	60

MRESULT EXPENTRY EditQuoteBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the edit quote box. If mp2 is Q_ADD when the dialogue is
 * initialised, add the quote to the database. If mp2 is Q_EDIT, edit a quote
 * already existing in the database.
 */
{
	static char	szTempFileName[L_tmpnam];
	static char	szOrigCode[20];
	static int	iMode;
	static HAPP	happEditor = NULLHANDLE;
	char 		szString[100], szMessage[200], szTitle[20];
	int		k, m, n;
	FILE		*f;
	BOOL		bNewCode;
	SEARCHINFO	si;

	switch (message) {
		case WM_INITDLG:
			happEditor = NULLHANDLE;
			tmpnam(szTempFileName);
			WinSendDlgItemMsg(hwnd, IDC_EDITQ_CODE, EM_SETTEXTLIMIT, MPFROMSHORT(QUOTE_MAX_CODE), 0);
			WinSendDlgItemMsg(hwnd, IDC_EDITQ_SOURCE, EM_SETTEXTLIMIT, MPFROMSHORT(QUOTE_MAX_SOURCE), 0);
			WinSendDlgItemMsg(hwnd, IDC_EDITQ_ACODE, EM_SETTEXTLIMIT, MPFROMSHORT(AUTHOR_MAX_CODE), 0);
			WinSendDlgItemMsg(hwnd, IDC_EDITQ_KEYWORDS, EM_SETTEXTLIMIT, MPFROMSHORT(QUOTE_MAX_KEYWORD * QUOTE_NUM_KEYWORD), 0);
			iMode = (int)LONGFROMMP(mp2);
			if (iMode == Q_EDIT) {
				WinLoadString(hab, 0, IDS_TITLE_EDITQ, sizeof(szTitle), szTitle);
				strcpy(szOrigCode, ps.szQuoteCode);
				WinSetDlgItemText(hwnd, IDC_EDITQ_CODE, ps.szQuoteCode);
				WinSetDlgItemText(hwnd, IDC_EDITQ_SOURCE, ps.pqi->szSource);
				WinSetDlgItemText(hwnd, IDC_EDITQ_ACODE, ps.pqi->szAuthorCode);
				szString[0] = '\0';
				k = 0;
				while ((ps.pqi->aszKeyword[k][0] != '\0') && (k < 5)) {
					strcat(szString, ps.pqi->aszKeyword[k]);
					strcat(szString, " ");
					k++;
				}
				WinSetDlgItemText(hwnd, IDC_EDITQ_KEYWORDS, szString);
				if ((f = fopen(szTempFileName, "w")) == NULL) {
					WinLoadString(hab, 0, IDS_OPENFAILED, sizeof(szString), szString);
					sprintf(szMessage, szString, szTempFileName);
					WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
					WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					WinDismissDlg(hwnd, TRUE);
				} else {
					if (ps.pszText != NULL)
						fputs(ps.pszText, f);
					fclose(f);
				}
			} else {
				WinLoadString(hab, 0, IDS_TITLE_ADDQ, sizeof(szTitle), szTitle);
				if ((ps.pqi = (QUOTEINFO *)malloc(sizeof(QUOTEINFO))) == NULL) {
					WinLoadString(hab, 0, IDS_NOMEM, sizeof(szMessage), szMessage);
					WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
					WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					WinDismissDlg(hwnd, TRUE);
				}
			}
			WinSetWindowText(hwnd, szTitle);
			WinEnableControl(hwnd, IDC_EDITQ_ABROWSE, ps.adb.dbfInfo != NULL);
			return (FALSE);

		case WM_APPTERMINATENOTIFY:
			happEditor = NULLHANDLE;
			WinEnableControl(hwnd, IDC_EDITQ_TEXT, TRUE);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_EDITQ_ABROWSE:
					ps.szAuthorCode[0] = '\0';
					si.iType = Q_FULL;
					si.pszString = NULL;
					WinDlgBox(HWND_DESKTOP, hwnd, ChooseAuthorBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
					if (ps.szAuthorCode[0] != '\0')
						WinSetDlgItemText(hwnd, IDC_EDITQ_ACODE, ps.szAuthorCode);
					return (FALSE);

				case IDC_EDITQ_TEXT:
					strreplace(prf.szEditorParams, "%", szTempFileName, szMessage);
					if ((happEditor = SpawnPM(hwnd, prf.szEditor, szMessage)) != NULLHANDLE)
						WinEnableControl(hwnd, IDC_EDITQ_TEXT, FALSE);
					return (FALSE);

				case IDC_EDITQ_OKAY:
					WinQueryDlgItemText(hwnd, IDC_EDITQ_CODE, sizeof(ps.szQuoteCode), ps.szQuoteCode);

					/* make sure the quote code is non-trivial */
					if (ps.szQuoteCode[0] == '\0') {
						WinLoadString(hab, 0, IDS_NOCODE, sizeof(szMessage), szMessage);
						WinLoadString(hab, 0, IDS_RETRY, sizeof(szTitle), szTitle);
						WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
						return (FALSE);
					}

					/* see if code already exists in the database */
					bNewCode = iMode == Q_ADD;
					bNewCode = bNewCode || ((iMode == Q_EDIT) && (strcmp(ps.szQuoteCode, szOrigCode) != 0));
					if (bNewCode) {
						if (QuoteExists(&(ps.qdb), ps.szQuoteCode)) {
							WinLoadString(hab, 0, IDS_REPLACECODE, sizeof(szString), szString);
							sprintf(szMessage, szString, ps.szQuoteCode);
							WinLoadString(hab, 0, IDS_CONFIRM, sizeof(szTitle), szTitle);
							if (WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_YESNO | MB_QUERY) != MBID_YES)
       								return (FALSE);
						}
					}


					/* read information from the dialogue */
					WinQueryDlgItemText(hwnd, IDC_EDITQ_SOURCE, sizeof(ps.pqi->szSource), ps.pqi->szSource);
					WinQueryDlgItemText(hwnd, IDC_EDITQ_ACODE, sizeof(ps.pqi->szAuthorCode), ps.pqi->szAuthorCode);
					WinQueryDlgItemText(hwnd, IDC_EDITQ_KEYWORDS, sizeof(szString), szString);
					k = 0;
					n = 0;
					while ((sscanf(szString + n, "%s%n", ps.pqi->aszKeyword[k], &m) == 1) && (k < QUOTE_NUM_KEYWORD)) {
						if (strlen(ps.pqi->aszKeyword[k]) == 0)
							break;
						n += m;
						k++;
					}
					for (; k < QUOTE_NUM_KEYWORD; k++)
						ps.pqi->aszKeyword[k][0] = '\0';
					if ((f = fopen(szTempFileName, "r")) != NULL) {
						ps.pszText = strfromf(f);
						fclose(f);
						DosDelete(szTempFileName);
					} else {
						WinLoadString(hab, 0, IDS_NOTEXT, sizeof(szMessage), szMessage);
						WinLoadString(hab, 0, IDS_RETRY, sizeof(szTitle), szTitle);
						WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
						return (FALSE);
					}
					DosDelete(szTempFileName);
					if (happEditor != NULLHANDLE)
						WinTerminateApp(happEditor);

					/* commit to database */
					if (bNewCode && (iMode == Q_EDIT))
						QuoteDeleteQuote(&(ps.qdb), szOrigCode);
					QuoteAddQuote(&(ps.qdb), ps.szQuoteCode, ps.pqi, ps.pszText);
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);

				case IDC_EDITQ_CANCEL:
					if (iMode == Q_ADD)
						QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
					if (happEditor != NULLHANDLE)
						WinTerminateApp(happEditor);
					DosDelete(szTempFileName);
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);
			}

		default:
			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


MRESULT EXPENTRY ChooseQuoteBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the choose quote box. This allows the user to select a
 * quote from a list of codes. The codes to display are determined by the
 * SEARCHINFO structure pointed to by mp2 passed with the WM_INITDLG. Depending
 * on the iType field:
 *
 * Q_FULL	- all quotes
 * Q_KEY	- quotes with the keyword in the pszString field
 * Q_TEXT	- quotes containing the text in the pszString field
 * Q_NAME	- quotes from the author with the code in the pszString field
 */
{
	static THREADINFO	ti;
	static HWND		hwndList, hwndCounter;
	static int		iCount;
	static char		szSearch[QUOTE_MAX_SEARCH];
	static TID		tidSearch;
	char			szMessage[200], szTitle[20];
	int			iIndex;
	SEARCHINFO		*psi;
	char			*pszText;

	switch (message) {
		case WM_INITDLG:
			WinLoadString(hab, 0, IDS_TITLE_QCODE, sizeof(szMessage), szMessage);
			WinSetWindowText(hwnd, szMessage);
			hwndList = WinWindowFromID(hwnd, IDC_CODE_CODE);
			hwndCounter = WinWindowFromID(hwnd, IDC_CODE_COUNT);
			WinSendMsg(hwndList, LM_DELETEALL, 0, 0);
			WinEnableControl(hwnd, IDC_CODE_OKAY, FALSE);
			WinEnableControl(hwnd, IDC_CODE_CANCEL, TRUE);
			psi = (SEARCHINFO *)PVOIDFROMMP(mp2);
			ti.hwndCaller = hwnd;
			if (psi->pszString != NULL)
				ti.pData = (void *)strcpy(szSearch, psi->pszString);
			else
				ti.pData = NULL;
			ti.pps = &ps;
			tidSearch = -1;
			switch (psi->iType) {
				case Q_FULL:
					tidSearch = _beginthread((THREADPROC)ThreadFindAllQuotes, NULL, THREADSTACK, (void *)&ti);
					break;

				case Q_KEY:
					tidSearch = _beginthread((THREADPROC)ThreadFindKeywords, NULL, THREADSTACK, (void *)&ti);
					break;

				case Q_TEXT:
					tidSearch = _beginthread((THREADPROC)ThreadFindQuoteText, NULL, THREADSTACK, (void *)&ti);
					break;

				case Q_NAME:
					tidSearch = _beginthread((THREADPROC)ThreadFindAuthorsQuotes, NULL, THREADSTACK, (void *)&ti);
					break;
			}

			if (tidSearch == -1) {
				WinLoadString(hab, 0, IDS_THREADFAILED, sizeof(szMessage), szMessage);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
				WinDismissDlg(hwnd, TRUE);
			}
			iCount = 0;
			return (FALSE);

		case QM_FOUND:
			QuoteGetQuote(&(ps.qdb), (char *)PVOIDFROMMP(mp1), NULL, &pszText);
			strabbrev(HTMLMakePlain(pszText), szMessage, QDLG_MAX_QUOTE);
			strcat(szMessage, " (");
			strcat(szMessage, (char *)PVOIDFROMMP(mp1));
			strcat(szMessage, ")");
			WinInsertLboxItem(hwndList, LIT_SORTASCENDING, szMessage);
			sprintf(szMessage, "%d", ++iCount);
			WinSetWindowText(hwndCounter, szMessage);
			return (FALSE);

		case QM_DONE:
			if (WinQueryLboxCount(hwndList) == 0) {
				WinLoadString(hab, 0, IDS_NOQUOTES, sizeof(szMessage), szMessage);
				WinLoadString(hab, 0, IDS_RETRY, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_INFORMATION);
				WinDismissDlg(hwnd, TRUE);
				return (FALSE);
			}
			tidSearch = -1;
			WinEnableControl(hwnd, IDC_CODE_OKAY, TRUE);
			WinEnableControl(hwnd, IDC_CODE_CANCEL, TRUE);
			return (FALSE);

		case QM_ERROR:
			WinLoadString(hab, 0, IDS_BADRX, sizeof(szMessage), szMessage);
			WinLoadString(hab, 0, IDS_RETRY, sizeof(szTitle), szTitle);
			WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
			WinDismissDlg(hwnd, TRUE);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_CODE_OKAY:
					iIndex = (int)WinQueryLboxSelectedItem(hwndList);
					if (iIndex != LIT_NONE) {
						WinQueryLboxItemText(hwndList, iIndex, szMessage, sizeof(szMessage));
						strencl(szMessage, ps.szQuoteCode, '(', ')');
					} else {
						ps.szQuoteCode[0] = '\0';
						ps.pszText = NULL;
					}
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);

				case IDC_CODE_CANCEL:
					if (tidSearch != -1) {
						/* cancel searching */
						DosKillThread(tidSearch);
						WinPostMsg(hwnd, QM_DONE, NULL, NULL);
					} else {
						/* cancel selection */
						ps.szQuoteCode[0] = '\0';
						ps.pszText = NULL;
						WinDismissDlg(hwnd, TRUE);
					}
					return (FALSE);
			}

		default:
			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


MRESULT EXPENTRY ImportQuotesBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the import quote database box.
 */
{
	static THREADINFO	ti;
	static IMPORTINFO	ii;
	static QUOTEDB		qdb;
	FILEDLG			fd;
	char			szString[CCHMAXPATH], szMessage[CCHMAXPATH], szTitle[20];

	switch (message) {
		case WM_INITDLG:
			WinCheckButton(hwnd, IDC_IMPORT_REPLACE_ASK, TRUE);
			WinSendDlgItemMsg(hwnd, IDC_IMPORT_SOURCE, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			return (FALSE);

		case QM_DONE:
			WinDismissDlg(hwnd, TRUE);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_IMPORT_BROWSE:
					memset(&fd, 0, sizeof(FILEDLG));
					fd.cbSize = sizeof(FILEDLG);
					fd.fl = FDS_OPEN_DIALOG | FDS_CENTER;
					dircat(strcpy(fd.szFullFile, prf.szDataPath), szQuoteExt);
					WinLoadString(hab, 0, IDS_TITLE_BROWSE, sizeof(szTitle), szTitle);
					fd.pszTitle = szTitle;
					WinFileDlg(HWND_DESKTOP, hwnd, &fd);
					if (fd.lReturn == DID_OK)
						WinSetDlgItemText(hwnd, IDC_IMPORT_SOURCE, dirnoext(fd.szFullFile));
					return (FALSE);

				case IDC_IMPORT_OKAY:
					WinQueryDlgItemText(hwnd, IDC_IMPORT_SOURCE, sizeof(fd.szFullFile), fd.szFullFile);
					if (QuoteOpenDB(fd.szFullFile, &qdb, S_IREAD)) {
						ti.hwndCaller = hwnd;
						ii.pdb = &qdb;
						ii.bAsk = WinQueryButtonCheckstate(hwnd, IDC_IMPORT_REPLACE_ASK);
						ii.bReplace = WinQueryButtonCheckstate(hwnd, IDC_IMPORT_REPLACE_ALL);
						ti.pData = &ii;
						ti.pps = &ps;
						if (_beginthread((THREADPROC)ThreadImportQuotes, NULL, THREADSTACK, (void *)&ti) == -1) {
							WinLoadString(hab, 0, IDS_THREADFAILED, sizeof(szString), szString);
							WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
							WinMessageBox(HWND_DESKTOP, hwnd, szString, szTitle, 0, MB_ERROR);
							WinDismissDlg(hwnd, TRUE);
						}
					} else {
						WinLoadString(hab, 0, IDS_OPENFAILED, sizeof(szString), szString);
						sprintf(szMessage, szString, fd.szFullFile);
						WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
						WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					}
					return (FALSE);

				case IDC_IMPORT_CANCEL:
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);
			}

		default:
			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


BOOL MenuOpenQuotes(HWND hwnd, char *pszDBName)
/*
 * Open a quote database.
 *
 * HWND hwnd		- the window handle we're running in
 * char *pszDBName	- database name to open; NULL to ask the user.
 *
 * Returns:		- TRUE if sucessful
 *			  FALSE otherwise.
 */
{
	FILEDLG		fd;
	BOOL		bCont, bRet;
	char		szString[200], szMessage[200], szTitle[20];
	QDBINFO		qdbinfo;

	bRet = FALSE;
	if (pszDBName == NULL) {
		memset(&fd, 0, sizeof(FILEDLG));
		fd.cbSize = sizeof(FILEDLG);
		fd.fl = FDS_OPEN_DIALOG | FDS_CENTER;
		dircat(strcpy(fd.szFullFile, prf.szDataPath), szQuoteExt);
		WinLoadString(hab, 0, IDS_TITLE_OPEN, sizeof(szTitle), szTitle);
		fd.pszTitle = szTitle;
		WinFileDlg(HWND_DESKTOP, hwnd, &fd);
	} else
		strcpy(fd.szFullFile, pszDBName);
	if ((pszDBName != NULL) || (fd.lReturn == DID_OK)) {
		bCont = TRUE;
		dirnoext(fd.szFullFile);
		if (!QuoteDBStat(fd.szFullFile, &qdbinfo)) {
			WinLoadString(hab, 0, IDS_MAKENEWDB, sizeof(szString), szString);
			sprintf(szMessage, szString, fd.szFullFile);
			WinLoadString(hab, 0, IDS_CONFIRM, sizeof(szTitle), szTitle);
			bCont = WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_QUERY | MB_YESNO) == MBID_YES;
			qdbinfo.flMode = S_IWRITE | S_IREAD;
		}
		if (bCont) {
			if (QuoteOpenDB(fd.szFullFile, &(ps.qdb), ps.iQdbMode = qdbinfo.flMode)) {
				strcpy(prf.szQuoteDB1, fd.szFullFile);
				dirfname(strcpy(ps.szQuoteDB, fd.szFullFile));
				bRet = TRUE;
			} else {
				WinLoadString(hab, 0, IDS_OPENFAILED, sizeof(szString), szString);
				sprintf(szMessage, szString, fd.szFullFile);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
			}
		}
	}

	return (bRet);
}


BOOL MenuAddQuote(HWND hwnd)
/*
 * Allow the user to add a quote. Returns TRUE if the quote was successfully added.
 */
{
	BOOL 		bRet;

	bRet = FALSE;
	ps.szQuoteCode[0] = '\0';
	ps.szAuthorCode[0] = '\0';
	QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
	AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
	WinDlgBox(HWND_DESKTOP, hwnd, EditQuoteBox, NULLHANDLE, IDD_EDITQUOTE, MPFROMLONG(Q_ADD));
	if (ps.pqi != NULL) {
		if (ps.adb.dbfInfo != NULL) {
			/* try to find the author in the current author database */
			if (AuthorGetAuthor(&(ps.adb), ps.pqi->szAuthorCode, &(ps.pai), &(ps.pszDesc)))
				strcpy(ps.szAuthorCode, ps.pqi->szAuthorCode);
		}
		bRet = TRUE;
	}

	return (bRet);
}

BOOL MenuEditQuote(HWND hwnd, int bCurrent)
/*
 * Allow the user to edit a pre-existing quote. Returns TRUE if the quote was
 * edited.
 *
 * int bCurrent	- edit the current quote (or provide a list)?
 */
{
	BOOL		bRet;
	SEARCHINFO	si;

	bRet = FALSE;
	if (!bCurrent) {
		si.iType = Q_FULL;
		si.pszString = NULL;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseQuoteBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
	}
	if (ps.szQuoteCode[0] != '\0') {
		QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
		AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
		QuoteGetQuote(&(ps.qdb), ps.szQuoteCode, &(ps.pqi), &(ps.pszText));
		WinDlgBox(HWND_DESKTOP, hwnd, EditQuoteBox, NULLHANDLE, IDD_EDITQUOTE, MPFROMLONG(Q_EDIT));
		ps.szAuthorCode[0] = '\0';
		if (ps.adb.dbfInfo != NULL) {
			/* try to find the author in the current author database */
			if (AuthorGetAuthor(&(ps.adb), ps.pqi->szAuthorCode, &(ps.pai), &(ps.pszDesc)))
				strcpy(ps.szAuthorCode, ps.pqi->szAuthorCode);
		}
		bRet = TRUE;
	}

	return (bRet);
}


void MenuDelQuote(HWND hwnd, int bCurrent)
/*
 * Allow the user to delete a quote.
 *
 * int bCurrent	- delete the current quote (or provide a list)?
 */
{
	SEARCHINFO	si;

	ps.szAuthorCode[0] = '\0';
	if (!bCurrent) {
		si.iType = Q_FULL;
		si.pszString = NULL;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseQuoteBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
	}
	if (ps.szQuoteCode[0] != '\0') {
		QuoteDeleteQuote(&(ps.qdb), ps.szQuoteCode);
		ps.szQuoteCode[0] = '\0';
		QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
	}
}


BOOL MenuGetQuoteAll(HWND hwnd)
/*
 * Allow the user to select a quote from all quotes.
 */
{
	BOOL		bRet;
	SEARCHINFO	si;

	bRet = FALSE;
	si.iType = Q_FULL;
	si.pszString = NULL;
	WinDlgBox(HWND_DESKTOP, hwnd, ChooseQuoteBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
	if (ps.szQuoteCode[0] != '\0') {
		QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
		AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
		ps.szAuthorCode[0] = '\0';
		QuoteGetQuote(&(ps.qdb), ps.szQuoteCode, &(ps.pqi), &(ps.pszText));
		bRet = TRUE;
		if (ps.adb.dbfInfo != NULL) {
			/* try to find the author in the current author database */
			if (AuthorGetAuthor(&(ps.adb), ps.pqi->szAuthorCode, &(ps.pai), &(ps.pszDesc)))
				strcpy(ps.szAuthorCode, ps.pqi->szAuthorCode);
			else
				ps.szAuthorCode[0] = '\0';
		} else
			ps.szAuthorCode[0] = '\0';
	}

	return (bRet);
}


BOOL MenuGetQuoteKeyword(HWND hwnd)
/*
 * Allow the user to search for quotes associated with a given keyword.
 */
{
	BOOL		bRet;
	SEARCHINFO	si;
	GETTEXT		gt;
	char		szString[QUOTE_MAX_SEARCH + 1], szTitle[100];

	bRet = FALSE;
	szString[0] = '\0';
	gt.iMaxLength = QUOTE_MAX_SEARCH;
	gt.pszString = szString;
	WinLoadString(hab, 0, IDS_TITLE_QS_KEY, sizeof(szTitle), szTitle);
	gt.pszTitle = szTitle;
	WinDlgBox(HWND_DESKTOP, hwnd, GetTextBox, NULLHANDLE, IDD_GETTEXT, MPFROMP(&gt));
	if (szString[0] != '\0') {
		si.iType = Q_KEY;
		si.pszString = szString;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseQuoteBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
		if (ps.szQuoteCode[0] != '\0') {
			QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
			AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
			ps.szAuthorCode[0] = '\0';
			QuoteGetQuote(&(ps.qdb), ps.szQuoteCode, &(ps.pqi), &(ps.pszText));
			bRet = TRUE;
			if (ps.adb.dbfInfo != NULL) {
				/* try to find the author in the current author database */
				if (AuthorGetAuthor(&(ps.adb), ps.pqi->szAuthorCode, &(ps.pai), &(ps.pszDesc)))
					strcpy(ps.szAuthorCode, ps.pqi->szAuthorCode);
			}
		}
	}

	return (bRet);
}


BOOL MenuGetQuoteText(HWND hwnd)
/*
 * Allow the user to search for a quote containing some given text.
 */
{
	BOOL		bRet;
	SEARCHINFO	si;
	GETTEXT		gt;
	char		szString[QUOTE_MAX_SEARCH + 1], szTitle[100];

	bRet = FALSE;
	szString[0] = '\0';
	gt.iMaxLength = QUOTE_MAX_SEARCH;
	gt.pszString = szString;
	WinLoadString(hab, 0, IDS_TITLE_QS_TEXT, sizeof(szTitle), szTitle);
	gt.pszTitle = szTitle;
	WinDlgBox(HWND_DESKTOP, hwnd, GetTextBox, NULLHANDLE, IDD_GETTEXT, MPFROMP(&gt));
	if (szString[0] != '\0') {
		si.iType = Q_TEXT;
		si.pszString = szString;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseQuoteBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
		if (ps.szQuoteCode[0] != '\0') {
			QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
			AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
			ps.szAuthorCode[0] = '\0';
			QuoteGetQuote(&(ps.qdb), ps.szQuoteCode, &(ps.pqi), &(ps.pszText));
			bRet = TRUE;
			if (ps.adb.dbfInfo != NULL) {
				/* try to find the author in the current author database */
				if (AuthorGetAuthor(&(ps.adb), ps.pqi->szAuthorCode, &(ps.pai), &(ps.pszDesc)))
					strcpy(ps.szAuthorCode, ps.pqi->szAuthorCode);
			}
		}
	}

	return (bRet);
}


BOOL MenuGetQuoteName(HWND hwnd)
/*
 * Allow the user to search for all quotes from a given author.
 */
{
	BOOL		bRet;
	SEARCHINFO	si;

	bRet = FALSE;
	si.iType = Q_FULL;
	si.pszString = NULL;
	WinDlgBox(HWND_DESKTOP, hwnd, ChooseAuthorBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
	if (ps.szAuthorCode[0] != '\0') {
		si.iType = Q_NAME;
		si.pszString = ps.szAuthorCode;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseQuoteBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
		if (ps.szQuoteCode[0] != '\0') {
			QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
			QuoteGetQuote(&(ps.qdb), ps.szQuoteCode, &(ps.pqi), &(ps.pszText));
			AuthorGetAuthor(&(ps.adb), ps.szAuthorCode, &(ps.pai), &(ps.pszDesc));
			bRet = TRUE;
		} else {
			ps.szAuthorCode[0] = '\0';
		}
	}

	return (bRet);
}
