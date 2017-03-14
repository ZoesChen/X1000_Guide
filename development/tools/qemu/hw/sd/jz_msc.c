/*
 * OMAP on-chip MMC/SD host emulation.
 *
 * Copyright (C) 2006-2007 Andrzej Zaborowski  <balrog@zabor.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 or
 * (at your option) version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/hw.h"
#include "hw/mips/jz.h"
#include "hw/sd/sd.h"
#include "hw/sysbus.h"

#define JZ_MSC_DEBUG 0
#define DPRINT(fmt, args...)              \
	do {                                  \
		if (JZ_MSC_DEBUG)              \
		fprintf(stderr, "JZ MSC: "fmt, ## args); \
	} while (0)

#define MSC_CTRL	/*W		0x0000		16*/0x00
#define MSC_CTRL_EXIT_MULTIPLE	BIT(7)
#define MSC_CTRL_RESET		BIT(3)
#define MSC_CTRL_START_OP	BIT(2)
#define MSC_CTRL_CLK_MSK	0x3
#define MSC_CTRL_CLK_START	0x2
#define MSC_CTRL_CLK_STOP	0x1
#define MSC_STAT	/*R		0x??000040  32*/0x04
#define MSC_STAT_AUTO_CMD12_DONE BIT(31)
#define MSC_STAT_PRG_DONE		BIT(13)
#define MSC_STAT_DATA_TRAN_DONE	BIT(12)
#define MSC_STAT_END_CMD_RES	BIT(11)
#define MSC_STAT_DATA_FIFO_EMPTY	BIT(6)
#define MSC_STAT_TIME_OUT_RES	BIT(1)
#define MSC_CLKRT	/*RW	0x0000		16*/0x08
#define MSC_CMDAT	/*RW	0x00005000	32*/0x0C
#define MSC_CMDAT_AUTO_CMD12	BIT(16)
#define msc_autocmd12(obj)	\
	(!!(((JZ_MscState *)obj)->msc_cmdat & MSC_CMDAT_AUTO_CMD12))
#define MSC_CMDAT_BUSY			BIT(6)
#define MSC_CMDAT_WRITE_READ	BIT(4)
#define msc_data_dir_read(obj)	\
	(!(((JZ_MscState *)obj)->msc_cmdat & MSC_CMDAT_WRITE_READ))
#define MSC_CMDAT_DATA_EN		BIT(3)
#define msc_cmd_has_data_trans(obj)	\
	(!!(((JZ_MscState *)obj)->msc_cmdat & MSC_CMDAT_DATA_EN))
#define MSC_CMDAT_RESPONSE_FORMAT_MSK	0x7
#define MSC_CMDAT_NO_RESPONSE			0x0
#define MSC_CMDAT_RESPONSE_R1_RB1		0x1
#define MSC_CMDAT_RESPONSE_R2			0x2
#define MSC_CMDAT_RESPONSE_R3			0x3
#define MSC_CMDAT_RESPONSE_R4			0x4
#define MSC_CMDAT_RESPONSE_R5			0x5
#define MSC_CMDAT_RESPONSE_R6			0x6
#define MSC_CMDAT_RESPONSE_R7			0x7
#define MSC_RESTO	/*RW	0x0100		16*/0x10
#define MSC_RDTO	/*RW	0x00FFFFFF	32*/0x14
#define MSC_BLKLEN	/*RW	0x0000		16*/0x18
#define MSC_NOB		/*RW	0x0000		16*/0x1C
#define MSC_SNOB	/*R		0x????		16*/0x20
#define MSC_IMASK	/*RW	0xFFFFFFFF	32*/0x24
#define MSC_IFLG	/*RW	0x2000		32*/0x28
#define MSC_IFLAG_AUTO_CMD12_DONE BIT(15)
#define MSC_IFLAG_TIME_OUT_RES	BIT(9)
#define MSC_IFLAG_END_CMD_RES	BIT(2)
#define MSC_IFLAG_PRG_DONE	BIT(1)
#define MSC_IFLAG_DATA_TRAN_DONE	BIT(0)
#define MSC_CMD		/*RW	0x00		8 */0x2C
#define MSC_ARG		/*RW	0x00000000	32*/0x30
#define MSC_RES		/*R		0x????		16*/0x34
#define MSC_RXFIFO	/*R		0x????????	32*/0x38
#define MSC_TXFIFO	/*W		0x????????	32*/0x3C
#define MSC_LPM		/*RW	0x00000000	32*/0x40
#define MSC_LPM_LPM		BIT(0)
#define MSC_LPM_ZERO	(~((BIT(29) - 1) - BIT(0)))
#define MSC_DMAC	/*RW	0x00000000	32*/0x44
#define MSC_DMANDA	/*RW	0x00000000	32*/0x48
#define MSC_DMADA	/*R		0x00000000	32*/0x4C
#define MSC_DMALEN	/*R		0x00000000	32*/0x50
#define MSC_DMACMD	/*R		0x00000000	32*/0x54
#define MSC_CTRL2	/*RW	0x00800000	32*/0x58
#define MSC_RTCNT	/*R		0x00000000	32*/0x5C

