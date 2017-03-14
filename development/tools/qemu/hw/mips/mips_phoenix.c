/*
 * QEMU/MIPS pseudo-board
 *
 * emulates a simple machine with ISA-like bus.
 * ISA IO space mapped to the 0x14000000 (PHYS) and
 * ISA memory at the 0x10000000 (PHYS, 16Mb in size).
 * All peripherial devices are attached to this "bus" with
 * the standard PC ISA addresses.
 */
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "cpu.h"
#include "hw/hw.h"
#include "hw/mips/mips.h"
#include "hw/mips/jz.h"
#include "hw/mips/cpudevs.h"
#include "hw/char/serial.h"
#include "net/net.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/block/flash.h"
#include "qemu/log.h"
#include "hw/mips/bios.h"
#include "hw/ide.h"
#include "hw/loader.h"
#include "elf.h"
#include "sysemu/block-backend.h"
#include "exec/address-spaces.h"
#include "sysemu/qtest.h"
#include "hw/pci/pci.h"
#include "exec/cpu_ldst.h"

#include "x1000_clk.c"
#define X1000_MAXMUN_BIOS_SIZE   (32 * 1024)

static struct _loaderparams {
	int ram_size;
	const char *kernel_filename;
	const char *kernel_cmdline;
	const char *initrd_filename;
} loaderparams;

#define LINUX_ARGS_ADDR  0x80002000
#define LINUX_ARGS_MAX_NUM	16
#define LINUX_ARGS_MAX_SIZE 4096
static int prom_index = 0;
static int prom_offset = 0;

static int GCC_FMT_ATTR(2, 3) prom_set(uint32_t *prom_buf, const char *string, ...)
{
	va_list ap;
	char *arg_buf;
	int max_arg_size, ret;

	if (prom_index > LINUX_ARGS_MAX_NUM ||
			prom_offset > LINUX_ARGS_MAX_SIZE)
		return -EINVAL;

	prom_buf[prom_index] = LINUX_ARGS_ADDR + LINUX_ARGS_MAX_NUM * sizeof(uint32_t) + prom_offset;
	arg_buf = (char *)prom_buf + LINUX_ARGS_MAX_NUM * sizeof(uint32_t) + prom_offset;
	max_arg_size = LINUX_ARGS_MAX_SIZE - prom_offset;

	va_start(ap, string);
	ret = vsnprintf(arg_buf, max_arg_size, string, ap);
	if (ret > 0) {
		prom_offset += ret;
		prom_index++;
		ret = 0;
	}
	va_end(ap);
	return ret;
}

static int64_t load_kernel(void)
{
	int64_t entry, kernel_high;
	long kernel_size, initrd_size;
	ram_addr_t initrd_offset;
	uint32_t *prom_buf;

	kernel_size = load_elf(loaderparams.kernel_filename, cpu_mips_kseg0_to_phys,
			NULL, (uint64_t *)&entry, NULL,
			(uint64_t *)&kernel_high, 0,
			EM_MIPS, 1, 0);
	if (kernel_size >= 0) {
		if ((entry & ~0x7fffffffULL) == 0x80000000)
			entry = (int32_t)entry;
	} else {
		fprintf(stderr, "qemu: could not load kernel '%s'\n",
				loaderparams.kernel_filename);
		exit(1);
	}

	/* load initrd */
	initrd_size = 0;
	initrd_offset = 0;
	if (loaderparams.initrd_filename) {
		initrd_size = get_image_size (loaderparams.initrd_filename);
		if (initrd_size > 0) {
			initrd_offset = (kernel_high + ~INITRD_PAGE_MASK) & INITRD_PAGE_MASK;
			if (initrd_offset + initrd_size > ram_size) {
				fprintf(stderr,
						"qemu: memory too small for initial ram disk '%s'\n",
						loaderparams.initrd_filename);
				exit(1);
			}
			initrd_size = load_image_targphys(loaderparams.initrd_filename,
					initrd_offset,
					ram_size - initrd_offset);
		}
		if (initrd_size == (target_ulong) -1) {
			fprintf(stderr, "qemu: could not load initial ram disk '%s'\n",
					loaderparams.initrd_filename);
			exit(1);
		}
	}

	if (!(*loaderparams.kernel_cmdline))
		return entry;

	/* Store command line.  */
	prom_buf = g_malloc((LINUX_ARGS_MAX_SIZE + LINUX_ARGS_MAX_NUM * sizeof(uint32_t)));
	prom_index = 0;
	prom_offset = 0;
	prom_set(prom_buf, "/0");
	if (initrd_size > 0)
		prom_set(prom_buf, "rd_start=0x%lx rd_size=%li %s",
				(unsigned long)cpu_mips_phys_to_kseg0(NULL, initrd_offset),
				initrd_size, loaderparams.kernel_cmdline);
	else
		prom_set(prom_buf, "%s", loaderparams.kernel_cmdline);

	rom_add_blob_fixed("prom", prom_buf, prom_offset + LINUX_ARGS_MAX_NUM *  sizeof(uint32_t),
			cpu_mips_kseg0_to_phys(NULL, LINUX_ARGS_ADDR));
	g_free(prom_buf);

	return entry;
}

