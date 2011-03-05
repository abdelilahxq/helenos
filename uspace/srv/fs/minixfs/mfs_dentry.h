/*
 * Copyright (c) 2011 Maurizio Lombardi
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

/** @addtogroup fs
 * @{
 */ 

#ifndef _MFS_DENTRY_H_
#define _MFS_DENTRY_H_

#include "mfs_const.h"

#define mfs_v2_dentry	mfs_v1_dentry
#define mfs_v1l_dentry	mfs_v2l_dentry

/*MinixFS V1 directory entry on-disk structure*/
struct mfs_v1_dentry {
	uint16_t d_inum;
	char d_name[V1_MAX_NAME_LEN];
} __attribute__ ((packed));

/*MinixFS V2 with 30-char filenames (Linux variant)*/
struct mfs_v2l_dentry {
	uint16_t d_inum;
	char d_name[V2E_MAX_NAME_LEN];
} __attribute__ ((packed));

/*MinixFS V3 directory entry on-disk structure*/
struct mfs_v3_dentry {
	uint32_t d_inum;
	char d_name[V3_MAX_NAME_LEN];
} __attribute__ ((packed));

#endif

/**
 * @}
 */ 

