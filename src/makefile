#		   D3X(tm) DOS-Extender v0.90 unleash `h' alpha
#				  core makefile
#					*
#			Copyright (c) 1998-2004 Daniel Borca
#					*
#				 dborca@yahoo.com
#			 http://www.geocities.com/dborca


.PHONY: clean realclean
.SUFFIXES: .exe .obj

AS = nasm
ASFLAGS = -O6 -fobj -Ikernel/
LD = alink
LDFLAGS =
CC = gcc
CFLAGS = -Wall -W -pedantic -ansi -O2

RM = del

.obj.exe:
	$(LD) $(LDFLAGS) $<

stubx.exe: stubx.c d3x1.h d3xd.h d3xw.h
	$(CC) -o $@ $(CFLAGS) -s $<
	-upx --best $@
	$@ -s $@

d3x1.h: fix.exe d3x1.exe
	$^ -e > $@

d3xd.h: fix.exe d3xd.exe
	$^ -e -p > $@

d3xw.h: fix.exe d3xw.exe
	$^ -e -n > $@

fix.exe: fix.c
	$(CC) -o $@ $(CFLAGS) fix.c

d3x1.obj: kernel/d3x.asm
	$(AS) -o $@ $(ASFLAGS) -DTARGET=0 $<

d3xd.obj: kernel/d3x.asm
	$(AS) -o $@ $(ASFLAGS) -DTARGET=1 $<

d3xw.obj: kernel/d3x.asm
	$(AS) -o $@ $(ASFLAGS) -DTARGET=2 -DI21API -DI33API $<

clean:
	-$(RM) d3x*.*
	-$(RM) fix.exe

realclean: clean
	-$(RM) stubx.exe