#define JZ_MSC_RSP_LEN (8 + 1)
typedef struct JZ_MscState {
	SysBusDevice  parent_obj;
	qemu_irq irq;
	MemoryRegion iomem;
	SDBus sdbus;

	uint32_t rsp_pos;
	uint16_t rsp[JZ_MSC_RSP_LEN];

	uint32_t *rxfifo;
	uint32_t rxf_pos;
	uint32_t rxf_len;
	uint32_t rxf_depth;

	uint32_t blklen;
	uint32_t blkcnt;

	uint32_t *txfifo;
	uint32_t txf_depth;
	uint32_t txf_pos;
	uint32_t txf_len;

	bool is_appcmd;
	bool is_prog;
	bool is_tran_done;

	qemu_irq inserted;
	qemu_irq readonly;

	int32_t id;

	/*register*/
	uint32_t msc_ctrl;
	uint32_t msc_stat;
	uint32_t msc_clkrt;
	uint32_t msc_cmdat;
	uint32_t msc_resto;
	uint32_t msc_rdto;
	uint32_t msc_blklen;
	uint32_t msc_nob;
	uint32_t msc_snob;
	uint32_t msc_imask;
	uint32_t msc_iflg;
	uint32_t msc_cmd;
	uint32_t msc_arg;
	uint32_t msc_lpm;
	uint32_t msc_dmac;
	uint32_t msc_dmanda;
	uint32_t msc_dmada;
	uint32_t msc_dmalen;
	uint32_t msc_dmacmd;
	uint32_t msc_ctrl2;
	uint32_t msc_rtcnt;

	bool clock_en;
	bool trans_exiting;
	bool trans_exited;
	/*private*/
	void *blk;
} JZ_MscState;

#define TYPE_JZ_MSC_DEVICE	"jz-msc"
#define TYPE_JZ_MSC_BUS	"jz-msc-bus"
#define JZ_MSC(obj)	\
	OBJECT_CHECK(JZ_MscState, (obj), TYPE_JZ_MSC_DEVICE)

int create_jz_msc(BlockBackend *blk, int id, hwaddr addr, qemu_irq irq, uint32_t depth)
{
	DeviceState *ds = qdev_create(NULL, TYPE_JZ_MSC_DEVICE);

	qdev_prop_set_int32(ds, "id", id);
	qdev_prop_set_ptr(ds, "blk", blk);
	if (depth != 0) qdev_prop_set_uint32(ds, "rfifo-depth", depth);
	if (depth != 0) qdev_prop_set_uint32(ds, "tfifo-depth", depth);
	qdev_init_nofail(ds);

	sysbus_connect_irq(SYS_BUS_DEVICE(ds), 0, irq);

	sysbus_mmio_map(SYS_BUS_DEVICE(ds), 0, addr);

	return 0;
}

