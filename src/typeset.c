/*
 * typeset.c
 *
 * HTML typesetting functions.
 *
 *      Created: 26th November, 1997 (split from paint.c)
 * Version 2.00: 21st December, 1997
 * Version 2.10: 8th September, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef GENERIC
# define INCL_GPI
# include <os2.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "html.h"
#include "typeset.h"

/* internal function prototypes */
#ifndef GENERIC
void TypesetGpiString(HPS, char *, POINTL *, int);
void TypesetGpiWord(HPS, char *, int *, int *, int, POINTL *);
#endif

#ifndef GENERIC
HMF TypesetMetafile(HAB hab, HWND hwnd, char **papszString, RECTL *prectlBoundary, FATTRS *pfattrs)
/*
 * Typeset an HTML string into a metafile, formatted for the given window.
 *
 * HAB hab			- handle to our anchor block
 * HWND hwnd			- the handle of the window to typeset for
 * char **papszString		- sequence of strings to be typeset, NULL to terminate
 * RECTL *prectlBoundary	- structure to receive boundary information
 * FATTRS *pfattrs		- font attribute structure for the font to typeset in
 *
 * Returns			- handle to the new metafile
 */
{
	DEVOPENSTRUC	dos;
	HDC		hdc;
	HPS		hps;
	SIZEL		sizel;
	FONTMETRICS	fm;
	FATTRS		fattrs;
	RECTL		rectl;
	POINTL		ptPos;
	int		i;

	/* open metafile */
	dos.pszLogAddress = NULL;
	dos.pszDriverName = "DISPLAY";
	hdc = DevOpenDC(hab, OD_METAFILE, "*", 2, (PDEVOPENDATA)&dos, NULLHANDLE);
	sizel.cx = sizel.cy = 0;
	hps = GpiCreatePS(hab, hdc, &sizel, PU_PELS | GPIA_ASSOC);

	/* create our fonts */
	memcpy(&fattrs, pfattrs, sizeof(FATTRS));
	GpiCreateLogFont(hps, NULL, IDF_NORMAL, &fattrs);
	fattrs.fsSelection = FATTR_SEL_ITALIC;
	GpiCreateLogFont(hps, NULL, IDF_ITALIC, &fattrs);
	fattrs.fsSelection = FATTR_SEL_BOLD;
	GpiCreateLogFont(hps, NULL, IDF_BOLD, &fattrs);
	fattrs.fsSelection = FATTR_SEL_BOLD | FATTR_SEL_ITALIC;
	GpiCreateLogFont(hps, NULL, IDF_BOLDITALIC, &fattrs);

	/* typeset the strings */
	GpiErase(hps);
	WinQueryWindowRect(hwnd, &rectl);
	GpiSetCharSet(hps, IDF_NORMAL);
	GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);
	ptPos.x = 0;
	ptPos.y = rectl.yTop - fm.lMaxBaselineExt;
	for (i = 0; papszString[i] != NULL; i++)
		TypesetGpiString(hps, papszString[i], &ptPos, rectl.xRight);

	/* destroy our fonts */
	GpiSetCharSet(hps, LCID_DEFAULT);
	GpiDeleteSetId(hps, IDF_NORMAL);
	GpiDeleteSetId(hps, IDF_ITALIC);
	GpiDeleteSetId(hps, IDF_BOLD);
	GpiDeleteSetId(hps, IDF_BOLDITALIC);

	/* calculate boundary information */
	prectlBoundary->xLeft = 0;
	prectlBoundary->yTop = rectl.yTop;
	prectlBoundary->xRight = rectl.xRight;
	prectlBoundary->yBottom = ptPos.y;

	/* close metafile */
	GpiAssociate(hps, NULLHANDLE);
	GpiDestroyPS(hps);
	return (DevCloseDC(hdc));
}
#endif


