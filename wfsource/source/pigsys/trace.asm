.386p
                NAME    TRACE
                EXTRN   addToCallList :BYTE
                EXTRN   removeFromCallList :BYTE

DGROUP          GROUP   CONST,CONST2,_DATA,_BSS

_TEXT           SEGMENT PARA PUBLIC USE32 'CODE'
                ASSUME  CS:_TEXT ,DS:DGROUP,SS:DGROUP

                PUBLIC  __PRO
__PRO:
		push	eax
		push	edx

                mov     edx,dword ptr -4[ebp]
		sub	edx,9
		push	edx
                call    near ptr addToCallList
                add     esp,00000004H

		pop	edx
		pop	eax

                ret


                PUBLIC  __EPI
__EPI:
		call	near ptr removeFromCallList
		ret


_TEXT           ENDS

CONST           SEGMENT DWORD PUBLIC USE32 'DATA'
L1              DB      5fH,5fH,50H,52H,4fH,00H
CONST           ENDS

CONST2          SEGMENT DWORD PUBLIC USE32 'DATA'
CONST2          ENDS

_DATA           SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA           ENDS

_BSS            SEGMENT DWORD PUBLIC USE32 'BSS'
_BSS            ENDS

                END
