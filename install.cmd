/*
 * install.cmd
 *
 * Cheesy install utility to put a Quoteriser folder with some icons in it on
 * the desktop.
 *
 * (C) 1997 Nicholas Paul Sheppard
 *
 * This file is distributed under the GNU General Public License. See the
 * file copying.txt for details.
 */

/* load RexxUtil functions we need */
call RxFuncAdd 'SysCreateObject', 'RexxUtil', 'SysCreateObject'

if SysCreateObject("WPFolder", "The Quoteriser", "<WP_DESKTOP>", "OBJECTID=<QUOTERISER>;ICONFILE="||Directory()||"\ETC\QUOTER.ICO") then
	do
	common = "ICONFILE="||Directory()||"\ETC\QUOTER.ICO;EXENAME="||Directory()||"\BIN\"
	okay = SysCreateObject("WPProgram", "The Quoteriser", "<QUOTERISER>", common||"QUOTER.EXE")
	okay = OKAY & SysCreateObject("WPProgram", "Quote-of-the-Day", "<QUOTERISER>", common||"QOTD.EXE")
	if okay then
		do
		say 'The Quoteriser successfully installed.'
		ret = 0
		end
	else
		do
		say 'A Quoteriser program object could not be created.'
		ret = 1
		end
	end
else
	do
	say 'A Quoteriser folder could not be created.'
	ret = 1
	end

Exit ret
