#			D3X DOS-Extender examples makefile
#					*
#				  (c) GrayWolf


all: ex1.exe coff.exe ex2.exe le.exe

ex1.exe: ex1.asm
	nasm -fobj $<
	alink $(<:.asm=.obj)
	stubx -s $@
	del $(<:.asm=.obj)

coff.exe: coff.S
	gcc -o $@ -Wall -s -nostdlib -x assembler-with-cpp $<
	stubx -s $@

ex2.exe: ex2.asm
	tasm /m /ml /p /q /t /z $<
	tlink /3 /c /d /x $(<:.asm=.obj)
	stubx -s $@
	del $(<:.asm=.obj)

le.exe: le.asm
	wasm $<
	wlink f $(<:.asm=.obj) form os2 le
	stubx -s $@
	del $(<:.asm=.obj)
