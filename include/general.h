/*
 * general.h
 *
 * Header file for miscellaneous functions.
 *
 *      Created: 18th January, 1997
 * Version 1.00: 18th March, 1997
 * Version 2.00: 15th December, 1997
 * Version 2.10: 10th December, 1998
 *
 * (C) 1997-1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_GENERAL_H
#define _QUOTERISER_GENERAL_H

#include <stdio.h>


/* functions in dir.c */
char	*dircat(char *, const char *);
char	*dirfname(char *);
char	*dirnoext(char *);
char	*dirsimp(char *);


/* functions in str.c */
int	cgetc(FILE *);
char	*strabbrev(char *, char *, int);
int	strboxf(FILE *f, char **, int *, int);
int	strcmpci(const char *, const char *);
char	*strencl(const char *, char *, char, char);
char	*stresctok(char *, const char *, char);
char	*strfchr(char *, char *, char);
char	*strfromf(FILE *f);
char	*strtncpy(char *, char *, int);
char	*strpre(char *, char *);
char	*strreplace(char *, const char *, const char *, char *);
char	*strrmchr(char *, char);

#endif
