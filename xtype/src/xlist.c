/*
 * xlist.c
 *
 * Linked list routines. Our linked lists are doubly-linked, with a head
 * and tail pointer, and a cursor.
 *
 *      Created: 2nd February 1998
 * Version 1.00: 1st November 1998
 *
 * (C) 1998 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#include <stdlib.h>
#include <string.h>
#include "xtype.h"


XLIST *xlappend(XLIST *xl, void *pData, int iSize)
/*
 * Append a node to the end of a linked list containing the given data.
 *
 * XLIST *xl	- the linked list to be appended to
 * void *pData	- a pointer to the data to put in the node
 * int iSize	- the size of the data in bytes
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xl, otherwise
 */
{
	LNODE *	pNew;	/* the new node */
	int	bEmpty;	/* is the linked list empty? */

	/* allocate the new node's structure */
	if (xl->pTail == NULL) {
		/* the list is empty */
		pNew = xl->pCursor = xl->pHead = (LNODE *)malloc(sizeof(LNODE));
		bEmpty = 1;
	} else { 
		/* the list is not empty */
		bEmpty = 0;
		pNew = xl->pTail->pNext = (LNODE *)malloc(sizeof(LNODE));
	}

	/* put the data in */
	if (pNew != NULL) {
		if ((pNew->pData = malloc(iSize)) == NULL) {
			/* back out of previous memory allocations */
			free(pNew);
			pNew = NULL;
			if (bEmpty)
				xl->pCursor = xl->pHead = NULL;
		} else {
			memcpy(pNew->pData, pData, iSize);
			pNew->iSize = iSize;
			pNew->pPrev = xl->pTail;
			pNew->pNext = NULL;
			xl->pTail = pNew;
			xl->iCount++;
		}
	}

	return ((pNew)? xl : NULL);
}


void *xlbehead(XLIST *xl, void *pData)
/*
 * Retrieve the data in the head of the list, then delete the head of
 * the list (for using the list as a queue or stack). The list cursor
 * will be set to the new head.
 *
 * XLIST *xl	- the list to be beheaded
 * void *pData	- a pointer to a buffer to hold the retrieved data (may be NULL)
 *
 * Returns	- NULL, if the list is empty
 *		  pData, otherwise
 */
{
	if (xl->pHead == NULL)
		return (NULL);

	xl->pCursor = xl->pHead;
	if (pData != NULL)
		xlget(xl, pData);
	xldelete(xl);

	return (pData);
}


void xldelete(XLIST *xl)
/*
 * Delete the current node from a linked list. The cursor will then
 * point to the next element in the list (if there is one).
 *
 * XLIST *xl	- the linked list to delete the node from
 */
{
	LNODE *	pNext;	/* the node after the deleted one */
	LNODE *	pPrev;	/* the node previous to the deleted one */

	if (xl->pCursor != NULL) {
		/* save the data we need later */
		pNext = xl->pCursor->pNext;
		pPrev = xl->pCursor->pPrev;

		/* de-allocate the data and the node */
		free(xl->pCursor->pData);
		free(xl->pCursor);

		/* point the cursor to the next node */
		if ((xl->pCursor = pNext) == NULL)
			/* the deleted node was the tail, so fix tail pointer */
			xl->pTail = pPrev;

		/* re-link the list */
		if (pPrev == NULL)
			/* the deleted node was the head, so fix head pointer */
			xl->pHead = pNext;
		else
			pPrev->pNext = pNext;

		xl->iCount--;
	}
}


void xlfree(XLIST *xl)
/*
 * De-allocate the memory used by a linked list.
 *
 * XLIST *xl	- the linked list to be de-allocated.
 */
{
	LNODE *	p;	/* temporary pointer */

	/* de-allocate all the nodes */
	xlrewind(xl);
	while (!xlend(xl)) {
		p = xl->pCursor;
		free(p->pData);
		xlnext(xl);
		free(p);
	}

	/* de-allocate the control structure */
	free(xl);
}


void *xlget(XLIST *xl, void *pData)
/*
 * Retrieve data from a linked list, at the cursor.
 *
 * XLIST *xl	- the linked list to retrive from.
 * void *pData	- a pointer to a buffer to receive the retrieved data
 *
 * Returns	- NULL if we are past the end of the linked list
 *		  pData, otherwise
 */
{
	/* test for end of list */
	if (xlend(xl)) {
		return (NULL);
	} else {
		memcpy(pData, xl->pCursor->pData, xl->pCursor->iSize);
		return (pData);
	}
}


XLIST *xlnew(void)
/*
 * Create a new linked list.
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  (a pointer to) the new linked list, otherwise
 */
{
	XLIST *	xl;	/* return value */

	if ((xl = (XLIST *)malloc(sizeof(XLIST))) != NULL) {
		xl->pHead = xl->pTail = xl->pCursor = NULL;
		xl->iCount = 0;
	}

	return (xl);
}


void *xlnext(XLIST *xl)
/*
 * Move the cursor of a linked list to the next item in the list.
 *
 * XLIST *xl	- the linked list to follow
 *
 * Returns	- NULL, if there are no more nodes
 *                a pointer to the next node's data, otherwise
 */
{
	if ((xl->pCursor = xl->pCursor->pNext) != NULL)
		return (xl->pCursor->pData);
	else
		return (NULL);
}


XLIST *xlprepend(XLIST *xl, void *pData, int iSize)
/*
 * Prepend a node to the head of a linked list containing the given data.
 *
 * XLIST *xl	- the linked list to be prepended to
 * void *pData	- a pointer to the data to put in the node
 * int iSize	- the size of the data in bytes
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xl, otherwise
 */
{
	LNODE *	pNew;	/* the new node */
	int	bEmpty;	/* is the linked list empty? */

	/* allocate the new node's structure */
	if (xl->pHead == NULL) {
		/* the list is empty */
		pNew = xl->pCursor = xl->pTail = (LNODE *)malloc(sizeof(LNODE));
		bEmpty = 1;
	} else { 
		/* the list is not empty */
		bEmpty = 0;
		pNew = (LNODE *)malloc(sizeof(LNODE));
	}

	/* put the data in */
	if (pNew != NULL) {
		if ((pNew->pData = malloc(iSize)) == NULL) {
			/* back out of previous memory allocations */
			free(pNew);
			pNew = NULL;
			if (bEmpty)
				xl->pCursor = xl->pTail = NULL;
		} else {
			memcpy(pNew->pData, pData, iSize);
			pNew->iSize = iSize;
			pNew->pPrev = NULL;
			pNew->pNext = xl->pHead;
			xl->pHead = pNew;
			xl->iCount++;
		}
	}

	return ((pNew)? xl : NULL);
}

void *xlprev(XLIST *xl)
/*
 * Move the cursor of a linked list to the previous item in the list.
 *
 * XLIST *xl	- the linked list to follow
 *
 * Returns	- NULL, if there are no prior nodes
 *		  a pointer to the previous node's data, otherwise
 */
{
	if ((xl->pCursor = xl->pCursor->pPrev) != NULL)
		return (xl->pCursor->pData);
	else
		return (NULL);
}


void *xlrewind(XLIST *xl)
/*
 * Move the cursor of a linked list to the start.
 *
 * XLIST *xl	- the linked list to be rewound
 *
 * Returns	- a pointer to the first node's data (NULL if the list is empty)
 */
{
	if ((xl->pCursor = xl->pHead) != NULL)
		return (xl->pCursor->pData);
	else
		return (NULL);
}
