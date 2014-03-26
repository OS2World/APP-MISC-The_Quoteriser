/*
 * quoterc.c
 *
 * Main program for the Quoteriser database compiler.
 *
 *      Created: 13th September, 1998
 * Version 1.00: 3rd November, 1998
 *
 * (C) 1998 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quoterc.h"


int main(int argc, char **argv)
{
	char *	pszCmdFile;	/* file name of command file */
	FILE *	fCmd;		/* stream handle of command file */
	QCERROR	qe;		/* errors from QCProcessCommands() */
	int	bAbort;		/* set to 1 to abort process */
	int	i;		/* counter */

	/* initialise variables */
	pszCmdFile = NULL;
	fCmd = NULL;
	bAbort = 0;

	if (argc < 2) {
		/* print help screen */
		printf("The Quoteriser: Database Compiler 1.00\n");
		printf("(C) 1998 Nicholas Paul Sheppard. This program is distributed\n");
		printf("under the GNU General Public License and comes with NO WARRANTY\n");
		printf("WHATSOEVER. See the file copying.txt for details.\n\n");
		printf("Usage: quoterc <file1> <file2> ...\n\n");
		printf("  where: <fileN>  is the name of a command (.qc) file\n\n");
		return (0);
	}

	for (i = 1; !bAbort && (i < argc); i++) {
		/* open command file */
		if ((pszCmdFile = (char *)malloc(strlen(argv[i]) + 4)) == NULL) {
			fprintf(stderr, "quoterc: memory allocation failure.\n");
			bAbort = 1;
		} else {
			strcpy(pszCmdFile, argv[i]);
			if ((fCmd = fopen(pszCmdFile, "r")) == NULL) {
				strcat(pszCmdFile, ".qc");
				if ((fCmd = fopen(pszCmdFile, "r")) == NULL) {
					fprintf(stderr, "quoterc: could not open %s or %s.\n", argv[i], pszCmdFile);
					bAbort = 1;
				}
			}
		}

		/* process command file */
		if (!bAbort) {
			if (QCProcessCommands(fCmd, &qe) != QC_ENONE) {
				fprintf(stderr, "%s: line %d: %s\n", pszCmdFile, qe.iLine, QCErrorString(qe.iError));
				bAbort = 1;
			}
		}

		/* clean up */
		free(pszCmdFile);
		if (fCmd != NULL)
			fclose(fCmd);
	}

	return (bAbort);
}
