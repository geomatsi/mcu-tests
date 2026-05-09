/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __RISCV_ENCODING_H__
#define __RISCV_ENCODING_H__

#define BIT(n)                          (1UL << (n))

#define CAUSE_MISALIGNED_FETCH          0x0
#define CAUSE_FETCH_ACCESS              0x1
#define CAUSE_ILLEGAL_INSTRUCTION       0x2
#define CAUSE_BREAKPOINT                0x3
#define CAUSE_MISALIGNED_LOAD           0x4
#define CAUSE_LOAD_ACCESS               0x5
#define CAUSE_MISALIGNED_STORE          0x6
#define CAUSE_STORE_ACCESS              0x7
#define CAUSE_USER_ECALL                0x8
#define CAUSE_SUPERVISOR_ECALL          0x9
#define CAUSE_VIRTUAL_SUPERVISOR_ECALL  0xa
#define CAUSE_MACHINE_ECALL             0xb
#define CAUSE_FETCH_PAGE_FAULT          0xc
#define CAUSE_LOAD_PAGE_FAULT           0xd
#define CAUSE_STORE_PAGE_FAULT          0xf
#define CAUSE_FETCH_GUEST_PAGE_FAULT    0x14
#define CAUSE_LOAD_GUEST_PAGE_FAULT     0x15
#define CAUSE_VIRTUAL_INST_FAULT        0x16
#define CAUSE_STORE_GUEST_PAGE_FAULT    0x17

#define MCAUSE_IRQ_MASK                 (1UL << (__riscv_xlen - 1))

#define MSTATUS_SIE                     (1UL << 1)
#define MSTATUS_MIE                     (1UL << 3)
#define MSTATUS_SPIE                    (1UL << 5)
#define MSTATUS_MPIE                    (1UL << 7)
#define MSTATUS_SPP                     (1UL << 8)
#define MSTATUS_MPP_SHIFT               11
#define MSTATUS_MPP                     (3UL << MSTATUS_MPP_SHIFT)

#define MIE_MSIE                        (1UL << 3)
#define MIE_MTIE                        (1UL << 7)

#define IRQ_S_SOFT                      1
#define IRQ_VS_SOFT                     2
#define IRQ_M_SOFT                      3
#define IRQ_S_TIMER                     5
#define IRQ_VS_TIMER                    6
#define IRQ_M_TIMER                     7
#define IRQ_S_EXT                       9
#define IRQ_VS_EXT                      10
#define IRQ_M_EXT                       11
#define IRQ_S_GEXT                      12
#define IRQ_PMU_OVF                     13

#endif /* __RISCV_ENCODING_H__ */