#ifndef GENERIC
void TypesetGpiString(HPS hps, char *pszString, POINTL *pptlPos, int iWidth)
/*
 * Typeset a string using some simple HTML and display it using GPI calls.
 *
 * HPS hps		- the handle to the presentation space on which to typeset
 * char *pszString	- the string to be typeset
 * POINTL *pptlPos	- starting position; end position is returned here
 * int iWidth		- the paragraph width
 */
{
	char		*pszCurrent, *pszNext;
	char		szChunk[30], szWord[30];
	int		aiFont[30], aiStart[30];
	int		iMacro, iTag, i;
	long		lWordWidth;
	BOOL		bHTMLEnd, bGotWord;
	BOOL		bItalics, bBold;
	POINTL		aptlExtent[TXTBOX_COUNT];
	POINTL		ptlCursor;
	FONTMETRICS	fm;

	pszCurrent = pszString;
	bHTMLEnd = FALSE;
	bItalics = bBold = FALSE;
	GpiSetCharSet(hps, IDF_NORMAL);
	GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);
	memcpy(&ptlCursor, pptlPos, sizeof(POINTL));
	i = 0;
	aiFont[0] = IDF_NORMAL;
	aiStart[0] = 0;
	while (!bHTMLEnd) {

		/* construct the next word we have to typeset */
		szWord[0] = '\0';
		bGotWord = FALSE;
		lWordWidth = 0;
		aiFont[0] = aiFont[i];
		i = 0;
		while (!bGotWord) {
			iTag = 0;
			switch (HTMLGetNextChunk(pszCurrent, szChunk, &pszNext)) {
				case HTML_WORD_END:
					bGotWord = TRUE;
				case HTML_WORD_MID:
					strcat(szWord, szChunk);
					GpiQueryTextBox(hps, strlen(szChunk), szChunk, TXTBOX_COUNT, aptlExtent);
					lWordWidth += aptlExtent[4].x - aptlExtent[1].x;
					break;

				case HTML_TAG_END:
					bGotWord = TRUE;
				case HTML_TAG_MID:
					switch (iTag = HTMLParseTag(szChunk)) {
						case HTML_PARAGRAPH:
						case HTML_LINEBREAK:
							bGotWord = TRUE;
							break;

						case HTML_ITALICS_START:
							bItalics = TRUE;
							GpiSetCharSet(hps, (bBold)? IDF_BOLDITALIC : IDF_ITALIC);
							aiFont[++i] = (bBold)? IDF_BOLDITALIC : IDF_ITALIC;
							aiStart[i] = strlen(szWord);
							break;

						case HTML_ITALICS_END:
							bItalics = FALSE;
							GpiSetCharSet(hps, (bBold)? IDF_BOLD : IDF_NORMAL);
							aiFont[++i] = (bBold)? IDF_BOLD : IDF_NORMAL;
							aiStart[i] = strlen(szWord);
							break;

						case HTML_BOLD_START:
							bBold = TRUE;
							GpiSetCharSet(hps, (bItalics)? IDF_BOLDITALIC : IDF_BOLD);
							aiFont[++i] = (bItalics)? IDF_BOLDITALIC : IDF_BOLD;
							aiStart[i] = strlen(szWord);
							break;

						case HTML_BOLD_END:
							bBold = FALSE;
							GpiSetCharSet(hps, (bItalics)? IDF_ITALIC : IDF_NORMAL);
							aiFont[++i] = (bItalics)? IDF_ITALIC : IDF_NORMAL;
							aiStart[i] = strlen(szWord);
							break;
					}
					break;

				case HTML_MACRO_END:
					bGotWord = TRUE;
				case HTML_MACRO_MID:
					if ((iMacro = HTMLParseMacro(szChunk)) != -1)
						sprintf(szChunk, "%c", iMacro);
					strcat(szWord, szChunk);
					GpiQueryTextBox(hps, strlen(szChunk), szChunk, TXTBOX_COUNT, aptlExtent);
					lWordWidth += aptlExtent[4].x - aptlExtent[1].x;
					break;

				case HTML_END:
					bGotWord = TRUE;
					bHTMLEnd = TRUE;
					break;
			}
			pszCurrent = pszNext;
		}


		if (szWord[0] != '\0')
			if ((ptlCursor.x + lWordWidth) <= iWidth) {
				/* the word will fit on this line */
				TypesetGpiWord(hps, szWord, aiFont, aiStart, i + 1, &ptlCursor);
				ptlCursor.x += lWordWidth + fm.lAveCharWidth;
			} else {
				/* the word needs to go on a new line */
				ptlCursor.x = 0;
				ptlCursor.y -= fm.lMaxBaselineExt;
				TypesetGpiWord(hps, szWord, aiFont, aiStart, i + 1, &ptlCursor);
				ptlCursor.x += lWordWidth + fm.lAveCharWidth;
			}
		if ((iTag == HTML_PARAGRAPH) || (iTag == HTML_LINEBREAK)) {
			/* insert a new line */
			ptlCursor.x = 0;
			ptlCursor.y -= fm.lMaxBaselineExt;
			if (iTag == HTML_PARAGRAPH)
				ptlCursor.y -= fm.lMaxBaselineExt;
		}
	}

	/* copy the finishing position into ptlPos to be returned */
	memcpy(pptlPos, &ptlCursor, sizeof(POINTL));
}
#endif