static void jz_msc_reset(DeviceState *dev)
{
	JZ_MscState *js = JZ_MSC(dev);

	js->msc_ctrl	= 0x0000;
	js->msc_stat	= 0x000040;
	js->msc_clkrt	= 0x0000;
	js->msc_cmdat	= 0x00005000;
	js->msc_resto	= 0x0100;
	js->msc_rdto	= 0x00FFFFFF;
	js->msc_blklen	= 0x0000;
	js->msc_nob		= 0x0000;
	js->msc_imask	= 0xFFFFFFFF;
	js->msc_iflg	= 0x2000;
	js->msc_cmd		= 0x00;
	js->msc_arg		= 0x00000000;
	js->msc_lpm		= 0x00000000;
	js->msc_dmac	= 0x00000000;
	js->msc_dmanda	= 0x00000000;
	js->msc_dmada	= 0x00000000;
	js->msc_dmalen	= 0x00000000;
	js->msc_dmacmd	= 0x00000000;
	js->msc_ctrl2	= 0x00800000;
	js->msc_rtcnt	= 0x00000000;

	js->clock_en = false;
	js->trans_exiting = false;
	js->trans_exited = false;
	js->is_appcmd = false;
	js->is_prog = false;
	js->rxf_pos = 0;
	js->rxf_len = 0;
	js->txf_pos = 0;
	js->rxf_len = 0;
}

static void jz_msc_do_command(JZ_MscState *js, uint8_t cmd, uint32_t arg, bool autocmd)
{
	SDRequest req;
	uint8_t respone[16];
	int rsplen, i, j;
	int timeout = 0;

	/*reset fifo*/
	if (msc_cmd_has_data_trans(js) && !autocmd) {
		js->msc_snob = 0;
		js->trans_exiting = false;
		js->trans_exited = false;
		if (msc_data_dir_read(js)) {
			js->rxf_pos = 0;
			js->rxf_len = 0;
		} else {
			js->txf_pos = 0;
			js->txf_len = 0;
		}
	}

	/*reset msc_stat*/
	js->msc_stat &= ~(MSC_STAT_END_CMD_RES | MSC_STAT_TIME_OUT_RES | MSC_STAT_AUTO_CMD12_DONE);
	js->rsp_pos = 0;

	req.cmd = cmd;
	req.arg = arg;
	req.crc = 0;	/*FIXME*/

	rsplen = sdbus_do_command(&js->sdbus, &req, respone);

	switch (js->msc_cmdat & MSC_CMDAT_RESPONSE_FORMAT_MSK) {
	case MSC_CMDAT_RESPONSE_R1_RB1:
	case MSC_CMDAT_RESPONSE_R3:
	case MSC_CMDAT_RESPONSE_R4:
	case MSC_CMDAT_RESPONSE_R5:
	case MSC_CMDAT_RESPONSE_R6:
	case MSC_CMDAT_RESPONSE_R7:
		if (rsplen < 4) {
			timeout = 1;
			break;
		}
		rsplen = 4;
		break;
	case MSC_CMDAT_RESPONSE_R2:
		if (rsplen < 16) {
			timeout = 1;
			break;
		}
		rsplen = 16;
		break;
	case MSC_CMDAT_NO_RESPONSE:
	default:
		rsplen = 0;
		break;
	}

	/*for (i =0; i < rsplen ; i++)
	  printf("ooo %d %x\n", i, respone[i]);*/
	if (rsplen) {
		for (i = 0; i < JZ_MSC_RSP_LEN; i++)
			js->rsp[i] = 0;
		js->rsp[0] = respone[0];
		for (i = 1, j = 1; i < (rsplen - 1); i += 2, j++) {
			js->rsp[j] = (uint16_t)(respone[i] << 8);
			js->rsp[j] |= respone[i + 1];
		}
		if (rsplen == 16)
			js->rsp[j] = (uint16_t)(respone[i]) << 8;
		else
			js->rsp[j] = (uint16_t)(respone[i]);
	}

	/*for (i = 0; i < 9; i++)
	  printf("xxx %d %x\n", i, js->rsp[i]);*/

	if (timeout) {
		js->msc_stat |= MSC_STAT_TIME_OUT_RES;
		js->msc_iflg |= MSC_IFLAG_TIME_OUT_RES;
		js->is_appcmd = false;
	} else {
		js->msc_stat &= ~MSC_STAT_TIME_OUT_RES;
		js->msc_iflg |= MSC_IFLAG_END_CMD_RES;
		js->msc_stat |= MSC_STAT_END_CMD_RES;

		if (req.cmd == 12 && !js->is_appcmd) {	/*CMD12*/
			if (js->msc_cmdat & MSC_CMDAT_AUTO_CMD12) {
				js->msc_stat |= MSC_STAT_AUTO_CMD12_DONE;
				js->msc_iflg |= MSC_IFLAG_AUTO_CMD12_DONE;
				js->msc_cmdat &= ~MSC_CMDAT_AUTO_CMD12;
			}
			if (js->is_prog) {
				js->msc_stat |= MSC_STAT_PRG_DONE;
				js->msc_iflg |= MSC_IFLAG_PRG_DONE;
			}
		} else {
			js->is_prog = false;
			js->msc_stat &= ~MSC_STAT_DATA_TRAN_DONE;
			js->msc_stat &= ~MSC_STAT_PRG_DONE;
		}

		if (req.cmd == 55 && !js->is_appcmd)	/*	CMD55*/
			js->is_appcmd = true;
		else
			js->is_appcmd = false;
	}

	return;
}

