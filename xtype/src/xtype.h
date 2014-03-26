/*
 * xtype.h
 *
 * Header file for xtype library of sophisticated data types.
 *
 *      Created: 21st January 1998
 * Version 1.00: 13th December 1998
 *
 * (C) 1998 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#ifndef XTYPES_H
#define XTYPES_H

#include <stdio.h>


/*******************************************************************************
 *
 * XARRAY - dynamically extensible array.
 */

/* type definition for extensible arrays */
typedef struct _XARRAY {
/* private */
	int	iElemSize;	/* size of an array element */
	int	iCount;		/* current number of elements in the array */
	int	iSize;		/* current size of the array space */
	int	iBlock;		/* size to grow array by */
	void *	pData;		/* the array data */
} XARRAY;


/* macros to typecast to normal arrays of some standard data types */
#define xacast(x)	((x)->pData)
#define xachar(x)	((char *)((x)->pData))
#define xaint(x)	((int *)(((x)->pData))
#define xalong(x)	((long *)(((x)->pData))
#define xapchar(x)	((char **)((x)->pData))
#define xapint(x)	((int **)((x)->pData))
#define xaplong(x)	((long **)((x)->pData))
#define xaxarray(x)	((XARRAY **)((x)->pData))
#define xaxlist(x)	((XLIST **)((x)->pData))
#define xaxstr(x)	((XSTR **)((x)->pData))

/* macro to return the current element count */
#define xalen(x)	((x)->iCount)

/* xarray.c - extensible array management */
XARRAY *	xaappend(XARRAY *, void *);		/* add an element */
void *		xacvt(XARRAY *);			/* convert to a normal array */
void		xafree(XARRAY *);			/* de-allocate an array */
void *		xaget(XARRAY *, int, void *);		/* retrieve an element */
XARRAY *	xainsert(XARRAY *, int, void *);	/* insert an element */
XARRAY *	xanew(int, int, int);			/* create a new array */
XARRAY *	xaput(XARRAY *, int, void *);		/* replace an element */
XARRAY *	xashrink(XARRAY *, int, int);		/* shrink an array */


/*******************************************************************************
 *
 * XLIST - linked list.
 */

/* type definition for a node */
typedef struct _LNODE {
/* private */
	struct _LNODE *	pNext;		/* pointer to next node */
	struct _LNODE *	pPrev;		/* pointer to previous node */
	void *		pData;		/* node contents */
	int		iSize;		/* node data size */
} LNODE;


/* type definition for linked lists */
typedef struct _XLIST {
/* private */
	LNODE *		pHead;		/* head pointer */
	LNODE *		pTail;		/* tail pointer */
	LNODE *		pCursor;	/* cursor */
	int		iCount;		/* list length */
} XLIST;


/* macro to test for end of linked list */
#define xlend(x)		((x)->pCursor == NULL)

/* macro to return pointer to current node's data */
#define xldata(x)		(((x)->pCursor != NULL)? (x)->pCursor->pData : NULL)

/* macro to return the length of the list */
#define xllen(x)		((x)->iCount)

/* macro to query size of current node's data */
#define xlsize(x)		(((x)->pCursor != NULL)? (x)->pCursor->iSize : NULL)

/* macros to use XLISTs as stacks */
#define xlpop(x, p)		xlbehead((x), (p))
#define xlpush(x, p, s)		xlprepend((x), (p), (s))

/* macros to use XLISTs as queues */
#define xldequeue(x, p);	xlbehead((x), (p))
#define xlenqueue(x, p, s)	xlappend((x), (p), (s))


/* xlist.c - linked list management */
XLIST *	xlappend(XLIST *, void *, int);		/* append a node */
void *	xlbehead(XLIST *, void *);		/* retrieve head node */
void	xldelete(XLIST *);			/* delete a node */
void	xlfree(XLIST *);			/* de-allocate a list */
void *	xlget(XLIST *, void *);			/* retrieve node data */
XLIST *	xlnew(void);				/* create a new list */
void *	xlnext(XLIST *);			/* move cursor to next node */
XLIST *	xlprepend(XLIST *, void *, int);	/* prepend a node */
void *	xlprev(XLIST *);			/* move cursor to previous node */
void *	xlrewind(XLIST *);			/* return cursor to head node */


/*******************************************************************************
 *
 * XSTR - dynamically extensible string.
 */

/* type definition for extensible strings */
typedef struct _XSTR {
/* private */
	int	iSize;		/* the current size of the of the space */
	int	iBlock;		/* size to grow string by */
	char	*pszString;	/* the string data */
} XSTR;


/* macro to typecast to a normal null-terminated string */
#define xstrcast(x)	((x)->pszString)

/* macro to return the nth character in the string */
#define xstrch(x, n)	((x)->pszString[n])

/* macro to return the length of an extensible string */
#define xstrlen(x)	(strlen((x)->pszString))

/* xstr.c - extensible string management */
XSTR *	fgetxstr(XSTR *, FILE *);		/* extensible version of fgets() */
XSTR *	xstrcat(XSTR *, char *);		/* concatenate a string */
XSTR *	xstrcatc(XSTR *, char);			/* concatenate a character */
XSTR *	xstrcpy(XSTR *, char *);		/* copy a string */
char *	xstrcvt(XSTR *);			/* convert to normal string */
XSTR *	xstrdel(XSTR *, int, int);		/* delete characters from the string */
void	xstrfree(XSTR *);			/* de-allocate a string */
XSTR *	xstrncat(XSTR *, char *, int);		/* concatenate a string up to a length */
XSTR *	xstrncpy(XSTR *, char *, int);		/* copy a string up to a length */
XSTR *	xstrnew(int, int);			/* create a new string */
XSTR *	xstrtrunc(XSTR *, int);			/* truncate a string */

#endif /* XTYPES_H */
