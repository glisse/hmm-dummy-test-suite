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

#define MAX_SLEEP_BEFORE_MUNMAP_NS 32000
#define BUFFER_SIZE (128 << 12)
#define NTIMES 1024

static struct hmm_buffer *_buffer = NULL;
static unsigned long _c = -1;
static int _ret = 0;


void *access_buffer(void *p)
{
    struct hmm_ctx *ctx = p;
    unsigned long c;

    for (c = 0; c < NTIMES; ++c) {
        unsigned long i, size;
        int *ptr;

        while (!_buffer || _c != c);
        size = hmm_buffer_nbytes(_buffer);
        /* Write buffer from its mirror using dummy driver. */
        if (!hmm_buffer_mirror_read(ctx, _buffer)) {
            /* Check buffer value only if we care. */
            for (i = 0, ptr = _buffer->mirror; i < size/sizeof(int); ++i) {
                if (ptr[i] != i) {
                    fprintf(stderr, "%s:%d Read succeeded but with invalid "
                            "value [%ld] = %d\n", __FILE__, __LINE__, i, ptr[i]);
                    _ret = -1;
                    return NULL;
                }
            }
        }
        /* Allow main thread to free the buffer. */
        _buffer = NULL;
    }

    return NULL;
}

static int hmm_test(struct hmm_ctx *ctx)
{
    pthread_t thread;


    if (pthread_create(&thread, NULL, access_buffer, ctx)) {
        fprintf(stderr, "%s:%d Thread creation failed !\n",
                __FILE__, __LINE__);
        return -1;
    }

    for (_c = 0; (_c < NTIMES) && !_ret; ++_c) {
        struct hmm_buffer *buffer;
        unsigned long i, size;
        int *ptr;

        HMM_BUFFER_NEW_ANON(buffer, BUFFER_SIZE);
        size = hmm_buffer_nbytes(buffer);
        /* Initialize write buffer a memory. */
        for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
            ptr[i] = i;
        }
        _buffer = buffer;
        hmm_nanosleep(hmm_random() % MAX_SLEEP_BEFORE_MUNMAP_NS);
        munmap(buffer->ptr, hmm_buffer_nbytes(buffer));
        buffer->ptr = NULL;
        while (_buffer);
        hmm_buffer_free(buffer);
    }

    pthread_join(thread, NULL);

    return _ret;
}

int main(int argc, const char *argv[])
{
    struct hmm_ctx _ctx = {
        .test_name = "anon read access and munmap stress test"
    };
    struct hmm_ctx *ctx = &_ctx;
    int ret;

    ret = hmm_ctx_init(ctx);
    if (ret) {
        goto out;
    }

    ret = hmm_test(ctx);
    hmm_ctx_fini(ctx);

out:
    printf("(%s)[%s] %s\n", ret ? "EE" : "OK", argv[0], ctx->test_name);
    return ret;
}
