/*
 * Xburst vir pci bus
 *
 * Baseon xburst_vpci.c
 */

#include "qemu/osdep.h"

#include "hw/hw.h"
#include "hw/pci/pci.h"
#include "hw/i386/pc.h"
#include "hw/mips/mips.h"
#include "hw/mips/jz.h"
#include "hw/pci/pci_host.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"

//#define DEBUG_XBURST

#ifdef DEBUG_XBURST
#define DPRINTF(fmt, ...) fprintf(stderr, "%s: " fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define DPRINTF(fmt, ...)
#endif

#define XBURST_PCILO_BASE		0x14000000

#define XBURST_PCICFG_BASE      0x1fe80000
#define XBURST_PCICFG_SIZE      0x00080000
#define XBURST_PCICFG_TOP       (XBURST_PCICFG_BASE+XBURST_PCICFG_SIZE-1)

#define XBURST_REG_BASE			0x1fe00000
#define XBURST_REGBASE          0x100

#define XBURST_INTERNAL_REG_BASE  (XBURST_REGBASE+XBURST_REG_BASE)
#define XBURST_INTERNAL_REG_SIZE  (0x70)

#define XBURST_SPCICONFIG_BASE  (XBURST_PCICFG_BASE)
#define XBURST_SPCICONFIG_SIZE  (XBURST_PCICFG_SIZE)


/* 1. Xburst h/w Configuration */
/* Power on register */
#define XBURST_BONGENCFG        (0x04 >> 2)   /*0x104 */

/* PCI address map control */
#define XBURST_PCIMAP_CFG       (0x18 >> 2)      /* 0x118 */

/* ICU Enable Regs - IntEn & IntISR are r/o. */
#define XBURST_INTENSET         (0x30 >> 2)      /* 0x130 */
#define XBURST_INTENCLR         (0x34 >> 2)      /* 0x134 */
#define XBURST_INTEN            (0x38 >> 2)      /* 0x138 */
#define XBURST_INTISR           (0x3c >> 2)      /* 0x13c */

#define XBURST_REGS             (0x70 >> 2)

/* PCI config for south bridge. type 0 */
#define XBURST_PCICONF_IDSEL_MASK      0xfffff800     /* [31:11] */
#define XBURST_PCICONF_IDSEL_OFFSET    11
#define XBURST_PCICONF_FUN_MASK        0x700    /* [10:8] */
#define XBURST_PCICONF_FUN_OFFSET      8
#define XBURST_PCICONF_REG_MASK        0xFC
#define XBURST_PCICONF_REG_OFFSET      0

/* PCI config for south bridge. type 1 */
#define XBURST_PCICONF_DEVNO_MASK	   0xf800		/* [15:11] */
#define XBURST_PCICONF_DEVNO_OFFSET	   11
#define XBURST_PCICONF_BUSNO_MASK	   0xff0000		/* [16:23]*/
#define XBURST_PCICONF_BUSNO_OFFSET	   16

#define PCI_ADDR(busno,devno,funno,regno)  \
	((((busno)<<16)&0xff0000) + (((devno)<<11)&0xf800) + (((funno)<<8)&0x700) + (regno))

typedef struct XburstState XburstState;

typedef struct PCIXburstState
{
	PCIDevice dev;
	XburstState *pcihost;
	uint32_t regs[XBURST_REGS];
	/* Xburst registers */
	MemoryRegion iomem;
} PCIXburstState;

struct XburstState {
	PCIHostState parent_obj;
	qemu_irq *pic;
	PCIXburstState *pci_dev;
};

#define TYPE_XBURST_PCI_HOST_BRIDGE "xburst-vpcihost"
#define XBURST_PCI_HOST_BRIDGE(obj) \
	OBJECT_CHECK(XburstState, (obj), TYPE_XBURST_PCI_HOST_BRIDGE)

#define TYPE_PCI_XBURST "xburst-vpci"
#define PCI_XBURST(obj) \
	OBJECT_CHECK(PCIXburstState, (obj), TYPE_PCI_XBURST)

static void pci_xburst_vpci_update_irq(struct XburstState *s)
{
	PCIXburstState *xburst_vpci_state = s->pci_dev;
	qemu_irq *pic = s->pic;

	if (xburst_vpci_state->regs[XBURST_INTISR] & xburst_vpci_state->regs[XBURST_INTEN])
		qemu_irq_raise(*pic);
	else
		qemu_irq_lower(*pic);
}

