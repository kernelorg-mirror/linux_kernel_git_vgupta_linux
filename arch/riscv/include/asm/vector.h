/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020 SiFive
 * Copyright (C) 2022 Rivos
 */

#ifndef __ASM_RISCV_VECTOR_H
#define __ASM_RISCV_VECTOR_H

#include <linux/types.h>
#include <asm/csr.h>

#ifdef __ASSEMBLY__

.macro RVV_ENABLE reg
	li      \reg, SR_VS
	csrs    CSR_STATUS, \reg
.endm

.macro RVV_DISABLE reg
	li      \reg, SR_VS
	csrc    CSR_STATUS, \reg
.endm

#else /* ! __ASSEMBLY__ */

#ifdef CONFIG_RISCV_ISA_V

extern struct static_key_false riscv_isa_ext_key_vector;

static __always_inline bool has_vector(void)
{
       return static_branch_likely(&riscv_isa_ext_key_vector);
}

static inline void rvv_enable(void) {
	unsigned int vs = SR_VS;
	csr_set(CSR_STATUS, vs);
}

static inline void rvv_disable(void) {
	unsigned int vs = SR_VS;
	csr_clear(CSR_STATUS, vs);
}

#else /* ! CONFIG_RISCV_ISA_V */

static __always_inline bool has_vector(void) { return false; }

#endif /* ! CONFIG_RISCV_ISA_V */

#endif /* ! __ASSEMBLY__ */

#endif /* ! __ASM_RISCV_VECTOR_H */
