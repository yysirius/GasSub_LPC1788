/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * yaffscfg2k.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

#include "yaffscfg.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "yaffs_k9f4g08u0b.h"
#include "yaffs_trace.h"
#include "yaffs_osglue.h"

#include <errno.h>

unsigned yaffs_trace_mask =

/*
  YAFFS_TRACE_SCAN |
	YAFFS_TRACE_GC |
*/
  YAFFS_TRACE_WRITE |
	YAFFS_TRACE_ERASE |
	YAFFS_TRACE_ERROR |
	YAFFS_TRACE_TRACING |
	YAFFS_TRACE_ALLOCATE |
	YAFFS_TRACE_BAD_BLOCKS |
	YAFFS_TRACE_VERIFY |
	0;

/* Configure the devices that will be used */

#include "yaffs_nand_drv.h"

int yaffs_start_up(void)
{
	static int start_up_called = 0;

	if(start_up_called)
		return 0;
	start_up_called = 1;

	/* Call the OS initialisation (eg. set up lock semaphore */
	yaffsfs_OSInitialisation();

	/* Install the various devices and their device drivers */
	yaffs_k9f4g08u0b_install_drv("nand");

	return 0;
}
