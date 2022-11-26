;께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께
;		   D3X(tm) DOS-Extender v0.90 unleash `h' alpha
;			    386SWAT debugger interface
;					
;			Copyright (c) 1998-2004 Daniel Borca
;					
;				 dborca@yahoo.com
;			 http://www.geocities.com/dborca
;께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께께


%define		SWAT	1


;같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
; This goes just after enter_pm label (before the call to main_switch)
;같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
%macro		SWAT_INI 0
		push	ds
		xor	eax,eax
		mov	es,ax
		cmp	dword [es:67h*4],eax
		pop	es
		je	.noswat
		mov	ax,0def0h
		int	67h
		or	ah,ah
		jnz	.noswat
		mov	ax,0def4h
		mov	ebx,dword [tss2+tssCr3]
		or	edx,byte -1
%ifdef	PAGED
		cmp	byte [platform],2
		je	.swat1
		mov	edx,100000h
	.swat1:
%endif
		int	67h
		mov	ax,0def2h
		mov	bx,swat_desc - GDT
		mov	di,swat_desc
		int	67h
		mov	ax,0def3h
		mov	bx,1
		mov	di,IDT+1*gate_size
		int	67h
		mov	ax,0def3h
		mov	bx,3
		mov	di,IDT+3*gate_size
		int	67h
%ifdef	PAGED
		cmp	byte [platform],2
		je	.swat2
		mov	ax,0def9h
		xor	ecx,ecx
		mov	es,cx
		mov	ch,4
		mov	edi,ecx
		add	edi,dword [pt_base]
		dec	ch
		int	67h
	.swat2:
%endif
	.noswat:
%endmacro


;같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
; This goes just before GDT_Length (after the last descriptor)
;같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
%macro		SWAT_GDT 0
swat_desc:	times (30 * descr_size) db 0
%endmacro
