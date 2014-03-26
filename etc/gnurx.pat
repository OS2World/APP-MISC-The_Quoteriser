4c4,5
< # This file is part of GNU Rx
---
> # This file has been modified from the GNU Rx distribution by Nicholas
> # Paul Sheppard.
20,25c21,28
< release:
< 	$(MAKE) -f Makefile.os2 gnurx.dll rx.lib \
< 	CC="gcc -Zomf -O" O=".obj" DFLAGS="-Zcrtdll -Zdll"
< debug:
< 	$(MAKE) -f Makefile.os2 rx.a \
< 	CC="gcc -g" O=".o" DFLAGS="" AR="ar" RANLIB="ar s"
---
> all: omf a.out
> 
> omf:
> 	$(MAKE) -f Makefile.rx rx.lib \
> 	CC="gcc -Zomf -O" O=".obj" DFLAGS="" AR="emxomfar" RANLIB="emxomfar s"
> a.out:
> 	$(MAKE) -f Makefile.rx rx.a \
> 	CC="gcc -O" O=".o" DFLAGS="" AR="ar" RANLIB="ar s"
41,42c44,47
< rx.lib: gnurx.def
< 	emximp -o $@ gnurx.def
---
> rx.lib: $(OBJS)
> 	rm -f $@
> 	$(AR) cr $@ $(OBJS)
> 	$(RANLIB) $@
