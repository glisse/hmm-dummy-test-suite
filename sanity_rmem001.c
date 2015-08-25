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
 * This test case check that we can migrate anonymous memory to device memory.
 */
#include "hmm_test_framework.h"

#define BUFFER_SIZE (256 << 12)


static int hmm_test(struct hmm_ctx *ctx)
{
    struct hmm_buffer *buffer;
    unsigned long i, size;
    int *ptr, ret = 0;

    HMM_BUFFER_NEW_ANON(buffer, BUFFER_SIZE);
    size = hmm_buffer_nbytes(buffer);

    /* Initialize buffer. */
    for (i = 0, ptr = buffer->ptr; i < size/sizeof(int); ++i) {
        ptr[i] = i;
    }

    /* Migrate buffer to remote memory. */
    hmm_buffer_mirror_migrate_to(ctx, buffer);
    if (buffer->nfaulted_dev_pages != buffer->npages) {
        fprintf(stderr, "(EE:%4d) migrated %ld pages out of %ld\n",
                __LINE__, (long)buffer->nfaulted_dev_pages, buffer->npages);
        ret = -1;
    }

    hmm_buffer_free(buffer);

    return ret;
}

int main(int argc, const char *argv[])
{
    struct hmm_ctx _ctx = {
        .test_name = "anon migration test"
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