static void jz_msc_do_transfer(JZ_MscState *js)
{
	uint32_t data8, data32 = 0;
	int i;

	if (!msc_cmd_has_data_trans(js))
		return;

	if (js->msc_snob >= js->msc_nob || js->trans_exited)
		return;

	if (msc_data_dir_read(js)) {
		for (;js->rxf_len < js->rxf_depth && js->blklen; js->rxf_len++) {
			data32 = 0;
			for (i = 0; i < sizeof(uint32_t)/sizeof(uint8_t) && js->blklen;
					i++) {
				data8 = sdbus_read_data(&js->sdbus);
				data32 |=  (data8 << (i * 8));
				js->blklen--;
			}
			js->rxfifo[(js->rxf_pos + js->rxf_len)%js->rxf_depth] = data32;
			js->msc_stat &= ~MSC_STAT_DATA_FIFO_EMPTY;
			if (!js->blklen) {
				js->msc_snob++;
				if (js->msc_nob > js->msc_snob) {
					js->blklen = js->msc_blklen;
				}
			}
		}
	} else {
		for (;js->txf_len; js->txf_len--) {
			data32 = js->txfifo[js->txf_pos];
			for (i = 0; i < sizeof(uint32_t)/sizeof(uint8_t) && js->blklen;
					i++) {
				data8 = (uint8_t)(data32 & 0xff);
				sdbus_write_data(&js->sdbus, data8);
				data32 = data32 >> 8;
				js->blklen--;
			}
			if ((++js->txf_pos) >= js->txf_depth)
				js->txf_pos = 0;
			if (!js->blklen) {
				js->msc_snob++;
				if (js->msc_nob > js->msc_snob)
					js->blklen = js->msc_blklen;
			}
		}
	}

	if ((js->msc_snob == js->msc_nob) || js->trans_exiting) {
		js->trans_exiting = false;
		js->trans_exited = true;
		js->msc_stat |= MSC_STAT_DATA_TRAN_DONE;
		js->msc_iflg |= MSC_IFLAG_DATA_TRAN_DONE;
		if (!msc_data_dir_read(js))
			js->is_prog = true;
		js->is_tran_done = true;
		if (msc_autocmd12(js))
			jz_msc_do_command(js, 12, 0, true);
	}
	return;
}

static void jz_msc_update_irq(JZ_MscState *js)
{
	if ((js->msc_iflg & !js->msc_imask) && js->clock_en)
		qemu_irq_raise(js->irq);
	else
		qemu_irq_lower(js->irq);
}

