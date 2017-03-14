/*
 * Ingenic X1000 ost
 *
 * Copyright (c) 2000 - 2013 Ingenic Electronics Co., Ltd.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qemu/timer.h"
#include "qemu/main-loop.h"
#include "qemu/compiler.h"
#include "qemu-common.h"
#include "hw/mips/jz.h"
#include "hw/ptimer.h"

#define X1000_OST_DEBUG  0

#define MONITOR_TIMES		(10)

#define DPRINT(fmt, args...)              \
	do {                                  \
		if (X1000_OST_DEBUG)              \
		fprintf(stderr, "X1000 OST: "fmt, ## args); \
	} while (0)
#define X1000_OSTCCR	(0x00)
#define X1000_OSTCCR_PRESCALE2_MSK	(0x3 << 2)
#define X1000_OSTCCR_PRESCALE2_1	(0x0 << 2)
#define X1000_OSTCCR_PRESCALE2_4	(0x1 << 2)
#define X1000_OSTCCR_PRESCALE2_16	(0x2 <<2)
#define X1000_OSTCCR_PRESCALE1_MSK	(0x3 << 0)
#define X1000_OSTCCR_PRESCALE1_1	(0x0 << 0)
#define X1000_OSTCCR_PRESCALE1_4	(0x1 << 0)
#define X1000_OSTCCR_PRESCALE1_16	(0x2 << 0)

#define X1000_OSTER	(0x04)
#define X1000_OSTESR (0x34)
#define X1000_OSTECR (0x38)
#define X1000_OSTER_OST2EN	BIT(1)
#define X1000_OSTER_OST1EN	BIT(0)

#define X1000_OSTCR	(0x08)
#define X1000_OSTCR_OST2CLR	BIT(1)
#define X1000_OSTCR_OST1CLR	BIT(0)

#define X1000_OST1FR (0x0c)
#define X1000_OST1FR_FFLAG	BIT(0)

#define X1000_OST1MR (0x10)
#define X1000_OST1MR_FMASK	BIT(0)

#define X1000_OST1DFR (0x14)
#define X1000_OST1CNT (0x18)
#define X1000_OST2CNTH (0x1c)
#define X1000_OST2CNTL (0x20)
#define X1000_OST2CNTHBUF (0x24)


#define jz_get_extclk()  (24*1000*1000)

typedef struct X1000OstState {
	SysBusDevice	parent_obj;
	MemoryRegion	iomem;

	/*register*/
	uint32_t	ostccr;
	uint32_t	oster;
	uint32_t	ostcr;
	uint32_t	ost1dfr;
	uint32_t	ost1fr;
	uint32_t	ost1mr;
	uint32_t	ost2cnthbuf;

	/*32bit event timer*/
	qemu_irq		irq;
	ptimer_state	*ptimer_state;

	/*64bit source timer*/
	int64_t		ost64_period;
	uint32_t	ost64_period_frac;
	uint64_t	ost64base_cnt;
	uint64_t	ost64base_ns;
	bool ost64enabled;
} X1000OstState;

#define TYPE_X1000_OST	"x1000-ost"
#define X1000_OST_DEVICE(obj)	\
	OBJECT_CHECK(X1000OstState, (obj), TYPE_X1000_OST);

static void x1000_ost_update_irq(X1000OstState *ost)
{
	if (ost->ost1fr & (~(ost->ost1mr)) & ost->oster)
		qemu_irq_raise(ost->irq);
	else
		qemu_irq_lower(ost->irq);
}

static void x1000_ost_event_bh_evt(void *opaque)
{
	X1000OstState *ost = opaque;

	ost->ost1fr |= X1000_OST1FR_FFLAG;

	DPRINT("x1000 ost interrupt %"PRId64" ns\n",  qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL));
	ptimer_set_count(ost->ptimer_state, ost->ost1dfr);

	x1000_ost_update_irq(ost);

	if (ost->oster & X1000_OSTER_OST1EN)
		ptimer_run(ost->ptimer_state, 1);
	else
		ptimer_stop(ost->ptimer_state);
}

static uint64_t caculate_ost64_count(X1000OstState *ost)
{
	int64_t now_ns = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
	uint64_t delta_ns = now_ns - ost->ost64base_ns;
	uint64_t div = ost->ost64_period;
	int clz1, clz2;
	int shift;

	/*just like ptimer_get_count*/
	DPRINT("ost64_period %"PRId64", ost64_period_frac %"PRIu32"\n", ost->ost64_period, ost->ost64_period_frac);
	DPRINT("now %"PRId64" ns, base %"PRId64" ns, delta_ns %"PRId64" ns\n", now_ns, ost->ost64base_ns, delta_ns);
	if (now_ns < ost->ost64base_ns)
		return 0;

	clz1 = clz64(delta_ns);
	clz2 = clz64(div);
	shift = clz1 < clz2 ? clz1 : clz2;

	delta_ns <<= shift;
	div <<= shift;
	DPRINT("shift = %d, delta_ns %"PRIu64", div %"PRIu64"\n", shift, delta_ns, div);

	if (shift >= 32) {
		div |= ((uint64_t)ost->ost64_period_frac << (shift - 32));
	} else {
		if (shift != 0)
			div |= (ost->ost64_period_frac >> (32 - shift));
		/* Look at remaining bits of period_frac and round div up if
		   necessary.  */
		if ((uint32_t)(ost->ost64_period_frac << shift))
			div += 1;
	}
	DPRINT("delta_ns %"PRIu64", div %"PRIu64", result %"PRIu64"\n", delta_ns, div, delta_ns / div);

	return delta_ns / div;
}

