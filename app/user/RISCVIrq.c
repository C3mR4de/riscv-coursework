#include "RISCVIrq.h"
#include "csr.h"
#include "scr1_csr_encoding.h"

void (*handler_map[RISCV_IRQ_NUMS])(void);

void RISCV_IRQ_Enable(unsigned int irq_num)
{
    set_csr(mie, 1 << irq_num);
}

void RISCV_IRQ_Disable(unsigned int irq_num)
{
    clear_csr(mie, 1 << irq_num);
}

void RISCV_IRQ_SetHandler(unsigned int irq_num, void(*handler)(void))
{
    handler_map[irq_num] = handler;
}

void RISCV_IRQ_GlobalEnable(void)
{
    set_csr(mstatus, MSTATUS_MIE);
}

void RISCV_IRQ_GlobalDisable(void)
{
    clear_csr(mstatus, MSTATUS_MIE);
}

void trap_handler(void)
{
    uint32_t mcause = read_csr(mcause);

    if ((mcause & MCAUSE_INT) != 0)
    {
        // handle interrupt
        mcause &= 0xFF;

        if (mcause >= RISCV_IRQ_NUMS)
        {
            while (true); // error
        }

        if (handler_map[mcause] != 0)
        {
            handler_map[mcause]();
        }
        else
        {
            while (true); // NO handler
        }
    }
}