#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "cpu.h"
#include "hw/hw.h"
#include "hw/sysbus.h"
#include "hw/mips/mips.h"
#include "hw/mips/jz.h"
#include "hw/mips/cpudevs.h"
#include "qemu/error-report.h"
#include "qemu/bitops.h"

typedef struct JZINTCBank {
	uint32_t icsr;
	uint32_t icmr;
	uint32_t icpr;
} JZINTCBank;

typedef struct JZINTCState {
	SysBusDevice parent_obj;
	JZINTCBank *banks;
	uint32_t nbanks;
	uint32_t bankoff;
#define IRQS_IN_BANK (32)
	qemu_irq  parent_irq;
	MemoryRegion mmio;
	uint32_t mmio_size;
} JZINTCState;

#define TYPE_JZ_INTC "jz-intc"
#define JZ_INTC(obj) \
	OBJECT_CHECK(struct JZINTCState, (obj), TYPE_JZ_INTC)

static void jz_update_irq(JZINTCState *jd)
{
	int i, update = 0;


	for (i = 0; i < jd->nbanks; i++) {
		jd->banks[i].icpr = jd->banks[i].icsr &
			(~(jd->banks[i].icmr));

		if (jd->banks[i].icpr)
			update = 1;
	}

	if (update)
		qemu_irq_raise(jd->parent_irq);
	else
		qemu_irq_lower(jd->parent_irq);

	return;
}

static void jz_intc_handler(void *opaque, int n, int level)
{
	JZINTCState *jd = (JZINTCState *)opaque;
	int bank = n / IRQS_IN_BANK;
	int irq = n % IRQS_IN_BANK;

	if (level)
		jd->banks[bank].icsr |= BIT(irq);
	else
		jd->banks[bank].icsr &= ~BIT(irq);

	jz_update_irq(jd);

	return;
}

static uint64_t jz_initc_mmio_read(void *opaque, hwaddr addr, unsigned size)
{

	JZINTCState *jd = (JZINTCState *)opaque;
	int bank = addr/jd->bankoff;

	switch (addr%jd->bankoff) {
	case 0x0:
		return jd->banks[bank].icsr;
	case 0x4:
		return jd->banks[bank].icmr;
	default:
	case 0x8:
	case 0xc:
		return 0xDEADBEEF;
	case 0x10:
		return jd->banks[bank].icpr;
	}

	return 0xDEADBEEF;
}

static void jz_initc_mmio_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
	JZINTCState *jd = (JZINTCState *)opaque;
	int bank = addr/jd->bankoff;

	switch (addr%jd->bankoff) {
	default:
	case 0x0:
	case 0x4:
	case 0x10:
		break;
	case 0x8:
		jd->banks[bank].icmr |= (data & 0xffffffff);
		jz_update_irq(jd);
		break;
	case 0xc:
		jd->banks[bank].icmr &= ~(data & 0xffffffff);
		jz_update_irq(jd);
		break;
	}
	return;
}

static const MemoryRegionOps jz_intc_mmio_ops = {
	.read = jz_initc_mmio_read,
	.write = jz_initc_mmio_write,
	.valid = {
		.min_access_size = 4,
		.max_access_size = 4,
	},
	.endianness = DEVICE_NATIVE_ENDIAN,
};

DeviceState *create_jz_intc_controller(CPUState *cpu, hwaddr addr,
		int banks, int size, int bankoff)
{
	MIPSCPU *mipscpu = MIPS_CPU(cpu);
	DeviceState *dev;
	SysBusDevice *busdev;

	if (!mipscpu)
		exit(-1);

	dev = qdev_create(NULL, "jz-intc");
	if (size)
		qdev_prop_set_int32(dev, "size", size);
	if (banks) {
		qdev_prop_set_int32(dev, "nbanks", banks);
		if (bankoff)
			qdev_prop_set_int32(dev, "bankoff", bankoff);
	}
	qdev_init_nofail(dev);

	busdev = SYS_BUS_DEVICE(dev);
	sysbus_mmio_map(busdev, 0, addr);
	sysbus_connect_irq(busdev, 0, mipscpu->env.irq[2]);
	return dev;
}

static int jz_intc_device_realize(SysBusDevice *dev)
{
	JZINTCState *jd = JZ_INTC(dev);

	sysbus_init_irq(dev, &jd->parent_irq);
	qdev_init_gpio_in(DEVICE(dev), jz_intc_handler, IRQS_IN_BANK * jd->nbanks);
	memory_region_init_io(&jd->mmio, OBJECT(dev), &jz_intc_mmio_ops,
			jd, "Interrupt Controller",
			jd->mmio_size);
	sysbus_init_mmio(dev, &jd->mmio);
	return 0;
}

static void jz_intc_reset_device(DeviceState *dev)
{
	JZINTCState *jd = JZ_INTC(dev);
	int i;

	jd->banks = g_malloc_n(jd->nbanks, sizeof(JZINTCBank));
	if (!jd->banks)
		exit(1);

	for (i = 0; i < jd->nbanks; i++) {
		jd->banks[i].icsr = 0x0;
		jd->banks[i].icmr = 0xFFFFFFFF;
		jd->banks[i].icpr = 0x0;
	}

	qemu_set_irq(jd->parent_irq, 0);
	return;
}

static Property jz_intc_properties[] = {
	DEFINE_PROP_UINT32("size", JZINTCState, mmio_size, 0x34),
	DEFINE_PROP_UINT32("bankoff", JZINTCState, bankoff, 0x20),
	DEFINE_PROP_UINT32("nbanks", JZINTCState, nbanks, 2),
	DEFINE_PROP_END_OF_LIST(),
};

static void jz_intc_class_init(ObjectClass *klass, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(klass);
	SysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(klass);

	k->init = jz_intc_device_realize;
	dc->reset = jz_intc_reset_device;
	dc->props = jz_intc_properties;
}

static struct TypeInfo jz_intc_info = {
	.name = TYPE_JZ_INTC,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(JZINTCState),
	.class_init = jz_intc_class_init,
};

static void jz_intc_register_types(void)
{
	type_register_static(&jz_intc_info);
}

type_init(jz_intc_register_types);