static void xburst_vpci_writel(void *opaque, hwaddr addr,
		uint64_t val, unsigned size)
{
	PCIXburstState *s = opaque;
	uint32_t saddr;
	int reset = 0;

	saddr = addr >> 2;

	DPRINTF("xburst_vpci_writel "TARGET_FMT_plx" val %"PRIx64" saddr %"PRIx32"\n", addr, val, saddr);
	switch (saddr) {
	case XBURST_PCIMAP_CFG:
		s->regs[saddr] = val;
		break;
	case XBURST_BONGENCFG:
		if (!(s->regs[saddr] & 0x04) && (val & 0x04)) {
			reset = 1; /* bit 2 jump from 0 to 1 cause reset */
		}
		s->regs[saddr] = val;
		if (reset) {
			qemu_system_reset_request();
		}
		break;
	case XBURST_INTENSET:
		s->regs[XBURST_INTENSET] = val;
		s->regs[XBURST_INTEN] |= val;
		pci_xburst_vpci_update_irq(s->pcihost);
		break;
	case XBURST_INTENCLR:
		s->regs[XBURST_INTENCLR] = val;
		s->regs[XBURST_INTEN] &= ~val;
		pci_xburst_vpci_update_irq(s->pcihost);
		break;
	case XBURST_INTEN:
	case XBURST_INTISR:
		DPRINTF("write to readonly xburst_vpci register %"PRIx32"\n", saddr);
		break;
	default:
		DPRINTF("write to unknown xburst_vpci register %"PRIx32"\n", saddr);
		break;
	}
}

static uint64_t xburst_vpci_readl(void *opaque, hwaddr addr,
		unsigned size)
{
	PCIXburstState *s = opaque;
	uint32_t saddr;

	saddr = addr >> 2;
	DPRINTF("xburst_vpci_readl "TARGET_FMT_plx"\n", addr);
	return s->regs[saddr];
}

static const MemoryRegionOps xburst_vpci_ops = {
	.read = xburst_vpci_readl,
	.write = xburst_vpci_writel,
	.endianness = DEVICE_NATIVE_ENDIAN,
	.valid = {
		.min_access_size = 4,
		.max_access_size = 4,
	},
};

static uint32_t xburst_vpci_sbridge_pciaddr(void *opaque, hwaddr addr)
{
	PCIXburstState *s = opaque;
	PCIHostState *phb = PCI_HOST_BRIDGE(s->pcihost);
	uint32_t cfgaddr;
	uint32_t busno;
	uint32_t devno;
	uint32_t funno;
	uint32_t regno;
	uint32_t pciaddr;

	/* support type0 pci config */
	if ((s->regs[XBURST_PCIMAP_CFG] & 0x10000) != 0x0) {
		cfgaddr = addr & 0xfffc;
		cfgaddr = (s->regs[XBURST_PCIMAP_CFG] &  0xffff) << 16;
		cfgaddr |= 0x1;
		devno = (cfgaddr & XBURST_PCICONF_DEVNO_MASK) >> XBURST_PCICONF_DEVNO_OFFSET;
		busno = (cfgaddr & XBURST_PCICONF_BUSNO_MASK) >> XBURST_PCICONF_BUSNO_OFFSET;
	} else {
		uint32_t idsel;
		cfgaddr = addr & 0xfffc;
		cfgaddr |= (s->regs[XBURST_PCIMAP_CFG] & 0xffff) << 16;
		idsel = (cfgaddr & XBURST_PCICONF_IDSEL_MASK) >> XBURST_PCICONF_IDSEL_OFFSET;
		if (idsel == 0) {
			fprintf(stderr, "error in xburst_vpci pci config address " TARGET_FMT_plx
					",pcimap_cfg=%x\n", addr, s->regs[XBURST_PCIMAP_CFG]);
			exit(1);
		}
		devno = ctz32(idsel);
		busno = pci_bus_num(phb->bus);
	}

	funno = (cfgaddr & XBURST_PCICONF_FUN_MASK) >> XBURST_PCICONF_FUN_OFFSET;
	regno = (cfgaddr & XBURST_PCICONF_REG_MASK) >> XBURST_PCICONF_REG_OFFSET;
	pciaddr = PCI_ADDR(busno, devno, funno, regno);

	DPRINTF("cfgaddr %x pciaddr %x busno %x devno %d funno %d regno %d\n",
			cfgaddr, pciaddr, busno, devno, funno, regno);

	return pciaddr;
}