static uint64_t x1000_ost_read(void *opaque, hwaddr addr, unsigned size)
{
	X1000OstState *ost = opaque;
	uint64_t tmp_cnt;

	DPRINT("read addr=0x%" HWADDR_PRIx "\n", addr);
	switch (addr & 0xff) {
	case X1000_OSTCCR:
		return ost->ostccr;
		break;
	case X1000_OSTER:
		return ost->oster;
	case X1000_OST1FR:
		return ost->ost1fr;
	case X1000_OST1MR:
		return ost->ost1mr;
	case X1000_OST1DFR:
		return ost->ost1dfr;
	case X1000_OST1CNT:
		return ost->ost1dfr - ptimer_get_count(ost->ptimer_state);
	case X1000_OST2CNTH:
		if (ost->ost64enabled)
			return caculate_ost64_count(ost) >> 32;
		else
			return ost->ost64base_cnt >> 32;
	case X1000_OST2CNTL:
		if (ost->ost64enabled)
			tmp_cnt = caculate_ost64_count(ost);
		else
			tmp_cnt = ost->ost64base_cnt;
		ost->ost2cnthbuf = tmp_cnt >> 32;
		return (tmp_cnt & 0xffffffff);
	case X1000_OST2CNTHBUF:
		return ost->ost2cnthbuf;
	case X1000_OSTESR:
	case X1000_OSTECR:
	case X1000_OSTCR:
	default:
		break;
	}
	return 0;
}

static void x1000_ost_write(void *opaque, hwaddr addr, uint64_t data,
		unsigned size)
{
	X1000OstState *ost = opaque;

	DPRINT("write addr=0x%" HWADDR_PRIx " val=0x%" PRIx64 "\n", addr, data);
	switch (addr & 0xff) {
	case X1000_OSTCCR:
		switch (data & X1000_OSTCCR_PRESCALE2_MSK) {
		default:
		case X1000_OSTCCR_PRESCALE2_1:
			ost->ost64_period = 1000000000ll / ((jz_get_extclk() / MONITOR_TIMES));
			ost->ost64_period_frac = (uint32_t)(((1000000000ll << 32) / (jz_get_extclk() / MONITOR_TIMES)) & 0xffffffffll);
			break;
		case X1000_OSTCCR_PRESCALE2_4:
			ost->ost64_period = 1000000000ll / (jz_get_extclk() / 4 / MONITOR_TIMES);
			ost->ost64_period_frac = (uint32_t)((1000000000ll << 32) / (jz_get_extclk() / 4 / MONITOR_TIMES) & 0xffffffffll);
			break;
		case X1000_OSTCCR_PRESCALE2_16:
			ost->ost64_period = 1000000000ll / (jz_get_extclk() / 16 / MONITOR_TIMES);
			ost->ost64_period_frac = (uint32_t)((1000000000ll << 32) / (jz_get_extclk() / 16 / MONITOR_TIMES) & 0xffffffffll);
			break;
		}
		switch (data & X1000_OSTCCR_PRESCALE1_MSK) {
		default:
		case X1000_OSTCCR_PRESCALE1_1:
			ptimer_set_freq(ost->ptimer_state, (jz_get_extclk()/MONITOR_TIMES));
			break;
		case X1000_OSTCCR_PRESCALE1_4:
			ptimer_set_freq(ost->ptimer_state, jz_get_extclk()/4/MONITOR_TIMES);
			break;
		case X1000_OSTCCR_PRESCALE1_16:
			ptimer_set_freq(ost->ptimer_state, jz_get_extclk()/16/MONITOR_TIMES);
			break;
		}
		ost->ostccr = data & (X1000_OSTCCR_PRESCALE1_MSK |
				X1000_OSTCCR_PRESCALE2_MSK);
		break;
	case X1000_OSTER:
		break;
	case X1000_OSTESR:
		if ((data & X1000_OSTER_OST2EN) && !ost->ost64enabled) {
			ost->ost64enabled = true;
			ost->ost64base_ns = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
		}

		if (data & X1000_OSTER_OST1EN)
			ptimer_run(ost->ptimer_state, 0);

		ost->oster |= data & (X1000_OSTER_OST2EN | X1000_OSTER_OST1EN);
		x1000_ost_update_irq(ost);
		break;
	case X1000_OSTECR:
		if ((data & X1000_OSTER_OST2EN) && ost->ost64enabled) {
			ost->ost64enabled = false;
			ost->ost64base_cnt = caculate_ost64_count(ost);
		}

		if (data & X1000_OSTER_OST1EN)
			ptimer_stop(ost->ptimer_state);

		ost->oster &= ~(data & (X1000_OSTER_OST2EN | X1000_OSTER_OST1EN));
		x1000_ost_update_irq(ost);
		break;
	case X1000_OSTCR:
		if (data & X1000_OSTCR_OST2CLR) {
			ost->ost64base_cnt = 0;
			if (ost->ost64enabled)
				ost->ost64base_ns = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
		}
		if (data & X1000_OSTCR_OST1CLR)
			ptimer_set_count(ost->ptimer_state, ost->ost1dfr);
		break;
	case X1000_OST1FR:
		ost->ost1fr = 0;
		x1000_ost_update_irq(ost);
		break;
	case X1000_OST1MR:
		if (data & X1000_OST1MR_FMASK)
			ost->ost1mr = X1000_OST1MR_FMASK;
		else
			ost->ost1mr = 0;
		x1000_ost_update_irq(ost);
		break;
	case X1000_OST1DFR:
		ptimer_set_count(ost->ptimer_state, (data + 1));
		ost->ost1dfr = (data + 1);
		break;
	case X1000_OST1CNT:
		ptimer_set_count(ost->ptimer_state,
				(ost->ost1dfr - data) > 0 ? (ost->ost1dfr - data) : 0);
		break;
	case X1000_OST2CNTH:
		ost->ost64base_cnt = (data << 32) | (ost->ost64base_cnt & 0xffffffff);
		if (ost->ost64enabled)
			ost->ost64base_ns = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
		break;
	case X1000_OST2CNTL:
		ost->ost64base_cnt = (data) | (ost->ost64base_cnt & (0xffffffffull << 32));
		if (ost->ost64enabled)
			ost->ost64base_ns = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
		break;
	case X1000_OST2CNTHBUF:
		break;
	default:
		break;
	}
}

