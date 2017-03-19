#ifndef _GUIDE_LOCATION_H_
#define _GUIDE_LOCATION_H_

#define MCU_DEV "/dev/mcu_dev"
int OpenMcuDev();
int ReadLocationInfo(int *LocationInfo);

int CloseMcuDev();
#endif

