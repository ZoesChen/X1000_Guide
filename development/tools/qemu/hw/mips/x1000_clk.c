/*
 * QEMU/x1000 pseudo-clk
 *
 * emulates a simple machine with ISA-like bus.
 * ISA IO space mapped to the 0x14000000 (PHYS) and
 * ISA memory at the 0x10000000 (PHYS, 16Mb in size).
 * All peripherial devices are attached to this "bus" with
 * the standard PC ISA addresses.
 */

#include "qemu/bitops.h"
#include "qom/cpu.h"

#define CPM_CPCCR   (0x00)
#define CPM_CPCSR   (0xd4)

#define CPM_DDRCDR  (0x2c)
#define CPM_I2SCDR  (0x60)
#define CPM_I2SCDR1 (0x70)
#define CPM_LPCDR   (0x64)
#define CPM_MSC0CDR (0x68)
#define CPM_MSC1CDR (0xa4)
#define CPM_USBCDR  (0x50)
#define CPM_MACCDR  (0x54)
#define CPM_SFCCDR  (0x74)
#define CPM_CIMCDR  (0x7c)
#define CPM_PCMCDR  (0x84)
#define CPM_PCMCDR1 (0xe0)
#define CPM_MPHYC   (0xe8)

#define CPM_INTR    (0xb0)
#define CPM_INTRE   (0xb4)
#define CPM_DRCG    (0xd0)
#define CPM_CPSPPR  (0x38)
#define CPM_CPPSR   (0x34)

#define CPM_USBPCR  (0x3c)
#define CPM_USBRDT  (0x40)
#define CPM_USBVBFIL    (0x44)
#define CPM_USBPCR1 (0x48)

#define CPM_CPAPCR  (0x10)
#define CPM_CPMPCR  (0x14)

#define CPM_LCR     (0x04)
#define CPM_PSWC0ST     (0x90)
#define CPM_PSWC1ST     (0x94)
#define CPM_PSWC2ST     (0x98)
#define CPM_PSWC3ST     (0x9c)
#define CPM_CLKGR   (0x20)
#define CPM_MESTSEL (0xec)
#define CPM_SRBC    (0xc4)
#define CPM_ERNG    (0xd8)
#define CPM_RNG     (0xdc)
#define CPM_SLBC    (0xc8)
#define CPM_SLPC    (0xcc)
#define CPM_OPCR    (0x24)
#define CPM_RSR     (0x08)

#define CPM_REG_SIZE (0x100)
#define CPM_REG_IDX(x) ((x)/sizeof(uint32_t))

static void jz_cpm_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
	uint32_t *reg = opaque;
	int index = CPM_REG_IDX(addr);

	if (size != sizeof(uint32_t) || addr%sizeof(uint32_t))
		return;

	switch (addr) {
	case CPM_CPCSR:
	case CPM_INTR:
	case CPM_INTRE:
		break;
	case CPM_DDRCDR:
		reg[index] = data & (~(BIT(28) | BIT(24)));
		break;
	case CPM_MACCDR:
	case CPM_MSC0CDR:
	case CPM_MSC1CDR:
	case CPM_USBCDR:
	case CPM_SFCCDR:
	case CPM_CIMCDR:
		reg[index] = data & (~BIT(28));
		break;
	case CPM_I2SCDR:
	case CPM_PCMCDR:
		reg[index] = data & (~(BIT(31) | BIT(30)));
		if (data & BIT(31))
			reg[index] |= BIT(30);
		if (data & BIT(30))
			reg[index] |= BIT(31);
		break;
	case CPM_LPCDR:
		reg[index] = data & (~BIT(27));
		break;
	case CPM_MPHYC:
		reg[index] = data & (~(BIT(29) | BIT(30)));
		break;
	case CPM_USBPCR1:
		reg[index] = data & (~BIT(31));
		break;
	case CPM_CPAPCR:
		reg[index] = data & (~(BIT(10) | BIT(15)));
		break;
	case CPM_CPMPCR:
		reg[index] = data & (~BIT(1));
		break;
	case CPM_CPPSR:
		if (reg[CPM_REG_IDX(CPM_CPSPPR)] == 0x5a5a)
			reg[index] = data;
		break;
	default:
		reg[index] = data;
		break;
	}
}

static uint64_t jz_cpm_read(void *opaque, hwaddr addr, unsigned size)
{
	uint32_t *reg = opaque;
	int index = CPM_REG_IDX(addr);

	if (size != sizeof(uint32_t) || addr%sizeof(uint32_t))
		return 0;

	switch (addr) {
	case CPM_CPCSR:
		reg[index] = (0x1f << 27);
		break;
	case CPM_RNG:
		if (reg[CPM_REG_IDX(CPM_ERNG)] & BIT(0)) {
			reg[index] = 0xfffff;
			reg[CPM_REG_IDX(CPM_ERNG)] &= ~BIT(31);			//FIXME
		}
		break;
	}
	return (uint64_t)reg[index];
}

static const MemoryRegionOps jz_cpm_ops = {
	.read = jz_cpm_read,
	.write = jz_cpm_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

static void jz_cpm_reset(void *opaque)
{
	uint32_t *reg = opaque;
	int i = 0;

	for (i = 0; i < CPM_REG_IDX(CPM_REG_SIZE); i++) {
		if (CPM_REG_IDX(CPM_CPPSR) == i)
			continue;
		reg[i] = 0;
	}

	reg[CPM_REG_IDX(CPM_DDRCDR)] = BIT(30);
	reg[CPM_REG_IDX(CPM_DRCG)] = BIT(0);
	reg[CPM_REG_IDX(CPM_USBPCR)] = 0x409919b8;
	reg[CPM_REG_IDX(CPM_CPSPPR)] = 0xa5a5;
	reg[CPM_REG_IDX(CPM_USBRDT)] = 0x2000096;
	reg[CPM_REG_IDX(CPM_USBVBFIL)] = 0xff0080;
	reg[CPM_REG_IDX(CPM_USBPCR1)] = 0x8d000000;
	reg[CPM_REG_IDX(CPM_CPAPCR)] = 0xa7000501;
	reg[CPM_REG_IDX(CPM_CPMPCR)] = 0xa7000081;
	reg[CPM_REG_IDX(CPM_CLKGR)] = 0x07FFFF10;
	reg[CPM_REG_IDX(CPM_OPCR)] = 0x00801500;

	return;
}

static void jz_cpm_init(void)
{
	MemoryRegion *cpm_iomem = g_new(MemoryRegion, 1);

	uint32_t* reg = g_new0(uint32_t, (CPM_REG_SIZE/sizeof(uint32_t)));

	memory_region_init_io(cpm_iomem, NULL, &jz_cpm_ops, reg, "IOMEM", CPM_REG_SIZE);
	memory_region_add_subregion(get_system_memory(), 0x10000000, cpm_iomem);
	qemu_register_reset(jz_cpm_reset, reg);
}
