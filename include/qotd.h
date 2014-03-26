/*
 * qotd.h
 *
 * Header file containing PM definitions for the Quoteriser's Quote-of-the-day
 * program.
 *
 *      Created: 31st March, 1997
 * Version 1.00: 9th April, 1997
 * Version 2.00: 21st December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_QOTD_H
#define _QUOTERISER_QOTD_H

#include <os2.h>
#include "types.h"

/* icon */
#define IDI_ICON		1

/* menu */
#define IDM_MAIN		2
#define IDM_EXIT		20
#define IDM_EDIT		21
#define IDM_COPY_TEXT		210
#define IDM_COPY_METAFILE	211
#define IDM_AGAIN		22

/* strings */
#define IDS_APPNAME		3000
#define IDS_OPENFAILED		3001
#define IDS_THREADFAILED	3002
#define IDS_INIFAILED		3003
#define IDS_EMPTYDB		3004
#define IDS_NODB		3005
#define IDS_SEARCHING		3006
#define IDS_ERROR		3007
#define IDS_UNSET		3008

/* variables in qotd.c */
extern HWND	hwndVertScroll;
extern PROFILE	prf;

#endif
