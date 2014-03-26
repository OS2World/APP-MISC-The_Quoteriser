/*
 * dir.c
 *
 * Miscellaneous path-name-handling functions.
 *
 *      Created: 25th November, 1997 (was part of general.c)
 * Verison 2.00: 15th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <string.h>
#include "general.h"

char *dircat(char *pszPath1, const char *pszPath2)
/*
 * Concatenate directory names.
 *
 * char *pszPath1	- first path in the concatenation
 * char *pszPath2	- second path in the concatenation
 *
 * Returns		- pszPath1
 */
{
	if ((pszPath1[0] != '\0') && (pszPath2[0] != '\0'))
		strcat(pszPath1, "\\");
	strcat(pszPath1, pszPath2);

	return (pszPath1);
}


char *dirfname(char *pszFileName)
/*
 * Exract the file name only from a path name.
 *
 * char *pszFileName	- the path name, to be overwritten with file name
 *
 * Returns		- pszFileName
 */
{
 	char *pch1, *pch2;

	/* look for last backslash */
	if ((pch2 = strrchr(pszFileName, '\\')) == NULL)
		/* no backslashes; look for colon (after drive letter) */
		if ((pch2 = strchr(pszFileName, ':')) == NULL)
			/* nothing to do */
				return (pszFileName);

	pch2++;
	pch1 = pszFileName;
	while ((*pch2) != '\0') {
		(*pch1) = (*pch2);
		pch1++;
		pch2++;
	}
	(*pch1) = '\0';

	return (pszFileName);
}


char *dirnoext(char *pszFileName)
/*
 * Strip the extension (if one exists) from a file name.
 *
 * char *pszFileName	- the file name to be stripped of its extension
 *
 * Returns:		- pszFileName
 */
{
	char *pchLastFS, *pchLastBS;

	pchLastFS = strrchr(pszFileName, '.');
	pchLastBS = strrchr(pszFileName, '\\');
	if (pchLastFS > pchLastBS)
		(*pchLastFS) = '\0';

	return (pszFileName);
}


char *dirsimp(char *pszPathName)
/*
 * Rationalise a path name i.e. remove . entries, move all .. entries
 * to the front if they cannot be removed and remove any repeated
 * directory separators.
 *
 * char *pszPathName	- the path name to be rationalised.
 *
 * Returns:		- pszPathName
 */
{
	char *pch1, *pch2, *pch3, *pchStart;
	char szPortion[256], szPathName[256];

	if (pszPathName[0] == '\0')
		/* trivial case */
		return (pszPathName);

	/* skip drive letter, if any */
	if (pszPathName[1] == ':')
		pch1 = pszPathName + 2;
	else
		pch1 = pszPathName;
	pchStart = pch1;

	/* skip leading backslash, if any */
	if ((*pch1) == '\\')
		pch1++;

	/* skip repeated initial directory separators */
	pch2 = strcpy(szPathName, pszPathName) + (pchStart - pszPathName);
	while ((*pch2) == '\\')
		pch2++;

	/* rationalise the rest of the path name */
	while ((*pch2) != '\0') {
		/* get next portion of path name */
		if ((pch3 = strchr(pch2, '\\')) == NULL)
			pch3 = strchr(pch2, '\0');
		strncpy(szPortion, pch2, pch3 - pch2);
		szPortion[pch3 - pch2] = '\0';
		pch2 = pch3;

		if (strcmp(szPortion, "..") == 0) {
			/* .. entry; move pch1 back a step */
			(*pch1) = '\0';
			if (pch1 > (pchStart + 1))
				pch1 -= 2;
			else if ((*pchStart) != '\\') {
				/* bring the .. entry back to the start */
				strpre(pch1, "..\\");
				pch1 = strchr(pch1, '\0');
			}
			while ((pch1 > pchStart) && ((*pch1) != '\\') && ((*pch1) != '.'))
				pch1--;
			if ((*pch1) == '\\') {
				pch1++;
			}
			if ((*pch1) == '.') {
				/* there are .. entries at the start */
				strpre(pch1 + 1, "\\..");
				pch1 = strchr(pch1, '\0');
			}
		} else if (strcmp(szPortion, ".") != 0) {
			/* normal entry; append */
			for (pch3 = szPortion; (*pch3) != '\0'; pch3++, pch1++)
				(*pch1) = (*pch3);
			(*pch1) = '\\';
			pch1++;
		}

		/* skip repeated directory separators */
		while ((*pch2) == '\\')
			pch2++;
	}

	/* strip trailing backslash */
	if (pch1 > pchStart) {
		if ((pch1 != (pchStart + 1)) || ((*pchStart) != '\\'))
			pch1--;
		(*pch1) = '\0';
	} else if ((*pch1) == '\\') {
		pch1++;
		(*pch1) = '\0';
	} else {
		(*pch1) = '\0';
	}

	return (pszPathName);
}