static void xburst_vpci_spciconf_writeb(void *opaque, hwaddr addr,
		uint32_t val)
{
	PCIXburstState *s = opaque;
	PCIDevice *d = PCI_DEVICE(s);
	PCIHostState *phb = PCI_HOST_BRIDGE(s->pcihost);
	uint32_t pciaddr;
	uint16_t status;

	DPRINTF("xburst_vpci_spciconf_writeb "TARGET_FMT_plx" val %x\n", addr, val);
	pciaddr = xburst_vpci_sbridge_pciaddr(s, addr);

	if (pciaddr == 0xffffffff) {
		return;
	}

	/* set the pci address in s->config_reg */
	phb->config_reg = (pciaddr) | (1u << 31);
	pci_data_write(phb->bus, phb->config_reg, val & 0xff, 1);

	/* clear PCI_STATUS_REC_MASTER_ABORT and PCI_STATUS_REC_TARGET_ABORT */
	status = pci_get_word(d->config + PCI_STATUS);
	status &= ~(PCI_STATUS_REC_MASTER_ABORT | PCI_STATUS_REC_TARGET_ABORT);
	pci_set_word(d->config + PCI_STATUS, status);
}

static void xburst_vpci_spciconf_writew(void *opaque, hwaddr addr,
		uint32_t val)
{
	PCIXburstState *s = opaque;
	PCIDevice *d = PCI_DEVICE(s);
	PCIHostState *phb = PCI_HOST_BRIDGE(s->pcihost);
	uint32_t pciaddr;
	uint16_t status;

	DPRINTF("xburst_vpci_spciconf_writew "TARGET_FMT_plx" val %x\n", addr, val);
	assert((addr & 0x1) == 0);

	pciaddr = xburst_vpci_sbridge_pciaddr(s, addr);

	if (pciaddr == 0xffffffff) {
		return;
	}

	/* set the pci address in s->config_reg */
	phb->config_reg = (pciaddr) | (1u << 31);
	pci_data_write(phb->bus, phb->config_reg, val, 2);

	/* clear PCI_STATUS_REC_MASTER_ABORT and PCI_STATUS_REC_TARGET_ABORT */
	status = pci_get_word(d->config + PCI_STATUS);
	status &= ~(PCI_STATUS_REC_MASTER_ABORT | PCI_STATUS_REC_TARGET_ABORT);
	pci_set_word(d->config + PCI_STATUS, status);
}

static void xburst_vpci_spciconf_writel(void *opaque, hwaddr addr,
		uint32_t val)
{
	PCIXburstState *s = opaque;
	PCIDevice *d = PCI_DEVICE(s);
	PCIHostState *phb = PCI_HOST_BRIDGE(s->pcihost);
	uint32_t pciaddr;
	uint16_t status;

	DPRINTF("xburst_vpci_spciconf_writel "TARGET_FMT_plx" val %x\n", addr, val);
	assert((addr & 0x3) == 0);

	pciaddr = xburst_vpci_sbridge_pciaddr(s, addr);

	if (pciaddr == 0xffffffff) {
		return;
	}

	/* set the pci address in s->config_reg */
	phb->config_reg = (pciaddr) | (1u << 31);
	pci_data_write(phb->bus, phb->config_reg, val, 4);

	/* clear PCI_STATUS_REC_MASTER_ABORT and PCI_STATUS_REC_TARGET_ABORT */
	status = pci_get_word(d->config + PCI_STATUS);
	status &= ~(PCI_STATUS_REC_MASTER_ABORT | PCI_STATUS_REC_TARGET_ABORT);
	pci_set_word(d->config + PCI_STATUS, status);
}

static uint32_t xburst_vpci_spciconf_readb(void *opaque, hwaddr addr)
{
	PCIXburstState *s = opaque;
	PCIDevice *d = PCI_DEVICE(s);
	PCIHostState *phb = PCI_HOST_BRIDGE(s->pcihost);
	uint32_t pciaddr;
	uint16_t status;

	DPRINTF("xburst_vpci_spciconf_readb "TARGET_FMT_plx"\n", addr);
	pciaddr = xburst_vpci_sbridge_pciaddr(s, addr);

	if (pciaddr == 0xffffffff) {
		return 0xff;
	}

	/* set the pci address in s->config_reg */
	phb->config_reg = (pciaddr) | (1u << 31);

	/* clear PCI_STATUS_REC_MASTER_ABORT and PCI_STATUS_REC_TARGET_ABORT */
	status = pci_get_word(d->config + PCI_STATUS);
	status &= ~(PCI_STATUS_REC_MASTER_ABORT | PCI_STATUS_REC_TARGET_ABORT);
	pci_set_word(d->config + PCI_STATUS, status);

	return pci_data_read(phb->bus, phb->config_reg, 1);
}

