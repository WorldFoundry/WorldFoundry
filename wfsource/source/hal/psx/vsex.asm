;
;	Exception Handler. (Assembler Support.)
;
;
; Ver		Date		Author			Desc
;----------------------------------------------------------------------------
; 0.1		09/08/95	Brian Marshall	Initial Version.

;	Register Save Structure used by exception handler...

rsreset
rzero		rw	1
rat			rw	1
rv0			rw	1
rv1			rw	1
ra0			rw	1
ra1			rw	1
ra2			rw	1
ra3			rw	1
rt0			rw	1
rt1			rw	1
rt2			rw	1
rt3			rw	1
rt4			rw	1
rt5			rw	1
rt6			rw	1
rt7			rw	1
rs0			rw	1
rs1			rw	1
rs2			rw	1
rs3			rw	1
rs4			rw	1
rs5			rw	1
rs6			rw	1
rs7			rw	1
rt8			rw	1
rt9			rw	1
rk0			rw	1
rk1			rw	1
rgp			rw	1
rsp			rw	1
rfp			rw	1
rra			rw	1
rhi			rw	1
rlo			rw	1
rsr			rw	1
rca			rw	1
repc		rw	1
badvaddr	rw	1
oldvector1	rw	1
oldvector2	rw	1
oldvector3	rw	1
oldvector4	rw	1
regsize		rb	0

		opt	at-
		opt	m+

		SECTION .text

		xdef	InstallExceptionHandler
		xdef	Registers
		xref	CExceptionHandler

;
;	Function to Hook the Exception vector, if the SN Debug stub hasn't
;
;	void InstallExceptionHandler(void);

InstallExceptionHandler
;		mfc0	v0,SR				;                   Get                   Status                    Register
		dw		$40026000
		andi	at,zero,$fffc		;                                                                               Mask
		and		at,at,v0			;                          Interrupts                            Off
;		mtc0	at,SR
		dw		$40816000
		nop
		nop							;    Load/Store    are    undefined    around    mtc0     so
									;    leave    an     extra    nop     juts    in     case...
		la		k0, Registers

		addiu	t1, zero, $80		; Address of Interrupt Vector
		nop
		lw		t2, (t1)			; Get Old Vector
		lui		t3, $3c1a			; Code that SN uses...
		addiu	t3, t3, $1fa0
		nop
		subu	t3, t3, t2
;		beqz	t3, @SNDebugIsInstalled
									; Decomment this line to have the exception
									; handler NOT hook the intterrupt if on a dev kit
									; so you can debug. It still inits okay on a debug
									; station.

		nop
		lw		t0, 4(t1)
		lw		t3, 8(t1)
		lw		t4, 12(t1)
		sw		t2, oldvector1(k0)
		sw		t0, oldvector2(k0)
		sw		t3, oldvector3(k0)
		sw		t4, oldvector4(k0)
		la		k0, ExceptionJump
		lw		t0, (k0)
		lw		t2, 4(k0)
		lw		t3, 8(k0)
		lw		t4, 12(k0)
		sw		t0, (t1)
		sw		t2, 4(t1)
		sw		t3, 8(t1)
		sw		t4, 12(t1)
		nop

@SNDebugIsInstalled
;		mtc0	v0,SR				; Restore Interrupt State
		dw		$40826000
		nop
		nop
		jr		ra					; And Return
		nop

ExceptionJump
		la		k0, ExceptionHandler
		jr		k0
		nop

;
;	The Actual Handler.
;

ExceptionHandler
		la		k0, Registers		; Get Save Area
		nop
		sw		at, rat(k0)			; Save at... etc
		sw		v0, rv0(k0)
;		mfc0	at, Cause
		dw		$40016800

		ori		v0, zero, %1110011111110
									; Pass External Interrupt and Syscall to OS
									; Trap the Rest...
		srl		at, at, 2
		andi	at, at, $1f			; Mask off bits...
		srlv	v0, v0, at
		andi	v0, v0, 1
		beqz 	v0, @CallOldHandler
		nop

;		Its a Genuine Exception.
;		Store the State and call the 'C' Handler...

@GenuineException
		sw		zero, rzero(k0)
		sw		v1, rv1(k0)
		sw		a0, ra0(k0)
		sw		a1, ra1(k0)
		sw		a2, ra2(k0)
		sw		a3, ra3(k0)
		sw		t0, rt0(k0)
		sw		t1, rt1(k0)
		sw		t2, rt2(k0)
		sw		t3, rt3(k0)
		sw		t4, rt4(k0)
		sw		t5, rt5(k0)
		sw		t6, rt6(k0)
		sw		t7, rt7(k0)
		sw		s0, rs0(k0)
		sw		s1, rs1(k0)
		sw		s2, rs2(k0)
		sw		s3, rs3(k0)
		sw		s4, rs4(k0)
		sw		s5, rs5(k0)
		sw		s6, rs6(k0)
		sw		s7, rs7(k0)
		sw		t8, rt8(k0)
		sw		t9, rt9(k0)
		sw		k0, rk0(k0)		; The only register not saved...
		sw		k1, rk1(k0)
		sw		gp, rgp(k0)
		sw		sp, rsp(k0)
		sw		fp, rfp(k0)
		sw		ra, rra(k0)
		mfhi	v0
		sw		v0, rhi(k0)
		mflo	v0
		sw		v0, rlo(k0)
;		mfc0	v0,SR				; Get Status Register
		dw		$40026000
		nop
		nop
		sw		v0, rsr(k0)
		sw		at, rca(k0)
;		mfc0	v0,EPC
		dw		$40027000
		nop
		nop
		sw		v0, repc(k0)

;		mfc0	v0,BADVADDR
		dw		$40024000
		nop
		nop
		sw		v0, badvaddr(k0)

;		Now call the 'C' Handler

		la		k0, CExceptionHandler
		jr		k0
		nop


@CallOldHandler
		lw		at, rat(k0)			; Restore Regs...
		lw		v0, rv0(k0)
									; Call Old Handler...
		addiu	k0, k0, oldvector1
		jr		k0
		nop

		cnop	0,4

Registers	dsb	regsize				; The Register Save Area...

