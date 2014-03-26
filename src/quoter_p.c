/*
 * quoter_p.c
 *
 * Painting routines for the Quoteriser
 *
 *      Created: 27th February, 1997 (as paint.c)
 * Version 1.00: 9th April, 1997 (as paint.c)
 * Version 2.00: 21st December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "quoter.h"
#include "types.h"
#include "threads.h"
#include "typeset.h"
#include "pmutil.h"


HAB habPaint;
HWND hwndPaint;

/* internal function prototypes */
char **PaintTypesetArray(THREADINFO *, char **, char *, long);
MRESULT EXPENTRY PaintWinProc(HWND, ULONG, MPARAM, MPARAM);


void ThreadPaint(THREADINFO *pti)
/*
 * Thread used to draw the contents of the client window. It creates a
 * window procedure PaintWinProc() to handle calls from the main window
 */
{
	HMQ hmq;
	QMSG qmsg;

	habPaint = WinInitialize(0);
	hmq = WinCreateMsgQueue(habPaint, 0);
	WinRegisterClass(habPaint, "QPAINT", (PFNWP)PaintWinProc, 0, 0);

	/* create the paint window */
	hwndPaint = WinCreateWindow(HWND_OBJECT, "QPAINT", "", 0, 0, 0, 0, 0,
				    HWND_OBJECT, HWND_BOTTOM, 0, pti, NULL);

	/* go */
	while (WinGetMsg(habPaint, &qmsg, (HWND)NULL, 0, 0))
		WinDispatchMsg(habPaint, &qmsg);

	/* clean up */
	WinDestroyWindow(hwndPaint);
	WinDestroyMsgQueue(hmq);
	WinTerminate(habPaint);
}


MRESULT EXPENTRY PaintWinProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
/*
 * Window procedure for the paint thread. As well as the usual window messages,
 * it supports the following:
 *
 * QM_PAINT	- re-paint the screen
 * QM_TYPESET	- typeset the current string (into a metafile for later painting)
 * QM_COPY	- copy the screen contents to the clipboard in the format specified in mp1
 */
{
	static THREADINFO	*pti;
	static HPS		hps;
	static HMF		hmf;
	static RECTL		rectlMetafile;
	char			szString[210], *apszTypeset[3], *pszTemp;
	void			*pClipboard;
	HMF			hmfTemp;

	switch (message) {
		case WM_CREATE:
			pti = (THREADINFO *)PVOIDFROMMP(mp1);
			hps = WinGetPS(pti->hwndCaller);
			hmf = NULLHANDLE;
			return (FALSE);

		case QM_PAINT:
			hps = WinBeginPaint(pti->hwndCaller, hps, NULL);
			GpiResetPS(hps, GRES_ALL);
			ScrollMetafile(hps, NULLHANDLE, hwndVertScroll, hmf);
			WinEndPaint(hps);
			return (FALSE);

		case QM_TYPESET:
			if (hmf != NULLHANDLE)
				GpiDeleteMetaFile(hmf);
			PaintTypesetArray(pti, apszTypeset, szString, sizeof(szString));
			hmf = TypesetMetafile(habPaint, pti->hwndCaller, apszTypeset, &rectlMetafile, &(prf.fattrs));
			ScrollSetGeometry(pti->hwndCaller, NULLHANDLE, hwndVertScroll, &rectlMetafile);
			WinSendMsg(hwndVertScroll, SBM_SETPOS, MPFROMSHORT(0), NULL);
			return (FALSE);

		case QM_COPY:
			switch (SHORT1FROMMP(mp1)) {
				case CF_TEXT:
					PaintTypesetArray(pti, apszTypeset, szString, sizeof(szString));
					pszTemp = TypesetASCII(apszTypeset, 0);
					if ((pszTemp != NULL) && WinOpenClipbrd(habPaint)) {
						WinEmptyClipbrd(habPaint);
						DosAllocSharedMem(&pClipboard, 0, strlen(pszTemp) + 1, PAG_READ | PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE);
						strcpy((char *)pClipboard, pszTemp);
						WinSetClipbrdData(habPaint, (ULONG)pClipboard, CF_TEXT, CFI_POINTER);
						WinCloseClipbrd(habPaint);
					}
					free(pszTemp);
					return (FALSE);

				case CF_METAFILE:
					if (WinOpenClipbrd(habPaint)) {
						hmfTemp = GpiCopyMetaFile(hmf);
						WinEmptyClipbrd(habPaint);
						WinSetClipbrdData(habPaint, (ULONG)hmfTemp, CF_METAFILE, CFI_HANDLE);
						WinCloseClipbrd(habPaint);
					}
					return (FALSE);
			}
			return (FALSE);

		case WM_DESTROY:
			WinReleasePS(hps);
			return (FALSE);

		default:
			return (WinDefWindowProc(hwnd, message, mp1, mp2));
	}

	return (FALSE);
}


char **PaintTypesetArray(THREADINFO *pti, char **papszTypeset, char *pszString, long lSize)
/*
 * Set up the typeset array ready for passing to TypesetMetafile() or TypesetASCII().
 *
 * THREADINFO *pti	- the THREADINFO structure of the calling thread
 * char **papszTypeset	- the typeset array (output)
 * char *pszString	- some space belonging to the calling thread we can use
 * long lSize		- size of the space pointed to by pszString
 *
 * Returns		- papszTypeset
 */
{
	int	i;
	BOOL	bPrintAuthor, bPrintSource;

	/* initialise typeset array to be empty */
	for (i = 0; i < 3; i++)
		papszTypeset[i] = NULL;

	/* fill the typeset array according to current display mode */
	switch (pti->pps->iMode) {
		case Q_SEARCHING:
			WinLoadString(habPaint, 0, IDS_SEARCHING, lSize, pszString);
			papszTypeset[0] = pszString;
			break;

		case Q_QUOTE:
			if (pti->pps->pszText != NULL)
				papszTypeset[0] = pti->pps->pszText;
			bPrintAuthor = strlen(pti->pps->szAuthorCode) > 0;
			bPrintSource = strlen(pti->pps->pqi->szSource) > 0;
			if (bPrintAuthor || bPrintSource)  {
				strcpy(pszString, "<p>- ");
				if (bPrintAuthor) {
					strcat(pszString, pti->pps->pai->szGivenName);
					if ((strlen(pti->pps->pai->szSurname) > 0) &&
						    (strlen(pti->pps->pai->szGivenName) > 0))
						strcat(pszString, " ");
					strcat(pszString, pti->pps->pai->szSurname);
				}
				if (bPrintAuthor && bPrintSource)
					strcat(pszString, ", ");
				if (bPrintSource) {
					strcat(pszString, "<i>");
					strcat(pszString, pti->pps->pqi->szSource);
					strcat(pszString, "</i>");
				}
				papszTypeset[1] = pszString;
			}
			break;

		case Q_AUTHOR:
			strcpy(pszString, "<b>");
			strcat(pszString, pti->pps->pai->szGivenName);
			if ((strlen(pti->pps->pai->szSurname) > 0) &&
			    (strlen(pti->pps->pai->szGivenName) > 0))
				strcat(pszString, " ");
			strcat(pszString, pti->pps->pai->szSurname);
			strcat(pszString, "</b> (");
			strcat(pszString, pti->pps->pai->szBirthYear);
			strcat(pszString, " - ");
			strcat(pszString, pti->pps->pai->szDeathYear);
			strcat(pszString, ")<br><br>");
			papszTypeset[0] = pszString;
			if (pti->pps->pszDesc != NULL)
				papszTypeset[1] = pti->pps->pszDesc;
			break;
	}

	return (papszTypeset);
}
