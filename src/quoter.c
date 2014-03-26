/*
 * quoter.c
 *
 * Main routines for the Quoteriser.
 *
 *      Created: 16th January, 1997
 * Version 1.00: 8th April, 1997
 * Version 2.00: 21st December, 1997
 * Version 2.10: 28th October, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#define INCL_WIN
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "quoter.h"
#include "quotes.h"
#include "authors.h"
#include "types.h"
#include "general.h"
#include "threads.h"
#include "pmutil.h"


HAB		hab;
HWND		hwndFrame, hwndMain, hwndMenu, hwndVertScroll;
char  		szAppName[64];
PROGSTATE	ps;

/* internal function prototypes */
void MenuSetUsedDBs(void);
void MenuSetWindowTitle(void);

int main(int argc, char *argv[])
{
	HMQ	hmq;
	QMSG	qmsg;
	ULONG	flFrameFlags;
	char	szMainClass[] = "MAIN";
	QDBINFO	qdbinfo;
	int	bQuoteOpen, bQuoteWritable;
	ADBINFO	adbinfo;
	int	bAuthorOpen, bAuthorWritable;
	int	i;

	/* initialise global variables */
	ps.szQuoteDB[0] = '\0';
	ps.szAuthorDB[0] = '\0';
	QuoteNullifyDB(&(ps.qdb));
	AuthorNullifyDB(&(ps.adb));

	/* initialise local variables */
	bQuoteOpen = 0;
	bQuoteWritable = 0;
	bAuthorOpen = 0;
	bAuthorWritable = 0;

	/* parse command line */
	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == 'q') {
			i++;
			if (i < argc) {
				if (QuoteDBStat(argv[i], &qdbinfo)) {
					if (QuoteOpenDB(argv[i], &(ps.qdb), ps.iQdbMode = qdbinfo.flMode)) {
						strcpy(prf.szQuoteDB1, argv[i]);
						dirfname(strcpy(ps.szQuoteDB, argv[i]));
						bQuoteOpen = 1;
						bQuoteWritable = (ps.iQdbMode & S_IWRITE) == S_IWRITE;
						SetUsedDB(prf.szQuoteDB1, NULL);
					}
				}
			}
		} else if (argv[i][1] == 'a') {
			i++;
			if (i < argc) {
				if (AuthorDBStat(argv[i], &adbinfo)) {
					if (AuthorOpenDB(argv[i], &(ps.adb), ps.iAdbMode = adbinfo.flMode)) {
						strcpy(prf.szAuthorDB1, argv[i]);
						dirfname(strcpy(ps.szAuthorDB, argv[i]));
						bAuthorOpen = 1;
						bAuthorWritable = (ps.iAdbMode & S_IWRITE) == S_IWRITE;
						SetUsedDB(NULL, prf.szAuthorDB1);
					}
				}
			}
		}
		i++;
	}

	if ((hab = WinInitialize(0)) == 0)
		return (1);
	hmq = WinCreateMsgQueue(hab, 0);

	/* display an introductory dialogue box */
	WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, IntroBoxProc, NULLHANDLE, IDD_INTRO, (PVOID)NULL);

	/* create the main window */
	flFrameFlags = FCF_TITLEBAR | FCF_SYSMENU | FCF_MENU | FCF_SIZEBORDER | FCF_MINMAX | FCF_TASKLIST | FCF_VERTSCROLL;
	WinRegisterClass(hab, szMainClass, (PFNWP)MainWinProc, CS_SIZEREDRAW, 0); 
	WinLoadString(hab, 0, IDS_APPNAME, sizeof(szAppName), szAppName);
	hwndFrame = WinCreateStdWindow(HWND_DESKTOP, 0, &flFrameFlags, szMainClass,
					szAppName, 0L, NULLHANDLE, IDM_MAIN, &hwndMain);
	if (hwndFrame != NULLHANDLE) {
		/* initialise the main window */
		GetWindowPosition(hwndFrame);
		MenuSetWindowTitle();

		/* get some window handles we need */
		hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
		hwndVertScroll = WinWindowFromID(hwndFrame, FID_VERTSCROLL);

		/* initialise menu item enable states */
		WinEnableMenuItem(hwndMenu, IDM_REORG_AUTHORS, bAuthorOpen);
		WinEnableMenuItem(hwndMenu, IDM_REORG_QUOTES, bQuoteOpen);
		WinEnableMenuItem(hwndMenu, IDM_CLOSE_AUTHORS, bAuthorOpen);
		WinEnableMenuItem(hwndMenu, IDM_CLOSE_QUOTES, bQuoteOpen);
		WinEnableMenuItem(hwndMenu, IDM_AUTHORS, bAuthorOpen);
		WinEnableMenuItem(hwndMenu, IDM_ADD_AUTHOR, bAuthorWritable);
		WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR, bAuthorWritable);
		WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, bAuthorWritable);
		WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_ALL, bAuthorWritable);
		WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR, bAuthorOpen);
		WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_ALL, bAuthorOpen);
		WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_NAME, bAuthorOpen);
		WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_DESC, bAuthorOpen);
		WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR, bAuthorWritable);
		WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, bAuthorWritable);
		WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_ALL, bAuthorWritable);
		WinEnableMenuItem(hwndMenu, IDM_IMPORT_AUTHORS, bAuthorWritable);
		WinEnableMenuItem(hwndMenu, IDM_QUOTES, bQuoteOpen);
		WinEnableMenuItem(hwndMenu, IDM_ADD_QUOTE, bQuoteWritable);
		WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE, bQuoteWritable);
		WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, bQuoteWritable);
		WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_ALL, bQuoteWritable);
		WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE, bQuoteOpen);
		WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_ALL, bQuoteOpen);
		WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_KEY, bQuoteOpen);
		WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_TEXT, bQuoteOpen);
		WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_NAME, bQuoteOpen);
		WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE, bQuoteWritable);
		WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, bQuoteWritable);
		WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_ALL, bQuoteWritable);
		WinEnableMenuItem(hwndMenu, IDM_IMPORT_QUOTES, bQuoteWritable);

		/* initialise last-used databases */
		if ((prf.szQuoteDB1[0] != '\0') || (prf.szAuthorDB1[0] != '\0'))
			MenuSetUsedDBs();

		/* disable scroll bar (painting will re-enable it later) */
		WinEnableWindow(hwndVertScroll, FALSE);

		while (WinGetMsg(hab, &qmsg, (HWND)NULL, 0, 0))
			WinDispatchMsg(hab, &qmsg);

		QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
		AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));

		WinDestroyWindow(hwndFrame);
	}

	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	return (0);
}


