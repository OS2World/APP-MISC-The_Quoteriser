/*
 * threads.h
 *
 * Header file for worker threads.
 *
 *      Created: 27th January, 1997
 * Version 1.00: 31st March, 1997
 * Version 2.00: 19th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_THREADS_H
#define _QUOTERISER_THREADS_H

#define INCL_WIN
#include <os2.h>
#include "types.h"

/* thread messages */
#define QM_DONE		WM_USER
#define QM_FOUND	(WM_USER + 1)
#define QM_PAINT	(WM_USER + 2)
#define QM_TYPESET	(WM_USER + 3)
#define QM_COPY		(WM_USER + 4)
#define QM_ERROR	(WM_USER + 5)
#define QM_START	(WM_USER + 6)

#define THREADSTACK	32768


/* window handles to threads */
extern HWND		hwndPaint;


/* type for thread procedures */
typedef void 		(*THREADPROC)(void *);


/* type to pass to thread procedures */
typedef struct _THREADINFO {
	HWND		hwndCaller;	/* handle of calling window */
	void		*pData;		/* pointer to data */
	PROGSTATE	*pps;		/* pointer to program state */
} THREADINFO;


/* type for ThreadImport...() threads */
typedef struct _IMPORTINFO {
	void		*pdb;		/* pointer to database to import from */
	BOOL		bAsk;		/* ask if duplicates should be replaced? */
	BOOL		bReplace;	/* replace duplicates? */
} IMPORTINFO;

/* threads in qthreads.c */
void ThreadFindAllQuotes(THREADINFO *);
void ThreadFindKeywords(THREADINFO *);
void ThreadFindQuoteText(THREADINFO *);
void ThreadFindAuthorsQuotes(THREADINFO *);
void ThreadFindRandomQuote(THREADINFO *);
void ThreadImportQuotes(THREADINFO *);

/* threads in athreads.c */
void ThreadFindAllAuthors(THREADINFO *);
void ThreadFindDescription(THREADINFO *);
void ThreadFindAuthorName(THREADINFO *);
void ThreadImportAuthors(THREADINFO *);

/* threads in paint.c */
void ThreadPaint(THREADINFO *);

#endif
