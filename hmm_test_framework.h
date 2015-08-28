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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


struct hmm_ctx {
    const char                  *test_name;
    int                         fd;
    pid_t                       pid;
};

int hmm_ctx_init(struct hmm_ctx *ctx);
void hmm_ctx_fini(struct hmm_ctx *ctx);


struct hmm_buffer {
    const char                  *name;
    void                        *ptr;
    void                        *mirror;
    unsigned long               npages;
    int                         fd;
    uint64_t                    nsys_pages;
    uint64_t                    nfaulted_sys_pages;
    uint64_t                    ndev_pages;
    uint64_t                    nfaulted_dev_pages;
};

struct hmm_buffer *hmm_buffer_new_anon(const char *name, unsigned long nbytes);
struct hmm_buffer *hmm_buffer_new_share(const char *name, unsigned long nbytes);
struct hmm_buffer *hmm_buffer_new_file(const char *name, int fd, unsigned long bytes);
int hmm_buffer_mirror_read(struct hmm_ctx *ctx,
                           struct hmm_buffer *buffer,
                           unsigned long size,
                           unsigned long offset);
int hmm_buffer_mirror_write(struct hmm_ctx *ctx,
                            struct hmm_buffer *buffer,
                            unsigned long size,
                            unsigned long offset);
int hmm_buffer_mirror_migrate_to(struct hmm_ctx *ctx, struct hmm_buffer *buffer);
int hmm_buffer_mprotect(struct hmm_buffer *buffer, int prot);
void hmm_buffer_free(struct hmm_buffer *buffer);
unsigned long hmm_buffer_nbytes(struct hmm_buffer *buffer);

#define HMM_BUFFER_NEW_ANON(r, n) (r)=hmm_buffer_new_anon(#r, n)
#define HMM_BUFFER_NEW_SHARE(r, n) (r)=hmm_buffer_new_share(#r, n)
#define HMM_BUFFER_NEW_FILE(r, f, n) (r)=hmm_buffer_new_file(#r, f, n)

int hmm_create_file(char *path, unsigned long size);

unsigned hmm_random(void);
void hmm_nanosleep(unsigned n);

#endif /* HMM_TEST_FRAMEWORK_H */