static uint32_t xburst_vpci_spciconf_readw(void *opaque, hwaddr addr)
{
	PCIXburstState *s = opaque;
	PCIDevice *d = PCI_DEVICE(s);
	PCIHostState *phb = PCI_HOST_BRIDGE(s->pcihost);
	uint32_t pciaddr;
	uint16_t status;

	DPRINTF("xburst_vpci_spciconf_readw "TARGET_FMT_plx"\n", addr);
	assert((addr & 0x1) == 0);

	pciaddr = xburst_vpci_sbridge_pciaddr(s, addr);

	if (pciaddr == 0xffffffff) {
		return 0xffff;
	}

	/* set the pci address in s->config_reg */
	phb->config_reg = (pciaddr) | (1u << 31);

	/* clear PCI_STATUS_REC_MASTER_ABORT and PCI_STATUS_REC_TARGET_ABORT */
	status = pci_get_word(d->config + PCI_STATUS);
	status &= ~(PCI_STATUS_REC_MASTER_ABORT | PCI_STATUS_REC_TARGET_ABORT);
	pci_set_word(d->config + PCI_STATUS, status);

	return pci_data_read(phb->bus, phb->config_reg, 2);
}

static uint32_t xburst_vpci_spciconf_readl(void *opaque, hwaddr addr)
{
	PCIXburstState *s = opaque;
	PCIDevice *d = PCI_DEVICE(s);
	PCIHostState *phb = PCI_HOST_BRIDGE(s->pcihost);
	uint32_t pciaddr;
	uint16_t status;

	DPRINTF("xburst_vpci_spciconf_readl "TARGET_FMT_plx"\n", addr);
	assert((addr & 0x3) == 0);

	pciaddr = xburst_vpci_sbridge_pciaddr(s, addr);

	if (pciaddr == 0xffffffff) {
		return 0xffffffff;
	}

	/* set the pci address in s->config_reg */
	phb->config_reg = (pciaddr) | (1u << 31);

	/* clear PCI_STATUS_REC_MASTER_ABORT and PCI_STATUS_REC_TARGET_ABORT */
	status = pci_get_word(d->config + PCI_STATUS);
	status &= ~(PCI_STATUS_REC_MASTER_ABORT | PCI_STATUS_REC_TARGET_ABORT);
	pci_set_word(d->config + PCI_STATUS, status);

	return pci_data_read(phb->bus, phb->config_reg, 4);
}

/* bridge PCI configure space. 0x1fe8 0000 - 0x1fef ffff */
static const MemoryRegionOps xburst_vpci_spciconf_ops = {
	.old_mmio = {
		.read = {
			xburst_vpci_spciconf_readb,
			xburst_vpci_spciconf_readw,
			xburst_vpci_spciconf_readl,
		},
		.write = {
			xburst_vpci_spciconf_writeb,
			xburst_vpci_spciconf_writew,
			xburst_vpci_spciconf_writel,
		},
	},
	.endianness = DEVICE_NATIVE_ENDIAN,
};

static void pci_xburst_vpci_set_irq(void *opaque, int irq_num, int level)
{
	XburstState *s = opaque;
	PCIXburstState *xburst_vpci_state = s->pci_dev;

	assert((irq_num < 32 && irq_num >= 0));

	if (level)
		xburst_vpci_state->regs[XBURST_INTISR] |= (1 << irq_num);
	else
		xburst_vpci_state->regs[XBURST_INTISR] &= ~(1 << irq_num);

	pci_xburst_vpci_update_irq(s);
}

static int pci_xburst_vpci_map_irq(PCIDevice *pci_dev, int irq_num)
{
	int slot = (pci_dev->devfn >> 3) & 0x1f;
	int irq;

	if (irq_num > 3)
		irq_num = 0;

	irq = (irq_num + slot + 4 * pci_bus_num(pci_dev->bus)) % 32;

	return irq;
}

static void xburst_vpci_reset(void *opaque)
{
	PCIXburstState *s = opaque;
	/* set the default value of registers */
	s->regs[XBURST_BONGENCFG] = 0x1384;
}

static const VMStateDescription vmstate_xburst_vpci = {
	.name = "Xburst",
	.version_id = 1,
	.minimum_version_id = 1,
	.fields = (VMStateField[]) {
		VMSTATE_PCI_DEVICE(dev, PCIXburstState),
		VMSTATE_END_OF_LIST()
	}
};

static int xburst_vpci_pcihost_initfn(SysBusDevice *dev)
{
	PCIHostState *phb = PCI_HOST_BRIDGE(dev);

	phb->bus = pci_register_bus(DEVICE(dev), "pci",
			pci_xburst_vpci_set_irq, pci_xburst_vpci_map_irq, dev,
			get_system_memory(), get_system_io(),
			PCI_DEVFN(1, 0), 32, TYPE_PCI_BUS);
	return 0;
}

