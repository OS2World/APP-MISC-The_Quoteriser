/*
 * settings.c
 *
 * Window procedures for settings dialogues and functions for managing
 * Quoteriser settings.
 *
 *      Created: 16th July, 1997 (split from quoter.c)
 * Version 2.00: 19th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#define INCL_WIN
#include <os2.h>
#include <string.h>
#include "quoter.h"
#include "general.h"
#include "threads.h"
#include "types.h"

PROFILE	prf;

/* internal function prototypes */
char *GetDefaultDocPath(char *);


MRESULT EXPENTRY SettingsBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the settings box. Used to set preferences for using the
 * Quoteriser.
 */
{
	char	szMessage[CCHMAXPATH], szTitle[20], szFamilyName[FACESIZE];
	FONTDLG	fd;

	switch (message) {
		case WM_INITDLG:
			WinSendDlgItemMsg(hwnd, IDC_SET_EDITOR, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			WinSendDlgItemMsg(hwnd, IDC_SET_EDPARAMS, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			WinSendDlgItemMsg(hwnd, IDC_SET_DATAPATH, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			WinSendDlgItemMsg(hwnd, IDC_SET_DOCPATH, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			WinSendDlgItemMsg(hwnd, IDC_SET_BROWSER, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			WinSendDlgItemMsg(hwnd, IDC_SET_BROWSERPARAMS, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			WinSetDlgItemText(hwnd, IDC_SET_EDITOR, prf.szEditor);
			WinSetDlgItemText(hwnd, IDC_SET_EDPARAMS, prf.szEditorParams);
			WinSetDlgItemText(hwnd, IDC_SET_DATAPATH, prf.szDataPath);
			WinSetDlgItemText(hwnd, IDC_SET_DOCPATH, prf.szDocPath);
			WinSetDlgItemText(hwnd, IDC_SET_BROWSER, prf.szBrowser);
			WinSetDlgItemText(hwnd, IDC_SET_BROWSERPARAMS, prf.szBrowserParams);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_SET_FONT:
					szFamilyName[0] = '\0';
					WinLoadString(hab, 0, IDS_SAMPLE, sizeof(szMessage), szMessage);
					WinLoadString(hab, 0, IDS_TITLE_FONT, sizeof(szTitle), szTitle);
					memset(&fd, 0, sizeof(FONTDLG));
					fd.cbSize = sizeof(FONTDLG);
					fd.pszTitle = szTitle;
					fd.pszPreview = szMessage;
					fd.pszFamilyname = szFamilyName;
					fd.usFamilyBufLen = sizeof(szMessage);
					fd.fl = FNTS_CENTER | FNTS_INITFROMFATTRS | FNTS_NOSYNTHESIZEDFONTS | FNTS_RESETBUTTON;
					fd.clrFore = CLR_BLACK;
					fd.clrBack = CLR_WHITE;
					memcpy(&(fd.fAttrs), &(prf.fattrs), sizeof(FATTRS));
					WinFontDlg(HWND_DESKTOP, hwnd, &fd);
					if (fd.lReturn == DID_OK) {
						memcpy(&(prf.fattrs), &(fd.fAttrs), sizeof(FATTRS));
						WinSendMsg(hwndPaint, QM_TYPESET, NULL, NULL);
						WinInvalidateRect(hwndMain, NULL, TRUE);
					}
					return (FALSE);

				case IDC_SET_OKAY:
					WinQueryDlgItemText(hwnd, IDC_SET_EDITOR, sizeof(prf.szEditor), prf.szEditor);
					WinQueryDlgItemText(hwnd, IDC_SET_EDPARAMS, sizeof(prf.szEditorParams), prf.szEditorParams);
					WinQueryDlgItemText(hwnd, IDC_SET_DATAPATH, sizeof(prf.szDataPath), prf.szDataPath);
					WinQueryDlgItemText(hwnd, IDC_SET_DOCPATH, sizeof(prf.szDocPath), prf.szDocPath);
					WinQueryDlgItemText(hwnd, IDC_SET_BROWSER, sizeof(prf.szBrowser), prf.szBrowser);
					WinQueryDlgItemText(hwnd, IDC_SET_BROWSERPARAMS, sizeof(prf.szBrowserParams), prf.szBrowserParams);
					SetINISettings();
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);

				case IDC_SET_DEFAULT:
					WinLoadString(hab, 0, IDS_DEFAULT, sizeof(szMessage), szMessage);
					WinLoadString(hab, 0, IDS_CONFIRM, sizeof(szTitle), szTitle);
					if (WinMessageBox(HWND_DESKTOP, hwnd, szMessage, szTitle, 0, MB_YESNO) == MBID_YES) {
						WinSetDlgItemText(hwnd, IDC_SET_EDITOR, "E.EXE");
						WinSetDlgItemText(hwnd, IDC_SET_EDPARAMS, "%");
						WinSetDlgItemText(hwnd, IDC_SET_DATAPATH, "");
						WinSetDlgItemText(hwnd, IDC_SET_DOCPATH, GetDefaultDocPath(szMessage));
						WinSetDlgItemText(hwnd, IDC_SET_BROWSER, "EXPLORE.EXE");
						WinSetDlgItemText(hwnd, IDC_SET_BROWSERPARAMS, "%");
					}
					return (FALSE);

				case IDC_SET_CANCEL:
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);
			}

		default:
			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


MRESULT EXPENTRY QuoteOfTheDayBox(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for modifying the settings used by the Quote-of-the-Day
 * program.
 */
{
	HINI		hini;
	FILEDLG		fd;
	char		szString[CCHMAXPATH];

	switch (message) {
		case WM_INITDLG:
			hini = PrfOpenProfile(hab, "QUOTER.INI");
			PrfQueryProfileString(hini, "QofD", "QDB", "", szString, sizeof(szString));
			WinSendDlgItemMsg(hwnd, IDC_QOTD_QDB, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			WinSetDlgItemText(hwnd, IDC_QOTD_QDB, szString);
			WinSendDlgItemMsg(hwnd, IDC_QOTD_ADB, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0);
			PrfQueryProfileString(hini, "QofD", "ADB", "", szString, sizeof(szString));
			WinSetDlgItemText(hwnd, IDC_QOTD_ADB, szString);
			PrfCloseProfile(hini);
			return (FALSE);

		case WM_COMMAND:
			switch (SHORT1FROMMP(mp1)) {
				case IDC_QOTD_QDB_BROWSE:
					memset(&fd, 0, sizeof(FILEDLG));
					fd.cbSize = sizeof(FILEDLG);
					fd.fl = FDS_OPEN_DIALOG | FDS_CENTER;
					dircat(strcpy(fd.szFullFile, prf.szDataPath), szQuoteExt);
					WinLoadString(hab, 0, IDS_TITLE_BROWSE, sizeof(szString), szString);
					fd.pszTitle = szString;
					WinFileDlg(HWND_DESKTOP, hwnd, &fd);
					if (fd.lReturn == DID_OK)
						WinSetDlgItemText(hwnd, IDC_QOTD_QDB, fd.szFullFile);
					return (FALSE);

				case IDC_QOTD_ADB_BROWSE:
					memset(&fd, 0, sizeof(FILEDLG));
					fd.cbSize = sizeof(FILEDLG);
					fd.fl = FDS_OPEN_DIALOG | FDS_CENTER;
					dircat(strcpy(fd.szFullFile, prf.szDataPath), szAuthorExt);
					WinLoadString(hab, 0, IDS_TITLE_BROWSE, sizeof(szString), szString);
					fd.pszTitle = szString;
					WinFileDlg(HWND_DESKTOP, hwnd, &fd);
					if (fd.lReturn == DID_OK)
						WinSetDlgItemText(hwnd, IDC_QOTD_ADB, fd.szFullFile);
					return (FALSE);

				case IDC_QOTD_OKAY:
					hini = PrfOpenProfile(hab, "QUOTER.INI");
					WinQueryDlgItemText(hwnd, IDC_QOTD_QDB, sizeof(szString), szString);
					PrfWriteProfileString(hini, "QofD", "QDB", szString);
					WinQueryDlgItemText(hwnd, IDC_QOTD_ADB, sizeof(szString), szString);
					PrfWriteProfileString(hini, "QofD", "ADB", szString);
					PrfCloseProfile(hini);
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);

				case IDC_QOTD_CANCEL:
					WinDismissDlg(hwnd, TRUE);
					return (FALSE);
			}

		default:
			return (WinDefDlgProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


void GetINISettings(void)
/*
 * Get the relevent settings from the QUOTER.INI file.
 */
{
	HINI		hiniProfile;
	HPS		hps;
	FONTMETRICS	fm;
	unsigned long	ulSize;
	char		szString[CCHMAXPATH];

	hiniProfile = PrfOpenProfile(hab, "QUOTER.INI");

	PrfQueryProfileString(hiniProfile, "Quoter", "Editor", "E.EXE", prf.szEditor, sizeof(prf.szEditor));
	PrfQueryProfileString(hiniProfile, "Quoter", "EditorParams", "%", prf.szEditorParams, sizeof(prf.szEditorParams));
	PrfQueryProfileString(hiniProfile, "Quoter", "DataPath", "", prf.szDataPath, sizeof(prf.szDataPath));
	PrfQueryProfileString(hiniProfile, "Quoter", "DocPath", GetDefaultDocPath(szString), prf.szDocPath, sizeof(prf.szDocPath));
	PrfQueryProfileString(hiniProfile, "Quoter", "QuoteDB1", "", prf.szQuoteDB1, sizeof(prf.szQuoteDB1));
	PrfQueryProfileString(hiniProfile, "Quoter", "AuthorDB1", "", prf.szAuthorDB1, sizeof(prf.szAuthorDB1));
	PrfQueryProfileString(hiniProfile, "Quoter", "Browser", "EXPLORE.EXE", prf.szBrowser, sizeof(prf.szBrowser));
	PrfQueryProfileString(hiniProfile, "Quoter", "BrowserParams", "%", prf.szBrowserParams, sizeof(prf.szBrowserParams));
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
}


void SetINISettings(void)
/*
 * Set the relevent settings in the QUOTER.INI file.
 */
{
	HINI	hiniProfile;

	hiniProfile = PrfOpenProfile(hab, "QUOTER.INI");

	PrfWriteProfileString(hiniProfile, "Quoter", "Editor", prf.szEditor);
	PrfWriteProfileString(hiniProfile, "Quoter", "EditorParams", prf.szEditorParams);
	PrfWriteProfileString(hiniProfile, "Quoter", "DataPath", prf.szDataPath);
	PrfWriteProfileString(hiniProfile, "Quoter", "DocPath", prf.szDocPath);
	PrfWriteProfileString(hiniProfile, "Quoter", "Browser", prf.szBrowser);
	PrfWriteProfileString(hiniProfile, "Quoter", "BrowserParams", prf.szBrowserParams);
	PrfWriteProfileData(hiniProfile, "Quoter", "Font", &(prf.fattrs), sizeof(FATTRS));

	PrfCloseProfile(hiniProfile);
}


void SetUsedDB(char *pszQuoteDB1, char *pszAuthorDB1)
/*
 * Save the last used databases to the QUOTER.INI file.
 *
 * char *pszQuoteDB1	- the last used quote database
 * char *pszAuthorDB1	- the last used author database
 */
{
	HINI	hiniProfile;

	hiniProfile = PrfOpenProfile(hab, "QUOTER.INI");

	if (pszQuoteDB1 != NULL)
		PrfWriteProfileString(hiniProfile, "Quoter", "QuoteDB1", pszQuoteDB1);
	if (pszAuthorDB1 != NULL)
		PrfWriteProfileString(hiniProfile, "Quoter", "AuthorDB1", pszAuthorDB1);

	PrfCloseProfile(hiniProfile);
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
	if (!PrfQueryProfileData(hiniProfile, "Quoter", "WindowPos", &swpMain, &ulSize)) {
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
	PrfWriteProfileData(hiniProfile, "Quoter", "WindowPos", &swpMain, sizeof(SWP));

	PrfCloseProfile(hiniProfile);
}


char *GetDefaultDocPath(char *pszDocPath)
/*
 * Compile the default path to the documentation, i.e. the 'doc' directory in
 * the parent directory of the current directory. I might have used getcwd()
 * for this, but EMX puts forward slashes in its getcwd(), so...
 *
 * char *pszDocPath	- variable to receive the default path
 *
 * Returns		- pszDocPath
 */
{
	unsigned long	ulDrive, ul;
	char		szDrive[4];

	DosQueryCurrentDisk(&ulDrive, &ul);
	DosQueryCurrentDir(ulDrive, pszDocPath, &ul);
	sprintf(szDrive, "%c:\\", 'A' + (int)ulDrive - 1);
	dirsimp(dircat(strpre(pszDocPath, szDrive), "..\\DOC"));

	return (pszDocPath);
}
