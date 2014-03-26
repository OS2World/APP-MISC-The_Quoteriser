/*
 * xarray.c
 *
 * Routines for extensible arrays. No bounds-checking is performed by any
 * of these functions.
 *
 *      Created: 25th January 1998
 * Version 1.00: 26th January 1998
 *
 * (C) 1998 Nicholas Paul Sheppard. See the file licence.txt for details.
 */

#include <memory.h>
#include <stdlib.h>
#include "xtype.h"


XARRAY *xaappend(XARRAY *xa, void *pElem)
/*
 * Append an element to the end of an extensible array.
 *
 * XARRAY *xa	- the array to be added to
 * void *pElem	- pointer to the element to be added
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xa, otherwise
 */
{
	void *	p;	/* temporary pointer */

	/* check that we have sufficient space */
	p = NULL;
	if (xa->iSize < (xalen(xa) + 1)) {
		if ((p = realloc(xa->pData, (xa->iSize + xa->iBlock) * xa->iElemSize)) == NULL) {
			return (NULL);
		} else {
			xa->pData = p;
			xa->iSize += xa->iBlock;
		}
	}

	/* add the element */
	xaput(xa, xa->iCount++, pElem);

	return (xa);
}


void *xacvt(XARRAY *xa)
/*
 * Convert an extensible array into a normal array (i.e. get rid of all the
 * extraneous structure that the extensible array uses). Do not use xa after
 * calling this function. Unless the array is of voids (?), the output from
 * this function should also be typecast appropriately.
 *
 * XARRAY *xa	- the extensible array to be converted
 *
 * Returns	- (a pointer to) the array block represented by xa
 */
{
	void *	p;	/* return value */

	if (xa != NULL) {
		p = xa->pData;
		free(xa);
	} else {
		p = NULL;
	}

	return (p);
}


void xafree(XARRAY *xa)
/*
 * De-allocate the memory used by an extensible array.
 *
 * XARRAY *xa	- the extensible array to be de-allocated
 */
{
	if (xa != NULL) {
		free(xa->pData);
		free(xa);
	}
}


void *xaget(XARRAY *xa, int iPos, void *pElem)
/*
 * Get the value of an extensible array element.
 *
 * XARRAY *xa	- the extensible array to have its element changed
 * int iPos	- the position of the element to set
 * void *pElem	- a pointer to receive the element
 *
 * Returns	- pElem
 */
{
	char *	p;	/* temporary pointer */

	/* make p point to the appropriate spot in the array */
	p = (char *)xa->pData;
	p += iPos * xa->iElemSize;

	/* copy the data into the output space */
	memcpy(pElem, p, xa->iElemSize);

	return (pElem);
}


XARRAY *xainsert(XARRAY *xa, int iPos, void *pElem)
/*
 * Insert an element into an extensible array.
 *
 * XARRAY *xa	- the extensible array to insert an element into
 * int iPos	- the position to insert the new element into
 * void *pElem	- pointer to the element to be added
 *
 * Returns	- NULL if there is a memory allocation failure
 *		  xa, otherwise
 */
{
	char *	p1;	/* temporary pointer */
	char *	p2;	/* temporary pointer */
	int	i;	/* counter */

	/* check that we have sufficient space */
	p1 = NULL;
	if (xa->iSize < (xalen(xa) + 1)) {
		if ((p1 = realloc(xa->pData, (xa->iSize + xa->iBlock) * xa->iElemSize)) == NULL) {
			return (NULL);
		} else {
			xa->pData = p1;
			xa->iSize += xa->iBlock;
		}
	}

	/* move elements after the insertion point */
	p1 = (char *)xa->pData;
	p1 += (xa->iCount - 1) * xa->iElemSize;
	p2 = p1 + xa->iElemSize;
	for (i = xa->iCount; i > iPos; i--) {
		memcpy(p2, p1, xa->iElemSize);
		p1 -= xa->iElemSize;
		p2 -= xa->iElemSize;
	}

	/* copy the new element in */
	memcpy(p1, pElem, xa->iElemSize);
	xa->iCount++;

	return (xa);
}


XARRAY *xanew(int iSize, int iBlock, int iElemSize)
/*
 * Create a new extensible array.
 *
 * int iSize		- the initial number of elements in the array
 * int iBlock		- the number of elements to grow the array by
 * int iElemSize	- the size of the array elements
 *
 * Returns		- NULL if there is a memory allocation failure
 *			  a pointer to he new extensible array, otherwise
 */
{
	XARRAY *	xa;	/* return value */

	if ((xa = (XARRAY *)malloc(sizeof(XARRAY))) != NULL) {
		xa->iSize = iSize;
		xa->iBlock = iBlock;
		xa->iElemSize = iElemSize;
		xa->iCount = 0;
		if ((xa->pData = calloc(iSize, iElemSize)) == NULL) {
			free(xa);
			xa = NULL;
		}
	}

	return (xa);
}


XARRAY *xaput(XARRAY *xa, int iPos, void *pElem)
/*
 * Set the value of an extensible array element. No bounds-checking is
 * performed.
 *
 * XARRAY *xa	- the extensible array to have its element changed
 * int iPos	- the position of the element to set
 * void *pElem	- a pointer to the value of the element
 *
 * Returns	- xa
 */
{
	char *	p;	/* temporary pointer */

	/* make p point to the appropriate spot in the array */
	p = (char *)xa->pData;
	p += iPos * xa->iElemSize;

	/* copy the new data into the array */
	memcpy(p, pElem, xa->iElemSize);

	return (xa);
}


XARRAY *xashrink(XARRAY *xa, int iBottom, int iTop)
/*
 * Shrink an array by removing a given range (inclusive).
 *
 * XARRAY *xa	- the extensible array to be shrunk
 * int iBottom	- lower bound of range to eliminate
 * int iTop	- upper bound of range to eliminate
 *
 * Returns	- xa
 */
{
	char *	p1;	/* temporary pointer */
	char *	p2;	/* temporary pointer */
	int	i;	/* counter */

	/* move elements above the range down */
	p1 = p2 = (char *)xa->pData;
	p1 += iBottom * xa->iElemSize;
	p2 += iTop * xa->iElemSize;
	for (i = iBottom; i <= iTop; i++) {
		memcpy(p1, p2, xa->iElemSize);
		p1 += xa->iElemSize;
		p2 += xa->iElemSize;
	}
	xa->iCount -= iTop - iBottom + 1;

	/* reduce the memory allocation, if we can */
	i = xa->iSize;
	while ((i - xa->iBlock) > xa->iCount)
		i -= xa->iBlock;
	if (i < xa->iSize) {
		xa->pData = realloc(xa->pData, i * xa->iElemSize);
		xa->iSize = i;
	}

	return (xa);
}
