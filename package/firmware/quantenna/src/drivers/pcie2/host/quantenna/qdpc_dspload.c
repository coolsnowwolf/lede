#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <asm/io.h>
#include <qtn/registers.h>
#include <qtn/shared_params.h>
#include <qtn/topaz_fwt_sw.h>
#include "qdpc_dspload.h"

static inline unsigned long
qdpc_dsp_to_host_addr(unsigned long dsp_addr)
{
        void *ret = bus_to_virt(dsp_addr);
        if (RUBY_BAD_VIRT_ADDR == ret) {
                panic("Converting out of range DSP address 0x%lx to host address\n", dsp_addr);
        }
        return virt_to_phys(ret);
}

static char* qdpc_dsp_read(struct file *file, int buflen)
{
	char *p = NULL;

	if (!file)
		return NULL;

	p = kmalloc(buflen*sizeof(unsigned char), GFP_ATOMIC);
	if (p)
		file->f_op->read(file, p, buflen, &file->f_pos);

	return p;
}

static int qdpc_dsp_install_firmware(struct file *file, u32 *dsp_start_addr)
{
        Elf32_Ehdr *ehdr;
        Elf32_Phdr *phdr, *phdr_o;
        u8* vaddr;
        int i, buflen;
	char *pdata = NULL;
	int e_phnum = 0;

	buflen = sizeof(Elf32_Ehdr);
	pdata = qdpc_dsp_read(file, buflen);
	if (!pdata) {
		return -1;
	}
        ehdr = (Elf32_Ehdr *)pdata;
	e_phnum = ehdr->e_phnum;
	kfree(pdata);

	buflen = e_phnum * sizeof(Elf32_Phdr);
	pdata = qdpc_dsp_read(file, buflen);
	if (!pdata) {
		return -1;
	}
	phdr = (Elf32_Phdr *)pdata;
	phdr_o = (Elf32_Phdr *)pdata;

        for(i = 0; i < e_phnum; i++, phdr++)
        {
		pdata = qdpc_dsp_read(file, phdr->p_filesz);
		if (!pdata) {
			return -1;
		}

                /* Skip blocks for DSP X/Y memory */
                if ((phdr->p_vaddr >= RUBY_DSP_XYMEM_BEGIN) && (phdr->p_vaddr <= RUBY_DSP_XYMEM_END)) {
                        if (pdata)
				kfree(pdata);
			continue;
                }
                unsigned long p_muc = qdpc_dsp_to_host_addr(phdr->p_vaddr);
                        printk("p_vaddr in ELF header is %p, "
                                "remapping to 0x%lx\n", (void *)phdr->p_vaddr, p_muc);
                /* Copy segment to right location */
                vaddr = ioremap_nocache(p_muc, phdr->p_memsz);

                /* Copy data */
                memcpy(vaddr, pdata, phdr->p_filesz);
                /* Clear BSS */
                memset(vaddr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);

                iounmap(vaddr);
		if (pdata)
			kfree(pdata);
        }

	if (phdr_o)
		kfree(phdr_o);

        *dsp_start_addr = ehdr->e_entry;

        return(0);
}

static void hal_dsp_start(u32 dsp_start_addr)
{
#ifdef CONFIG_ARCH_ARC
        /* Check that we can start this address */
        if (dsp_start_addr & ((1 << RUBY_SYS_CTL_DSP_REMAP_SHIFT) - 1)) {
                panic("DSP address 0x%x cannot be used as entry point\n", (unsigned)dsp_start_addr);
        }
        /* Tells DSP from which address start execution */
        writel(RUBY_SYS_CTL_DSP_REMAP_VAL(dsp_start_addr), RUBY_SYS_CTL_DSP_REMAP);
#else
        /* Swap upper and lower half words for DSP instruction */
        dsp_start_addr = ((dsp_start_addr >> 16) & 0xFFFF) | (dsp_start_addr << 16);

        /* Push the jump instr and location into the mbx */
        *(volatile u32*)IO_ADDRESS(UMS_REGS_MB + UMS_MBX_DSP_PUSH)
                = DSP_JUMP_INSTR_SWAP;
        *(volatile u32*)IO_ADDRESS(UMS_REGS_MB + UMS_MBX_DSP_PUSH)
                = dsp_start_addr;
#endif
}

void hal_enable_dsp(void)
{
#ifdef CONFIG_ARCH_ARC
        const unsigned long reset = RUBY_SYS_CTL_RESET_DSP_ALL;

        qtn_txbf_lhost_init();

        writel(reset, RUBY_SYS_CTL_CPU_VEC_MASK);
        writel(reset, RUBY_SYS_CTL_CPU_VEC);
        writel(0, RUBY_SYS_CTL_CPU_VEC_MASK);
#else
        /* Bring the DSP out of reset */
        *(volatile u32 *)IO_ADDRESS(SYS_RESET_VECTOR_MASK) = DSP_RESET;
        *(volatile u32 *)IO_ADDRESS(SYS_RESET_VECTOR) = DSP_RESET;
#endif
}

int qdpc_dsp_open(void)
{
	struct	file *file = NULL;
	mm_segment_t fs;
	u32 dsp_start_addr = 0;

	file = filp_open(QDCP_DSP_FILE_NAME, O_RDONLY, 0);
	if(IS_ERR(file)) {
		printk("error occured while opening file %s, exiting...\n", QDCP_DSP_FILE_NAME);
		return -1;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	qdpc_dsp_install_firmware(file, &dsp_start_addr);

        hal_dsp_start(dsp_start_addr);
        hal_enable_dsp();

	filp_close(file, NULL);
	set_fs(fs);

	return 0;
}
