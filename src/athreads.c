/*
 * athreads.c
 *
 * Threads for use with author databases.
 *
 *      Created: 27th January, 1997
 * Version 1.00: 19th March, 1997
 * Version 2.00: 18th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdlib.h>
#include <stdio.h>
#include "authors.h"
#include "threads.h"
#include "types.h"


void ThreadFindAllAuthors(THREADINFO *pti)
/*
 * Start a thread to find all authors in the open database. 
 */
{
	HAB		hab;
	HMQ		hmq;
	AUTHORSEARCH	as;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* perform the search */
	if (AuthorFullSearchInit(&(pti->pps->adb), &as)) {
		WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(as.szLastCode), 0);
		while (AuthorFullSearchNext(&as))
			WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(as.szLastCode), 0);
	}
	WinSendMsg(pti->hwndCaller, QM_DONE, 0, 0);

	/* clean up */
	AuthorFullSearchEnd(&as);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}


void ThreadFindDescription(THREADINFO *pti)
/*
 * Start a thread to find all authors in the open database with a given string
 * in their description.
 */
{
	HAB		hab;
	HMQ		hmq;
	AUTHORSEARCH	as;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* perform the search */
	switch (AuthorDescSearchInit(&(pti->pps->adb), (char *)(pti->pData), &as)) {
		case -1:
			WinSendMsg(pti->hwndCaller, QM_ERROR, 0, 0);
			break;

		case 1:
			WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(as.szLastCode), 0);
			while (AuthorDescSearchNext(&as))
				WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(as.szLastCode), 0);

		case 0:
			WinSendMsg(pti->hwndCaller, QM_DONE, 0, 0);
			break;
	}

	/* clean up */
	AuthorDescSearchEnd(&as);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}


void ThreadFindAuthorName(THREADINFO *pti)
/*
 * Start a thread to find all authors in the open database with a given name.
 */
{
	HAB		hab;
	HMQ		hmq;
	AUTHORSEARCH	as;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* perform the search */
	switch (AuthorNameSearchInit(&(pti->pps->adb), (char *)(pti->pData), &as)) {
		case -1:
			WinSendMsg(pti->hwndCaller, QM_ERROR, 0, 0);
			break;

		case 1:
			WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(as.szLastCode), 0);
			while (AuthorNameSearchNext(&as))
				WinSendMsg(pti->hwndCaller, QM_FOUND, MPFROMP(as.szLastCode), 0);

		case 0:
			WinSendMsg(pti->hwndCaller, QM_DONE, 0, 0);
			break;
	}

	/* clean up */
	AuthorNameSearchEnd(&as);
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}


void ThreadImportAuthors(THREADINFO *pti)
/*
 * Import a quote database in pti->pps->padb. The pData entry in pti should
 * point to an IMPORTINFO structure describing the import parameters.
 */
{
	HAB		hab;
	HMQ		hmq;
	AUTHORSEARCH	as;
	IMPORTINFO	*pii;
	BOOL		bOkay;
	char		szString[100];
	AUTHORINFO	*pai;
	char		*pszDesc;

	/* set up a message queue */
	if ((hab = WinInitialize(0)) == NULLHANDLE)
		return;
	hmq = WinCreateMsgQueue(hab, 0);

	/* import, quote by quote */
	pii = (IMPORTINFO *)pti->pData;
	if (AuthorFullSearchInit((AUTHORDB *)pii->pdb, &as)) {
		do {
			bOkay = !AuthorExists(&(pti->pps->adb), as.szLastCode);
			if (!bOkay) {
				if (pii->bAsk) {
					sprintf(szString, "%s exists. Replace it?", as.szLastCode);
					if (WinMessageBox(HWND_DESKTOP, pti->hwndCaller, szString, "Confirmation", 0, MB_QUERY | MB_YESNO) == MBID_YES)
						bOkay = TRUE;
				} else if (pii->bReplace)
						bOkay = TRUE;
			}
			if (bOkay) {
				AuthorGetAuthor((AUTHORDB *)pii->pdb, as.szLastCode, &pai, &pszDesc);
				AuthorAddAuthor(&(pti->pps->adb), as.szLastCode, pai, pszDesc);
				AuthorFreeAuthor(&pai, &pszDesc);
			}
		} while (AuthorFullSearchNext(&as));
	}

	/* tell the calling window that we're finished */
	WinSendMsg(pti->hwndCaller, QM_DONE, NULL, NULL);

	/* clean up */
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
	_endthread();
}
