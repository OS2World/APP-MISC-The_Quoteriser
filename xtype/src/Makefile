################################################################################
#
# Makefile for xtype library.
#
#      Created: 10th September 1998
# Version 1.00: 10th September 1998
#
# (C) 1998 Nicholas Paul Sheppard. See the file licence.txt for details
#


################################################################################
#
# Program names and options. Change appropriately.
#
# CC      - compiler name
# CFLAGS  - flags to pass to the compiler
# AR      - name of librarian
# ARFLAGS - flags to pass to the librarian
# INSTALL - file installation utility
# ZIP     - name of archiver
# ZFLAGS  - flags to pass to the archiver
# RM      - file deletion utility
#
CC      = gcc
CFLAGS  = -Wall -O2
AR      = ar
ARFLAGS = -r
INSTALL = copy
RM      = del


################################################################################
#
# File and directory names. Change appropriately.
#
# O - extension for object files
# L - extension for libraries
# V - version number
# Z - extension for archives
#
# HEADER  - path and file name for installed header file
# LIBRARY - path and file name for installed library file
#
O = .o
L = .a

HEADER = \emx\include\xtype.h
LIBRARY = \emx\lib\xtype$(L)


################################################################################
#
# Targets. Do not change.
#

all: xtype$(L)

install: $(HEADER) $(LIBRARY)

clean: clean-objs

spotless: clean spotless-libs


################################################################################
#
# Pattern rules. Change appropriately (these are for GNU Make and GCC).
#

%$(L):
	$(AR) $(ARFLAGS) $@ $^

%$(O): %.c
	$(CC) $(CFLAGS) -o $@ -c $*.c


################################################################################
#
# Dependencies. Do not change.
#

OBJS = xarray$(O) xlist$(O) xstr$(O)

xtype$(L): $(OBJS)

xarray$(O): xarray.c xtype.h
xlist$(O): xlist.c xtype.h
xstr$(O): xstr.c xtype.h

$(HEADER): xtype.h
	$(INSTALL) xtype.h $(HEADER)

$(LIBRARY): xtype$(L)
	$(INSTALL) xtype$(L) $(LIBRARY)


################################################################################
#
# Additional cleaning targets. Do not change.
#

clean-objs:
	$(RM) $(OBJS)

spotless-libs:
	$(RM) xtype$(L) $(HEADER) $(LIBRARY)
