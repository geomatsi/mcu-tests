/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2010-2020, The Regents of the University of California
 * (Regents).  All Rights Reserved.
 */

#include <stdint.h>

#include "htif.h"

#define HTIF_DATA_BITS		48
#define HTIF_DATA_MASK		((1ULL << HTIF_DATA_BITS) - 1)
#define HTIF_DATA_SHIFT		0
#define HTIF_CMD_BITS		8
#define HTIF_CMD_MASK		((1ULL << HTIF_CMD_BITS) - 1)
#define HTIF_CMD_SHIFT		48
#define HTIF_DEV_BITS		8
#define HTIF_DEV_MASK		((1ULL << HTIF_DEV_BITS) - 1)
#define HTIF_DEV_SHIFT		56

#define HTIF_DEV_SYSTEM		0
#define HTIF_DEV_CONSOLE	1

#define HTIF_CONSOLE_CMD_PUTC	1

#define TOHOST_CMD(dev, cmd, payload) \
	(((uint64_t)(dev) << HTIF_DEV_SHIFT) | \
	 ((uint64_t)(cmd) << HTIF_CMD_SHIFT) | \
	 (uint64_t)(payload))

static volatile uint64_t tohost;
static volatile uint64_t fromhost;

void htif_putc(char ch)
{
	while (tohost)
		fromhost = 0;

	tohost = TOHOST_CMD(HTIF_DEV_CONSOLE, HTIF_CONSOLE_CMD_PUTC, ch);
}

void htif_exit(uint32_t reason)
{
	while (tohost)
		fromhost = 0;

	tohost = TOHOST_CMD(HTIF_DEV_SYSTEM, 0, (reason << 1) | 1);
	while (1);
}
