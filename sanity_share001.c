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
 * This test case check that on fork we can access parent value of anon buffer
 * and that writting anon buffer from child does not affect parent's copy.
 */
#include "hmm_test_framework.h"
#include <sys/wait.h>

#define BUFFER_SIZE (1024 << 12)

struct hmm_buffer *buffer;


static int hmm_test(struct hmm_ctx *ctx)
{
    unsigned long size;
    int *ptr, i;

    size = hmm_buffer_nbytes(buffer);

    /* Check buffer value. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        if (ptr[i] != i) {
            fprintf(stderr, "(EE)[%s:%d] Expected %d got %d\n",
                    __FILE__, __LINE__, i, ptr[i]);
            return -1;
        }
    }

    /* Initialize write buffer a memory. */
    for (i = 0, ptr = buffer->mirror; i < size/sizeof(int); ++i) {
        ptr[i] = -i;
    }

    /* Write buffer from its mirror using dummy driver. */
    if (hmm_buffer_mirror_write(ctx, buffer)) {
        return -1;
    }

    /* Check buffer value. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        if (ptr[i] != -i) {
            fprintf(stderr, "(EE)[%s:%d] Expected %d got %d\n",
                    __FILE__, __LINE__, i, ptr[i]);
            return -1;
        }
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    struct hmm_ctx _ctx = {
        .test_name = "share memory test"
    };
    struct hmm_ctx *ctx = &_ctx;
    unsigned long size;
    int *ptr, ret, i;
    pid_t pid;

    /* Allocate and initialize buffer. */
    HMM_BUFFER_NEW_SHARE(buffer, BUFFER_SIZE);
    size = hmm_buffer_nbytes(buffer);
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        ptr[i] = i;
    }

    pid = fork();

    switch (pid) {
    case 0:
        ret = hmm_ctx_init(ctx);
        if (!ret) {
            ret = hmm_test(ctx);
            hmm_ctx_fini(ctx);
        }
        return ret;
    case -1:
        ret = -1;
        goto out;
    default:
        ret = -1;
        while (wait(&ret) != pid);
        if (!ret) {
            /* Check that buffer values did change. */
            for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
                if (ptr[i] != -i) {
                    fprintf(stderr, "(EE)[%s:%d] Expected %d got %d\n",
                            __FILE__, __LINE__, i, ptr[i]);
                    ret = -1;
                    break;
                }
            }
        }
        break;
    }

out:
    printf("(%s)[%s] %s\n", ret ? "EE" : "OK", argv[0], ctx->test_name);
    return ret;
}
