
;=============================================================================
; tasker.asm: assembly language routines for the PIGS multi-tasker
;=============================================================================
; this assembly file must first be pre-processed with a c pre-processor
; so that we can change the naming conventions on the fly

#ifdef __SW_3S
#define	CURRENTTASK currentTask
#define	SYSTEMSTATE systemState
#define _TASKERSAVESYSREGSANDDISPATCHCURRENTTASK _TaskerSaveSysRegsAndDispatchCurrentTask
#define _TASKERRESTORESYSREGS _TaskerRestoreSysRegs
#define _TASKERDISPATCHCURRENTTASK _TaskerDispatchCurrentTask
#define  TASKSWITCH TaskSwitch
#define	_RESCHEDULE _Reschedule
#define	_HALSETCLOCK _HALSetClock
#define _HALSTEALINT8 _HALStealInt8
#define _HALRESTOREINT8 _HALRestoreInt8
#define _HALINT8HANDLER _HALInt8Handler
#else
#define	CURRENTTASK _currentTask
#define	SYSTEMSTATE _systemState
#define _TASKERSAVESYSREGSANDDISPATCHCURRENTTASK _TaskerSaveSysRegsAndDispatchCurrentTask_
#define _TASKERRESTORESYSREGS _TaskerRestoreSysRegs_
#define _TASKERDISPATCHCURRENTTASK _TaskerDispatchCurrentTask_
#define  TASKSWITCH TaskSwitch_
#define	_RESCHEDULE _Reschedule_
#define	_HALSETCLOCK _HALSetClock_
#define _HALSTEALINT8 _HALStealInt8_
#define _HALRESTOREINT8 _HALRestoreInt8_
#define _HALINT8HANDLER _HALInt8Handler_
#endif

;=============================================================================

.386
.model	flat

_DATA	SEGMENT DWORD PUBLIC 'DATA'
	EXTRN	CURRENTTASK:DWORD
	EXTRN	SYSTEMSTATE:DWORD
oldInt8Seg	dw	0
oldInt8Off	dw	0
_DATA	ENDS

_TEXT	SEGMENT DWORD PUBLIC 'CODE'
	ASSUME	cs:_TEXT, ds:_DATA
	EXTRN	_RESCHEDULE:NEAR

;=============================================================================

procState struc					; note: this needs to be kept
						; in sync with the C version
procState_stackBase	dd	?
procState_stackSize	dd	?
procState_eip	dd	?
procState_esp	dd	?
procState_efl	dd	?
procState_eax	dd	?
procState_ebx	dd	?
procState_ecx	dd	?
procState_edx	dd	?
procState_esi	dd	?
procState_edi	dd	?
procState_ebp	dd	?
;procState_cs    dd      ?     see comments in _PROCSTA.H
;procState_ds    dd      ?     structure _ProcState
;procState_es    dd      ?
;procState_ss    dd      ?
procState_fs    dd      ?
procState_gs    dd      ?
;procState_cr0	dd	?
;procState_cr2	dd	?
;procState_cr3	dd	?
;procState_dr0	dd	?
;procState_dr1	dd	?
;procState_dr2	dd	?
;procState_dr3	dd	?
;procState_dr6	dd	?
;procState_dr7 	dd	?
;procState_tr3	dd	?
;procState_tr4	dd	?
;procState_tr5	dd	?
procState_SizeOf	LABEL
procState ends

;-----------------------------------------------------------------------------

node struc					; note: this needs to be kept
						; in sync with the C version
node_next	dd	1
node_prev	dd	1
node_pri	dd	1
node_SizeOf	LABEL
node ends

;-----------------------------------------------------------------------------

task	struc
task_link	db	node_SizeOf DUP (?)
task_taskState	db procState_SizeOf DUP (?)
; more stuff here, but I don't need it yet
;	char _name[TASKNAMELEN];
;	int8 _flags;
;
;	int32 _sigWait;
;	int32 _sigAlloc;
;	int32 _sigRecieved;
;
;	short _forbidCount;			// if not zero, task switching forbidden
;	short _disableCount;			// if not zero, Interruptstask switching forbidden
task ENDS

