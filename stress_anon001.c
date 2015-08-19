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

static struct hmm_test_result result;
struct hmm_ctx _ctx = {
    .test_name = "anon page reclaimation test"
};

static unsigned long NBUFFERS = 512;
static unsigned long BUFFER_NPAGES = 1024;

const struct hmm_test_result *hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer **buffers;
    unsigned long j, nrefaults;

    buffers = malloc(NBUFFERS * sizeof(void*));
    if (buffers == NULL) {
        result.ret = -ENOMEM;
        return &result;
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
        HMM_BUFFER_NEW_ANON(buffers[j], BUFFER_NPAGES);
        size = hmm_buffer_nbytes(ctx, buffers[j]);

        /* Initialize buffer. */
        for (i = 0, ptr = buffers[j]->ptr; i < size/sizeof(int); ++i) {
            ptr[i] = i;
        }

        /* Read buffer to its mirror using dummy driver. */
        hmm_buffer_mirror_read(ctx, buffers[j]);

        /* Check mirror value. */
        for (i = 0, ptr = buffers[j]->mirror; i < size/sizeof(int); ++i) {
            if (ptr[i] != i) {
                result.ret = -1;
                return &result;
            }
        }
    }

    /* Go over all the buffers again and check that we can still read the
     * proper value. Given the amount of buffer it is likely that some
     * memory previously allocated was reclaim and thus dummy driver had
     * to refault.
     */
    for (j = 0, nrefaults = 0; j < NBUFFERS; ++j) {
        unsigned long i, size = hmm_buffer_nbytes(ctx, buffers[j]);
        int *ptr;

        printf("\r(..) Re-checking buffers[%ld]", j);
        /* Read buffer to its mirror using dummy driver. */
        hmm_buffer_mirror_read(ctx, buffers[j]);

        nrefaults += buffers[j]->nfaulted_sys_pages;

        /* Check mirror value. */
        for (i = 0, ptr = buffers[j]->mirror; i < size/sizeof(int); ++i) {
            if (ptr[i] != i) {
                result.ret = -1;
                return &result;
            }
        }
    }

    for (j = 0; j < NBUFFERS; ++j) {
        hmm_buffer_free(ctx, buffers[j]);
    }

    if (nrefaults < BUFFER_NPAGES) {
        /* We failed to exhaust memory retry again with bigger buffer. */
        printf("\r(..) Failed to exhaust memory (%ld)", nrefaults);
        BUFFER_NPAGES *= 2;
        goto retry;
    }

    /* Make sure we clear the line. */
    printf("\r                              ");

    result.ret = 0;
    return &result;
}
