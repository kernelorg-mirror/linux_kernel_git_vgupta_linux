/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020 SiFive
 * Copyright (C) 2022 Rivos
 */

#ifndef __ASM_RISCV_VECTOR_H
#define __ASM_RISCV_VECTOR_H

#include <linux/types.h>
#include <asm/asm-offsets.h>
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
extern unsigned long riscv_vsize;

extern void __vstate_save(struct __riscv_v_state *save_to, void *datap);
extern void __vstate_restore(struct __riscv_v_state *restore_from, void *datap);

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

static inline void __vstate_clean(struct pt_regs *regs)
{
	regs->status = (regs->status & ~(SR_VS)) | SR_VS_CLEAN;
}

static inline void vstate_off(struct task_struct *task,
			      struct pt_regs *regs)
{
	regs->status = (regs->status & ~SR_VS) | SR_VS_OFF;
}

static inline void vstate_save(struct task_struct *task,
			       struct pt_regs *regs)
{
	if ((regs->status & SR_VS) == SR_VS_DIRTY) {
		struct __riscv_v_state *vstate = &(task->thread.vstate);

		__vstate_save(vstate, vstate->datap);
		__vstate_clean(regs);
	}
}

static inline void vstate_restore(struct task_struct *task,
				  struct pt_regs *regs)
{
	if ((regs->status & SR_VS) != SR_VS_OFF) {
		struct __riscv_v_state *vstate = &(task->thread.vstate);

		__vstate_restore(vstate, vstate->datap);
		__vstate_clean(regs);
	}
}

static inline void __switch_to_vector(struct task_struct *prev,
				   struct task_struct *next)
{
	struct pt_regs *regs;

	regs = task_pt_regs(prev);
	if (unlikely(regs->status & SR_SD))
		vstate_save(prev, regs);
	vstate_restore(next, task_pt_regs(next));
}

#else /* ! CONFIG_RISCV_ISA_V */

#define riscv_vsize (0)
#define vstate_save(task, regs)			do { } while (0)
#define vstate_restore(task, regs)		do { } while (0)
#define __switch_to_vector(__prev, __next)	do { } while (0)
static __always_inline bool has_vector(void) { return false; }

#endif /* ! CONFIG_RISCV_ISA_V */

#endif /* ! __ASSEMBLY__ */

#endif /* ! __ASM_RISCV_VECTOR_H */