/* Small bootloader */
static void write_small_bootloader (CPUMIPSState *env, uint8_t *base, int64_t kernel_addr)
{
	uint32_t *p = (uint32_t *) base;

	/*init uart*/
	stl_p(p++, 0x3c031003);									    /* lui v1, 0x9003 */
	stl_p(p++, 0x3465200c);									    /* ori a1, v1, 0x200c */
	stl_p(p++, 0x3c020000);									    /* lui v0, 0x0000 */
	stl_p(p++, 0x34420093);									    /* ori v0, v0, 0x3 */
	stl_p(p++, 0xaca20000);									    /* sw  v0, 0(a1) */
	stl_p(p++, 0x34642004);									    /* ori a0, v1, 0x2004 */
	stl_p(p++, 0x2402000d);									    /* li  v0, 13 */
	stl_p(p++, 0xac820000);									    /* sw  v0, 0(a0) */
	stl_p(p++, 0x24020003);									    /* li  v0, 3 */
	stl_p(p++, 0xaca20000);									    /* sw  v0, 0(a1) */
	stl_p(p++, 0x34632008);									    /* ori v1, v1, 0x2008 */
	stl_p(p++, 0x24020011);									    /* li  v0, 0x11 */
	stl_p(p++, 0xac620000);									    /* sw  v0, 0(v1) */

	/*init kernel args*/
	stl_p(p++, 0x3c040000);                                      /* lui a0, 0 */
	stl_p(p++, 0x34840000 | (prom_index & 0xffff));              /* ori a0, a0, prom_index*/
	stl_p(p++, 0x3c050000 | ((LINUX_ARGS_ADDR >> 16) & 0xffff)); /* lui a1, high(LINUX_ARGS_ADDR) */
	stl_p(p++, 0x34a50000 | (LINUX_ARGS_ADDR & 0xffff));         /* ori a1, a1, low(LINUX_ARGS_ADDR) */

	/*jump to kernel entry*/
	stl_p(p++, 0x3c1f0000 | ((kernel_addr >> 16) & 0xffff));     /* lui ra, high(kernel_addr) */;
	stl_p(p++, 0x37ff0000 | (kernel_addr & 0xffff));             /* ori ra, ra, low(kernel_addr) */
	stl_p(p++, 0x03e00008);                                      /* jr ra */
	stl_p(p++, 0x00000000);                                      /* nop */
}

static void network_init (PCIBus *pci_bus)
{
	int i;

	for(i = 0; i < nb_nics; i++) {
		NICInfo *nd = &nd_table[i];
		if (nd->model)
			pci_nic_init_nofail(nd, pci_bus, nd->model, NULL);
	}
}

static void main_cpu_reset(void *opaque)
{
	MIPSCPU *cpu = (MIPSCPU *)opaque;
	cpu_reset(CPU(cpu));
}

