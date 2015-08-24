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
 * This test case check that we can write a file backed memory range through
 * the dummy device driver.
 */
#include "hmm_test_framework.h"


#define NPAGES  256

static int hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer *buffer;
    unsigned long i, size;
    char path[64] = {0};
    int *ptr, ret = 0;
    int fd;

    fd = hmm_create_file(ctx, path, NPAGES);
    if (fd < 0) {
        return -1;
    }

    HMM_BUFFER_NEW_FILE(buffer, fd, NPAGES);
    size = hmm_buffer_nbytes(ctx, buffer);

    /* Initialize write buffer a memory. */
    for (i = 0, ptr = buffer->mirror; i < size/sizeof(int); ++i) {
        ptr[i] = i;
    }

    /* Write buffer from its mirror using dummy driver. */
    if (hmm_buffer_mirror_write(ctx, buffer)) {
        ret = -1;
        goto out;
    }

    /* Check buffer value. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        if (ptr[i] != i) {
            ret = -1;
            goto out;
        }
    }
    hmm_buffer_free(ctx, buffer);

    /* Close the file and check that it get written to disk. */
    close(fd);
    fd = open(path, O_RDWR);
    if (fd < 0) {
        ret = -1;
        goto out;
    }
    HMM_BUFFER_NEW_FILE(buffer, fd, NPAGES);
    size = hmm_buffer_nbytes(ctx, buffer);
    /* Check buffer value. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        if (ptr[i] != i) {
            ret = -1;
            goto out;
        }
    }
    hmm_buffer_free(ctx, buffer);

out:
    unlink(path);
    return ret;
}

int main(int argc, const char *argv[])
{
    struct hmm_ctx _ctx = {
        .test_name = "file write test"
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
