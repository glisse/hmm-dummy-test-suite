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
 * This is a dummy driver to exercice the HMM (heterogeneous memory management)
 * API of the kernel. It allow an userspace program to expose its whole address
 * space through the hmm dummy driver file.
 */
#ifndef _UAPI_LINUX_HMM_DUMMY_H
#define _UAPI_LINUX_HMM_DUMMY_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/irqnr.h>

struct hmm_dummy_read {
	uint64_t		address;
	uint64_t		size;
	uint64_t		ptr;
	uint64_t		nsys_pages;
	uint64_t		nfaulted_sys_pages;
	uint64_t		ndev_pages;
	uint64_t		nfaulted_dev_pages;
	uint64_t		reserved[9];
};

struct hmm_dummy_write {
	uint64_t		address;
	uint64_t		size;
	uint64_t		ptr;
	uint64_t		nsys_pages;
	uint64_t		nfaulted_sys_pages;
	uint64_t		ndev_pages;
	uint64_t		nfaulted_dev_pages;
	uint64_t		reserved[9];
};

struct hmm_dummy_migrate {
	uint64_t		address;
	uint64_t		size;
	uint64_t		nfaulted_sys_pages;
	uint64_t		nfaulted_dev_pages;
	uint64_t		reserved[12];
};

/* Expose the address space of the calling process through hmm dummy dev file */
#define HMM_DUMMY_EXPOSE_MM	_IO('H', 0x00)
#define HMM_DUMMY_READ		_IOWR('H', 0x01, struct hmm_dummy_read)
#define HMM_DUMMY_WRITE		_IOWR('H', 0x02, struct hmm_dummy_write)
#define HMM_DUMMY_MIGRATE_TO	_IOWR('H', 0x03, struct hmm_dummy_migrate)

#endif /* _UAPI_LINUX_HMM_DUMMY_H */
