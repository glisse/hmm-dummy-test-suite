/*
 * Copyright 2013 Red Hat Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Authors: Jérôme Glisse <jglisse@redhat.com>
 */
/*
 * This test case stress test concurrency btw HMM access to anonymous memory
 * and mumap. Hoping that it trigger call to mmu_notifier invalidate while
 * dummy device access the range.
 */
#include "hmm_test_framework.h"
#include <pthread.h>

static struct hmm_test_result result;
struct hmm_ctx _ctx = {
    .test_name = "anon read access and munmap stress test"
};

#define NTIMES 1024
#define BUFFER_NPAGES 128
#define MAX_SLEEP_BEFORE_MUNMAP_NS 32000

static struct hmm_buffer *_buffer = NULL;
static unsigned long _c = -1;

void *access_buffer(void *p)
{
    struct hmm_ctx *ctx = p;
    unsigned long c;

    for (c = 0; c < NTIMES; ++c) {
        unsigned long i, size;
        int *ptr;

        while (!_buffer || _c != c);
        size = hmm_buffer_nbytes(ctx, _buffer);
        /* Write buffer from its mirror using dummy driver. */
        if (!hmm_buffer_mirror_read(ctx, _buffer)) {
            /* Check buffer value only if we care. */
            for (i = 0, ptr = _buffer->mirror; i < size/sizeof(int); ++i) {
                if (ptr[i] != i) {
                    fprintf(stderr, "%s:%d Read succeeded but with invalid "
                            "value [%ld] = %d\n", __FILE__, __LINE__, i, ptr[i]);
                    result.ret = -1;
                    return &result;
                }
            }
        }
        /* Allow main thread to free the buffer. */
        _buffer = NULL;
    }

    return NULL;
}

const struct hmm_test_result *hmm_test(struct hmm_ctx *ctx)
{
    pthread_t thread;

    result.ret = 0;

    if (pthread_create(&thread, NULL, access_buffer, ctx)) {
        fprintf(stderr, "%s:%d Thread creation failed !\n",
                __FILE__, __LINE__);
        result.ret = -1;
        return &result;
    }

    for (_c = 0; (_c < NTIMES) && !result.ret; ++_c) {
        struct hmm_buffer *buffer;
        unsigned long i, size;
        int *ptr;

        HMM_BUFFER_NEW_ANON(buffer, BUFFER_NPAGES);
        size = hmm_buffer_nbytes(ctx, buffer);
        /* Initialize write buffer a memory. */
        for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
            ptr[i] = i;
        }
        _buffer = buffer;
        hmm_nanosleep(hmm_random() % MAX_SLEEP_BEFORE_MUNMAP_NS);
        munmap(buffer->ptr, buffer->npages << ctx->page_shift);
        buffer->ptr = NULL;
        while (_buffer);
        hmm_buffer_free(ctx, buffer);
    }

    pthread_join(thread, NULL);

    return &result;
}
