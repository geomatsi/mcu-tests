#ifndef ACLINT_H
#define ACLINT_H

#define CLINT_BASE        (0x02000000UL)
#define REG_MSWI0         ((volatile unsigned long*)(CLINT_BASE + 0x00000000UL))
#define REG_MTIME         ((volatile unsigned long*)(CLINT_BASE + 0x0000bff8UL))
#define REG_MTIMECMP      ((volatile unsigned long*)(CLINT_BASE + 0x00004000UL))

#endif /* ACLINT_H */
