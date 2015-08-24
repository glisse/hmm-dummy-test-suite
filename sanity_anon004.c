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
 * This test case check that we can not access anonymous memory range that is
 * protected with PROT_NONE through the dummy device driver.
 */
#include "hmm_test_framework.h"


static int hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer *buffer;
    unsigned long i, size;
    int *ptr;

    HMM_BUFFER_NEW_ANON(buffer, 1024);
    size = hmm_buffer_nbytes(ctx, buffer);

    /* Initialize write buffer a memory. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        ptr[i] = i;
    }

    /* Clear mirror. */
    for (i = 0, ptr = buffer->mirror; i < size/sizeof(int); ++i) {
        ptr[i] = 0;
    }

    /* Protect buffer. */
    if (hmm_buffer_mprotect(ctx, buffer, PROT_NONE)) {
        return -1;
    }

    /* Try to read, we expect an error so ignore it. */
    hmm_buffer_mirror_read(ctx, buffer);
    /* Check buffer value. */
    for (i = 0, ptr = buffer->mirror; i < size/sizeof(int); ++i) {
        if (ptr[i]) {
            return -1;
        }
    }

    /* Write buffer from its mirror using dummy driver. Ignore error as we
     * expect it to fail.
     */
    hmm_buffer_mirror_write(ctx, buffer);
    /* Unprotect buffer before checking value. */
    if (hmm_buffer_mprotect(ctx, buffer, PROT_READ)) {
        return -1;
    }
    /* Check buffer value. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        if (ptr[i] != i) {
            return -1;
        }
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    struct hmm_ctx _ctx = {
        .test_name = "anon mprotect with PROT_NONE test"
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
