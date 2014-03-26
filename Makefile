#
# Makefile for the OS/2 Quoteriser
#
#      Created: 18th January, 1997
# Version 1.00: 11th April, 1997
# Version 2.00: 21st December, 1997
# Version 2.01: 28th December, 1997
# Version 2.10: 1st December, 1998
#
# (C) 1997-1998 Nicholas Paul Sheppard
#
# This file is distributed under the GNU General Public License. See the
# file COPYING for details.
#


#
# Directories. Mine are for compiling files from sub-directories of the
# current directory into binaries in different sub-directories of the
# current directory.
#
# SRC   - directory for source files
# INCL  - directory for header files
# BIN   - directory for normal executables
# DEBUG - directory for debug-enabled executables
# DOC   - directory for documentation
# DIST  - directory into which to place distribution files
# ETC   - directory for misellaneous files (icons)
#

SRC   = src
INCL  = include
BIN   = bin
DEBUG = debug
DOC   = doc
DIST  = .
ETC   = etc

#
# Compiler and linker names, options, directories, etc., common to the normal and
# debug compilations.
#
# CC       - name of the compiler
# LINK     - name of the linker
# LIBS     - path to pre-compiled libraries
# RC       - resource compiler
# RFLAGS   - options to pass to the resource compiler
# IMPLIB   - name of the import library creator
# IFLAGS   - options to pass to the import library creator
#

CC       = gcc
LINK     = gcc
LIBS     = /emx/lib
RC       = rc
RFLAGS   = -x2 -I /emx/include -I $(INCL)
IMPLIB   = emximp
IFLAGS   =

#
# Other programs
#
# RM   - command to use for deletion of files
# ZIP  - command to use for archiving files
#
RM  = rm -f
ZIP = zip -9 -u

#
# Targets. We use a nasty recursive make here to compile the normal
# and debug versions. The options are:
#
# CFLAGS  - options to pass to the compiler
# LXFLAGS - options to pass to the linker for .EXEs
# LLFLAGS - options to pass to the linker for .DLLs
# LCFLAGS - options to pass to the linker for the DLL containing the C library functions
# CDEF    - definition file for the DLL containing the C library functions
# O       - extension for object files (including the .)
# L       - extension for library files (including the .)
# OUT     - the output directory
#
#
# Note that there is a 'package' target at the end of the makefile, as are
# the clean-* targets that actually do the work
#

.PHONY: \
bin debug exes clean spotless package \
clean-bin clean-debug clean-objs spotless-bin spotless-debug spotless-exes

all: bin debug

bin:
	$(MAKE) exes \
                "CFLAGS=-I $(INCL) -Zomf -Zmt -m486 -O2 -Wall" \
                "LXFLAGS=-Zomf -Zcrtdll=quoter1 -Zmt -s" \
                "LLFLAGS=-Zomf -Zcrtdll=quoter1 -Zdll -Zmt" \
                "LCFLAGS=-Zomf -Zsys -Zdll -Zmt" "CDEF=$(SRC)/quoter1.def" \
                "O=.obj" "L=.lib" "OUT=$(BIN)"

debug:
	$(MAKE) exes \
                "CFLAGS=-I $(INCL) -Zmt -g" \
                "LXFLAGS=-Zmt -g" \
                "LLFLAGS=-Zmt -Zdll -g" \
                "LCFLAGS=-Zmt -Zdll -g" "CDEF=$(SRC)/quoter1d.def" \
                "O=.o" "L=.a" "OUT=$(DEBUG)"

exes: \
$(OUT)/quoter.exe $(OUT)/qotd.exe $(OUT)/sig.exe $(OUT)/quoterc.exe \
$(OUT)/quoterdb.dll $(OUT)/quoterla.dll $(OUT)/quoter1.dll

clean: clean-bin clean-debug

clean-bin:
	$(MAKE) clean-objs "O=.obj" "L=.lib" "OUT=$(BIN)"

