/*
 * qotd.c
 *
 * Main procedures for the Quoteriser's Quote-of-the-Day utility.
 *
 *      Created: 31st March, 1997
 * Version 1.00: 9th April, 1997
 * Version 2.00: 21st December, 1997
 * Version 2.01: 8th January, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#define INCL_WIN
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include "qotd.h"
#include "threads.h"
#include "types.h"
#include "general.h"
#include "quotes.h"
#include "authors.h"
#include "pmutil.h"

HAB		hab;
HWND		hwndFrame, hwndMain, hwndMenu, hwndVertScroll;
char		szAppName[64];
PROGSTATE	ps;
PROFILE		prf;


MRESULT EXPENTRY MainWinProc(HWND, ULONG, MPARAM, MPARAM);
void GetWindowPosition(HWND);
void SaveWindowPosition(HWND);
int GetINISettings(HWND);

int main(void)
{
   	HMQ	hmq;
	QMSG	qmsg;
	ULONG	flFrameFlags = FCF_TITLEBAR | FCF_SYSMENU | FCF_MENU | FCF_SIZEBORDER | FCF_MINMAX | FCF_TASKLIST | FCF_VERTSCROLL;
	char	szMainClass[] = "QOTDMAIN";

	if ((hab = WinInitialize(0)) == 0)
		return (1);
	hmq = WinCreateMsgQueue(hab, 0);

	/* create the main window */
	WinRegisterClass(hab, szMainClass, (PFNWP)MainWinProc, CS_SIZEREDRAW, 0); 
	WinLoadString(hab, 0, IDS_APPNAME, sizeof(szAppName), szAppName);
	hwndFrame = WinCreateStdWindow(HWND_DESKTOP, 0, &flFrameFlags, szMainClass,
					szAppName, 0L, NULLHANDLE, IDM_MAIN, &hwndMain);

	if (hwndFrame != NULLHANDLE) {
		GetWindowPosition(hwndFrame);
		hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
		hwndVertScroll = WinWindowFromID(hwndFrame, FID_VERTSCROLL);

		/* initialise menu item enable states */
		WinEnableMenuItem(hwndMenu, IDM_AGAIN, FALSE);

		/* disable scroll bar (painting will re-enable it later) */
		WinEnableWindow(hwndVertScroll, FALSE);

		while (WinGetMsg(hab, &qmsg, (HWND)NULL, 0, 0))
			WinDispatchMsg(hab, &qmsg);

		WinDestroyWindow(hwndFrame);
	}

	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	return (0);
}


