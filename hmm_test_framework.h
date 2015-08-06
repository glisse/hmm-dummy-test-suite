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
 * Framework for creating test case and stress test for HMM through the dummy
 * kernel driver. Each test should have one function name hmm_test() taking a
 * pointer to hmm_ctx struct as unique parameter and returning a pointer to an
 * hmm_test_result struct.
 *
 * HMM stands for Heterogeneous Memory Management, it is an helper layer inside
 * the linux kernel to help device driver mirror a process address space on to
 * the device. This allow device's thread to use the same address space which
 * makes communication and data exchange with CPU thread a lot easier.
 *
 * This framework sole purpose is to exercise various code path inside kernel
 * to make sure that HMM performs as expected and also to flush out any bugs.
 */
#ifndef HMM_TEST_FRAMEWORK_H
#define HMM_TEST_FRAMEWORK_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

enum hmm_result {
    TEST_OK,
    TEST_ERR,
};

struct hmm_test_result {
    int                         ret;
};

struct hmm_ctx {
    const char                  *test_name;
    int                         fd;
    pid_t                       pid;
    unsigned                    page_size;
    unsigned                    page_shift;
    unsigned long               page_mask;
};

/* This is the entry point for each test. */
const struct hmm_test_result *hmm_test(struct hmm_ctx *ctx);


struct hmm_buffer {
    const char                  *name;
    void                        *ptr;
    void                        *mirror;
    unsigned long               npages;
    int                         fd;
    uint64_t                    nsys_pages;
    uint64_t                    nfaulted_sys_pages;
};

struct hmm_buffer *hmm_buffer_new_anon(struct hmm_ctx *ctx,
                                       const char *name,
                                       unsigned long npages);
struct hmm_buffer *hmm_buffer_new_file(struct hmm_ctx *ctx,
                                       const char *name,
                                       int fd,
                                       unsigned long npages);
int hmm_buffer_mirror_read(struct hmm_ctx *ctx, struct hmm_buffer *buffer);
int hmm_buffer_mirror_write(struct hmm_ctx *ctx, struct hmm_buffer *buffer);
void hmm_buffer_free(struct hmm_ctx *ctx, struct hmm_buffer *buffer);

static inline unsigned long hmm_buffer_nbytes(struct hmm_ctx *ctx,
                                              struct hmm_buffer *buffer)
{
    return buffer->npages << ctx->page_shift;
}

#define HMM_BUFFER_NEW_ANON(r, n) (r)=hmm_buffer_new_anon(ctx, #r, n)
#define HMM_BUFFER_NEW_FILE(r, f, n) (r)=hmm_buffer_new_file(ctx, #r, f, n)

int hmm_create_file(struct hmm_ctx *ctx, char *path, unsigned npages);

#endif /* HMM_TEST_FRAMEWORK_H */