;=============================================================================
; save current register state into system register set, and jump to DispatchCURRENTTASK
; this is only called by _TaskerStart

	PUBLIC	_TASKERSAVESYSREGSANDDISPATCHCURRENTTASK

_TASKERSAVESYSREGSANDDISPATCHCURRENTTASK:
	; first, save current task state
	push	edi
	mov	edi,offset SYSTEMSTATE		; get system task structure structure

	mov	[edi+task_taskState+procState_eax],eax
	mov	[edi+task_taskState+procState_ebx],ebx
	mov	[edi+task_taskState+procState_ecx],ecx
	mov	[edi+task_taskState+procState_edx],edx
	mov	[edi+task_taskState+procState_esi],esi
	mov	[edi+task_taskState+procState_ebp],ebp

        mov     ax,fs           ; FS is a 16 bit register
        mov     [edi+task_taskState+procState_fs],eax
        mov     ax,gs
        mov     [edi+task_taskState+procState_gs],eax

        pushfd
	pop	eax
	mov	[edi+task_taskState+procState_efl],eax

	pop	eax
        mov     [edi+task_taskState+procState_edi],eax

	pop	eax				; get return address
	push	eax				; put it back
	mov	[edi+task_taskState+procState_eip],eax		; save pc

	mov	eax,esp
	mov	[edi+task_taskState+procState_esp],eax	; save stack ptr

	jmp	_TASKERDISPATCHCURRENTTASK

;=============================================================================
; restore system register set, this does not return to caller
; this is only called by _TaskerStop

	PUBLIC	_TASKERRESTORESYSREGS

_TASKERRESTORESYSREGS:
	pop	eax				; remove return address

	mov	edi,offset SYSTEMSTATE		; get CURRENTTASK structure

	mov	eax,[edi+task_taskState+procState_esp]	; set stack ptr
	mov	esp,eax

	mov	eax,[edi+task_taskState+procState_eip]		; set pc
	push	eax

	mov	ebx,[edi+task_taskState+procState_ebx]
	mov	ecx,[edi+task_taskState+procState_ecx]
	mov	edx,[edi+task_taskState+procState_edx]
	mov	esi,[edi+task_taskState+procState_esi]
	mov	ebp,[edi+task_taskState+procState_ebp]

        mov     eax,[edi+task_taskState+procState_fs]
        mov     fs,ax
        mov     eax,[edi+task_taskState+procState_gs]
        mov     gs,ax

        mov     eax,[edi+task_taskState+procState_efl]
	push	eax

	mov	eax,[edi+task_taskState+procState_edi]
	push	eax
	mov	eax,[edi+task_taskState+procState_eax]
	pop	edi
	popfd
	ret							; transfer execution

;=============================================================================
; this is only called from _RESCHEDULE, it restores the current tasks registers
; and passes control to it

	PUBLIC	_TASKERDISPATCHCURRENTTASK

_TASKERDISPATCHCURRENTTASK:
	pop	eax				; remove return address

	mov	edi,CURRENTTASK		; get CURRENTTASK structure

	mov	eax,[edi+task_taskState+procState_esp]	; set stack ptr
	mov	esp,eax

	mov	eax,[edi+task_taskState+procState_eip]		; set pc
	push	eax

	mov	ebx,[edi+task_taskState+procState_ebx]
	mov	ecx,[edi+task_taskState+procState_ecx]
	mov	edx,[edi+task_taskState+procState_edx]
	mov	esi,[edi+task_taskState+procState_esi]
	mov	ebp,[edi+task_taskState+procState_ebp]

        mov     eax,[edi+task_taskState+procState_fs]
        mov     fs,ax
        mov     eax,[edi+task_taskState+procState_gs]
        mov     gs,ax

	mov	eax,[edi+task_taskState+procState_efl]
	push	eax

	mov	eax,[edi+task_taskState+procState_edi]
	push	eax
	mov	eax,[edi+task_taskState+procState_eax]
	pop	edi
	popfd
	ret							; transfer execution

