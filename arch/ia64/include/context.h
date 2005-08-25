/*
 * Copyright (C) 2005 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ia64_CONTEXT_H__
#define __ia64_CONTEXT_H__

#include <arch/types.h>

/*
 * context_save() and context_restore() are both leaf procedures.
 * No need to allocate scratch area.
 */
#define SP_DELTA	0

#ifdef context_set
#undef context_set
#endif

#define context_set(c, _pc, stack, size) 			\
	(c)->pc = (__address) _pc;				\
	(c)->bsp = ((__address) stack) + (sizeof(the_t));	\
	(c)->sp = ((__address) stack) + (size) - SP_DELTA;

struct context {

	/*
	 * Application registers
	 */
	__u64 ar_pfs;
	__u64 ar_unat_caller;
	__u64 ar_unat_callee;
	__u64 ar_rsc;
	__u64 bsp;	/* ar_bsp */
	__u64 ar_rnat;
	__u64 ar_lc;
	__u64 ar_ec;
	__u64 ar_ccv;
	__u64 ar_csd;
	__u64 ar_ssd;

	/*
	 * General registers
	 */
	__u64 r1;
	__u64 r2;
	__u64 r3;
	__u64 r4;
	__u64 r5;
	__u64 r6;
	__u64 r7;
	__u64 r8;
	__u64 r9;
	__u64 r10;
	__u64 r11;
	__u64 sp;		/* r12 */
	__u64 r13;
	__u64 r14;
	__u64 r15;
	__u64 r16;
	__u64 r17;
	__u64 r18;
	__u64 r19;
	__u64 r20;
	__u64 r21;
	__u64 r22;
	__u64 r23;
	__u64 r24;
	__u64 r25;
	__u64 r26;
	__u64 r27;
	__u64 r28;
	__u64 r29;
	__u64 r30;
	__u64 r31;
	
	/*
	 * Branch registers
	 */
	__u64 pc;		/* b0 */
	__u64 b1;
	__u64 b2;
	__u64 b3;
	__u64 b4;
	__u64 b5;
	__u64 b6;
	__u64 b7;

	/*
	 * Predicate registers
	 */
	__u64 pr;
	
	pri_t pri;
} __attribute__ ((packed));

#endif
