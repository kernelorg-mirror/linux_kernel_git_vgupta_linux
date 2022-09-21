/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _UAPI_ASM_RISCV_SIGCONTEXT_H
#define _UAPI_ASM_RISCV_SIGCONTEXT_H

#include <asm/ptrace.h>

/* The Magic number for signal context frame header. */
#define RVV_MAGIC	0x53465457
#define END_MAGIC	0x0

/* The size of END signal context header. */
#define END_HDR_SIZE	0x0

/* Every optional extension state needs to have the hdr. */
struct __riscv_ctx_hdr {
	__u32 magic;
	__u32 size;
};

struct __sc_riscv_v_state {
	struct __riscv_ctx_hdr head;
	struct __riscv_v_state v_state;
} __attribute__((aligned(16)));

/*
 * Signal context structure
 *
 * This contains the context saved before a signal handler is invoked;
 * it is restored by sys_sigreturn / sys_rt_sigreturn.
 */
struct sigcontext {
	struct user_regs_struct sc_regs;
	union __riscv_fp_state sc_fpregs;
	/*
	 * 4K + change: for vector state and future expansion.
	 * This is sufficient for VLENB (vec register width) of 128.
	 * For wider implementations this is handled at runtime.
	 * Note: The alignment of 16 is ABI mandated for stack entries.
	 */
	__u8 sc_extn[4224] __attribute__((__aligned__(16)));
};

#endif /* _UAPI_ASM_RISCV_SIGCONTEXT_H */
