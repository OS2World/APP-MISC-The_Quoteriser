/*
 * typeset.h
 *
 * Header file for typesetting functions for HTML strings.
 *
 *      Created: 26th November, 1997
 * Version 2.00: 18th December, 1997
 * Version 2.10: 7th September, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_TYPESET_H
#define _QUOTERISER_TYPESET_H

#ifndef GENERIC
# include <os2.h>
#endif

/* font identifiers */
#ifndef GENERIC
# define IDF_NORMAL		50
# define IDF_ITALIC		51
# define IDF_BOLD		52
# define IDF_BOLDITALIC		53
#endif

#ifndef GENERIC
HMF	TypesetMetafile(HAB, HWND, char **, RECTL *, FATTRS *);
#endif

char	*TypesetASCII(char **, int);

#endif
