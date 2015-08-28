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
 * This test case check that allocating enough buffers lead to page
 * reclaimation and the HMM invalidation code path get exercise.
 */
#include "hmm_test_framework.h"

static unsigned long BUFFER_SIZE = (512 << 12);
#define NBUFFERS 512


static int hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer **buffers;
    unsigned long j, nrefaults;

    buffers = malloc(NBUFFERS * sizeof(void*));
    if (buffers == NULL) {
        return -ENOMEM;
    }

retry:
    /* This will allocate several buffers each with the same number of page.
     * We initialize each of them and we check read them back through the
     * dummy driver.
     */
    for (j = 0; j < NBUFFERS; ++j) {
        unsigned long i, size;
        int *ptr;

        printf("\r(..) Allocating buffers[%ld]", j);
        HMM_BUFFER_NEW_ANON(buffers[j], BUFFER_SIZE);
        size = hmm_buffer_nbytes(buffers[j]);

        /* Initialize buffer. */
        for (i = 0, ptr = buffers[j]->ptr; i < size/sizeof(int); ++i) {
            ptr[i] = i;
        }

        /* Read buffer to its mirror using dummy driver. */
        hmm_buffer_mirror_read(ctx, buffers[j], -1UL, 0);

        /* Check mirror value. */
        for (i = 0, ptr = buffers[j]->mirror; i < size/sizeof(int); ++i) {
            if (ptr[i] != i) {
                return -1;
            }
        }
    }

    /* Go over all the buffers again and check that we can still read the
     * proper value. Given the amount of buffer it is likely that some
     * memory previously allocated was reclaim and thus dummy driver had
     * to refault.
     */
    for (j = 0, nrefaults = 0; j < NBUFFERS; ++j) {
        unsigned long i, size = hmm_buffer_nbytes(buffers[j]);
        int *ptr;

        printf("\r(..) Re-checking buffers[%ld]", j);
        /* Read buffer to its mirror using dummy driver. */
        hmm_buffer_mirror_read(ctx, buffers[j], -1UL, 0);

        nrefaults += buffers[j]->nfaulted_sys_pages;

        /* Check mirror value. */
        for (i = 0, ptr = buffers[j]->mirror; i < size/sizeof(int); ++i) {
            if (ptr[i] != i) {
                return -1;
            }
        }
    }

    for (j = 0; j < NBUFFERS; ++j) {
        hmm_buffer_free(buffers[j]);
    }

    if (nrefaults < (BUFFER_SIZE >> 12)) {
        /* We failed to exhaust memory retry again with bigger buffer. */
        printf("\r(..) Failed to exhaust memory (%ld)", nrefaults);
        BUFFER_SIZE *= 2;
        goto retry;
    }

    /* Make sure we clear the line. */
    printf("\r                              \r");

    return 0;
}

int main(int argc, const char *argv[])
{
    struct hmm_ctx _ctx = {
        .test_name = "anon page reclaimation test"
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
