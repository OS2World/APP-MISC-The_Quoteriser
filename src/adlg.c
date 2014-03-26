/*
 * adlg.c
 *
 * Routines for author menu options and dialogue boxes.
 *
 *      Created: 16th July, 1997 (split from quoter.c)
 * Version 2.00: 21st December, 1997
 * Version 2.10: 10th December, 1998
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
#include <stdio.h>
#include <stdlib.h>
#include "quoter.h"
#include "authors.h"
#include "threads.h"
#include "general.h"
#include "pmutil.h"

MRESULT EXPENTRY EditAuthorBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the edit quote box. If mp2 is Q_ADD when the dialogue is
 * initialised, add the author to the database. If mp2 is Q_EDIT, edit an author
 * already existing in the database.
 */
{
	static char	szTempFileName[L_tmpnam];
	static char	szOrigCode[20];
	static int	iMode;
	static HAPP	happEditor = NULLHANDLE;
	char 		szString[100], szMessage[200], szTitle[20];
	FILE		*f;
	BOOL		bNewCode;

	switch (message) {
		case WM_INITDLG:
			happEditor = NULLHANDLE;
			tmpnam(szTempFileName);
			WinSendDlgItemMsg(hwnd, IDC_EDITA_CODE, EM_SETTEXTLIMIT, MPFROMSHORT(AUTHOR_MAX_CODE), 0);
			WinSendDlgItemMsg(hwnd, IDC_EDITA_SURNAME, EM_SETTEXTLIMIT, MPFROMSHORT(AUTHOR_MAX_SURNAME), 0);
			WinSendDlgItemMsg(hwnd, IDC_EDITA_GNAME, EM_SETTEXTLIMIT, MPFROMSHORT(AUTHOR_MAX_GNAME), 0);
			WinSendDlgItemMsg(hwnd, IDC_EDITA_BORN, EM_SETTEXTLIMIT, MPFROMSHORT(AUTHOR_MAX_BIRTH), 0);
			WinSendDlgItemMsg(hwnd, IDC_EDITA_DIED, EM_SETTEXTLIMIT, MPFROMSHORT(AUTHOR_MAX_DEATH), 0);
			iMode = (int)LONGFROMMP(mp2);
			if (iMode == Q_EDIT) {
				WinLoadString(hab, 0, IDS_TITLE_EDITA, sizeof(szTitle), szTitle);
				strcpy(szOrigCode, ps.szAuthorCode);
				WinSetDlgItemText(hwnd, IDC_EDITA_CODE, ps.szAuthorCode);
				WinSetDlgItemText(hwnd, IDC_EDITA_SURNAME, ps.pai->szSurname);
				WinSetDlgItemText(hwnd, IDC_EDITA_GNAME, ps.pai->szGivenName);
				WinSetDlgItemText(hwnd, IDC_EDITA_BORN, ps.pai->szBirthYear);
				WinSetDlgItemText(hwnd, IDC_EDITA_DIED, ps.pai->szDeathYear);
				if ((f = fopen(szTempFileName, "w")) == NULL) {
					WinLoadString(hab, 0, IDS_OPENFAILED, sizeof(szString), szString);
					sprintf(szMessage, szString, szTempFileName);
					WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
					WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					WinDismissDlg(hwnd, TRUE);
				} else {
					if (ps.pszDesc != NULL)
						fputs(ps.pszDesc, f);
					fclose(f);
				}
			} else {
				WinLoadString(hab, 0, IDS_TITLE_ADDA, sizeof(szTitle), szTitle);
				if ((ps.pai = (AUTHORINFO *)malloc(sizeof(AUTHORINFO))) == NULL) {
					WinLoadString(hab, 0, IDS_NOMEM, sizeof(szMessage), szMessage);
					WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
					WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					WinDismissDlg(hwnd, TRUE);
				}
			}
			WinSetWindowText(hwnd, szTitle);
			return (FALSE);

		case WM_APPTERMINATENOTIFY:
			happEditor = NULLHANDLE;
			WinEnableControl(hwnd, IDC_EDITA_DESC, TRUE);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_EDITA_DESC:
					strreplace(prf.szEditorParams, "%", szTempFileName, szMessage);
					if ((happEditor = SpawnPM(hwnd, prf.szEditor, szMessage)) != NULLHANDLE)
						WinEnableControl(hwnd, IDC_EDITA_DESC, FALSE);
					return (FALSE);

				case IDC_EDITA_OKAY:
					WinQueryDlgItemText(hwnd, IDC_EDITA_CODE, sizeof(ps.szAuthorCode), ps.szAuthorCode);

					/* make sure the author code is non-trivial */
					if (ps.szAuthorCode[0] == '\0') {
						WinLoadString(hab, 0, IDS_NOCODE, sizeof(szMessage), szMessage);
						WinLoadString(hab, 0, IDS_RETRY, sizeof(szTitle), szTitle);
						WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
						return (FALSE);
					}

					/* see if code already exists in the database */
					bNewCode = iMode == Q_ADD;
					bNewCode = bNewCode || ((iMode == Q_EDIT) && (strcmp(ps.szAuthorCode, szOrigCode) != 0));
					if (bNewCode) {
						if (AuthorExists(&(ps.adb), ps.szAuthorCode)) {
							WinLoadString(hab, 0, IDS_REPLACECODE, sizeof(szString), szString);
							sprintf(szMessage, szString, ps.szAuthorCode);
							WinLoadString(hab, 0, IDS_CONFIRM, sizeof(szTitle), szTitle);
							if (WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_YESNO | MB_QUERY) != MBID_YES)
       								return (FALSE);
						}
					}

					/* delete old code if we have changed it */
					if (bNewCode && (iMode == Q_EDIT))
						AuthorDeleteAuthor(&(ps.adb), szOrigCode);

					/* read information from the dialogue */
					WinQueryDlgItemText(hwnd, IDC_EDITA_SURNAME, sizeof(ps.pai->szSurname), ps.pai->szSurname);
					WinQueryDlgItemText(hwnd, IDC_EDITA_GNAME, sizeof(ps.pai->szGivenName), ps.pai->szGivenName);
					WinQueryDlgItemText(hwnd, IDC_EDITA_BORN, sizeof(ps.pai->szBirthYear), ps.pai->szBirthYear);
					WinQueryDlgItemText(hwnd, IDC_EDITA_DIED, sizeof(ps.pai->szDeathYear), ps.pai->szDeathYear);
					if ((f = fopen(szTempFileName, "r")) != NULL) {
						ps.pszDesc = strfromf(f);
						fclose(f);
						DosDelete(szTempFileName);
					} else
						ps.pszDesc = "";
					if (happEditor != NULLHANDLE)
						WinTerminateApp(happEditor);
					AuthorAddAuthor(&(ps.adb), ps.szAuthorCode, ps.pai, ps.pszDesc);
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);

				case IDC_EDITA_CANCEL:
					if (iMode == Q_ADD)
						AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
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