;=============================================================================
; this may be called from any task, it saves the current task state and
; passes control to _RESCHEDULE

	PUBLIC	TASKSWITCH

TASKSWITCH:
	; first, save current task state
	push	edi
	mov	edi,CURRENTTASK		; get CURRENTTASK structure

	mov	[edi+task_taskState+procState_eax],eax
	mov	[edi+task_taskState+procState_ebx],ebx
	mov	[edi+task_taskState+procState_ecx],ecx
	mov	[edi+task_taskState+procState_edx],edx
	mov	[edi+task_taskState+procState_esi],esi
	mov	[edi+task_taskState+procState_ebp],ebp

        mov     ax,fs
        mov     [edi+task_taskState+procState_fs],eax
        mov     ax,gs
        mov     [edi+task_taskState+procState_gs],eax

	pushfd
	pop	eax
	mov	[edi+task_taskState+procState_efl],eax

	pop	eax
	mov	[edi+task_taskState+procState_edi],eax

	pop	eax				; remove return address
	mov	[edi+task_taskState+procState_eip],eax		; save pc

	mov	eax,esp
	mov	[edi+task_taskState+procState_esp],eax	; save stack ptr

	; now, setup environment for task switcher code
	mov	eax,offset _RESCHEDULE
	push	eax
	ret

;=============================================================================
; hardware support calls

#if HAL_INTERRUPTS==1

; void _HALSetClock(int16 rate)
; where rate is 1192755.2 / desired rate(in hz)

	PUBLIC	_HALSETCLOCK
_HALSETCLOCK:
	push	ebp
	mov	ebp,esp
#ifdef __SW_3S
	mov	bx,[ebp+8]		; get input parameter off stack
#else
	mov	bx,ax			; get input parameter from ax
#endif
	mov	al,36H
	out     43H,al
	mov     al,bl
	out     40H,al
	mov     al,bh
	out     40H,al

	pop	 ebp
	ret

;===============================================================================
;   StealInt8: Point INT8 to our code				                         *)
;===============================================================================

	PUBLIC _HALSTEALINT8
_HALSTEALINT8:
;	push	es
;	mov     ax,03508H
;	int	21h
;	mov     cs:[oldInt8Off],bx
;	mov     cs:[oldInt8Seg],es
;	pop     es
;	push	ds
;	mov     dx,cs
;	mov     ds,dx
;	mov     dx,_INT8HANDLER
;	mov     ax,02508H
;	int     021H
;	pop     ds
	ret

;===============================================================================

	PUBLIC _HALRESTOREINT8
_HALRESTOREINT8:
;	push	ds
;	mov     dx,cs:[oldInt8Off]
;	mov     ax,cs:[oldInt8Seg]
;	mov     ds,ax
;	mov     ax,02508H
;	int     021H
;	pop	ds
	ret

;=============================================================================

	PUBLIC _INT8HANDLER
_INT8HANDLER:
						; starting housekeeping
	push	ax
	push	ds
;	mov		ds,cs:[intDs]

						; our custom int code
;	mov		al,[_soundOn]
;	or		al,al
;	jz		NoSound
;NoSound:
;	inc		word [_intCounter]
;						; ending housekeeping
;	dec		word [count18]
;	jz		ChainThrough			; if every 4th time, do other int
;	mov		al,20H
;	out		20H,al					; clear interrupt bit
;	pop		ds
;	pop		ax
	iret

ChainThrough:
;	mov		ax,[intChainCount]
;	mov		[count18],ax
;
;	pop		ds
;	pop		ax
;	jmp		dword cs:[oldInt8]		; chain through to old int 8
;=============================================================================
#endif
;=============================================================================
_TEXT	ENDS
;=============================================================================
END
;=============================================================================