MRESULT EXPENTRY MainWinProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the main window.
 */
{
	char			szString[200], szMessage[200], szTitle[20];
	static HAPP		happEditor = NULLHANDLE;
	static THREADINFO	ti;
	BOOL			bWritable = 0;
	RECTL			rectl;

	switch (message) {
		case WM_CREATE:
			/* get settings from QUOTER.INI */
			GetINISettings();

			/* initialise global variables */
			happEditor = NULLHANDLE;
			ps.iMode = Q_NONE;
			ps.pqi = NULL;
			ps.pai = NULL;
			ps.pszText = ps.pszDesc = NULL;
			ti.hwndCaller = hwnd;
			ti.pps = &ps;

			/* initialise threads */
			if (_beginthread((THREADPROC)ThreadPaint, NULL, THREADSTACK, (void *)(&ti)) == -1) {
				WinLoadString(hab, 0, IDS_THREADFAILED, sizeof(szMessage), szMessage);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
				return ((MRESULT)TRUE);
			}
			return (FALSE);

		case WM_ERASEBACKGROUND:
			return ((MRESULT)TRUE);

		case WM_APPTERMINATENOTIFY:
			happEditor = NULLHANDLE;
			return (FALSE);

		case WM_PRESPARAMCHANGED:
		case WM_SIZE:
			WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
			WinInvalidateRect(hwndMain, NULL, TRUE);
			return (FALSE);

		case WM_PAINT:
			WinSendMsg(hwndPaint, QM_PAINT, NULL, NULL);
			return (FALSE);

		case WM_DESTROY:
			/* free all of our memory */
			QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
			AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));

			/* ensure that the databases are closed before exiting */
			if (ps.qdb.dbfInfo != NULL)
				QuoteCloseDB(&(ps.qdb));
			if (ps.adb.dbfInfo != NULL)
				AuthorCloseDB(&(ps.adb));

			/* save window position in QUOTER.INI */
			SaveWindowPosition(hwndFrame);

			return (FALSE);

		case WM_VSCROLL:
			WinQueryWindowRect(hwndMain, &rectl);
			WinScrollWindow(hwndMain, 0L, ScrollMsgHandler(hwndVertScroll, message, mp1, mp2, &rectl, 1), &rectl, NULL, NULLHANDLE, NULL, SW_INVALIDATERGN);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDM_OPEN_QUOTES:
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					if (MenuOpenQuotes(hwnd, NULL)) {
						SetUsedDB(prf.szQuoteDB1, NULL);
						MenuSetUsedDBs();
						bWritable = (ps.iQdbMode & S_IWRITE) == S_IWRITE;
						WinEnableMenuItem(hwndMenu, IDM_OPEN_QUOTES, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_REORG_QUOTES, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_CLOSE_QUOTES, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_USEDDB1, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_QUOTES, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_ADD_QUOTE, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_ALL, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_ALL, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_KEY, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_TEXT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_ALL, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_IMPORT_QUOTES, bWritable);
						if (ps.adb.dbfInfo != NULL)
							WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_NAME, TRUE);
						QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
						AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
						WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
						WinInvalidateRect(hwndMain, NULL, TRUE);
						MenuSetWindowTitle();
					}
					return (FALSE);

				case IDM_OPEN_AUTHORS:
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					if (MenuOpenAuthors(hwnd, NULL)) {
						SetUsedDB(NULL, prf.szAuthorDB1);
						MenuSetUsedDBs();
						bWritable = (ps.iAdbMode & S_IWRITE) == S_IWRITE;
						WinEnableMenuItem(hwndMenu, IDM_OPEN_AUTHORS, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_REORG_AUTHORS, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_CLOSE_AUTHORS, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_USEDDB1, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_AUTHORS, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_ADD_AUTHOR, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_ALL, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_ALL, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_NAME, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_DESC, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_ALL, bWritable);
						WinEnableMenuItem(hwndMenu, IDM_IMPORT_AUTHORS, bWritable);
						if (ps.qdb.dbfInfo != NULL)
							WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_NAME, TRUE);
						QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
						AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
						WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
						WinInvalidateRect(hwndMain, NULL, TRUE);
						MenuSetWindowTitle();
					}
					return (FALSE);

				case IDM_REORG_QUOTES:
					if (_beginthread((THREADPROC)QuoteReorganiseDB, NULL, THREADSTACK, (void *)&(ps.qdb)) == -1) {
						WinLoadString(hab, 0, IDS_THREADFAILED, sizeof(szMessage), szMessage);
						WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
						WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					}
					return (FALSE);

				case IDM_REORG_AUTHORS:
					if (_beginthread((THREADPROC)AuthorReorganiseDB, NULL, THREADSTACK, (void *)&(ps.adb)) == 1) {
						WinLoadString(hab, 0, IDS_THREADFAILED, sizeof(szMessage), szMessage);
						WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
						WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					}
					return (FALSE);

				case IDM_CLOSE_QUOTES:
					ps.iMode = Q_NONE;
					QuoteCloseDB(&(ps.qdb));
					ps.szQuoteDB[0] = '\0';
					WinEnableMenuItem(hwndMenu, IDM_OPEN_QUOTES, TRUE);
					WinEnableMenuItem(hwndMenu, IDM_REORG_QUOTES, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_CLOSE_QUOTES, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_QUOTES, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_ADD_QUOTE, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_ALL, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_ALL, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_KEY, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_TEXT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_NAME, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_ALL, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_IMPORT_QUOTES, FALSE);
					if (ps.adb.dbfInfo == NULL)
						WinEnableMenuItem(hwndMenu, IDM_USEDDB1, TRUE);
					QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					MenuSetWindowTitle();
					return (FALSE);

				case IDM_CLOSE_AUTHORS:
					ps.iMode = Q_NONE;
					AuthorCloseDB(&(ps.adb));
					ps.szAuthorDB[0] = '\0';
					WinEnableMenuItem(hwndMenu, IDM_OPEN_AUTHORS, TRUE);
					WinEnableMenuItem(hwndMenu, IDM_REORG_AUTHORS, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_CLOSE_AUTHORS, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_AUTHORS, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_ADD_AUTHOR, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_ALL, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_ALL, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_NAME, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_DESC, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_NAME, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_ALL, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_IMPORT_AUTHORS, FALSE);
					if (ps.qdb.dbfInfo == NULL)
						WinEnableMenuItem(hwndMenu, IDM_USEDDB1, TRUE);
					AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					MenuSetWindowTitle();
					return (FALSE);

				case IDM_USEDDB1:
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					if (prf.szQuoteDB1[0] != '\0') {
						if (MenuOpenQuotes(hwnd, prf.szQuoteDB1)) {
							bWritable = (ps.iQdbMode & S_IWRITE) == S_IWRITE;
							WinEnableMenuItem(hwndMenu, IDM_OPEN_QUOTES, FALSE);
							WinEnableMenuItem(hwndMenu, IDM_REORG_QUOTES, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_CLOSE_QUOTES, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_QUOTES, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_ADD_QUOTE, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_ALL, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_ALL, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_KEY, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_TEXT, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_ALL, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_IMPORT_QUOTES, TRUE);
							if (ps.adb.dbfInfo != NULL)
								WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_NAME, TRUE);
							QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
							AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
							WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
							WinInvalidateRect(hwndMain, NULL, TRUE);
							MenuSetWindowTitle();
						}
					}
					if (prf.szAuthorDB1[0] != '\0') {
						if (MenuOpenAuthors(hwnd, prf.szAuthorDB1)) {
							bWritable = (ps.iAdbMode & S_IWRITE) == S_IWRITE;
							WinEnableMenuItem(hwndMenu, IDM_OPEN_AUTHORS, FALSE);
							WinEnableMenuItem(hwndMenu, IDM_REORG_AUTHORS, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_CLOSE_AUTHORS, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_AUTHORS, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_ADD_AUTHOR, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_ALL, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_ALL, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_NAME, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_GET_AUTHOR_DESC, TRUE);
							WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_ALL, bWritable);
							WinEnableMenuItem(hwndMenu, IDM_IMPORT_AUTHORS, TRUE);
							if (ps.qdb.dbfInfo != NULL)
								WinEnableMenuItem(hwndMenu, IDM_GET_QUOTE_NAME, TRUE);
							QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
							AuthorFreeAuthor(&(ps.pai), &(ps.pszDesc));
							WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
							WinInvalidateRect(hwndMain, NULL, TRUE);
							MenuSetWindowTitle();
						}
					}
					return (FALSE);

				case IDM_SETTINGS:
					WinDlgBox(HWND_DESKTOP, hwnd, SettingsBox, NULLHANDLE, IDD_SETTINGS, NULL);
					return (FALSE);

				case IDM_COPY_TEXT:
					if ((ps.iMode == Q_QUOTE) || (ps.iMode == Q_AUTHOR))
						WinSendMsg(hwndPaint, QM_COPY, MPFROMSHORT(CF_TEXT), NULL);
					return (FALSE);

				case IDM_COPY_METAFILE:
					if ((ps.iMode == Q_QUOTE) || (ps.iMode == Q_AUTHOR))
						WinSendMsg(hwndPaint, QM_COPY, MPFROMSHORT(CF_METAFILE), NULL);
					return (FALSE);
 
				case IDM_ADD_QUOTE:
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuAddQuote(hwnd)) {
						ps.iMode = Q_QUOTE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_EDIT_QUOTE_CURRENT:
				case IDM_EDIT_QUOTE_ALL:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuEditQuote(hwnd, SHORT1FROMMP(mp1) == IDM_EDIT_QUOTE_CURRENT)) {
						ps.iMode = Q_QUOTE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					QuoteDeleteQuote(&(ps.qdb), ps.szQuoteCode);
					ps.szQuoteCode[0] = '\0';
					QuoteFreeQuote(&(ps.pqi), &(ps.pszText));
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_DEL_QUOTE_CURRENT:
				case IDM_DEL_QUOTE_ALL:
					ps.iMode = Q_SEARCHING;
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					MenuDelQuote(hwnd, SHORT1FROMMP(mp1) == IDM_DEL_QUOTE_CURRENT);
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_GET_QUOTE_ALL:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuGetQuoteAll(hwnd)) {
						ps.iMode = Q_QUOTE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_GET_QUOTE_KEY:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuGetQuoteKeyword(hwnd)) {
						ps.iMode = Q_QUOTE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_GET_QUOTE_TEXT:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuGetQuoteText(hwnd)) {
						ps.iMode = Q_QUOTE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_GET_QUOTE_NAME:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuGetQuoteName(hwnd)) {
						ps.iMode = Q_QUOTE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_IMPORT_QUOTES:
					ps.iMode = Q_SEARCHING;
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					WinDlgBox(HWND_DESKTOP, hwnd, ImportQuotesBox, NULLHANDLE, IDD_IMPORT, NULL);
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_ADD_AUTHOR:
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuAddAuthor(hwnd)) {
						ps.iMode = Q_AUTHOR;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_EDIT_AUTHOR_CURRENT:
				case IDM_EDIT_AUTHOR_ALL:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuEditAuthor(hwnd, SHORT1FROMMP(mp1) == IDM_EDIT_AUTHOR_CURRENT)) {
						ps.iMode = Q_AUTHOR;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_DEL_AUTHOR_CURRENT:
				case IDM_DEL_AUTHOR_ALL:
					ps.iMode = Q_SEARCHING;
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					MenuDelAuthor(hwnd, SHORT1FROMMP(mp1) == IDM_DEL_AUTHOR_CURRENT);
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_GET_AUTHOR_ALL:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuGetAuthorAll(hwnd)) {
						ps.iMode = Q_AUTHOR;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_GET_AUTHOR_DESC:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuGetAuthorDesc(hwnd)) {
						ps.iMode = Q_AUTHOR;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);


				case IDM_GET_AUTHOR_NAME:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					if (MenuGetAuthorName(hwnd)) {
						ps.iMode = Q_AUTHOR;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, TRUE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, TRUE);
					} else {
						ps.iMode = Q_NONE;
						WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
						WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					}
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_IMPORT_AUTHORS:
					ps.iMode = Q_SEARCHING;
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					WinDlgBox(HWND_DESKTOP, hwnd, ImportAuthorsBox, NULLHANDLE, IDD_IMPORT, NULL);
					ps.iMode = Q_NONE;
					WinEnableMenuItem(hwndMenu, IDM_EDIT_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_AUTHOR_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_EDIT_QUOTE_CURRENT, FALSE);
					WinEnableMenuItem(hwndMenu, IDM_DEL_QUOTE_CURRENT, FALSE);
					WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
					WinInvalidateRect(hwndMain, NULL, TRUE);
					return (FALSE);

				case IDM_QOTD:
					WinDlgBox(HWND_DESKTOP, hwnd, QuoteOfTheDayBox, NULLHANDLE, IDD_QOTD, NULL);
					return (FALSE);

				case IDM_EXIT:
					ps.iMode = Q_NONE;
					if (happEditor != NULLHANDLE)
						WinTerminateApp(happEditor);
					WinSendMsg(hwnd, WM_CLOSE, NULL, NULL);
					return (FALSE);

				case IDM_GENHELP:
					if (happEditor == NULLHANDLE) {
						dircat(strcpy(szString, prf.szDocPath), "quoter.htm");
						strreplace(prf.szBrowserParams, "%", szString, szMessage);
						if ((happEditor = SpawnPM(hwnd, prf.szBrowser, szMessage)) == NULLHANDLE) {
							WinLoadString(hab, 0, IDS_SPAWNFAILED, sizeof(szString), szString);
							sprintf(szMessage, szString, prf.szEditor);
							WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
							WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
						}
					}
					return (FALSE);

				case IDM_LICENSE:
					if (happEditor == NULLHANDLE) {
						dircat(strcpy(szString, prf.szDocPath), "copying.txt");
						strreplace(prf.szBrowserParams, "%", szString, szMessage);
						if ((happEditor = SpawnPM(hwnd, prf.szBrowser, szMessage)) == NULLHANDLE) {
							WinLoadString(hab, 0, IDS_SPAWNFAILED, sizeof(szString), szString);
							sprintf(szMessage, szString, prf.szEditor);
							WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
							WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
						}
					}
					return (FALSE);

				case IDM_ABOUT:
					WinDlgBox(HWND_DESKTOP, hwnd, AboutBoxProc, NULLHANDLE, IDD_ABOUT, NULL);
					return (FALSE);
				}
			return (FALSE);

		default:
			return (WinDefWindowProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


MRESULT EXPENTRY IntroBoxProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the intro box. Does nothing except disappear when
 * either the timer is up, or the "Okay" button is pressed.
 */
{
	unsigned long	ulWidth, ulHeight;
	RECTL		rectl;

	switch (message) {
		case WM_INITDLG:
			WinStartTimer(hab, hwnd, IDT_INTRO, 5000);
			ulWidth = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
			ulHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
			WinQueryWindowRect(hwnd, &rectl);
			WinSetWindowPos(hwnd, HWND_TOP, (ulWidth - rectl.xRight) >> 1, (ulHeight - rectl.yTop) >> 1,
					0, 0, SWP_MOVE | SWP_ACTIVATE);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_INTRO_OKAY:
					WinStopTimer(hab, hwnd, IDT_INTRO);
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);
			}

		case WM_TIMER:
			WinStopTimer(hab, hwnd, IDT_INTRO);
			WinDismissDlg(hwnd, TRUE);
			return (FALSE);

		default:
			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


MRESULT EXPENTRY AboutBoxProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the about box. Does nothing except disappear when the
 * "Okay" button is pressed.
 */
{
	switch (message) {
		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_ABOUT_OKAY:
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);
			}

		default:
			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


MRESULT EXPENTRY GetTextBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the text entry dialogue box. Places a string entered
 * by the user into a GETTEXT structure pointed to by mp2.
 *
 * The string is set to be empty if the box is cancelled.
 */
{
	static GETTEXT	*pgt;

	switch (message) {
		case WM_INITDLG:
			pgt = (GETTEXT *)PVOIDFROMMP(mp2);
			WinSendDlgItemMsg(hwnd, IDC_GETTEXT_TEXT, EM_SETTEXTLIMIT, MPFROMLONG(pgt->iMaxLength), 0);
			if (pgt->pszTitle != NULL)
				WinSetWindowText(hwnd, pgt->pszTitle);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_GETTEXT_OKAY:
					WinQueryDlgItemText(hwnd, IDC_GETTEXT_TEXT, pgt->iMaxLength, pgt->pszString);
					WinDismissDlg(hwnd, TRUE);
					break;

				case IDC_GETTEXT_CANCEL:
					pgt->pszString[0] = '\0';
					WinDismissDlg(hwnd, TRUE);
					break;
			}

		default:

			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


void MenuSetUsedDBs(void)
/*
 * Set the value of the last-used-databases item in the File menu.
 */
{
	HWND		hwndFileMenu;
	MENUITEM	mi;
	BOOL		bExists;
	char		szMenuText[CCHMAXPATH * 2], szTemp[CCHMAXPATH];

	/* see if the menu item already exists */
	WinSendMsg(hwndMenu, MM_QUERYITEM, MPFROM2SHORT(IDM_FILE, FALSE), &mi);
	hwndFileMenu = mi.hwndSubMenu;
	bExists = (int)WinSendMsg(hwndFileMenu, MM_ITEMPOSITIONFROMID, MPFROM2SHORT(IDM_USEDDB1, FALSE), 0) != MIT_NONE;

	if (!bExists) {
		/* add a menu separator */
		mi.iPosition = MIT_END;
		mi.afStyle = MIS_SEPARATOR;
		mi.afAttribute = 0;
		mi.id = 0;
		mi.hwndSubMenu = 0;
		mi.hItem = NULLHANDLE;
		WinSendMsg(hwndFileMenu, MM_INSERTITEM, &mi, NULL);
	}

	/* determine the text to put in the menu item */
	szMenuText[0] = '\0';
	if (prf.szQuoteDB1[0] != '\0') {
		strcpy(szTemp, prf.szQuoteDB1);
		strcat(szMenuText, dirfname(szTemp));
	}
	if (prf.szAuthorDB1[0] != '\0') {
		if (prf.szQuoteDB1[0] != '\0')
			strcat(szMenuText, " & ");
		strcpy(szTemp, prf.szAuthorDB1);
		strcat(szMenuText, dirfname(szTemp));
	}

	/* put it in */
	if (bExists)
		WinSendMsg(hwndFileMenu, MM_SETITEMTEXT, MPFROMSHORT(IDM_USEDDB1), szMenuText);
	else {
		mi.iPosition = MIT_END;
		mi.afStyle = MIS_TEXT;
		mi.afAttribute = 0;
		mi.id = IDM_USEDDB1;
		mi.hwndSubMenu = 0;
		mi.hItem = NULLHANDLE;
		WinSendMsg(hwndFileMenu, MM_INSERTITEM, &mi, szMenuText);
	}
}


void MenuSetWindowTitle(void)
/*
 * Set the main window title to "The Quoteriser: <quotes> & <authors>".
 */
{
	char	szTitle[CCHMAXPATH * 2];

	/* determine the text to put in the title */
	WinLoadString(hab, 0, IDS_APPNAME, sizeof(szTitle), szTitle);
	if (ps.szQuoteDB[0] != '\0') {
		strcat(szTitle, ": ");
		strcat(szTitle, ps.szQuoteDB);
	}
	if (ps.szAuthorDB[0] != '\0') {
		if (ps.szQuoteDB[0] != '\0')
			strcat(szTitle, " & ");
		else
			strcat(szTitle, ": ");
		strcat(szTitle, ps.szAuthorDB);
	}

	/* put it in */
	WinSetWindowText(hwndFrame, szTitle);
}
