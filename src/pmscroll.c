/*
 * pmscroll.c
 *
 * Generic scroll bar utilities.
 *
 *      Created: 4th December, 1997
 * Version 2.00: 4th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include "pmutil.h"


int ScrollMsgHandler(HWND hwndScroll, ULONG message, MPARAM mp1, MPARAM mp2, RECTL *prectlMain, int iStep)
/*
 * Generic routine for handling WM_VSCROLL and WM_HSCROLL messages. This routine will
 * update the position of the scroll bar appropriately and return the change in the
 * scroll bar's position.
 *
 * HWND hwndScroll	- window handle of the scroll bar being scrolled
 * ULONG message	- the message we're handling (WM_VSCROLL or WM_HSCROLL)
 * MPARAM mp1		- first m-parameter passed with the WM_VSCROLL message
 * MPARAM mp2		- second m-parameter passed with the WM_VSCROLL message
 * RECTL *prectlMain	- window rectangle of the window we are scrolling
 * int iStep		- distance to move scroll bar in response to SB_LINE*
 *
 * Returns		- the amount the scroll bar has moved by
 */
{
	int iInit, iFinal, iLower, iUpper;
	MRESULT mr;

	/* idiot protection */
	if ((message != WM_VSCROLL) && (message != WM_HSCROLL))
		return (0);

	/* get initial position of scroll bar */
	iFinal = iInit = SHORT1FROMMR(WinSendMsg(hwndScroll, SBM_QUERYPOS, NULL, NULL));

	/* get scroll bar limits */
	mr = WinSendMsg(hwndScroll, SBM_QUERYRANGE, NULL, NULL);
	iLower = SHORT1FROMMR(mr);
	iUpper = SHORT2FROMMR(mr);

	/* calculate final position of scroll bar according to the m-parameters */
	switch (SHORT2FROMMP(mp2)) {
		case 1: /* = SB_LINEUP, SB_LINELEFT */
			if ((iFinal = iInit - iStep) < iLower)
				iFinal = iLower;
			break;

		case 2: /* = SB_LINEDOWN, SB_LINERIGHT */
			if ((iFinal = iInit + iStep) > iUpper)
				iFinal = iUpper;
			break;

		case 3: /* = SB_PAGEUP, SB_PAGELEFT */
			if (message == WM_VSCROLL)
				iFinal = iInit - prectlMain->yTop + prectlMain->yBottom;
			else
				iFinal = iInit - prectlMain->xRight + prectlMain->xLeft;
			if (iFinal < iLower)
				iFinal = iLower;
			break;

		case 4: /* = SB_PAGEDOWN, SB_PAGERIGHT */
			if (message == WM_VSCROLL)
				iFinal = iInit + prectlMain->yTop - prectlMain->yBottom;
			else
				iFinal = iInit + prectlMain->xRight - prectlMain->xLeft;
			if (iFinal > iUpper)
				iFinal = iUpper;
			break;

		case SB_SLIDERPOSITION:
		case SB_SLIDERTRACK:
			iFinal = SHORT1FROMMP(mp2);
			break;
	}
	WinSendMsg(hwndScroll, SBM_SETPOS, MPFROMSHORT(iFinal), NULL);

	return (iFinal - iInit);
}