static void xburst_vpci_realize(PCIDevice *dev, Error **errp)
{
	PCIXburstState *s = PCI_XBURST(dev);
	SysBusDevice *sysbus = SYS_BUS_DEVICE(s->pcihost);
	PCIHostState *phb = PCI_HOST_BRIDGE(s->pcihost);

	/* Xburst Bridge, built on FPGA, VENDOR_ID/DEVICE_ID are "undefined" */
	pci_config_set_prog_interface(dev->config, 0x00);

	/* set the north bridge register mapping */
	memory_region_init_io(&s->iomem, OBJECT(s), &xburst_vpci_ops, s,
			"pcic-register", XBURST_INTERNAL_REG_SIZE);
	sysbus_init_mmio(sysbus, &s->iomem);
	sysbus_mmio_map(sysbus, 0, XBURST_INTERNAL_REG_BASE);

	/* set the pci configure  mapping */
	memory_region_init_io(&phb->data_mem, OBJECT(s), &xburst_vpci_spciconf_ops, s,
			"pci-config", XBURST_SPCICONFIG_SIZE);
	sysbus_init_mmio(sysbus, &phb->data_mem);
	sysbus_mmio_map(sysbus, 1, XBURST_SPCICONFIG_BASE);

	/* set the default value of xburst_vpci pci config */
	pci_set_word(dev->config + PCI_COMMAND, 0x0000);
	pci_set_word(dev->config + PCI_STATUS, 0x0000);
	pci_set_word(dev->config + PCI_SUBSYSTEM_VENDOR_ID, 0x0000);
	pci_set_word(dev->config + PCI_SUBSYSTEM_ID, 0x0000);

	pci_set_byte(dev->config + PCI_INTERRUPT_LINE, 0x00);
	pci_set_byte(dev->config + PCI_INTERRUPT_PIN, 0x01);
	pci_set_byte(dev->config + PCI_MIN_GNT, 0x3c);
	pci_set_byte(dev->config + PCI_MAX_LAT, 0x00);

	qemu_register_reset(xburst_vpci_reset, s);
}

PCIBus *xburst_vpci_register(qemu_irq *pic)
{
	DeviceState *dev;
	XburstState *pcihost;
	PCIHostState *phb;
	PCIXburstState *s;
	PCIDevice *d;

	dev = qdev_create(NULL, TYPE_XBURST_PCI_HOST_BRIDGE);
	phb = PCI_HOST_BRIDGE(dev);
	pcihost = XBURST_PCI_HOST_BRIDGE(dev);
	pcihost->pic = pic;
	qdev_init_nofail(dev);

	/* set the pcihost pointer before xburst_vpci_initfn is called */
	d = pci_create(phb->bus, PCI_DEVFN(0, 0), TYPE_PCI_XBURST);
	s = PCI_XBURST(d);
	s->pcihost = pcihost;
	pcihost->pci_dev = s;
	qdev_init_nofail(DEVICE(d));

	return phb->bus;
}

static void xburst_vpci_class_init(ObjectClass *klass, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(klass);
	PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);

	k->realize = xburst_vpci_realize;
	k->vendor_id = 0xdf53;
	k->device_id = 0x00d5;
	k->revision = 0x01;
	k->class_id = PCI_CLASS_BRIDGE_HOST;
	dc->desc = "Host bridge";
	dc->vmsd = &vmstate_xburst_vpci;
	/*
	 * PCI-facing part of the host bridge, not usable without the
	 * host-facing part, which can't be device_add'ed, yet.
	 */
	dc->cannot_instantiate_with_device_add_yet = true;
}

static const TypeInfo xburst_vpci_info = {
	.name          = TYPE_PCI_XBURST,
	.parent        = TYPE_PCI_DEVICE,
	.instance_size = sizeof(PCIXburstState),
	.class_init    = xburst_vpci_class_init,
};

static void xburst_vpci_pcihost_class_init(ObjectClass *klass, void *data)
{
	SysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(klass);

	k->init = xburst_vpci_pcihost_initfn;
}

static const TypeInfo xburst_vpci_pcihost_info = {
	.name          = TYPE_XBURST_PCI_HOST_BRIDGE,
	.parent        = TYPE_PCI_HOST_BRIDGE,
	.instance_size = sizeof(XburstState),
	.class_init    = xburst_vpci_pcihost_class_init,
};

static void xburst_vpci_register_types(void)
{
	type_register_static(&xburst_vpci_pcihost_info);
	type_register_static(&xburst_vpci_info);
}

type_init(xburst_vpci_register_types)