static CPUUnassignedAccess real_do_unassigned_access;

static void xburst_do_unassigned_access(CPUState *cpu, hwaddr addr,
		bool is_write, bool is_exec, int opaque,
		unsigned size)
{
	if ((unsigned int)addr < 0x10000000U) {
		if (qemu_loglevel_mask(LOG_GUEST_ERROR)) {
			qemu_log("MAYBE io address 0x%x is_write %d is_exec %d\n",
					(0x10000000U + (unsigned int)addr),
					is_write, is_exec);
		}
		return;
	}

	(*real_do_unassigned_access)(cpu, addr, is_write, is_exec, opaque, size);
}

static bool (*real_mips_has_work)(CPUState *cpu);

static bool xburst_has_work(CPUState *cpu)
{
	if (cpu->halted) {
		MIPSCPU *mips = MIPS_CPU(cpu);
		CPUMIPSState *env = &mips->env;
		if (env->CP0_Cause & (BIT(3) << CP0Ca_IP))
			return true;
	}

	return (*real_mips_has_work)(cpu);
}

static void (*real_do_interrupt)(CPUState *cpu);

static void xburst_do_interrupt(CPUState *cpu)
{
	struct CPUMIPSState* env = cpu->env_ptr;
	if (cpu->exception_index == EXCP_RI) {
		uint32_t ri_code = cpu_ldl_code(env, env->active_tc.PC);
		if (((ri_code >> 16) & 0xffff) == 0x7008) {
			cpu->exception_index = EXCP_NONE;
			if (qemu_loglevel_mask(LOG_GUEST_ERROR)) {
				qemu_log("INGENIC RI CODE: PC "TARGET_FMT_lx" CODE %"PRIx32"\n",
						env->active_tc.PC, ri_code);
			}
			env->active_tc.PC += sizeof(target_ulong);
			return;
		}
	}
	return real_do_interrupt(cpu);
}