static uint64_t jz_msc_read(void *opaque, hwaddr addr, unsigned size)
{
	JZ_MscState *js = opaque;

	//DPRINT("read addr=0x%" HWADDR_PRIx " \n", addr);
	switch (addr) {
	case MSC_STAT	:
		return js->msc_stat;
	case MSC_CLKRT	:
		return js->msc_clkrt;
	case MSC_CMDAT	:
		return js->msc_cmdat;
	case MSC_RESTO	:
		return js->msc_resto;
	case MSC_RDTO	:
		return js->msc_rdto;
	case MSC_BLKLEN	:
		return js->msc_blklen;
	case MSC_NOB	:
		return js->msc_nob;
	case MSC_SNOB	:
		return js->msc_snob;
	case MSC_IMASK	:
		return js->msc_imask;
	case MSC_IFLG	:
		return js->msc_iflg;
	case MSC_CMD	:
		return js->msc_cmd;
	case MSC_ARG	:
		return js->msc_arg;
	case MSC_LPM	:
		return js->msc_lpm;
	case MSC_DMAC	:
		return js->msc_dmac;
	case MSC_DMANDA	:
		return js->msc_dmanda;
	case MSC_DMADA	:
		return js->msc_dmada;
	case MSC_DMALEN	:
		return js->msc_dmalen;
	case MSC_DMACMD	:
		return js->msc_dmacmd;
	case MSC_CTRL2	:
		return js->msc_ctrl2;
	case MSC_RTCNT	:		/*fixme*/
	case MSC_RES	:
		if (js->rsp_pos >= JZ_MSC_RSP_LEN)
			js->rsp_pos = 0;
		return js->rsp[js->rsp_pos++];
	case MSC_RXFIFO	:
		{
			uint32_t data32 = 0;
			if (js->rxf_len) {
				data32 = js->rxfifo[js->rxf_pos];
				if (++js->rxf_pos >= js->rxf_depth)
					js->rxf_pos = 0;
				js->rxf_len--;
			}
			jz_msc_do_transfer(js);
			if (!js->rxf_len)
				js->msc_stat |= MSC_STAT_DATA_FIFO_EMPTY;
			jz_msc_update_irq(js);
			return data32;
		}
		break;
	default:
		break;
	}
	return 0;
}

static void jz_msc_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
	JZ_MscState *js = opaque;

	DPRINT("write addr=0x%" HWADDR_PRIx " val=0x%" PRIx64 "\n", addr, data);
	switch (addr) {
	case MSC_CLKRT	:
		js->msc_clkrt = data;
		break;
	case MSC_RESTO	:
		js->msc_resto = data;
		break;
	case MSC_RDTO	:
		js->msc_rdto = data;
		break;
	case MSC_BLKLEN	:
		js->msc_blklen = data;
		break;
	case MSC_NOB	:
		js->msc_nob = data;
		break;
	case MSC_CMD	:
		js->msc_cmd = data;
		break;
	case MSC_ARG	:
		js->msc_arg = data;
		break;
	case MSC_LPM	:
		if (data & MSC_LPM_LPM)
			js->clock_en = true;
		else
			js->clock_en = false;

		js->msc_lpm = (data & MSC_LPM_ZERO);
		break;
	case MSC_CTRL2	:
		js->msc_ctrl2 = data;
		break;
		/*case MSC_DMAC	:break;
		  case MSC_DMANDA	:break;*/
	case MSC_CTRL	:
		if (data & MSC_CTRL_RESET)
			jz_msc_reset(DEVICE(js));

		if ((data & MSC_CTRL_CLK_MSK) && (!(js->msc_lpm & MSC_LPM_LPM))) {
			if ((data & MSC_CTRL_CLK_MSK) == MSC_CTRL_CLK_START)
				js->clock_en = true;
			if ((data & MSC_CTRL_CLK_MSK) == MSC_CTRL_CLK_STOP)
				js->clock_en = false;
		}

		if (data & MSC_CTRL_EXIT_MULTIPLE)
			js->trans_exiting = true;

		if ((data & MSC_CTRL_START_OP) && js->clock_en) {
			jz_msc_do_command(js, (uint8_t)(js->msc_cmd & 0x3f), js->msc_arg, false);
			if (msc_cmd_has_data_trans(js)) {
				js->blklen = js->msc_blklen;
			}
			jz_msc_do_transfer(js);
			jz_msc_update_irq(js);
		}
		break;
	case MSC_CMDAT	:
		js->msc_cmdat = data;
		break;
	case MSC_IMASK	:
		break;
	case MSC_IFLG	:
		break;
	case MSC_TXFIFO	:
		js->txfifo[(js->txf_pos + js->txf_len)%js->txf_depth] = data;
		js->txf_len++;
		jz_msc_do_transfer(js);
		jz_msc_update_irq(js);
		break;
	default:
		break;
	}
}