MRESULT EXPENTRY ChooseAuthorBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the choose author box. This allows the user to select an
 * author from a list of codes.
 */
{
	static THREADINFO	ti;
	static HWND		hwndList, hwndCounter;
	static int		iCount;
	static char		szSearch[AUTHOR_MAX_SEARCH];
	static TID		tidSearch;
	char			szMessage[200], szTitle[20];
	int			iIndex;
	SEARCHINFO		*psi;
	AUTHORINFO		*pai;

	switch (message) {
		case WM_INITDLG:
			WinLoadString(hab, 0, IDS_TITLE_ACODE, sizeof(szMessage), szMessage);
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
					tidSearch = _beginthread((THREADPROC)ThreadFindAllAuthors, NULL, THREADSTACK, (void *)&ti);
					break;

				case Q_TEXT:
					tidSearch = _beginthread((THREADPROC)ThreadFindDescription, NULL, THREADSTACK, (void *)&ti);
					break;

				case Q_NAME:
					tidSearch = _beginthread((THREADPROC)ThreadFindAuthorName, NULL, THREADSTACK, (void *)&ti);
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
			AuthorGetAuthor(&(ps.adb), (char *)PVOIDFROMMP(mp1), &pai, NULL);
			strcpy(szMessage, pai->szSurname);
			if ((pai->szGivenName[0] != '\0') && (pai->szSurname[0] != '\0')) {
				strcat(szMessage, ", ");
			}
			strcat(szMessage, pai->szGivenName);
			strcat(szMessage, " (");
			strcat(szMessage, (char *)PVOIDFROMMP(mp1));
			strcat(szMessage, ")");
			WinInsertLboxItem(hwndList, LIT_SORTASCENDING, szMessage);
			AuthorFreeAuthor(&pai, NULL);
			sprintf(szMessage, "%d", ++iCount);
			WinSetWindowText(hwndCounter, szMessage);
			return (FALSE);

		case QM_DONE:
			if (WinQueryLboxCount(hwndList) == 0) {
				WinLoadString(hab, 0, IDS_NOAUTHORS, sizeof(szMessage), szMessage);
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
						strencl(szMessage, ps.szAuthorCode, '(', ')');
					} else {
						ps.szAuthorCode[0] = '\0';
						ps.pszDesc = NULL;
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
						ps.szAuthorCode[0] = '\0';
						ps.pszDesc = NULL;
						WinDismissDlg(hwnd, TRUE);
					}
					return (FALSE);
			}

		case WM_CONTROL:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_CODE_CODE:
					switch (SHORT2FROMMP(mp1)) {
						case LN_ENTER:
							WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDC_CODE_OKAY), 0);
							return (FALSE);
					}
			}


		default:
			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