MRESULT EXPENTRY MainWinProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * WIndow procedure for the main window.
 */
{
	static THREADINFO	ti;
	char			szString[200], szMessage[200], szTitle[20];
	QDBINFO			qdbinfo;
	ADBINFO			adbinfo;
	RECTL			rectl;

	switch (message) {
		case WM_CREATE:
			if (!GetINISettings(hwnd))
				return ((MRESULT)TRUE);
			ps.iMode = Q_SEARCHING;
			ps.szAuthorCode[0] = '\0';
			ps.szQuoteCode[0] = '\0';
			ps.pqi = NULL;
			ps.pai = NULL;
			ps.pszText = ps.pszDesc = NULL;
			ti.hwndCaller = hwnd;
			ti.pps = &ps;
			if (_beginthread((THREADPROC)ThreadPaint, NULL, THREADSTACK, (void *)(&ti)) == -1) {
				WinLoadString(hab, 0, IDS_THREADFAILED, sizeof(szMessage), szMessage);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
				return ((MRESULT)TRUE);
			}
			return (FALSE);

		case QM_START:
			WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
			WinSendMsg(hwndPaint, QM_PAINT, NULL, NULL);
			if (ps.szQuoteDB[0] == '\0') {
				WinLoadString(hab, 0, IDS_UNSET, sizeof(szMessage), szMessage);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
				WinSendMsg(hwnd, WM_CLOSE, NULL, NULL);
				return (FALSE);
			}
			if (!QuoteDBStat(ps.szQuoteDB, &qdbinfo)) {
				WinLoadString(hab, 0, IDS_NODB, sizeof(szString), szString);
				sprintf(szMessage, szString, ps.szQuoteDB);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
				WinSendMsg(hwnd, WM_CLOSE, NULL, NULL);
				return (FALSE);
			}
			if (!QuoteOpenDB(ps.szQuoteDB, &(ps.qdb), S_IREAD)) {
				WinLoadString(hab, 0, IDS_OPENFAILED, sizeof(szString), szString);
				sprintf(szMessage, szString, ps.szQuoteDB);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
				WinSendMsg(hwnd, WM_CLOSE, NULL, NULL);
				return (FALSE);
			}
			if (ps.szAuthorDB[0] != '\0') {
				if (!AuthorDBStat(ps.szAuthorDB, &adbinfo)) {
					WinLoadString(hab, 0, IDS_NODB, sizeof(szString), szString);
					sprintf(szMessage, szString, ps.szAuthorDB);
					WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
					WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					WinSendMsg(hwnd, WM_CLOSE, NULL, NULL);
					return (FALSE);
				}
				if (!AuthorOpenDB(ps.szAuthorDB, &(ps.adb), S_IREAD)) {
					WinLoadString(hab, 0, IDS_OPENFAILED, sizeof(szString), szString);
					sprintf(szMessage, szString, ps.szAuthorDB);
					WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
					WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					WinSendMsg(hwnd, WM_CLOSE, NULL, NULL);
					return (FALSE);
				}
			} else
				AuthorNullifyDB(&(ps.adb));
			if (_beginthread((THREADPROC)ThreadFindRandomQuote, NULL, THREADSTACK, (void *)(&ti)) == -1) {
				WinLoadString(hab, 0, IDS_THREADFAILED, sizeof(szMessage), szMessage);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
				WinSendMsg(hwnd, WM_CLOSE, NULL, NULL);
				return (FALSE);
			}
			return (FALSE);

		case WM_ERASEBACKGROUND:
			return ((MRESULT)TRUE);

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
			if (ps.qdb.dbfInfo != NULL)
				QuoteCloseDB(&(ps.qdb));
			if (ps.adb.dbfInfo != NULL)
				AuthorCloseDB(&(ps.adb));

			/* save the window position in QUOTER.INI */
			SaveWindowPosition(hwndFrame);

			return (FALSE);

		case QM_FOUND:
			if (PVOIDFROMMP(mp1) == NULL) {
				WinLoadString(hab, 0, IDS_EMPTYDB, sizeof(szMessage), szMessage);
				WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
				WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
			} else {
				strcpy(ps.szQuoteCode, (char *)PVOIDFROMMP(mp1));
				QuoteGetQuote(&(ps.qdb), ps.szQuoteCode, &(ps.pqi), &(ps.pszText));
				if ((ps.adb.dbfInfo != NULL) && (ps.pqi->szAuthorCode[0] != '\0')) {
					strcpy(ps.szAuthorCode, ps.pqi->szAuthorCode);
					if (!AuthorGetAuthor(&(ps.adb), ps.szAuthorCode, &(ps.pai), &(ps.pszDesc)))
						ps.szAuthorCode[0] = '\0';
				}
				ps.iMode = Q_QUOTE;
				WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
				WinInvalidateRect(hwndMain, NULL, TRUE);
				WinEnableMenuItem(hwndMenu, IDM_AGAIN, TRUE);
			}
			return (FALSE);

		case WM_VSCROLL:
			WinQueryWindowRect(hwndMain, &rectl);
			WinScrollWindow(hwndMain, 0L, ScrollMsgHandler(hwndVertScroll, message, mp1, mp2, &rectl, 1), &rectl, NULL, NULLHANDLE, NULL, SW_INVALIDATERGN);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDM_EXIT:
					ps.iMode = Q_NONE;
					WinSendMsg(hwnd, WM_CLOSE, NULL, NULL);
					return (FALSE);

				case IDM_COPY_TEXT:
					if ((ps.iMode == Q_QUOTE) || (ps.iMode == Q_AUTHOR))
						WinSendMsg(hwndPaint, QM_COPY, MPFROMSHORT(CF_TEXT), NULL);
					return (FALSE);

				case IDM_COPY_METAFILE:
					if ((ps.iMode == Q_QUOTE) || (ps.iMode == Q_AUTHOR))
						WinSendMsg(hwndPaint, QM_COPY, MPFROMSHORT(CF_METAFILE), NULL);
					return (FALSE);

				case IDM_AGAIN:
					ps.iMode = Q_SEARCHING;
					ps.szAuthorCode[0] = '\0';
					ps.szQuoteCode[0] = '\0';
					ti.hwndCaller = hwnd;
					ti.pps = &ps;
					WinEnableMenuItem(hwndMenu, IDM_AGAIN, FALSE);
					if (_beginthread((THREADPROC)ThreadFindRandomQuote, NULL, THREADSTACK, (void *)(&ti)) == -1) {
						WinLoadString(hab, 0, IDS_THREADFAILED, sizeof(szMessage), szMessage);
						WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
						WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
					}
					return (FALSE);
			}

		default:
			return (WinDefWindowProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


void GetWindowPosition(HWND hwnd)
/*
 * Set the window position from the QUOTER.INI file.
 */
{
	HINI		hiniProfile;
	SWP		swpMain;
	unsigned long	ulSize;

	hiniProfile = PrfOpenProfile(hab, "QUOTER.INI");

	ulSize = sizeof(SWP);
	if (!PrfQueryProfileData(hiniProfile, "QofD", "WindowPos", &swpMain, &ulSize)) {
		/* QUOTER.INI doesn't have a window size in it, so make up a default */
		swpMain.hwndInsertBehind = HWND_TOP;
		swpMain.fl = SWP_ACTIVATE | SWP_SIZE | SWP_MOVE;
		swpMain.x = 20;
		swpMain.y = 20;
		swpMain.cx = 600;
		swpMain.cy = 310;
		WinQueryTaskSizePos(hab, 0, &swpMain);
	}
	WinSetWindowPos(hwnd, HWND_TOP, swpMain.x, swpMain.y, swpMain.cx, swpMain.cy,
			SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW);

	PrfCloseProfile(hiniProfile);
}


void SaveWindowPosition(HWND hwnd)
/*
 * Save the window position to the QUOTER.INI file.
 */
{
	HINI	hiniProfile;
	SWP	swpMain;

	hiniProfile = PrfOpenProfile(hab, "QUOTER.INI");

	WinQueryWindowPos(hwnd, &swpMain);
	PrfWriteProfileData(hiniProfile, "QofD", "WindowPos", &swpMain, sizeof(SWP));

	PrfCloseProfile(hiniProfile);
}


int GetINISettings(HWND hwnd)
/*
 * Get the relevent settings from the QUOTER.INI file.
 *
 * Returns	- 1 if successful
 * 		  0 on failure
 */
{
	HINI		hiniProfile;
	char		szMessage[200], szTitle[20];
	FONTMETRICS	fm;
	HPS		hps;
	unsigned long	ulSize;

	if ((hiniProfile = PrfOpenProfile(hab, "QUOTER.INI")) == NULLHANDLE) {
		WinLoadString(hab, 0, IDS_INIFAILED, sizeof(szMessage), szMessage);
		WinLoadString(hab, 0, IDS_ERROR, sizeof(szTitle), szTitle);
		WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_ERROR);
		return (0);
	}

	PrfQueryProfileString(hiniProfile, "QofD", "QDB", "", ps.szQuoteDB, sizeof(ps.szQuoteDB));
	PrfQueryProfileString(hiniProfile, "QofD", "ADB", "", ps.szAuthorDB, sizeof(ps.szAuthorDB));
	dirnoext(ps.szQuoteDB);
	dirnoext(ps.szAuthorDB);
	ulSize = sizeof(FATTRS);
	if (!PrfQueryProfileData(hiniProfile, "Quoter", "Font", &(prf.fattrs), &ulSize)) {
		/* no font information, so make up a default */
		hps = WinGetPS(hwndMain);
		GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);
		prf.fattrs.usRecordLength = sizeof(FATTRS);
		prf.fattrs.fsSelection = 0;
		prf.fattrs.lMatch = 0;
		strcpy(prf.fattrs.szFacename, fm.szFacename);
		prf.fattrs.idRegistry = 0;
		prf.fattrs.usCodePage = fm.usCodePage;
		prf.fattrs.lMaxBaselineExt = fm.lMaxBaselineExt;
		prf.fattrs.lAveCharWidth = fm.lAveCharWidth;
		prf.fattrs.fsType = 0;
		prf.fattrs.fsFontUse = 0;
		WinReleasePS(hps);
	}

	PrfCloseProfile(hiniProfile);
	return (1);
}
