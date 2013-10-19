;
; jsimdcpu.asm - SIMD instruction support check
;
; x86 SIMD extension for IJG JPEG library
; Copyright (C) 1999-2006, MIYASAKA Masaru.
; For conditions of distribution and use, see copyright notice in jsimdext.inc
;
; This file should be assembled with NASM (Netwide Assembler),
; can *not* be assembled with Microsoft's MASM or any compatible
; assembler (including Borland's Turbo Assembler).
; NASM is available from http://nasm.sourceforge.net/ or
; http://sourceforge.net/project/showfiles.php?group_id=6208
;
; Last Modified : August 23, 2005
;
; [TAB8]

%include "jsimdext.inc"

; --------------------------------------------------------------------------
	SECTION	SEG_CONST

	alignz	16

copyright:
	db	" x86 SIMD ext for IJG lib V", JPEG_SIMDEXT_VER_STR
	db	" Copyright 2006, MIYASAKA Masaru "

	alignz	16

; --------------------------------------------------------------------------
	SECTION	SEG_TEXT
	BITS	32
;
; Check if the CPU supports SIMD instructions
;
; GLOBAL(unsigned int)
; jpeg_simd_cpu_support (void)
;

	align	16
	global	EXTN(jpeg_simd_cpu_support)

EXTN(jpeg_simd_cpu_support):
	push	ebx
;	push	ecx		; need not be preserved
;	push	edx		; need not be preserved
;	push	esi		; unused
	push	edi

	xor	edi,edi			; simd support flag

	pushfd
	pop	eax
	mov	edx,eax
	xor	eax, 1<<21		; flip ID bit in EFLAGS
	push	eax
	popfd
	pushfd
	pop	eax
	xor	eax,edx
	jz	short .return		; CPUID is not supported

	; Check for MMX, SSE and SSE2 instruction support
	xor	eax,eax
	cpuid
	test	eax,eax
	jz	short .return

	xor	eax,eax
	inc	eax
	cpuid
	mov	eax,edx			; eax = Standard feature flags

	test	eax, 1<<23		; bit23:MMX
	jz	short .no_mmx
	or	edi, byte JSIMD_MMX
.no_mmx:
	test	eax, 1<<25		; bit25:SSE
	jz	short .no_sse
	or	edi, byte JSIMD_SSE
.no_sse:
	test	eax, 1<<26		; bit26:SSE2
	jz	short .no_sse2
	or	edi, byte JSIMD_SSE2
.no_sse2:

	; Check for 3DNow! instruction support
	mov	eax, 0x80000000
	cpuid
	cmp	eax, 0x80000000
	jbe	short .return

	mov	eax, 0x80000001
	cpuid
	mov	eax,edx			; eax = Extended feature flags

	test	eax, 1<<31		; bit31:3DNow!(vendor independent)
	jz	short .no_3dnow
	or	edi, byte JSIMD_3DNOW
.no_3dnow:

.return:
	mov	eax,edi

	pop	edi
;	pop	esi		; unused
;	pop	edx		; need not be preserved
;	pop	ecx		; need not be preserved
	pop	ebx
	ret
