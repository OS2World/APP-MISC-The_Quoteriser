/*
 * pmspawn.c
 *
 * Routines for spawning programs.
 *
 *      Created: 4th December, 1997
 * Version 2.00: 4th December, 1997
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

#define INCL_WIN
#include <os2.h>
#include <string.h>
#include "pmutil.h"


HAPP SpawnPM(HWND hwndNotify, char *pszApplication, char *pszCmdLine)
/*
 * Spawn a PM application.
 *
 * HWND hwndNotify	- handle of the window to notify when the application terminates
 * char *pszApplication	- the name of the application to spawn
 * char *pszCmdLine	- the command line to pass to the application
 *
 * Returns: 		- handle to the editor application on success
 *			  NULLHANDLE on failure
 */
{
	PROGDETAILS	pd;

	/* set up application details */
	memset(&pd, 0, sizeof(PROGDETAILS));
	pd.Length = sizeof(PROGDETAILS);
	pd.progt.progc = PROG_PM;
	pd.progt.fbVisible = SHE_VISIBLE;
	pd.pszExecutable = pszApplication;
	pd.pszParameters = pszCmdLine;
	pd.pszStartupDir = ".";

	/* go */
	return (WinStartApp(hwndNotify, &pd, pszCmdLine, NULL, 0));
}
