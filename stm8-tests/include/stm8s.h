#ifndef __STM8S_H__
#define __STM8S_H__

#include <stdint.h>

#define PA_BASE 0x5000
#define PB_BASE 0x5005
#define PC_BASE 0x500A
#define PD_BASE 0x500F

#define Px_ODR_OFFSET 0x0
#define Px_IDR_OFFSET 0x1
#define Px_DDR_OFFSET 0x2
#define Px_CR1_OFFSET 0x3
#define Px_CR2_OFFSET 0x4

#define PB_ODR (*(volatile uint8_t *)(PB_BASE + Px_ODR_OFFSET))
#define PB_IDR (*(volatile uint8_t *)(PB_BASE + Px_IDR_OFFSET))
#define PB_DDR (*(volatile uint8_t *)(PB_BASE + Px_DDR_OFFSET))
#define PB_CR1 (*(volatile uint8_t *)(PB_BASE + Px_CR1_OFFSET))
#define PB_CR2 (*(volatile uint8_t *)(PB_BASE + Px_CR2_OFFSET))

#endif /* __STM8S_H__ */