#ifndef GENERIC
void TypesetGpiWord(HPS hps, char *pszWord, int *aiFont, int *aiStart, int n, POINTL *pptlPos)
/*
 * Print a single word using GPI calls.
 *
 * HPS hps		- the handle of the presentation space to print on
 * char *pszWord	- the word to print
 * int *aiFont		- the vector containing IDs of fonts to use
 * int *aiStart		- the vector containing the starting positions for fonts
 * int n		- the length of aiFont and aiStart
 * POINTL *pptlPos	- the point to print the word at
 */
{
	char	szBlock[30], *pszCursor;
	POINTL	aptlExtent[TXTBOX_COUNT];
	POINTL	ptlCursor;
	int	i, k;

	memcpy(&ptlCursor, pptlPos, sizeof(POINTL));
	pszCursor = pszWord;
	for (i = 0; i < n; i++)
		if (pszCursor[0] != '\0') {
			if (i == (n - 1)) {
				strcpy(szBlock, pszCursor);
				k = strlen(pszCursor);
			} else {
				if ((k = aiStart[i + 1] - aiStart[i]) > 0) {
					strncpy(szBlock, pszCursor, k);
					szBlock[k] = '\0';
					pszCursor += k;
				}
			}
			if (k > 0) {
				GpiSetCharSet(hps, aiFont[i]);
				GpiCharStringAt(hps, &ptlCursor, strlen(szBlock), szBlock);
				GpiQueryTextBox(hps, strlen(szBlock), szBlock, TXTBOX_COUNT, aptlExtent);
				memcpy(&ptlCursor, aptlExtent + 4, sizeof(POINTL));
			}
		}
}
#endif


char *TypesetASCII(char **papszString, int iWidth)
/*
 * Typeset an HTML string into ASCII text, simplifying the HTML where neccessary.
 * Space for the string will be allocated.
 *
 * char **papszString	- sequence of strings to be typeset, NULL to terminate
 * int iWidth		- the maximum width of a line of text (0 if no maximum)
 *
 * Returns		- NULL if memory could not be allocated
 *			  pointer to the typeset string otherwise
 */
{
	char	*pszRet, *pszCurrent, *pszNext;
	char	szChunk[30], szWord[30];
	int	i, iCounter, iTag, iMacro;
	int	bHTMLEnd, bGotWord, bFirst;

	/* allocate memory */
	iCounter = 0;
	for (i = 0; papszString[i] != NULL; i++)
		iCounter += strlen(papszString[i]);
	if ((pszRet = (char *)malloc(iCounter)) == NULL)
		return (NULL);

	/* typeset the strings */
	iCounter = 0;
	for (i = 0; papszString[i] != NULL; i++) {
		pszCurrent = papszString[i];
		bHTMLEnd = 0;
		while (!bHTMLEnd) {

			/* construct the next word to typeset */
			szWord[0] = '\0';
			bGotWord = 0;
			while (!bGotWord) {
				iTag = 0;
				switch (HTMLGetNextChunk(pszCurrent, szChunk, &pszNext)) {
					case HTML_WORD_END:
						bGotWord = 1;
					case HTML_WORD_MID:
						strcat(szWord, szChunk);
						break;

					case HTML_TAG_END:
						bGotWord = 1;
					case HTML_TAG_MID:
						switch (iTag = HTMLParseTag(szChunk)) {
							case HTML_LINEBREAK:
							case HTML_PARAGRAPH:
								bGotWord = 1;
								break;

							/*
							 * We can't render any font changes
							 * in ASCII, so HTML_ITALICS_*, etc.,
							 * are ignored here.
							 */
						}
						break;

					case HTML_MACRO_END:
						bGotWord = 1;
					case HTML_MACRO_MID:
						bGotWord = 1;
						if ((iMacro = HTMLParseMacro(szChunk)) != -1)
							sprintf(szChunk, "%c", iMacro);
						strcat(szWord, szChunk);
						break;

					case HTML_END:
						bGotWord = 1;
						bHTMLEnd = 1;
						break;
				}
				pszCurrent = pszNext;
			}

			/* add the word to the output string */
			if (szWord[0] != '\0') {
				bFirst = iCounter == 0;
				iCounter += strlen(szWord) + 1;
				if ((iCounter <= iWidth) || (iWidth == 0)) {
					/* the word will fit on this line */
					if (!bFirst)
						strcat(pszRet, " ");
					strcat(pszRet, szWord);
				} else {
					/* the word needs to go on a new line */
					strcat(pszRet, "\n");
					strcat(pszRet, szWord);
					iCounter = strlen(szWord);
				}
			}

			/* check for user-enforced new-lines */
			if (iTag == HTML_LINEBREAK) {
				strcat(pszRet, "\n");
				iCounter = 0;
			} else if (iTag == HTML_PARAGRAPH) {
				strcat(pszRet, "\n\n");
				iCounter = 0;
			}
		}
	}

	return (pszRet);
}
