/*
 * TI OMAP processors UART emulation.
 *
 * Copyright (C) 2006-2008 Andrzej Zaborowski  <balrog@zabor.org>
 * Copyright (C) 2007-2009 Nokia Corporation
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
#include "qemu/fifo8.h"
#include "qemu/timer.h"
#include "sysemu/char.h"
#include "hw/hw.h"
#include "hw/sysbus.h"
#include "hw/char/serial.h"
#include "hw/mips/jz.h"
#include "exec/address-spaces.h"

//#define DEBUG_SERIAL

/* UARTs */
typedef struct JZ_UARTState {
	SysBusDevice parent_obj;
	uint8_t rbr;
	uint8_t thr;
	uint16_t divider;
	uint8_t	ier;
	uint8_t iir;
	uint8_t fcr;
	uint8_t lcr;
	uint8_t	mcr;
	uint8_t lsr;
	uint8_t msr;
	uint8_t scr;
	uint8_t isr;
	uint16_t mr;
	uint16_t acr;
	uint8_t rcr;
	uint8_t tcr;

	Fifo8 recv_fifo;
	uint8_t rx_fifo_size;
	int recv_fifo_itl;

	Fifo8 xmit_fifo;
	uint8_t tx_fifo_size;
	int thr_ipending;

	int tsr_retry;
	uint8_t tsr;	/*transmit shift register*/

	int char_transmit_time;
	QEMUTimer *fifo_timeout_timer;
	int timeout_ipending;           /* timeout interrupt pending state */


	QEMUTimer *modem_status_poll;
	int poll_msl;

	int last_break_enable;

	uint32_t baudbase;

	MemoryRegion iomem;

	hwaddr base;

	qemu_irq irq;

	CharDriverState *chr;

	/*for debug*/
	uint8_t channel;
} JZ_UARTState;

#define UART_LCR_DLAB	0x80	/* Divisor latch access bit */

#define UART_IER_MSI	0x08	/* Enable Modem status interrupt */
#define UART_IER_RLSI	0x04	/* Enable receiver line status interrupt */
#define UART_IER_THRI	0x02	/* Enable Transmitter holding register int. */
#define UART_IER_RDI	0x01	/* Enable receiver data interrupt */

#define UART_IIR_NO_INT	0x01	/* No interrupts pending */
#define UART_IIR_ID	0x06	/* Mask for the interrupt ID */

#define UART_IIR_MSI	0x00	/* Modem status interrupt */
#define UART_IIR_THRI	0x02	/* Transmitter holding register empty */
#define UART_IIR_RDI	0x04	/* Receiver data interrupt */
#define UART_IIR_RLSI	0x06	/* Receiver line status interrupt */
#define UART_IIR_CTI    0x0C    /* Character Timeout Indication */

#define UART_IIR_FENF   0x80    /* Fifo enabled, but not functionning */
#define UART_IIR_FE     0xC0    /* Fifo enabled */

/*
 * These are the definitions for the Modem Control Register
 */
#define UART_MCR_MDCE	0x80	/* Enable modem Control*/
#define UART_MCR_FCM	0x40	/* Flow control mode*/
#define UART_MCR_LOOP	0x10	/* Enable loopback test mode */
#define UART_MCR_OUT2	0x08	/* Out2 complement */
#define UART_MCR_OUT1	0x04	/* Out1 complement */
#define UART_MCR_RTS	0x02	/* RTS complement */
#define UART_MCR_DTR	0x01	/* DTR complement */

/*
 * These are the definitions for the Modem Status Register
 */
#define UART_MSR_DCD	0x80	/* Data Carrier Detect */
#define UART_MSR_RI	0x40	/* Ring Indicator */
#define UART_MSR_DSR	0x20	/* Data Set Ready */
#define UART_MSR_CTS	0x10	/* Clear to Send */
#define UART_MSR_DDCD	0x08	/* Delta DCD */
#define UART_MSR_TERI	0x04	/* Trailing edge ring indicator */
#define UART_MSR_DDSR	0x02	/* Delta DSR */
#define UART_MSR_DCTS	0x01	/* Delta CTS */
#define UART_MSR_ANY_DELTA 0x0F	/* Any of the delta bits! */