clean-debug:
	$(MAKE) clean-objs "O=.o" "L=.a" "OUT=$(DEBUG)"

spotless: spotless-bin spotless-debug

spotless-bin: clean-bin
	$(MAKE) spotless-exes "OUT=$(BIN)"

spotless-debug: clean-debug
	$(MAKE) spotless-exes "OUT=$(DEBUG)"

#
# Rules for linking.
#

$(OUT)/%.exe:
	$(LINK) $(LXFLAGS) -o $@ $^

$(OUT)/%.dll:
	$(LINK) $(LLFLAGS) -o $@ $^

#
# Rules for creating import libraries
#

$(OUT)/%$(L):
	$(IMPLIB) $(IFLAGS) -o $@ $^

#
# Rules for resource files
#

$(OUT)/%.res: $(SRC)/%.rc
	$(RC) $(RFLAGS) -r $< $@

#
# Rules for object files
#

$(OUT)/%$(O): $(SRC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

#
# quoter1.dll contains the C library functions, so it has a different rule from the
# normal .DLL rule.
#
$(OUT)/quoter1.dll: $(CDEF) \
$(OUT)/str$(O) $(OUT)/dir$(O) $(OUT)/pmscroll$(O) $(OUT)/pmspawn$(O)
	$(LINK) $(LCFLAGS) -o $@ $^ -lxtype

#
# Executable dependencies
#

$(OUT)/quoter.exe: \
$(OUT)/quoter$(O) $(OUT)/quoter_p$(O) $(OUT)/settings$(O) $(OUT)/adlg$(O) $(OUT)/qdlg$(O) \
$(OUT)/quoterdb$(L) $(OUT)/quoterla$(L) $(OUT)/quoter1$(L) \
$(OUT)/quoter.res $(SRC)/quoter.def

$(OUT)/qotd.exe: \
$(OUT)/qotd$(O) $(OUT)/qotd_p$(O) \
$(OUT)/quoterdb$(L) $(OUT)/quoterla$(L) $(OUT)/quoter1$(L) \
$(OUT)/qotd.res $(SRC)/qotd.def

$(OUT)/sig.exe: \
$(OUT)/sig$(O) \
$(OUT)/quoterdb$(L) $(OUT)/quoterla$(L) $(OUT)/quoter1$(L) $(SRC)/sig.def

$(OUT)/quoterc.exe: \
$(OUT)/quoterc$(O) \
$(OUT)/quoterdb$(L) $(OUT)/quoterla$(L) $(OUT)/quoter1$(L) $(SRC)/quoterc.def

$(OUT)/quoterdb.dll: \
$(OUT)/qdb$(O) $(OUT)/quotes$(O) $(OUT)/qsearch$(O) $(OUT)/qthreads$(O) \
$(OUT)/adb$(O) $(OUT)/authors$(O) $(OUT)/asearch$(O) $(OUT)/athreads$(O) \
$(SRC)/quoterdb.def $(OUT)/quoter1$(L) $(OUT)/quoterla$(L) $(LIBS)/gdbm$(L) \
$(LIBS)/rx$(L)

$(OUT)/quoterla.dll: \
$(OUT)/html$(O) $(OUT)/htmlattr$(O) $(OUT)/qccmd$(O) $(OUT)/qcdata$(O) \
$(OUT)/typeset$(O) \
$(SRC)/quoterla.def $(OUT)/quoter1$(L) $(OUT)/quoterdb$(L)


#
# Import library dependencies
#

$(OUT)/quoterdb$(L): $(SRC)/quoterdb.def
$(OUT)/quoterla$(L): $(SRC)/quoterla.def
$(OUT)/quoter1$(L): $(CDEF)

#
# Object file dependencies
#

$(OUT)/adb$(O): $(SRC)/adb.c $(INCL)/authors.h
$(OUT)/adlg$(O): $(SRC)/adlg.c $(INCL)/quoter.h $(INCL)/types.h $(INCL)/authors.h $(INCL)/threads.h $(INCL)/general.h $(INCL)/pmutil.h
$(OUT)/asearch$(O): $(SRC)/asearch.c $(INCL)/authors.h
$(OUT)/athreads$(O): $(SRC)/athreads.c $(INCL)/authors.h $(INCL)/threads.h $(INCL)/types.h
$(OUT)/authors$(O): $(SRC)/authors.c $(INCL)/authors.h
$(OUT)/dir$(O): $(SRC)/dir.c $(INCL)/general.h
$(OUT)/html$(O): $(SRC)/html.c $(INCL)/html.h $(INCL)/general.h
$(OUT)/htmlattr$(O): $(SRC)/htmlattr.c $(INCL)/html.h $(INCL)/general.h
$(OUT)/pmscroll$(O): $(SRC)/pmscroll.c $(INCL)/pmutil.h
$(OUT)/pmspawn$(O): $(SRC)/pmspawn.c $(INCL)/pmutil.h
$(OUT)/qccmd$(O): $(SRC)/qccmd.c $(INCL)/authors.h $(INCL)/general.h $(INCL)/quoterc.h $(INCL)/quotes.h
$(OUT)/qcdata$(O): $(SRC)/qcdata.c $(INCL)/authors.h $(INCL)/quoterc.h $(INCL)/quotes.h
$(OUT)/qdb$(O): $(SRC)/qdb.c $(INCL)/quotes.h
$(OUT)/qdlg$(O): $(SRC)/qdlg.c $(INCL)/quoter.h $(INCL)/types.h $(INCL)/quotes.h $(INCL)/threads.h $(INCL)/general.h $(INCL)/pmutil.h
$(OUT)/qotd$(O): $(SRC)/qotd.c $(INCL)/qotd.h $(INCL)/general.h $(INCL)/authors.h $(INCL)/quotes.h $(INCL)/types.h $(INCL)/threads.h $(INCL)/pmutil.h
$(OUT)/qotd.res: $(SRC)/qotd.rc $(INCL)/qotd.h $(ETC)/quoter.ico
$(OUT)/qotd_p$(O): $(SRC)/qotd_p.c $(INCL)/qotd.h $(INCL)/types.h $(INCL)/threads.h $(INCL)/typeset.h $(INCL)/pmutil.h
$(OUT)/qsearch$(O): $(SRC)/qsearch.c $(INCL)/quotes.h
$(OUT)/qthreads$(O): $(SRC)/qthreads.c $(INCL)/quotes.h $(INCL)/threads.h $(INCL)/types.h
$(OUT)/quotes$(O): $(SRC)/quotes.c $(INCL)/quotes.h
$(OUT)/quoter$(O): $(SRC)/quoter.c $(INCL)/quoter.h $(INCL)/general.h $(INCL)/authors.h $(INCL)/quotes.h $(INCL)/types.h $(INCL)/threads.h $(INCL)/pmutil.h
$(BIN)/quoterc$(O): $(SRC)/quoterc.c $(INCL)/quoterc.h
$(OUT)/quoter.res: $(SRC)/quoter.rc $(SRC)/settings.rc $(SRC)/adlg.rc $(SRC)/qdlg.rc $(INCL)/quoter.h $(ETC)/quoter.ico
$(OUT)/quoter_p$(O): $(SRC)/quoter_p.c $(INCL)/quoter.h $(INCL)/types.h $(INCL)/threads.h $(INCL)/typeset.h $(INCL)/pmutil.h
$(OUT)/settings$(O): $(SRC)/settings.c $(INCL)/quoter.h $(INCL)/threads.h $(INCL)/types.h
$(OUT)/sig$(O): $(SRC)/sig.c $(INCL)/quotes.h $(INCL)/authors.h $(INCL)/html.h $(INCL)/general.h $(INCL)/typeset.h
$(OUT)/str$(O): $(SRC)/str.c $(INCL)/general.h
$(OUT)/typeset$(O): $(SRC)/typeset.c $(INCL)/typeset.h $(INCL)/html.h

#
# Include file dependencies
#
#(INCL)/quoterc.h: $(INCL)/authors.h $(INCL)/quotes.h
$(INCL)/quotes.h: $(INCL)/authors.h
$(INCL)/threads.h: $(INCL)/types.h
$(INCL)/types.h: $(INCL)/quotes.h $(INCL)/authors.h

#
# Rules for cleaning up the mess we've made
#

clean-objs:
	if exist $(OUT)/*$(O) $(RM) $(OUT)/*$(O)
	if exist $(OUT)/*$(L) $(RM) $(OUT)/*$(L)
	if exist $(OUT)/*.res $(RM) $(OUT)/*.res

spotless-exes:
	if exist $(OUT)/*.exe $(RM) $(OUT)/*.exe
	if exist $(OUT)/*.dll $(RM) $(OUT)/*.dll

#
# Rules for producing the distribution packages.
#
package: $(DIST)/quot210x.zip $(DIST)/quot210s.zip

$(DIST)/%.zip:
	$(ZIP) $@ $^

#
# This is the list of files that go into the quot210x.zip (binary) distribution
# file.
#
$(DIST)/quot210x.zip: \
install.cmd readme.txt $(DOC)/copying.txt $(ETC)/quoter.ico \
$(BIN)/qotd.exe $(BIN)/quoter.exe $(BIN)/sig.exe $(BIN)/quoterc.exe \
$(BIN)/quoterdb.dll $(BIN)/quoterla.dll $(BIN)/quoter1.dll \
$(DOC)/history.txt $(DOC)/intro.htm $(DOC)/quoter.htm $(DOC)/quoterc.htm \
$(DOC)/qotd.htm $(DOC)/sig.htm

#
# This is the list of files that go into the quot210s.zip (source) distribution
# file.
#
$(DIST)/quot210s.zip: \
bin debug build.cmd install.cmd readme.txt $(DOC)/copying.txt $(ETC)/quoter.ico \
Makefile Makefile.unx $(ETC)/gdbm.pat $(ETC)/gnurx.pat \
$(INCL)/authors.h $(INCL)/general.h $(INCL)/html.h $(INCL)/pmutil.h \
$(INCL)/qotd.h $(INCL)/quoter.h $(INCL)/quoterc.h $(INCL)/quotes.h \
$(INCL)/threads.h $(INCL)/types.h $(INCL)/typeset.h \
$(SRC)/adb.c $(SRC)/adlg.c $(SRC)/asearch.c $(SRC)/athreads.c $(SRC)/authors.c \
$(SRC)/dir.c $(SRC)/html.c $(SRC)/htmlattr.c $(SRC)/pmscroll.c $(SRC)/pmspawn.c \
$(SRC)/qccmd.c $(SRC)/qcdata.c $(SRC)/qdb.c $(SRC)/qdlg.c $(SRC)/qotd.c \
$(SRC)/qotd_p.c $(SRC)/qsearch.c $(SRC)/qthreads.c $(SRC)/quoter.c $(SRC)/quoterc.c \
$(SRC)/quoter_p.c $(SRC)/quotes.c $(SRC)/settings.c $(SRC)/sig.c $(SRC)/str.c \
$(SRC)/typeset.c $(SRC)/adlg.rc $(SRC)/qdlg.rc $(SRC)/qotd.rc $(SRC)/quoter.rc \
$(SRC)/settings.rc $(SRC)/qotd.def $(SRC)/quoter.def $(SRC)/sig.def \
$(SRC)/quoter.def $(SRC)/quoter1.def $(SRC)/quoterc.def $(SRC)/quoterdb.def \
$(SRC)/quoter1d.def $(SRC)/quoterla.def \
$(DOC)/history.txt $(DOC)/intro.htm $(DOC)/quoter.htm $(DOC)/quoterc.htm \
$(DOC)/qotd.htm $(DOC)/sig.htm $(DOC)/source.htm
