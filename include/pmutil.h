/*
 * pmutil.h
 *
 * Header for various PM utility functions.
 *
 *      Created: 4th December, 1997
 * Version 2.00: 4th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _NPS_PMUTIL_H
#define _NPS_PMUTIL_H

#define INCL_WIN
#define INCL_GPI
#include <os2.h>

/* functions in pmscroll.c */
int	ScrollMsgHandler(HWND, ULONG, MPARAM, MPARAM, RECTL *, int);
void	ScrollMetafile(HPS, HWND, HWND, HMF);
void	ScrollSetGeometry(HWND, HWND, HWND, RECTL *);

/* functions in pmspawn.c */
HAPP	SpawnPM(HWND, char *, char *);

#endif