#define UART_LSR_TEMT	0x40	/* Transmitter empty */
#define UART_LSR_THRE	0x20	/* Transmit-hold-register empty */
#define UART_LSR_BI	0x10	/* Break interrupt indicator */
#define UART_LSR_FE	0x08	/* Frame error indicator */
#define UART_LSR_PE	0x04	/* Parity error indicator */
#define UART_LSR_OE	0x02	/* Overrun error indicator */
#define UART_LSR_DR	0x01	/* Receiver data ready */
#define UART_LSR_INT_ANY 0x1E	/* Any of the lsr-interrupt-triggering status bits */

/* Interrupt trigger levels. The byte-counts are for 16550A - in newer UARTs the byte-count for each ITL is higher. */

#define UART_FCR_ITL_1      0x00 /* 1 byte ITL */
#define UART_FCR_ITL_2      0x40 /* 4 bytes ITL */
#define UART_FCR_ITL_3      0x80 /* 8 bytes ITL */
#define UART_FCR_ITL_4      0xC0 /* 14 bytes ITL */

#define UART_FCR_UME		0x10	/* UART Mode Select*/
#define UART_FCR_DMS        0x08    /* DMA Mode Select */
#define UART_FCR_XFR        0x04    /* XMIT Fifo Reset */
#define UART_FCR_RFR        0x02    /* RCVR Fifo Reset */
#define UART_FCR_FE         0x01    /* FIFO Enable */

#define MAX_XMIT_RETRY      4

