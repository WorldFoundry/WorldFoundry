;==============================================================================
; readme.txt: (this file) a description of each file type in this directory.
;==============================================================================

; assumtions:
;		the make system assumes you have 3d studio installed in \3ds4

; extensions:
;	.oas: source file used to create .oad file (there should be one per object)
		Object desription, creates a c struct for use in the game in <objectname.ht>
		and an .oad file for editing in 3D Studio (see example.oas for an
		example)

;==============================================================================
; files you can change:

objects.mac		; master list of all objects in velocity
				; there should be a .oas file for each object listed here

*.oas			; you must create a .oas file for each object you add to
				; objects.mac

; files you shouldn't change
cstruct.pl		; perl script to fix object names to be valid C names

oad.h			; header file describing the structures in the .oad file.  Used
				; to create the .oad file, and included by the 3D Studio PXP
				; so it can read the .oad file

types3ds.s		; generates *.oad, the arrays for 3dstudio dialog boxes (for each .oas file)

iff.s			; generates *.iff.txt & *.iff, the oad files for iff2lvl and the level editors

oadtypes.s		; generates *.ht, the c struct for each .oas file, which then may be
				; included in velocity to enable reading of the object data
				; from a level

objects.hs		; generates objects.h, a C header file containing a enumeration of all
				; object types

objects.s		; generates objects.c, a C switch statment constructing the correct object
				; based on the enumeration created by objects.hs

objects.mas		; generates objects.mak, a makefile for building all .oad & .ht files

objects.ins		; generates objects.inc, a file with include statments for including
				; all objects .hp files

objects.es		; generates objects.e, list of objectid's

objects.mis		; generates objects.mi, used by velocity make file

;==============================================================================
; files which are created by the make process:
objects.h		; created by objects.hs, enumerates all of the objects

objects.c		; created by objects.s, C switch statment construct the correct
				; object based on the enumeration created by objects.hs

*.oad:			; created from the .oas file, contains the .oad information for 3d studio and the level converter
*.ht:			; c header file containing struct for object *

objects.mak:	; makefile for building all .oad and .ht files

objects.inc:		; file with include statements for all objects .hp files

objects.e:		; a simple list of objectid's, used to include in a .c file to
				; create an enum matching the one on objects.h

objects.mi		; make include, list of dependancies for .obj files

;==============================================================================
