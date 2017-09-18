/*
 * Copyright (c) 2017 Jakub Jermar
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

/** @addtogroup generic
 * @{
 */
/** @file
 */

#ifndef KERN_CAP_H_
#define KERN_CAP_H_

#include <typedefs.h>
#include <ipc/ipc.h>
#include <adt/list.h>
#include <synch/mutex.h>

#define MAX_CAPS  64

typedef enum {
	CAP_TYPE_INVALID,
	CAP_TYPE_ALLOCATED,
	CAP_TYPE_PHONE,
	CAP_TYPE_IRQ,
	CAP_TYPE_MAX
} cap_type_t;

typedef struct cap {
	cap_type_t type;
	int handle;

	bool (* can_reclaim)(struct cap *);

	/* Link to the task's capabilities of the same type. */
	link_t link;

	/* The underlying kernel object. */
	void *kobject;
} cap_t;

typedef struct cap_info {
	mutex_t lock;

	list_t type_list[CAP_TYPE_MAX];

	cap_t *caps;
} cap_info_t;

struct task;

extern void caps_task_alloc(struct task *);
extern void caps_task_free(struct task *);
extern void caps_task_init(struct task *);
extern bool caps_apply_to_all(struct task *, cap_type_t,
    bool (*)(cap_t *, void *), void *);
extern void caps_lock(struct task *);
extern void caps_unlock(struct task *);

extern void cap_initialize(cap_t *, int);
extern cap_t *cap_get(struct task *, int, cap_type_t);
extern int cap_alloc(struct task *);
extern void cap_publish(struct task *, int, cap_type_t, void *);
extern cap_t *cap_unpublish(struct task *, int, cap_type_t);
extern void cap_free(struct task *, int);

#endif

/** @}
 */