#ifdef DEBUG_SERIAL
#define DPRINTF(fmt, ...) \
	do { fprintf(stderr, "serial: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) \
	do {} while (0)
#endif

#define TYPE_JZ_UART "jz_uart"
#define JZ_UART(obj) \
	OBJECT_CHECK(JZ_UARTState, (obj), TYPE_JZ_UART)

#define jz_get_extclk()  (24*1000*1000)

void create_jz_uart(CharDriverState *chr, hwaddr base, qemu_irq irq,
		uint8_t channel, uint8_t rx_depth, uint8_t tx_depth)
{
	struct DeviceState *dev = qdev_create(NULL, TYPE_JZ_UART);
	struct SysBusDevice *bus = SYS_BUS_DEVICE(dev);
	char label[20];

	/*set chardev*/
	if (!chr) {
		snprintf(label, ARRAY_SIZE(label), "null-%08x", (unsigned int)base);
		chr = qemu_chr_new(label, "null", NULL);
		if (!(chr))
			exit(1);
	}

	qdev_prop_set_chr(dev, "chardev", chr);
	if (rx_depth) qdev_prop_set_uint8(dev, "rx_depth", rx_depth);
	if (tx_depth) qdev_prop_set_uint8(dev, "tx_depth", tx_depth);
	if (tx_depth) qdev_prop_set_uint8(dev, "channel", channel);
	qdev_init_nofail(dev);

	sysbus_mmio_map(bus, 0, base);
	sysbus_connect_irq(bus, 0, irq);
}

static void jz_uart_receive(void *opaque, const uint8_t *buf, int size);

static inline void recv_fifo_put(JZ_UARTState *s, uint8_t chr)
{
	/* Receive overruns do not overwrite FIFO contents. */
	if (!fifo8_is_full(&s->recv_fifo))
		fifo8_push(&s->recv_fifo, chr);
	else
		s->lsr |= UART_LSR_OE;
	s->rcr = fifo8_num_used(&s->recv_fifo);
}

static void jz_uart_update_irq(JZ_UARTState *s)
{
	uint8_t tmp_iir = UART_IIR_NO_INT;

	if ((s->ier & UART_IER_RLSI) && (s->lsr & UART_LSR_INT_ANY)) {
		tmp_iir = UART_IIR_RLSI;
	} else if ((s->ier & UART_IER_RDI) && s->timeout_ipending) {
		/* Note that(s->ier & UART_IER_RDI) can mask this interrupt,
		 * this is not in the specification but is observed on existing
		 * hardware.  */
		tmp_iir = UART_IIR_CTI;
	} else if ((s->ier & UART_IER_RDI) && (s->lsr & UART_LSR_DR) &&
			(!(s->fcr & UART_FCR_FE) ||
			 s->recv_fifo.num >= s->recv_fifo_itl)) {
		tmp_iir = UART_IIR_RDI;
	} else if ((s->ier & UART_IER_THRI) && s->thr_ipending) {
		tmp_iir = UART_IIR_THRI;
	} else if ((s->ier & UART_IER_MSI) && (s->msr & UART_MSR_ANY_DELTA)) {
		tmp_iir = UART_IIR_MSI;
	}

	s->iir = tmp_iir | (s->iir & 0xF0);

	if (tmp_iir != UART_IIR_NO_INT) {
		qemu_irq_raise(s->irq);
	} else {
		qemu_irq_lower(s->irq);
	}
}



static void jz_uart_update_parameters(JZ_UARTState *s)
{
	int speed, parity, data_bits, stop_bits, frame_size;
	QEMUSerialSetParams ssp;
	int32_t speed_arr[31] = {
		50,     75,     110,    134,    150,
		200,    300,    600,    1200,   1800,
		2400,   4800,   9600,   19200,  38400,
		57600,  115200, 230400, 460800, 500000,
		576000, 921600, 1000000,1152000,1500000,
		2000000,2500000,3000000,3500000,4000000,
		/*end for noting*/
		4100000,
	};

	if (s->divider == 0)
		return;

	/* Start bit. */
	frame_size = 1;
	if (s->lcr & 0x08) {
		/* Parity bit. */
		frame_size++;
		if (s->lcr & 0x10)
			parity = 'E';
		else
			parity = 'O';
	} else {
		parity = 'N';
	}
	if (s->lcr & 0x04)
		stop_bits = 2;
	else
		stop_bits = 1;

	data_bits = (s->lcr & 0x03) + 5;
	frame_size += data_bits + stop_bits;

	if (!s->mr) {
		speed = s->baudbase / s->divider / 16;
	} else {
		int div, i, acs = 0;

		for (i = 0; i < (frame_size + 1); i++)
			if (s->acr & BIT(i))
				acs++;
		/*
		 * speed = (baudbase / divider / M ) * ((M * frame_size) / (M * frame_size + acs));
		 * speed = (baudbase * frame_size) / (divider * (M * frame_size + acs));
		 */
		speed = s->baudbase * frame_size;
		div = (frame_size * s->mr + acs) * s->divider;
		speed = speed/div;
	}
	/*adjust boudrate*/
	{
		int i;
		for (i = 0; i < 30; i++) {
			if (speed <= speed_arr[i] +
					(speed_arr[i+1] - speed_arr[i])/2) {
				speed = speed_arr[i];
				break;
			}
		}
		if (i == 30) {
			printf("boudrate is wrong\n");
			speed = 115200;	/*default*/
		}
	}

	ssp.speed = speed;
	ssp.parity = parity;
	ssp.data_bits = data_bits;
	ssp.stop_bits = stop_bits;
	s->char_transmit_time =  (NANOSECONDS_PER_SECOND / speed) * frame_size;
	qemu_chr_fe_ioctl(s->chr, CHR_IOCTL_SERIAL_SET_PARAMS, &ssp);

	printf("label %s devpath %s\n", s->chr->label, s->chr->filename);
	DPRINTF("uart[%d] speed=%d parity=%c data=%d stop=%d\n", s->channel,
			speed, parity, data_bits, stop_bits);
}

static void jz_uart_update_msl(JZ_UARTState *s)
{
	uint8_t omsr;
	int flags;

	timer_del(s->modem_status_poll);

	if (qemu_chr_fe_ioctl(s->chr,CHR_IOCTL_SERIAL_GET_TIOCM, &flags) == -ENOTSUP) {
		s->poll_msl = -1;
		return;
	}

	omsr = s->msr;
	s->msr = (flags & CHR_TIOCM_CTS) ? s->msr | UART_MSR_CTS : s->msr & ~UART_MSR_CTS;

	if (s->msr != omsr) {
		/* Set delta bits */
		s->msr = s->msr | ((s->msr >> 4) ^ (omsr >> 4));
		jz_uart_update_irq(s);
	}

	/* The real 16550A apparently has a 250ns response latency to line status changes.
	   We'll be lazy and poll only every 10ms, and only poll it at all if MSI interrupts are turned on */

	if (s->poll_msl) {
		timer_mod(s->modem_status_poll, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) +
				NANOSECONDS_PER_SECOND / 100);
	}
}

static gboolean jz_uart_xmit(GIOChannel *chan, GIOCondition cond, void *opaque)
{
	JZ_UARTState *s = opaque;

	do {
		assert(!(s->lsr & UART_LSR_TEMT));
		if (s->tsr_retry <= 0) {
			assert(!(s->lsr & UART_LSR_THRE));

			if (s->fcr & UART_FCR_FE) {
				assert(!fifo8_is_empty(&s->xmit_fifo));
				s->tsr = fifo8_pop(&s->xmit_fifo);
				if (!s->xmit_fifo.num) {
					s->lsr |= UART_LSR_THRE;
				}
			} else {
				s->tsr = s->thr;
				s->lsr |= UART_LSR_THRE;
			}
			if ((s->lsr & UART_LSR_THRE) && !s->thr_ipending) {
				s->thr_ipending = 1;
				jz_uart_update_irq(s);
			}
		}

		if (s->mcr & UART_MCR_LOOP) {
			/* in loopback mode, say that we just received a char */
			jz_uart_receive(s, &s->tsr, 1);
		} else if (qemu_chr_fe_write(s->chr, &s->tsr, 1) != 1) {
			if (s->tsr_retry >= 0 && s->tsr_retry < MAX_XMIT_RETRY &&
					qemu_chr_fe_add_watch(s->chr, G_IO_OUT|G_IO_HUP,
						jz_uart_xmit, s) > 0) {
				s->tsr_retry++;
				return FALSE;
			}
			s->tsr_retry = 0;
		} else {
			s->tsr_retry = 0;
		}

		/* Transmit another byte if it is already available. It is only
		   possible when FIFO is enabled and not empty. */
	} while (!(s->lsr & UART_LSR_THRE));

	s->lsr |= UART_LSR_TEMT;

	return FALSE;
}

/* Setter for FCR.
   is_load flag means, that value is set while loading VM state
   and interrupt should not be invoked */
static void jz_uart_write_fcr(JZ_UARTState *s, uint8_t val)
{
	/* Set fcr - val only has the bits that are supposed to "stick" */
	s->fcr = val;

	if (val & UART_FCR_FE) {
		s->iir |= UART_IIR_FE;
		/* Set recv_fifo trigger Level */
		switch (val & 0xC0) {
		case UART_FCR_ITL_1:
			s->recv_fifo_itl = 1;
			break;
		case UART_FCR_ITL_2:
			s->recv_fifo_itl = 16;
			break;
		case UART_FCR_ITL_3:
			s->recv_fifo_itl = 32;
			break;
		case UART_FCR_ITL_4:
			s->recv_fifo_itl = 60;
			break;
		}
	} else {
		s->iir &= ~UART_IIR_FE;
	}
}

static void jz_uart_write(void *opaque, hwaddr addr, uint64_t val,
		unsigned size)
{
	struct JZ_UARTState *s = (struct JZ_UARTState *) opaque;

	DPRINTF("uart[%d] write addr=0x%" HWADDR_PRIx " val=0x%" PRIx64 "\n", s->channel, addr, val);
	addr &= 0xff;
	switch (addr) {
	case 0x0:
		if (s->lcr & UART_LCR_DLAB) {
			s->divider = (s->divider & 0xff00) | (val << 8);
		} else {
			s->thr = (uint8_t) val;
			if(s->fcr & UART_FCR_FE) {
				/* xmit overruns overwrite data, so make space if needed */
				if (fifo8_is_full(&s->xmit_fifo)) {
					fifo8_pop(&s->xmit_fifo);
				}
				fifo8_push(&s->xmit_fifo, s->thr);
			}
			s->thr_ipending = 0;
			s->lsr &= ~UART_LSR_THRE;
			s->lsr &= ~UART_LSR_TEMT;
			jz_uart_update_irq(s);
			if (s->tsr_retry <= 0) {
				jz_uart_xmit(NULL, G_IO_OUT, s);
			}
		}
		break;
	case 0x4:
		if (s->lcr & UART_LCR_DLAB) {
			s->divider = (s->divider & 0x00ff) | (val & 0xff);
		} else {
			uint8_t changed = (s->ier ^ val) & 0x0f;
			s->ier = val & 0x0f;
			/* If the backend device is a real serial port, turn polling of the modem
			 * status lines on physical port on or off depending on UART_IER_MSI state.
			 */
			if ((changed & UART_IER_MSI) && s->poll_msl >= 0) {
				if (s->ier & UART_IER_MSI) {
					s->poll_msl = 1;
					jz_uart_update_msl(s);
				} else {
					timer_del(s->modem_status_poll);
					s->poll_msl = 0;
				}
			}

			/* Turning on the THRE interrupt on IER can trigger the interrupt
			 * if LSR.THRE=1, even if it had been masked before by reading IIR.
			 * This is not in the datasheet, but Windows relies on it.  It is
			 * unclear if THRE has to be resampled every time THRI becomes
			 * 1, or only on the rising edge.  Bochs does the latter, and Windows
			 * always toggles IER to all zeroes and back to all ones, so do the
			 * same.
			 *
			 * If IER.THRI is zero, thr_ipending is not used.  Set it to zero
			 * so that the thr_ipending subsection is not migrated.
			 */
			if (changed & UART_IER_THRI) {
				if ((s->ier & UART_IER_THRI) && (s->lsr & UART_LSR_THRE)) {
					s->thr_ipending = 1;
				} else {
					s->thr_ipending = 0;
				}
			}

			if (changed) {
				jz_uart_update_irq(s);
			}
		}
		break;
	case 0x8:
		/* Did the enable/disable flag change? If so, make sure FIFOs get flushed */
		if ((val ^ s->fcr) & UART_FCR_FE) {
			val |= UART_FCR_XFR | UART_FCR_RFR;
		}

		/* FIFO clear */

		if (val & UART_FCR_RFR) {
			s->lsr &= ~(UART_LSR_DR | UART_LSR_BI);
			timer_del(s->fifo_timeout_timer);
			s->timeout_ipending = 0;
			fifo8_reset(&s->recv_fifo);
		}

		if (val & UART_FCR_XFR) {
			s->lsr |= UART_LSR_THRE;
			s->thr_ipending = 1;
			fifo8_reset(&s->xmit_fifo);
		}

		if (((val ^ s->fcr) & val) & UART_FCR_UME)
			jz_uart_update_parameters(s);

		jz_uart_write_fcr(s, val & 0xD9);
		jz_uart_update_irq(s);
		break;
	case 0xc:
		{
			int break_enable;
			s->lcr = val;
			break_enable = (val >> 6) & 1;
			if (break_enable != s->last_break_enable) {
				s->last_break_enable = break_enable;
				qemu_chr_fe_ioctl(s->chr, CHR_IOCTL_SERIAL_SET_BREAK,
						&break_enable);
			}
		}
		break;
	case 0x10:
		{
			//hardware flow module function enabLE
			int flags;
			int old_mcr = s->mcr;
			s->mcr = val & 0x1f;
			if (val & UART_MCR_LOOP)
				break;

			if (s->poll_msl >= 0 && old_mcr != s->mcr) {

				qemu_chr_fe_ioctl(s->chr,CHR_IOCTL_SERIAL_GET_TIOCM, &flags);

				flags &= ~(CHR_TIOCM_RTS);

				if (val & UART_MCR_RTS)
					flags |= CHR_TIOCM_RTS;

				qemu_chr_fe_ioctl(s->chr,CHR_IOCTL_SERIAL_SET_TIOCM, &flags);
				/* Update the modem status after a one-character-send wait-time, since there may be a response
				   from the device/computer at the other end of the serial line */
				timer_mod(s->modem_status_poll, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + s->char_transmit_time);
			}
		}
		break;
	case 0x1c:
		s->scr = val & 0xff;
		break;
	case 0x20:
		s->isr = val & 0xff;
		break;
	case 0x24:
		s->mr = val & 0x1f;
		break;
	case 0x28:
		s->acr = val & 0xfff;
		break;
	default:
		break;
	}
}

static uint64_t jz_uart_read(void *opaque, hwaddr addr, unsigned size)
{
	struct JZ_UARTState *s = (struct JZ_UARTState *) opaque;
	uint32_t ret;

	addr &= 0xff;
	switch(addr) {
	case 0x0:
		if (s->lcr & UART_LCR_DLAB) {
			ret = s->divider & 0xff;
		} else {
			if(s->fcr & UART_FCR_FE) {
				ret = fifo8_is_empty(&s->recv_fifo) ?
					0 : fifo8_pop(&s->recv_fifo);
				if (s->recv_fifo.num == 0) {
					s->lsr &= ~(UART_LSR_DR | UART_LSR_BI);
				} else {
					timer_mod(s->fifo_timeout_timer, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + s->char_transmit_time * 4);
				}
				s->timeout_ipending = 0;
			} else {
				ret = s->rbr;
				s->lsr &= ~(UART_LSR_DR | UART_LSR_BI);
			}
			jz_uart_update_irq(s);
			if (!(s->mcr & UART_MCR_LOOP)) {
				/* in loopback mode, don't receive any data */
				qemu_chr_accept_input(s->chr);
			}
		}
		break;
	case 0x4:
		if (s->lcr & UART_LCR_DLAB) {
			ret = (s->divider >> 8) & 0xff;
		} else {
			ret = s->ier;
		}
		break;
	case 0x8:
		ret = s->iir;
		if ((ret & UART_IIR_ID) == UART_IIR_THRI) {
			s->thr_ipending = 0;
			jz_uart_update_irq(s);
		}
		break;
	case 0xc:
		ret = s->lcr;
		break;
	case 0x10:
		ret = s->mcr;
		break;
	case 0x14:
		ret = s->lsr;
		/* Clear break and overrun interrupts */
		if (s->lsr & (UART_LSR_BI|UART_LSR_OE)) {
			s->lsr &= ~(UART_LSR_BI|UART_LSR_OE);
			jz_uart_update_irq(s);
		}
		break;
	case 0x18:
		if (s->mcr & UART_MCR_LOOP) {
			if (s->mcr & UART_MCR_RTS)
				ret = UART_MSR_CTS;
			else
				ret = 0;
		} else {
			if (s->poll_msl >= 0)
				jz_uart_update_msl(s);
			ret = s->msr;
			/* Clear delta bits & msr int after read, if they were set */
			if (s->msr & UART_MSR_ANY_DELTA) {
				s->msr &= 0xF0;
				jz_uart_update_irq(s);
			}
		}
		break;
	case 0x1c:
		ret = s->scr;
		break;
	case 0x20:
		/*FIXME*/
		ret = s->isr;
		break;
	case 0x24:
		ret = s->mr;
		break;
	case 0x28:
		ret = s->acr;
	case 0x40:
		ret = fifo8_num_used(&s->recv_fifo);
		break;
	case 0x44:
		ret = fifo8_num_used(&s->xmit_fifo);
		break;
	default:
		ret = 0xff;
	}

	DPRINTF("uart[%d] read addr=0x%" HWADDR_PRIx " val=0x%02x\n", s->channel, addr, ret);
	return ret;
}

static const MemoryRegionOps jz_uart_ops = {
	.read = jz_uart_read,
	.write = jz_uart_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

static int jz_uart_can_receive_sub(JZ_UARTState *s)
{
	if(s->fcr & UART_FCR_FE) {
		if (fifo8_is_full(&s->recv_fifo))
			return 0;
		/*
		 * Advertise (fifo.itl - fifo.count) bytes when count < ITL, and 1
		 * if above. If UART_FIFO_LENGTH - fifo.count is advertised the
		 * effect will be to almost always fill the fifo completely before
		 * the guest has a chance to respond, effectively overriding the ITL
		 * that the guest has set.
		 */
		return (fifo8_num_used(&s->recv_fifo) <= s->recv_fifo_itl) ?
			s->recv_fifo_itl - fifo8_num_used(&s->recv_fifo) : 1;
	}
	return !(s->lsr & UART_LSR_DR);
}

/* There's data in recv_fifo and s->rbr has not been read for 4 char transmit times */
static void fifo_timeout_int (void *opaque)
{
	JZ_UARTState *s = opaque;

	if (!fifo8_is_empty(&s->recv_fifo)) {
		s->timeout_ipending = 1;
		jz_uart_update_irq(s);
	}
}

static int jz_uart_can_receive(void *opaque)
{
	JZ_UARTState *s = opaque;
	return jz_uart_can_receive_sub(s);
}

static void jz_uart_receive(void *opaque, const uint8_t *buf, int size)
{
	JZ_UARTState *s = opaque;

	if(s->fcr & UART_FCR_FE) {
		int i;
		for (i = 0; i < size; i++) {
			recv_fifo_put(s, buf[i]);
		}
		s->lsr |= UART_LSR_DR;

		/* call the timeout receive callback in 4 char transmit time */
		timer_mod(s->fifo_timeout_timer, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + s->char_transmit_time * 4);
	} else {
		if (s->lsr & UART_LSR_DR)
			s->lsr |= UART_LSR_OE;
		s->rbr = buf[0];
		s->lsr |= UART_LSR_DR;
	}
	jz_uart_update_irq(s);
}

static void jz_uart_event(void *opaque, int event)
{
	JZ_UARTState *s = opaque;
	if (event == CHR_EVENT_BREAK) {
		s->rbr = 0x0;
		recv_fifo_put(s, 0x0);
		s->lsr |= UART_LSR_BI | UART_LSR_DR;
		jz_uart_update_irq(s);
	}
}

static void jz_uart_reset(struct DeviceState *dev)
{
	JZ_UARTState *s = JZ_UART(dev);

	s->rbr = 0;
	s->thr = 0;
	s->divider = 0;
	s->ier = 0;
	s->iir = 1;
	s->fcr = 0;
	s->lcr = 0;
	s->mcr = 0;
	s->lsr = 0x60;
	s->msr = 0;
	s->scr = 0;
	s->isr = 0;
	s->mr = 0;
	s->acr = 0;
	s->rcr = 0;
	s->tcr = 0;

	timer_del(s->fifo_timeout_timer);
	timer_del(s->modem_status_poll);
	s->poll_msl = 0;
	s->timeout_ipending = 0;
	s->thr_ipending = 0;
	s->last_break_enable = 0;

	fifo8_reset(&s->recv_fifo);
	fifo8_reset(&s->xmit_fifo);
}

static int jz_uart_realize(struct SysBusDevice *dev)
{
	JZ_UARTState *s = JZ_UART(dev);

	if (!s->chr)
		return 0;

	memory_region_init_io(&s->iomem, OBJECT(dev), &jz_uart_ops, s, "jz-uart", 0x48);
	sysbus_init_mmio(dev, &s->iomem);
	sysbus_init_irq(dev, &s->irq);

	s->modem_status_poll = timer_new_ns(QEMU_CLOCK_VIRTUAL, (QEMUTimerCB *) jz_uart_update_msl, s);
	s->fifo_timeout_timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, (QEMUTimerCB *) fifo_timeout_int, s);
	s->baudbase = jz_get_extclk();

	fifo8_create(&s->xmit_fifo, s->tx_fifo_size);
	fifo8_create(&s->recv_fifo, s->rx_fifo_size);

	qemu_chr_add_handlers(s->chr, jz_uart_can_receive,
			jz_uart_receive, jz_uart_event, s);

	jz_uart_reset(DEVICE(dev));
	return 0;
}

static struct Property jz_uart_properties[] = {
	DEFINE_PROP_CHR("chardev", JZ_UARTState, chr),
	DEFINE_PROP_UINT8("rx_depth", JZ_UARTState, rx_fifo_size, 64),
	DEFINE_PROP_UINT8("tx_depth", JZ_UARTState, tx_fifo_size, 64),
	DEFINE_PROP_UINT8("channel", JZ_UARTState, channel, 0),
	DEFINE_PROP_END_OF_LIST(),
};

static void jz_uart_class_init(ObjectClass *kclass, void *data)
{
	struct DeviceClass *dc = DEVICE_CLASS(kclass);
	SysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(kclass);

	k->init = jz_uart_realize;
	dc->reset = jz_uart_reset;
	dc->props = jz_uart_properties;
}

static TypeInfo jz_uart_info = {
	.name = TYPE_JZ_UART,
	.parent	= TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(JZ_UARTState),
	.class_init = jz_uart_class_init,
};

static void jz_uart_register(void)
{
	type_register_static(&jz_uart_info);
}
type_init(jz_uart_register);
