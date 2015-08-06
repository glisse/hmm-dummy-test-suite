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
 * This test case check that we can read an anonymous memory range through
 * the dummy device driver.
 */
#include "hmm_test_framework.h"

static struct hmm_test_result result;

const struct hmm_test_result *hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer *buffer;
    unsigned long i, size;
    int *ptr;

    HMM_BUFFER_NEW_ANON(buffer, 1024);
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

    result.ret = 0;
    return &result;
}