static void mips_xburst_phoenix_init(MachineState *machine)
{
	ram_addr_t ram_size = machine->ram_size;
	const char *cpu_model = machine->cpu_model;
	const char *kernel_filename = machine->kernel_filename;
	const char *kernel_cmdline = machine->kernel_cmdline;
	const char *initrd_filename = machine->initrd_filename;
	char *filename;
	MemoryRegion *ram = g_new(MemoryRegion, 1);
	MemoryRegion *ram_high = g_new(MemoryRegion, 1);
	MemoryRegion *bios = g_new(MemoryRegion, 1);
	MemoryRegion *iomem = g_new(MemoryRegion, 1);
	MemoryRegion *tcsm = g_new(MemoryRegion, 1);
	DeviceState *intc;
	uint32_t kernel_entry = 0xbfc00000;
	int bios_size = 0;
	MIPSCPU *cpu;
	CPUClass *cc;
	PCIBus *pci_bus;
	CPUMIPSState *env;
	DriveInfo           *dinfo;
	BlockBackend        *blk = NULL;

	/* init CPUs */
	if (cpu_model == NULL)
		cpu_model = "xburst1-x1000";

	cpu = cpu_mips_init(cpu_model);
	if (cpu == NULL) {
		fprintf(stderr, "Unable to find CPU definition\n");
		exit(1);
	}
	cc = CPU_GET_CLASS(cpu);
	real_do_unassigned_access = cc->do_unassigned_access;
	cc->do_unassigned_access = xburst_do_unassigned_access;
	real_do_interrupt =	cc->do_interrupt;
	cc->do_interrupt = xburst_do_interrupt;
	env = &cpu->env;
	real_mips_has_work = cc->has_work;
	cc->has_work = xburst_has_work;

	qemu_register_reset(main_cpu_reset, cpu);

	/* allocate RAM */
	if (ram_size > (1024 << 20)) {
		fprintf(stderr, "qemu: Too much memory for this machine: %d MB, maximum 1024 MB\n",
				((unsigned int)ram_size / (1 << 20)));
		exit(1);
	}

	memory_region_allocate_system_memory(ram_high, NULL, "RAM", ram_size);
	memory_region_add_subregion(get_system_memory(), 0x20000000, ram_high);

	memory_region_init_alias(ram, NULL, "RAM.L",
			ram_high,
			0x0,
			(ram_size < (256 << 20)) ? ram_size : (256 << 20));
	memory_region_add_subregion(get_system_memory(), 0, ram);

	memory_region_init_io(iomem, NULL, NULL, NULL, "IOMEM", 0x4000000);
	memory_region_add_subregion_overlap(get_system_memory(), 0x10000000, iomem, 0);

	memory_region_init_ram(bios, NULL, "BIOS", X1000_MAXMUN_BIOS_SIZE, &error_fatal);
	vmstate_register_ram_global(bios);
	memory_region_set_readonly(bios, true);
	memory_region_add_subregion(get_system_memory(), 0x1fc00000, bios);
#ifdef TARGET_TCSM_SIZE
	memory_region_init_ram(tcsm, NULL, "TCSM", TARGET_TCSM_SIZE * 1024, &error_fatal);
	vmstate_register_ram_global(tcsm);
	memory_region_add_subregion(get_system_memory(), 0xf4000000, tcsm);
#endif

	if (kernel_filename) {
		loaderparams.ram_size = ram_size;
		loaderparams.kernel_filename = kernel_filename;
		loaderparams.kernel_cmdline = kernel_cmdline;
		loaderparams.initrd_filename = initrd_filename;
		kernel_entry = load_kernel();
	}

	/*load bootrom on rom*/
	filename = qemu_find_file(QEMU_FILE_TYPE_BIOS, bios_name?:BIOS_FILENAME);
	if (filename)
		bios_size = get_image_size(filename);

	g_free(filename);

	if ((bios_size > 0) && (bios_size <= X1000_MAXMUN_BIOS_SIZE))
		load_image_targphys(filename, 0x1fc00000, bios_size);
	else
		write_small_bootloader(env, memory_region_get_ram_ptr(bios), kernel_entry);

	/* Init CPU internal devices */
	cpu_mips_irq_init_cpu(env);
	cpu_mips_clock_init(env);

	intc = create_jz_intc_controller(CPU(cpu), 0x10001000, 2, 0x34, 0x20);
	if (!intc) {
		fprintf(stderr, "qemu: can not found interrupt controller on phoenix\n");
		exit(1);
	}

	create_jz_uart(serial_hds[2], 0x10030000, qdev_get_gpio_in(DEVICE(intc), (19 + 32)), 0, 64, 64);
	create_jz_uart(serial_hds[1], 0x10031000, qdev_get_gpio_in(DEVICE(intc), (18 + 32)), 1, 64, 64);
	create_jz_uart(serial_hds[0], 0x10032000, qdev_get_gpio_in(DEVICE(intc), (17 + 32)), 2, 64, 64);

	dinfo = drive_get(IF_SD, 0, 0);
	if (!dinfo)
		fprintf(stderr, "qemu: missing SecureDigital device\n");
	else
		blk = blk_by_legacy_dinfo(dinfo);
	create_jz_msc(blk, 0, 0x13450000, qdev_get_gpio_in(DEVICE(intc), (5 + 32)), 128);
	create_jz_msc(NULL, 1, 0x13460000, qdev_get_gpio_in(DEVICE(intc), (4 + 32)), 128);

	jz_cpm_init();

	create_jz_ost(0x12000000, cpu->env.irq[3]);

	pci_bus = xburst_vpci_register((qemu_irq *)&(env->irq[4]));

	network_init(pci_bus);
}

static void mips_machine_init(MachineClass *mc)
{
	mc->desc = "mips xburst phoenix platform";
	mc->init = mips_xburst_phoenix_init;
	mc->default_ram_size = (1024 * 1024 * 1024);
}

DEFINE_MACHINE("phoenix", mips_machine_init)