void ScrollMetafile(HPS hps, HWND hwndHorizScroll, HWND hwndVertScroll, HMF hmf)
/*
 * Translate and play a metafile so that it shows the correct portion according
 * the position of the scroll bars. The scrolls bars should have ranges
 * corresponding to the boundary of the metafile.
 *
 * HPS hps		- presentation space to play the metafile in
 * HWND hwndHorizScroll	- handle of horizontal scroll bar (NULLHANDLE if none)
 * HWND hwndVertScroll	- handle of vertical scroll bar (NULLHANDLE if none)
 * HMF hmf		- the metafile
 */
{
	long			alOptions[PMF_DEFAULTS + 1];
	MATRIXLF		matlf;
	POINTL			pointl;

	/* translate metafile according to scrollbar positions */
	if (hwndHorizScroll != NULLHANDLE)
		pointl.x = SHORT1FROMMR(WinSendMsg(hwndHorizScroll, SBM_QUERYPOS, NULL, NULL));
	else
		pointl.x = 0;
	if (hwndVertScroll != NULLHANDLE)
		pointl.y = SHORT1FROMMR(WinSendMsg(hwndVertScroll, SBM_QUERYPOS, NULL, NULL));
	else
		pointl.y = 0;
	GpiTranslate(hps, &matlf, TRANSFORM_REPLACE, &pointl);
	GpiSetDefaultViewMatrix(hps, 9L, &matlf, TRANSFORM_REPLACE);
			
	/* paint screen using the metafile */
	alOptions[PMF_SEGBASE] = 0L;
	alOptions[PMF_LOADTYPE] = LT_NOMODIFY;
	alOptions[PMF_RESOLVE] = RS_DEFAULT;
	alOptions[PMF_LCIDS] = LC_LOADDISC;
	alOptions[PMF_RESET] = RES_DEFAULT;
	alOptions[PMF_SUPPRESS] = SUP_DEFAULT;
	alOptions[PMF_COLORTABLES] = CTAB_REPLACE;
	alOptions[PMF_COLORREALIZABLE] = CREA_DEFAULT;
	alOptions[PMF_DEFAULTS] = DDEF_IGNORE;
	GpiPlayMetaFile(hps, hmf, PMF_DEFAULTS, alOptions, 0, 0, NULL);
}


void ScrollSetGeometry(HWND hwnd, HWND hwndHorizScroll, HWND hwndVertScroll, RECTL *prectl)
/*
 * Set the range and size of scroll bars such that they will scroll over the
 * given rectangle. The thumbs will be initialised to the top and left.
 * Unnecessary scroll bars will be disabled.
 *
 * HWND hwnd		- the window to have its scroll bars set
 * HWND hwndHorizScroll	- handle of the horizontal scroll bar (NULLHANDLE if none)
 * HWND hwndVertScroll  - handle of the vertical scroll bar (NULLHANDLE if none)
 * RECTL *prectl	- the rectangle the window will scroll over
 */
{
	RECTL rectlMain;
	int iRange;

	/* retrieve the size of window */
	WinQueryWindowRect(hwnd, &rectlMain);

	if (hwndHorizScroll != NULLHANDLE) {
		if ((rectlMain.xRight - rectlMain.xLeft) < (prectl->xRight - prectl->xLeft)) {
			WinEnableWindow(hwndHorizScroll, TRUE);
			iRange = (prectl->xRight - prectl->xLeft) - (rectlMain.xRight - rectlMain.xLeft);
			WinSendMsg(hwndHorizScroll, SBM_SETSCROLLBAR, MPFROMSHORT(0), MPFROM2SHORT(0, iRange));
			WinSendMsg(hwndHorizScroll, SBM_SETTHUMBSIZE, MPFROM2SHORT(rectlMain.xRight - rectlMain.xLeft, prectl->xRight - prectl->xLeft), NULL);
		} else {
			WinEnableWindow(hwndHorizScroll, FALSE);
		}
	}

	if (hwndVertScroll != NULLHANDLE) {
		if ((rectlMain.yTop - rectlMain.yBottom) < (prectl->yTop - prectl->yBottom)) {
			WinEnableWindow(hwndVertScroll, TRUE);
			iRange = (prectl->yTop - prectl->yBottom) - (rectlMain.yTop - rectlMain.yBottom);
			WinSendMsg(hwndVertScroll, SBM_SETSCROLLBAR, MPFROMSHORT(0), MPFROM2SHORT(0, iRange));
			WinSendMsg(hwndVertScroll, SBM_SETTHUMBSIZE, MPFROM2SHORT(rectlMain.yTop - rectlMain.yBottom, prectl->yTop - prectl->yBottom), NULL);
		} else {
			WinEnableWindow(hwndVertScroll, FALSE);
		}
	}
}