static const MemoryRegionOps jz_msc_ops = {
	.read = jz_msc_read,
	.write = jz_msc_write,
	.endianness = DEVICE_LITTLE_ENDIAN,
	.valid = {
		.min_access_size = sizeof(uint8_t),
		.max_access_size = sizeof(uint32_t),
		.unaligned = false,
	},
};

static int jz_msc_realize(SysBusDevice *dev)
{
	JZ_MscState *js = JZ_MSC(dev);
	DeviceState *card = NULL;
	char *name = NULL;
	static int32_t id = 0;
	Error *err = NULL;

	if (js->id >= 0)
		name = g_strdup_printf("msc%i", js->id);
	else
		name = g_strdup_printf("msc%i", id++);

	memory_region_init_io(&js->iomem, OBJECT(dev), &jz_msc_ops, js, name, 0x1000);

	sysbus_init_mmio(dev, &js->iomem);

	sysbus_init_irq(dev, &js->irq);

	qbus_create_inplace(&js->sdbus, sizeof(SDBus), TYPE_JZ_MSC_BUS, DEVICE(dev), name);

	js->rxfifo = qemu_memalign(sizeof(uint32_t), js->rxf_depth * sizeof(uint32_t));
	js->txfifo = qemu_memalign(sizeof(uint32_t), js->txf_depth * sizeof(uint32_t));

	card = qdev_create(qdev_get_child_bus(DEVICE(dev), name), TYPE_SD_CARD);
	if (js->blk != NULL) {
		qdev_prop_set_drive(card, "drive", (BlockBackend *)js->blk, &err);
		if (err)
			error_report("failed to init SD card: %s", error_get_pretty(err));
	}
	qdev_init_nofail(card);
	return 0;
}

static Property jz_msc_props[] = {
	DEFINE_PROP_INT32("id", JZ_MscState, id, -1),
	DEFINE_PROP_UINT32("rfifo-depth", JZ_MscState, rxf_depth, 128),
	DEFINE_PROP_UINT32("tfifo-depth", JZ_MscState, txf_depth, 128),
	DEFINE_PROP_PTR("blk", JZ_MscState, blk),
	DEFINE_PROP_END_OF_LIST(),
};

static void jz_msc_class_init(ObjectClass *klass, void *data)
{
	SysBusDeviceClass *sc = SYS_BUS_DEVICE_CLASS(klass);
	DeviceClass *dc = DEVICE_CLASS(klass);

	dc->reset = jz_msc_reset;

	dc->props = jz_msc_props;

	sc->init = jz_msc_realize;
}

static void jz_msc_bus_set_inserted(DeviceState *dev, bool inserted)
{
	JZ_MscState *js = JZ_MSC(dev);

	if (js->inserted)
		qemu_set_irq(js->inserted, inserted);
}

static void jz_msc_bus_set_readonly(DeviceState *dev, bool readonly)
{
	JZ_MscState *js = JZ_MSC(dev);

	if (js->readonly)
		qemu_set_irq(js->readonly, readonly);
}

static void jz_msc_bus_class_init(ObjectClass *klass, void *data)
{
	SDBusClass *sdc = SD_BUS_CLASS(klass);
	sdc->set_inserted = jz_msc_bus_set_inserted;
	sdc->set_readonly = jz_msc_bus_set_readonly;
}

static void jz_msc_type_init(void)
{
	const TypeInfo msc_bus_info = {
		.name = TYPE_JZ_MSC_BUS,
		.parent = TYPE_SD_BUS,
		.class_init = jz_msc_bus_class_init,
	};
	const TypeInfo msc_info = {
		.name = TYPE_JZ_MSC_DEVICE,
		.parent = TYPE_SYS_BUS_DEVICE,
		.instance_size = sizeof(JZ_MscState),
		.class_init = jz_msc_class_init,
	};
	type_register_static(&msc_bus_info);
	type_register_static(&msc_info);
}
type_init(jz_msc_type_init);
