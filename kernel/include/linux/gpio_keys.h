#ifndef _GPIO_KEYS_H
#define _GPIO_KEYS_H

#include <gpio.h>

#define A90_GUIDE_KEY0 		10
#define A90_GUIDE_KEY1 		1
#define A90_GUIDE_KEY2 		2
#define A90_GUIDE_KEY3 		3
#define A90_GUIDE_KEY4 		4
#define A90_GUIDE_KEY5 		5
#define A90_GUIDE_KEY6 		6
#define A90_GUIDE_KEY7 		7
#define A90_GUIDE_KEY8 		8
#define A90_GUIDE_KEY9 		9
#define A90_GUIDE_KEYCE		11
#define A90_GUIDE_KEYOK		12
#define A90_GUIDE_KEYP		13 // Play & Pause
#define A90_GUIDE_KEY_VALUEUP 14
#define A90_GUIDE_KEY_VALUEDOWN 15
#define A90_GUIDE_KEY_BACK	16


#define GPIO_SW1_KEY	GPIO_PA(10)
#define GPIO_SW2_KEY	GPIO_PA(11)

struct device;

struct gpio_keys_button {
	/* Configuration parameters */
	unsigned int code;	/* input event code (KEY_*, SW_*) */
	int gpio;		/* -1 if this key does not support gpio */
	int active_low;
	const char *desc;
	unsigned int type;	/* input event type (EV_KEY, EV_SW, EV_ABS) */
	int wakeup;		/* configure the button as a wake-up source */
	int debounce_interval;	/* debounce ticks interval in msecs */
	bool can_disable;
	int value;		/* axis value for EV_ABS */
	unsigned int irq;	/* Irq number in case of interrupt keys */
	int gpio_pullup;
};

struct gpio_keys_platform_data {
	struct gpio_keys_button *buttons;
	int nbuttons;
	unsigned int poll_interval;	/* polling interval in msecs -
					   for polling driver only */
	unsigned int rep:1;		/* enable input subsystem auto repeat */
	int (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
	const char *name;		/* input device name */
};

#endif
