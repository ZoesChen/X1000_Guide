#ifndef __JZ_H__

#define __JZ_H__

DeviceState *create_jz_intc_controller(CPUState *cpu, hwaddr addr,
		int banks, int size, int bankoff);
void create_jz_uart(CharDriverState *chr, hwaddr base, qemu_irq irq,
		uint8_t channel, uint8_t rx_depth, uint8_t tx_depth);
void create_jz_ost(hwaddr base, qemu_irq irq);
int create_jz_msc(BlockBackend *blk, int id, hwaddr addr, qemu_irq irq, uint32_t depth);
PCIBus *xburst_vpci_register(qemu_irq *pic);
#endif
