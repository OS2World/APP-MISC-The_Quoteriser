/*
 * qthreads.c
 *
 * Threads for use with quote databases.
 *
 *      Created: 27th January, 1997
 * Version 1.00: 10th April, 1997
 * Version 2.00: 18th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "quotes.h"
#include "threads.h"
#include "types.h"


void ThreadFindAllQuotes(THREADINFO *pti)
/*
 * Find all quotes in a database. The code of each quote is returned to the
 * calling window by sending a QM_FOUND message to it with the first parameter
 * set to be a pointer to the code. The thread sends a QM_DONE message to the
 * caller when it is finished.
 */
{
	HAB		hab;
	HMQ		hmq;
	QUOTESEARCH	qs;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* perform the search */
	if (QuoteFullSearchInit(&(pti->pps->qdb), &qs)) {
		WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);
		while (QuoteFullSearchNext(&qs))
			WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);
	}
	WinSendMsg(pti->hwndCaller, QM_DONE, 0, 0);

	/* clean up */
	QuoteFullSearchEnd(&qs);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}


void ThreadFindKeywords(THREADINFO *pti)
/*
 * Find all quotes in a database with the keyword given in the pData field of
 * the THREADINFO structure. The code of each quote is returned to the calling
 * window by sending a QM_FOUND message to it with the first parameter set to
 * be a pointer to the code. The thread sends a QM_DONE message to the caller
 * when it is finished.
 */
{
	HAB		hab;
	HMQ		hmq;
	QUOTESEARCH	qs;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* perform the search */
	switch (QuoteKeySearchInit(&(pti->pps->qdb), (char *)(pti->pData), &qs)) {
		case -1:
			WinSendMsg(pti->hwndCaller, QM_ERROR, 0, 0);
			break;

		case 1:
			WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);
			while (QuoteKeySearchNext(&qs))
				WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);

		case 0:
			WinSendMsg(pti->hwndCaller, QM_DONE, 0, 0);
			break;
	}

	/* clean up */
	QuoteKeySearchEnd(&qs);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}


void ThreadFindQuoteText(THREADINFO *pti)
/*
 * Find all quotes in a database containing the text given in the pData field
 * of the THREADINFO structure. The code of each quote is returned to the
 * calling window by sending a QM_FOUND message to it with the first parameter
 * set to be a pointer to the code. The thread sends a QM_DONE message to the
 * caller when it is finished.
 */
{
	HAB		hab;
	HMQ		hmq;
	QUOTESEARCH	qs;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* perform the search */
	switch (QuoteTextSearchInit(&(pti->pps->qdb), (char *)(pti->pData), &qs)) {
		case -1:
			WinSendMsg(pti->hwndCaller, QM_ERROR, 0, 0);
			break;

		case 1:
			WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);
			while (QuoteTextSearchNext(&qs))
				WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);

		case 0:
			WinSendMsg(pti->hwndCaller, QM_DONE, 0, 0);
			break;
	}

	/* clean up */
	QuoteTextSearchEnd(&qs);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}


void ThreadFindAuthorsQuotes(THREADINFO *pti)
/*
 * Find all quotes in a database from the author whose code is given in the
 * pData field of the the THREADINFO structure. The code of each quote is
 * returned to the calling window by sending a QM_FOUND message to it with the
 * first parameter set to be a pointer to the code. The thread sends a QM_DONE
 * message to the caller when it is finished.
 */
{
	HAB		hab;
	HMQ		hmq;
	QUOTESEARCH	qs;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* perform the search */
	if (QuoteAuthorSearchInit(&(pti->pps->qdb), (char *)(pti->pData), &qs)) {
		WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);
		while (QuoteAuthorSearchNext(&qs))
			WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);
	}
	WinSendMsg(pti->hwndCaller, QM_DONE, 0, 0);

	/* clean up */
	QuoteAuthorSearchEnd(&qs);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}


void ThreadFindRandomQuote(THREADINFO *pti)
/*
 * Choose a random quote from a database. When one has been chosen, a QM_FOUND
 * message is sent to the calling window with the first parameter set to be a
 * pointer to the code chosen. If the database is empty, place NULL in this
 * first parameter instead.
 */
{
	HAB		hab;
	HMQ		hmq;
	QUOTESEARCH	qs;
	int		i, iQuote;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* count the number of quotes in the database */
	i = 0;
	if (QuoteFullSearchInit(&(pti->pps->qdb), &qs)) {
		i++;
		while (QuoteFullSearchNext(&qs))
			i++;
	}
	QuoteFullSearchEnd(&qs);

	if (i == 0)
		/* empty database */
		WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(NULL), 0);
	else {
		/* choose a random quote */
		if (i > 1) {
			srand(time(NULL));
			iQuote = rand() % i;
		} else
			iQuote = 0;
		for (QuoteFullSearchInit(&(pti->pps->qdb), &qs), i = 0; i < iQuote; QuoteFullSearchNext(&qs), i++);
		WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(qs.szLastCode), 0);
		QuoteFullSearchEnd(&qs);
	}

	/* clean up */
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}


void ThreadImportQuotes(THREADINFO *pti)
/*
 * Import a quote database in pti->pps->pqdb. The pData entry in pti should
 * point to an IMPORTINFO structure describing the import parameters.
 */
{
	HAB		hab;
	HMQ		hmq;
	QUOTESEARCH	qs;
	IMPORTINFO	*pii;
	BOOL		bOkay;
	char		szString[100];
	QUOTEINFO	*pqi;
	char		*pszText;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* import, quote by quote */
	pii = (IMPORTINFO *)pti->pData;
	if (QuoteFullSearchInit((QUOTEDB *)pii->pdb, &qs)) {
		do {
			bOkay = !QuoteExists(&(pti->pps->qdb), qs.szLastCode);
			if (!bOkay) {
				if (pii->bAsk) {
					sprintf(szString, "%s exists. Replace it?", qs.szLastCode);
					if (WinMessageBox(HWND_DESKTOP, pti->hwndCaller, szString, "Confirmation", 0, MB_QUERY | MB_YESNO) == MBID_YES)
						bOkay = TRUE;
				} else if (pii->bReplace)
						bOkay = TRUE;
			}
			if (bOkay) {
				QuoteGetQuote((QUOTEDB *)pii->pdb, qs.szLastCode, &pqi, &pszText);
				QuoteAddQuote(&(pti->pps->qdb), qs.szLastCode, pqi, pszText);
				QuoteFreeQuote(&pqi, &pszText);
			}
		} while (QuoteFullSearchNext(&qs));
	}

	/* tell the calling window that we're finished */
	WinSendMsg(pti->hwndCaller, QM_DONE, NULL, NULL);

	/* clean up */
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}
