
ALL : play .SYMBOLIC

play : play.exe .SYMBOLIC

play.exe : play.cpe
	cpe2x /CA play.cpe

play.cpe : play.obj
	psylink @psx.ln,play.cpe

play.obj : play.c
	set C_INCLUDE_PATH=$(%PSX_DIR)\include
	ccpsx -O3 -c -I f:\psxtools\psx\include play.c

################################################################################
#
#tuto1:
#	ccpsx -O3 -Xo$80040000 tuto1.c -otuto1.cpe,tuto1.sym
#
#tuto2:
#	ccpsx -O3 -Xo$80040000 tuto2.c -otuto2.cpe,tuto2.sym
#
#tuto3:
#	ccpsx -O3 -Xo$80040000 tuto3.c -otuto3.cpe,tuto3.sym
#
#for_menu:
#	ccpsx -O3 -Xo$80040000 play.c \psx\lib\none2.obj -o play.cpe,play.sym
#	cpe2x play.cpe
#	del play.cpe
#
#	ccpsx -O3 -Xo$80040000 tuto1.c \psx\lib\none2.obj -o tuto1.cpe,tuto1.sym
#	cpe2x tuto1.cpe
#	del tuto1.cpe
#
#	ccpsx -O3 -Xo$80040000 tuto2.c \psx\lib\none2.obj -o tuto2.cpe,tuto2.sym
#	cpe2x tuto2.cpe
#	del tuto2.cpe
#
#	ccpsx -O3 -Xo$80040000 tuto3.c \psx\lib\none2.obj -o tuto3.cpe,tuto3.sym
#	cpe2x tuto3.cpe
#	del tuto3.cpe
################################################################################

clean : .SYMBOLIC
	del *.obj *.cpe *.sym *.map
