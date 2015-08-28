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
 * This test case check that HMM properly handle truncation.
 */
#include "hmm_test_framework.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define BUFFER_SIZE (256 << 12)


static int hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer *buffer;
    unsigned long i, size;
    char path[64] = {0};
    int *ptr, ret = 0;
    int fd;

    fd = hmm_create_file(path, BUFFER_SIZE);
    if (fd < 0) {
        return -1;
    }

    HMM_BUFFER_NEW_FILE(buffer, fd, BUFFER_SIZE);
    size = hmm_buffer_nbytes(buffer);

    /* Initialize buffer memory. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        ptr[i] = i;
    }

    ftruncate(fd, size / 2);

    if (hmm_buffer_mirror_read(ctx, buffer, size / 2, 0)) {
        ret = -1;
        goto out;
    }
    if (hmm_buffer_mirror_read(ctx, buffer, size / 2, size / 2)) {
        ret = errno;
    }

    /* Close the file and check that it get written to disk. */
out:
    close(fd);
    unlink(path);
    hmm_buffer_free(buffer);

    return ret ? ret == EFAULT ? 0 : -1 : 0;
}

int main(int argc, const char *argv[])
{
    struct hmm_ctx _ctx = {
        .test_name = "file truncate test"
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