MRESULT EXPENTRY ImportAuthorsBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the import author database box.
 */
{
	static THREADINFO	ti;
	static IMPORTINFO	ii;
	static AUTHORDB		adb;
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
					dircat(strcpy(fd.szFullFile, prf.szDataPath), szAuthorExt);
					WinLoadString(hab, 0, IDS_TITLE_BROWSE, sizeof(szTitle), szTitle);
					fd.pszTitle = szTitle;
					WinFileDlg(HWND_DESKTOP, hwnd, &fd);
					if (fd.lReturn == DID_OK)
						WinSetDlgItemText(hwnd, IDC_IMPORT_SOURCE, dirnoext(fd.szFullFile));
					return (FALSE);

				case IDC_IMPORT_OKAY:
					WinQueryDlgItemText(hwnd, IDC_IMPORT_SOURCE, sizeof(fd.szFullFile), fd.szFullFile);
					if (AuthorOpenDB(fd.szFullFile, &adb, S_IREAD)) {
						ti.hwndCaller = hwnd;
						ii.pdb = &adb;
						ii.bAsk = WinQueryButtonCheckstate(hwnd, IDC_IMPORT_REPLACE_ASK);
						ii.bReplace = WinQueryButtonCheckstate(hwnd, IDC_IMPORT_REPLACE_ALL);
						ti.pData = &ii;
						ti.pps = &ps;
						if (_beginthread((THREADPROC)ThreadImportAuthors, NULL, THREADSTACK, (void *)&ti) == -1) {
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


BOOL MenuOpenAuthors(HWND hwnd, char *pszDBName)
/*
 * Open an author database.
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
	ADBINFO		adbinfo;

	bRet = FALSE;
	if (pszDBName == NULL) {
		memset(&fd, 0, sizeof(FILEDLG));
		fd.cbSize = sizeof(FILEDLG);
		fd.fl = FDS_OPEN_DIALOG | FDS_CENTER;
		dircat(strcpy(fd.szFullFile, prf.szDataPath), szAuthorExt);
		WinLoadString(hab, 0, IDS_TITLE_OPEN, sizeof(szTitle), szTitle);
		fd.pszTitle = szTitle;
		WinFileDlg(HWND_DESKTOP, hwnd, &fd);
	} else
		strcpy(fd.szFullFile, pszDBName);
	if ((pszDBName != NULL) || (fd.lReturn == DID_OK)) {
		bCont = TRUE;
		dirnoext(fd.szFullFile);
		if (!AuthorDBStat(fd.szFullFile, &adbinfo)) {
			WinLoadString(hab, 0, IDS_MAKENEWDB, sizeof(szString), szString);
			sprintf(szMessage, szString, fd.szFullFile);
			WinLoadString(hab, 0, IDS_CONFIRM, sizeof(szTitle), szTitle);
			bCont = WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_QUERY | MB_YESNO) == MBID_YES;
			adbinfo.flMode = S_IWRITE | S_IREAD;
		}
		if (bCont) {
			if (AuthorOpenDB(fd.szFullFile, &(ps.adb), ps.iAdbMode = adbinfo.flMode)) {
				strcpy(prf.szAuthorDB1, fd.szFullFile);
				dirfname(strcpy(ps.szAuthorDB, fd.szFullFile));
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


BOOL MenuAddAuthor(HWND hwnd)
/*
 * Allow the user to add an author. Returns TRUE if the author was successfully added.
 */
{
	BOOL		bRet;

	bRet = FALSE;
	ps.szQuoteCode[0] = '\0';
	ps.szAuthorCode[0] = '\0';
	QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
	AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
	WinDlgBox(HWND_DESKTOP, hwnd, EditAuthorBox, NULLHANDLE, IDD_EDITAUTHOR, MPFROMLONG(Q_ADD));
	if (ps.pai != NULL)
		bRet = TRUE;

	return (bRet);
}


BOOL MenuEditAuthor(HWND hwnd, int bCurrent)
/*
 * Allow the user to edit an author. Returns TRUE if the author as edited.
 *
 * int bCurrent	- edit the current author (or provide a list)?
 */
{
	BOOL		bRet;
	SEARCHINFO	si;

	bRet = FALSE;
	ps.szQuoteCode[0] = '\0';
	if (!bCurrent) {
		ps.szAuthorCode[0] = '\0';
		si.iType = Q_FULL;
		si.pszString = NULL;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseAuthorBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
	}
	if (ps.szAuthorCode[0] != '\0') {
		AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
		AuthorGetAuthor(&(ps.adb), ps.szAuthorCode, &(ps.pai), &(ps.pszDesc));
		WinDlgBox(HWND_DESKTOP, hwnd, EditAuthorBox, NULLHANDLE, IDD_EDITAUTHOR, MPFROMLONG(Q_EDIT));
		bRet = TRUE;
	}

	return (bRet);
}


void MenuDelAuthor(HWND hwnd, int bCurrent)
/*
 * Allow the user to delete an author.
 *
 * int bCurrent	- delete the current quote (or provide a list)?
 */
{
	SEARCHINFO	si;

	ps.szQuoteCode[0] = '\0';
	if (!bCurrent) {
		ps.szAuthorCode[0] = '\0';
		si.iType = Q_FULL;
		si.pszString = NULL;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseAuthorBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
	}
	if (ps.szAuthorCode[0] != '\0') {
		AuthorDeleteAuthor(&(ps.adb), ps.szAuthorCode);
		ps.szAuthorCode[0] = '\0';
		AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
	}
}


BOOL MenuGetAuthorAll(HWND hwnd)
/*
 * Allow the user to select an author from all authors in the database.
 * Returns TRUE if a selection was made.
 */
{
	BOOL		bRet;
	SEARCHINFO	si;

	bRet = FALSE;
	ps.szQuoteCode[0] = '\0';
	ps.szAuthorCode[0] = '\0';
	si.iType = Q_FULL;
	si.pszString = NULL;
	WinDlgBox(HWND_DESKTOP, hwnd, ChooseAuthorBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
	if (ps.szAuthorCode[0] != '\0') {
		AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
		AuthorGetAuthor(&(ps.adb), ps.szAuthorCode, &(ps.pai), &(ps.pszDesc));
		bRet = TRUE;
	}

	return (bRet);
}


BOOL MenuGetAuthorDesc(HWND hwnd)
/*
 * Allow the user to select from all authors containing a given string
 * in their description. Returns TRUE if a selection was made.
 */
{
	BOOL		bRet;
	SEARCHINFO	si;
	GETTEXT		gt;
	char		szString[AUTHOR_MAX_SEARCH + 1], szTitle[100];

	bRet = FALSE;
	ps.szQuoteCode[0] = '\0';
	ps.szAuthorCode[0] = '\0';
	szString[0] = '\0';
	gt.iMaxLength = AUTHOR_MAX_SEARCH;
	gt.pszString = szString;
	WinLoadString(hab, 0, IDS_TITLE_AS_DESC, sizeof(szTitle), szTitle);
	gt.pszTitle = szTitle;
	WinDlgBox(HWND_DESKTOP, hwnd, GetTextBox, NULLHANDLE, IDD_GETTEXT, MPFROMP(&gt));
	if (szString[0] != '\0') {
		si.iType = Q_TEXT;
		si.pszString = szString;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseAuthorBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
		if (ps.szAuthorCode[0] != '\0') {
			AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
			AuthorGetAuthor(&(ps.adb), ps.szAuthorCode, &(ps.pai), &(ps.pszDesc));
			bRet = TRUE;
		}
	}

	return (bRet);
}


BOOL MenuGetAuthorName(HWND hwnd)
/*
 * Allow the user to select from all authors with a given name. Returns
 * TRUE if a selection was made.
 */
{
	BOOL		bRet;
	SEARCHINFO	si;
	GETTEXT		gt;
	char		szString[AUTHOR_MAX_SEARCH + 1], szTitle[100];

	bRet = FALSE;
	ps.szQuoteCode[0] = '\0';
	ps.szAuthorCode[0] = '\0';
	szString[0] = '\0';
	gt.iMaxLength = AUTHOR_MAX_SEARCH;
	gt.pszString = szString;
	WinLoadString(hab, 0, IDS_TITLE_AS_NAME, sizeof(szTitle), szTitle);
	gt.pszTitle = szTitle;
	WinDlgBox(HWND_DESKTOP, hwnd, GetTextBox, NULLHANDLE, IDD_GETTEXT, MPFROMP(&gt));
	if (szString[0] != '\0') {
		si.iType = Q_NAME;
		si.pszString = szString;
		WinDlgBox(HWND_DESKTOP, hwnd, ChooseAuthorBox, NULLHANDLE, IDD_CHOOSECODE, MPFROMP(&si));
		if (ps.szAuthorCode[0] != '\0') {
			AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
			AuthorGetAuthor(&(ps.adb), ps.szAuthorCode, &(ps.pai), &(ps.pszDesc));
			bRet = TRUE;
		}
	}

	return (bRet);
}
