#ifndef RISCV_IRQ_H
#define RISCV_IRQ_H

#define RISCV_CLINT_MSIP0_ADDR (RISCV_CLINT_ADDR + 0)

enum
{
    RISCV_IRQ_MSI = 3,
    RISCV_IRQ_MTI = 7,
    RISCV_IRQ_MEI = 11,
    RISCV_IRQ_SSI = 1,
    RISCV_IRQ_STI = 5,
    RISCV_IRQ_SEI = 9,
    RISCV_IRQ_USI = 0,
    RISCV_IRQ_UTI = 4,
    RISCV_IRQ_UEI = 8,
};

#define RISCV_IRQ_NUMS (RISCV_IRQ_MEI + 1)

enum
{
    RISCV_IRQ_MASK_MSI = (1UL << RISCV_IRQ_MSI),
    RISCV_IRQ_MASK_MTI = (1UL << RISCV_IRQ_MTI),
    RISCV_IRQ_MASK_MEI = (1UL << RISCV_IRQ_MEI),
    RISCV_IRQ_MASK_SSI = (1UL << RISCV_IRQ_SSI),
    RISCV_IRQ_MASK_STI = (1UL << RISCV_IRQ_STI),
    RISCV_IRQ_MASK_SEI = (1UL << RISCV_IRQ_SEI),
    RISCV_IRQ_MASK_USI = (1UL << RISCV_IRQ_USI),
    RISCV_IRQ_MASK_UTI = (1UL << RISCV_IRQ_UTI),
    RISCV_IRQ_MASK_UEI = (1UL << RISCV_IRQ_UEI),
};

void RISCV_IRQ_Init(void);
void RISCV_IRQ_Enable(unsigned int irq);
void RISCV_IRQ_Disable(unsigned int irq);
void RISCV_IRQ_SetHandler(unsigned int irq, void (*handler)(void));
void RISCV_IRQ_GlobalEnable(void);
void RISCV_IRQ_GlobalDisable(void);

#endif