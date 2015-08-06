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
 * This test case stress check that we can read an anonymous memory range
 * through the dummy device driver.
 */
#include "hmm_test_framework.h"

static struct hmm_test_result result;
struct hmm_ctx _ctx = {
    .test_name = "anon allocation and read stress test"
};

static unsigned long NTIMES = 512;
static unsigned long NPAGES = 1024;

const struct hmm_test_result *hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer *buffer;
    unsigned long c, i, size;
    int *ptr;

    for (c = 0; c < NTIMES; ++c) {
        HMM_BUFFER_NEW_ANON(buffer, NPAGES);
        size = hmm_buffer_nbytes(ctx, buffer);

        /* Initialize buffer. */
        for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
            ptr[i] = i;
        }

        /* Read buffer to its mirror using dummy driver. */
        hmm_buffer_mirror_read(ctx, buffer);

        /* Check mirror value. */
        for (i = 0, ptr = buffer->mirror; i < size/sizeof(int); ++i) {
            if (ptr[i] != i) {
                result.ret = -1;
                return &result;
            }
        }

        hmm_buffer_free(ctx, buffer);
    }

    result.ret = 0;
    return &result;
}
