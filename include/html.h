/*
 * html.h
 *
 * Header file for HTML parsing functions.
 *
 *      Created: 4th March, 1997
 * Version 1.00: 6th March, 1997
 * Version 2.00: 9th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#ifndef _QUOTERISER_HTML_H
#define _QUOTERISER_HTML_H


/* types of chunks we can return */
#define HTML_WORD_END		0
#define HTML_WORD_MID		1
#define HTML_TAG_END		2
#define HTML_TAG_MID   		3
#define HTML_MACRO_END		4
#define HTML_MACRO_MID		5
#define HTML_END		6


/* tag translations */
#define HTML_INVALID		-1
#define HTML_UNKNOWN		0
#define HTML_PARAGRAPH		1
#define HTML_ITALICS_START	2
#define HTML_ITALICS_END	3
#define HTML_BOLD_START		4
#define HTML_BOLD_END		5
#define HTML_LINEBREAK 		6
#define HTML_TABLE_ROW		7
#define HTML_TABLE_DATA		8

/* limits */
#define HTML_MAX_TAG		100	/* maximum length of a tag */
#define HTML_MAX_ELEM		10	/* maximum length of a tag element */
#define HTML_MAX_MACRO		10	/* maximum length of a macro */

/* alignments */
#define HTML_ALIGN_INVALID	0
#define HTML_ALIGN_LEFT		1
#define HTML_ALIGN_RIGHT	2
#define HTML_ALIGN_TOP		3
#define HTML_ALIGN_BOTTOM	4
#define HTML_ALIGN_MIDDLE	5

/* type for holding attributes of a TD tag */
typedef struct _HTML_TDATTR {
	int bNoWrap;
	int iRowSpan;
	int iColSpan;
	int iAlign;
	int iVAlign;
} HTML_TDATTR;


/* functions in html.c */
int	HTMLGetNextChunk(char *, char *, char **);
int	HTMLParseTag(char *);
int	HTMLParseMacro(char *);
char	*HTMLMakePlain(char *);

/* functions in htmlattr.c */
void	HTMLAttrTD(char *, HTML_TDATTR *);
char	*HTMLAttrFirst(char *, char *, int, char *, int);
char	*HTMLAttrNext(char *, char *, int, char *, int);

#endif