static const MemoryRegionOps x1000_ost_ops = {
	.valid = {
		.min_access_size = sizeof(uint32_t),
		.max_access_size = sizeof(uint32_t),	//??
	},
	.read = x1000_ost_read,
	.write = x1000_ost_write,
	.endianness = DEVICE_LITTLE_ENDIAN,
};

void create_jz_ost(hwaddr base, qemu_irq irq)
{
	struct DeviceState *dev = qdev_create(NULL, TYPE_X1000_OST);

	qdev_init_nofail(dev);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, base);
	sysbus_connect_irq(SYS_BUS_DEVICE(dev), 0, irq);
}

static void x1000_ost_reset(DeviceState *dev)
{
	X1000OstState *ost = X1000_OST_DEVICE(dev);

	ost->ostccr = 0x0;
	ost->oster = 0x0;
	ost->ostcr = 0x0;
	ost->ost1dfr = ~0;
	ost->ost1fr = 0x0;
	ost->ost1mr = 0x1;

	ptimer_stop(ost->ptimer_state);
	ptimer_set_limit(ost->ptimer_state, 0xffffffff, 1);
	ptimer_set_freq(ost->ptimer_state, jz_get_extclk()/MONITOR_TIMES);

	ost->ost64_period_frac = (uint32_t)(((1000000000ll << 32) / (jz_get_extclk()/MONITOR_TIMES)) & 0xffffffffll);
	ost->ost64_period = 1000000000LL / (jz_get_extclk()/MONITOR_TIMES);
	ost->ost64base_cnt = 0;
	ost->ost64base_ns = 0;
	ost->ost64enabled = false;
}

static void x1000_ost_instance_init(Object *obj)
{
	X1000OstState *ost = X1000_OST_DEVICE(obj);
	QEMUBH *bh;

	bh = qemu_bh_new(x1000_ost_event_bh_evt, ost);
	ost->ptimer_state = ptimer_init(bh);

	x1000_ost_reset(DEVICE(obj));

	memory_region_init_io(&ost->iomem, obj, &x1000_ost_ops, ost, "x1000-ost", 0x3a);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &ost->iomem);
	sysbus_init_irq(SYS_BUS_DEVICE(obj), &ost->irq);
}

static void x1000_ost_class_init(ObjectClass *klass, void *data)
{
	DeviceClass	*dc = DEVICE_CLASS(klass);
	dc->reset = x1000_ost_reset;
}

static void x1000_ost_init(void)
{
	const TypeInfo x1000_ost_type = {
		.name = TYPE_X1000_OST,
		.parent = TYPE_SYS_BUS_DEVICE,
		.instance_size = sizeof(X1000OstState),
		.instance_init = x1000_ost_instance_init,
		.class_init = x1000_ost_class_init,
	};
	type_register_static(&x1000_ost_type);
}
type_init(x1000_ost_init);
