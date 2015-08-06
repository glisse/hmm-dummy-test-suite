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

static struct hmm_test_result result;
struct hmm_ctx _ctx = {
    .test_name = "file write test"
};

const struct hmm_test_result *hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer *buffer;
    unsigned long i, size;
    char path[64] = {0};
    int *ptr;
    int fd;

    fd = hmm_create_file(ctx, path, NPAGES);
    if (fd < 0) {
        result.ret = -1;
        return &result;
    }

    HMM_BUFFER_NEW_FILE(buffer, fd, NPAGES);
    size = hmm_buffer_nbytes(ctx, buffer);

    /* Initialize write buffer a memory. */
    for (i = 0, ptr = buffer->mirror; i < size/sizeof(int); ++i) {
        ptr[i] = i;
    }

    /* Write buffer from its mirror using dummy driver. */
    hmm_buffer_mirror_write(ctx, buffer);

    /* Check buffer value. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        if (ptr[i] != i) {
            result.ret = -1;
            goto out;
        }
    }
    hmm_buffer_free(ctx, buffer);

    /* Close the file and check that it get written to disk. */
    close(fd);
    fd = open(path, O_RDWR);
    if (fd < 0) {
        result.ret = -1;
        goto out;
    }
    HMM_BUFFER_NEW_FILE(buffer, fd, NPAGES);
    size = hmm_buffer_nbytes(ctx, buffer);
    /* Check buffer value. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        if (ptr[i] != i) {
            result.ret = -1;
            goto out;
        }
    }
    hmm_buffer_free(ctx, buffer);

    result.ret = 0;
out:
    unlink(path);
    return &result;
}
