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


static int hmm_test(struct hmm_ctx *ctx)
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
    if (hmm_buffer_mirror_read(ctx, buffer)) {
        return -1;
    }

    /* Check mirror value. */
    for (i = 0, ptr = buffer->mirror; i < size/sizeof(int); ++i) {
        if (ptr[i] != i) {
            return -1;
        }
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    struct hmm_ctx _ctx = {
        .test_name = "anon read test"
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
