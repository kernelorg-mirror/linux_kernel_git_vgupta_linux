// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023-24 Rivos Inc.
 */

#include <linux/binfmts.h>
#include <linux/elf.h>

#undef pr_fmt
#define pr_fmt(fmt) "rv-elf-attr: " fmt

#define PT_RISCV_ATTRIBUTES		0x70000003

#define RV_ATTR_TAG_stack_align		4
#define RV_ATTR_TAG_arch		5
#define RV_ATTR_TAG_unaligned_access	6

#define RV_ATTR_SEC_SZ			SZ_1K

static void rv_elf_attr_int(u64 tag, u64 val)
{
}

static void rv_elf_attr_str(u64 tag, const char *str)
{
}

static u64
decode_uleb128 (unsigned char **dpp)
{
  unsigned char *bp = *dpp;
  unsigned char byte;
  unsigned shift = 0;
  u64 result = 0;

  while (1)
    {
      byte = *bp++;
      result |= (byte & 0x7f) << shift;
      if ((byte & 0x80) == 0)
	break;
      shift += 7;
    }
  *dpp = bp;
  return result;
}

/*
 * Parse .riscv.attributes elf section to get the various compile time
 * attributes such as -march, unaligned access and so no.
 */
static int rv_parse_elf_attributes(struct file *f, const struct elf_phdr *phdr,
				   struct arch_elf_state *state)
{
	unsigned char buf[RV_ATTR_SEC_SZ];
	unsigned char *p;
	ssize_t n;
	int ret=0;
	loff_t pos;

	pr_info("Section .riscv.attributes found\n");

	/* Assume a reasonable size for now */
	if (phdr->p_filesz > sizeof(buf))
		return -ENOEXEC;

	memset(buf, 0, RV_ATTR_SEC_SZ);
	pos = phdr->p_offset;
	n = kernel_read(f, &buf, phdr->p_filesz, &pos);

	if (n < 0)
		return -EIO;

	p = buf;
	p++;				/* format-version (1B) */

	while ((p - buf) < n) {

		unsigned char *vendor_start;
		u32 len;

		/*
		 * Organized as "vendor" sub-section(s).
		 * Current only 1 specified "riscv"
		 */

		p += 4;			/* sub-section length (4B) */
		while (*p++ != '\0');	/* vendor name string */
		p++;			/* Tag_File (1B) */
		len = *(u32 *)p;	/* data length (4B) */
		p += 4;

		len -= 5;		/* data length includes Tag and self length */
		vendor_start = p;
		while((p - vendor_start) < len) {

			u64 tag = decode_uleb128(&p);
			unsigned char *val_str;
			u64 val_n;

			switch(tag) {
			case RV_ATTR_TAG_stack_align:
				val_n = decode_uleb128(&p);
				break;

			case RV_ATTR_TAG_unaligned_access:
				val_n = decode_uleb128(&p);
				pr_info("Tag_RISCV_unaligned_access =%llu\n", val_n);
				rv_elf_attr_int(tag, val_n);
				break;

			case RV_ATTR_TAG_arch:
				val_str = p;
				while (*p++ != '\0');
				pr_info("Tag_RISCV_arch =[%s]\n", val_str);
				rv_elf_attr_str(tag, val_str);
				break;

			default:
				val_n = decode_uleb128(&p);
				pr_info("skipping Unrecognized Tag [%llu]=%llu\n", tag, val_n);
				break;
			}
		}
	}

	return ret;
}

/*
 * Hook invoked from common elf loader to parse any arch specific elf segments
 */
int arch_elf_pt_proc(void *_ehdr, void *_phdr, struct file *elf,
		     bool is_interp, struct arch_elf_state *state)
{
	struct elf_phdr *phdr = _phdr;
	int ret = 0;

	if (phdr->p_type == PT_RISCV_ATTRIBUTES && !is_interp)
		ret = rv_parse_elf_attributes(elf, phdr, state);

	return ret;
}

int arch_check_elf(void *_ehdr, bool has_interpreter, void *_interp_ehdr,
		   struct arch_elf_state *state)
{
	return 0;
}
