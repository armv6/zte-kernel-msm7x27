/* Copyright (c) 2002,2007-2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __ADRENO_RINGBUFFER_H
#define __ADRENO_RINGBUFFER_H

/*
 * Adreno ringbuffer sizes in bytes - these are converted to
 * the appropriate log2 values in the code
 */

#define KGSL_RB_SIZE (32 * 1024)
#define KGSL_RB_BLKSIZE 16

/* CP timestamp register */
#define	REG_CP_TIMESTAMP		 REG_SCRATCH_REG0


struct kgsl_device;
struct kgsl_device_private;

#define GSL_RB_MEMPTRS_SCRATCH_COUNT	 8
struct kgsl_rbmemptrs {
	int  rptr;
	int  wptr_poll;
};

#define GSL_RB_MEMPTRS_RPTR_OFFSET \
	(offsetof(struct kgsl_rbmemptrs, rptr))

#define GSL_RB_MEMPTRS_WPTRPOLL_OFFSET \
	(offsetof(struct kgsl_rbmemptrs, wptr_poll))

struct adreno_ringbuffer {
	struct kgsl_device *device;
	uint32_t flags;

	struct kgsl_memdesc buffer_desc;

	struct kgsl_memdesc memptrs_desc;
	struct kgsl_rbmemptrs *memptrs;

	/*ringbuffer size */
	unsigned int sizedwords;

	unsigned int wptr; /* write pointer offset in dwords from baseaddr */
	unsigned int rptr; /* read pointer offset in dwords from baseaddr */
	uint32_t timestamp;
};

#define GSL_RB_WRITE(ring, gpuaddr, data) \
	do { \
		*ring = data; \
		wmb(); \
		kgsl_cffdump_setmem(gpuaddr, data, 4); \
		ring++; \
		gpuaddr += sizeof(uint); \
	} while (0)

/* enable timestamp (...scratch0) memory shadowing */
#define GSL_RB_MEMPTRS_SCRATCH_MASK 0x1
#define GSL_RB_INIT_TIMESTAMP(rb)

/* mem rptr */
#define GSL_RB_CNTL_NO_UPDATE 0x0 /* enable */
#define GSL_RB_GET_READPTR(rb, data) \
	do { \
		*(data) = rb->memptrs->rptr; \
	} while (0)

#define GSL_RB_CNTL_POLL_EN 0x0 /* disable */

int adreno_ringbuffer_issueibcmds(struct kgsl_device_private *dev_priv,
				struct kgsl_context *context,
				struct kgsl_ibdesc *ibdesc,
				unsigned int numibs,
				uint32_t *timestamp,
				unsigned int flags);

int adreno_ringbuffer_init(struct kgsl_device *device);

int adreno_ringbuffer_start(struct adreno_ringbuffer *rb,
				unsigned int init_ram);

void adreno_ringbuffer_stop(struct adreno_ringbuffer *rb);

void adreno_ringbuffer_close(struct adreno_ringbuffer *rb);

void adreno_ringbuffer_issuecmds(struct kgsl_device *device,
					unsigned int flags,
					unsigned int *cmdaddr,
					int sizedwords);

void kgsl_cp_intrcallback(struct kgsl_device *device);

int adreno_ringbuffer_extract(struct adreno_ringbuffer *rb,
				unsigned int *temp_rb_buffer,
				int *rb_size);

void
adreno_ringbuffer_restore(struct adreno_ringbuffer *rb, unsigned int *rb_buff,
			int num_rb_contents);

static inline int adreno_ringbuffer_count(struct adreno_ringbuffer *rb,
	unsigned int rptr)
{
	if (rb->wptr >= rptr)
		return rb->wptr - rptr;
	return rb->wptr + rb->sizedwords - rptr;
}

/* Increment a value by 4 bytes with wrap-around based on size */
static inline unsigned int adreno_ringbuffer_inc_wrapped(unsigned int val,
							unsigned int size)
{
	return (val + sizeof(unsigned int)) % size;
}

#endif  /* __ADRENO_RINGBUFFER_H */
