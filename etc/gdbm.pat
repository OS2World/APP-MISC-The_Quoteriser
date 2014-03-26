13c13
< 	gdbmreorg.c gdbmseq.c gdbmsync.c gdbmsetopt.c gdbmerrno.c\
---
> 	gdbmreorg.c gdbmseq.c gdbmsync.c gdbmsetopt.c gdbmerrno.c gdbmexists.o\
24c24
< 	gdbmreorg.o gdbmseq.o gdbmsync.o gdbmsetopt.o gdbmerrno.o\
---
> 	gdbmreorg.o gdbmseq.o gdbmsync.o gdbmsetopt.o gdbmerrno.o gdbmexists.o\
39c39
< gdbm.a: $(DBM_OF) $(NDBM_OF) $(GDBM_OF) gdbm.h
---
> gdbm.a: autoconf.h $(DBM_OF) $(NDBM_OF) $(GDBM_OF) gdbm.h
89a90
> gdbmexists.o:	gdbmdefs.h gdbmerrno.h
