#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>


//=======================字符设备驱动模板开始 ===========================//
#define CHAR_DEV_DEVICE_NAME   "char_dev"   // 是应当连接到这个编号范围的设备的名字，出现在/proc/devices和sysfs中
#define CHAR_DEV_NODE_NAME    "char_dev"   // 节点名，出现在/dev中
#define CHAR_DEV_CLASS_NAME   "char_dev_class"   //出现在/sys/devices/virtual/和/sys/class/中
struct class *char_dev_class;  // class结构用于自动创建设备结点 
static int major = 0;       // 0表示动态分配主设备号，可以设置成未被系统分配的具体的数字。
static struct cdev char_dev_cdev;// 定义一个cdev结构

static int data = 10;

struct ChargeInfo {
	int isCharging;
	int isLowPower;
}chargeInfo;

#define IND_CHARGE	GPIO_PB(18)
#define BVL_ALRT		GPIO_PB(19)
#define LED_B		GPIO_PA(8)	//LOW EFFECT
#define LED_G		GPIO_PA(9)	//LOW EFFECT
#define LED_R		GPIO_PA(10)	//LOW EFFECT
#define TF_POWER	GPIO_PB(17) //LOW EFFECT
#define AUDIO_POWER	GPIO_PC(23)
#define SPEAKER_SHUTDOWN GPIO_PB(06) // HIGH EFFECT
#define EAR_SHUTDOWN GPIO_PB(07)//HIGH EFFECT

#define CHARGING 		0
#define UNCHARGING		1
#define LOWPOWER		0
#define NOMALPOWER	1

#define LED_R_LIGHT		0x01
#define LED_G_LIGHT		0x10
#define LED_B_LIGHT		0x11

#define ON	0
#define OFF	1

// 进行初始化设置，打开设备，对应应用空间的open 系统调用 
int char_dev_open(struct inode *inode, struct file *filp)
{
    //  这里可以进行一些初始化
    printk("char_dev device open.\n");
    gpio_direction_output(EAR_SHUTDOWN,1);

	gpio_direction_output(LED_R, ON);
    return 0;
}
 
// 释放设备，关闭设备，对应应用空间的close 系统调用
static int char_dev_release (struct inode *node, struct file *file)
{
    //  这里可以进行一些资源的释放
    printk("char_dev device release.\n");
    return 0;
}
// 实现读功能，读设备，对应应用空间的read 系统调用
/*__user. 这种注解是一种文档形式, 注意, 一个指针是一个不能被直接解引用的
用户空间地址. 对于正常的编译, __user 没有效果, 但是它可被外部检查软件使
用来找出对用户空间地址的错误使用.*/
ssize_t char_dev_read(struct file *file,char __user *buff,size_t count,loff_t *offp)
{
    chargeInfo.isCharging = (gpio_get_value_cansleep(IND_CHARGE) == 1) ? CHARGING : UNCHARGING;
    chargeInfo.isLowPower = (gpio_get_value_cansleep(BVL_ALRT) == 1) ? LOWPOWER : NOMALPOWER;
	printk("%s, %s\n", chargeInfo.isCharging ? "Charging" : "UnCharged", chargeInfo.isLowPower ? "LOW" : "NORMAL");
	copy_to_user(buff, &chargeInfo, sizeof(chargeInfo));
    return 0;
}
// 实现写功能，写设备，对应应用空间的write 系统调用
ssize_t char_dev_write(struct file *file,const char __user *buff,size_t count,loff_t *offp)
{
    printk("char_dev device write.\n");
    int cmd;
    copy_from_user(&cmd, buff, sizeof(int));
    switch(cmd) {
        case LED_R_LIGHT:
		gpio_set_value(LED_R, ON);
	break;
	case LED_G_LIGHT:
		gpio_set_value(LED_G, ON);
	break;
	case LED_B_LIGHT:
		gpio_set_value(LED_B, ON);
	break;
	default:
	break;
    }
    return 0;
}

static long
charge_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("char_dev device ioctl, cmd %d.\n", cmd);
	switch (cmd) {
		case LED_R_LIGHT:
			printk("LED_R_LIGHT\n");
			gpio_set_value(LED_G, OFF);
			gpio_set_value(LED_B, OFF);
			gpio_set_value(LED_R, ON);
			
		break;
		case LED_G_LIGHT:
			printk("LED_G_LIGHT\n");
			gpio_set_value(LED_G, ON);
			gpio_set_value(LED_B, OFF);
			gpio_set_value(LED_R, OFF);

		break;
		case LED_B_LIGHT:
			printk("LED_B_LIGHT\n");
			gpio_set_value(LED_G, OFF);
			gpio_set_value(LED_B, ON);
			gpio_set_value(LED_R, OFF);

		break;
		default:
		break;
	}
	return 0;
}
 
// 实现主要控制功能，控制设备，对应应用空间的ioctl系统调用
static int char_dev_ioctl(struct inode *inode,struct file *file,unsigned int cmd,unsigned long arg)
{  
    printk("char_dev device ioctl.\n");
    return 0;
}

//  file_operations 结构体设置，该设备的所有对外接口在这里明确，此处只写出了几常用的
static struct file_operations char_dev_fops = 
{
    .owner = THIS_MODULE,
    .open  = char_dev_open,      // 打开设备 
    .release = char_dev_release, // 关闭设备 
    .read  = char_dev_read,      // 实现设备读功能 
    .write = char_dev_write,     // 实现设备写功能 
    .unlocked_ioctl =  charge_unlocked_ioctl,
};

// 设备建立子函数，被char_dev_init函数调用  
static void char_dev_setup_cdev(struct cdev *dev, int minor, struct file_operations *fops)
{
    int err, devno = MKDEV(major, minor);
    cdev_init(dev, fops);//对cdev结构体进行初始化
    dev->owner = THIS_MODULE;
    dev->ops = fops;
    err = cdev_add(dev, devno, 1);//参数1是应当关联到设备的设备号的数目. 常常是1
    if(err)
    {
        printk(KERN_NOTICE "Error %d adding char_dev %d.\n", err, minor);
    }
    printk("char_dev device setup.\n");
}

static void charge_gpio_init()
{
	gpio_request(IND_CHARGE, "ind_charge"); 
	gpio_request(BVL_ALRT, "bvl_alrt"); 
	gpio_request(LED_R, "led_r"); 
	gpio_request(LED_G, "led_g");
	gpio_request(LED_B, "led_b");
	gpio_request(TF_POWER, "tf_power");
	gpio_request(AUDIO_POWER, "audioPower");
	gpio_request(SPEAKER_SHUTDOWN, "speaker");
	printk("\n*******************Request EAR_SHUTDOWN ********************\n");
	gpio_request(EAR_SHUTDOWN, "ear");

	gpio_direction_input(IND_CHARGE);
	gpio_direction_input(BVL_ALRT);
	gpio_direction_output(LED_R, OFF);
	gpio_direction_output(LED_G, OFF);
	gpio_direction_output(LED_B, OFF);
	gpio_direction_output(TF_POWER, ON);
	gpio_direction_output(AUDIO_POWER,1);
	//gpio_direction_output(SPEAKER_SHUTDOWN,1);
	
	gpio_direction_output(EAR_SHUTDOWN,1);
}

//   设备初始化 
static int char_dev_init(void)
{
    int result;
    dev_t dev = MKDEV(major, 0);//将主次编号转换为一个dev_t类型
    printk("Enter into char_dev_init\n");
    if(major)
    {
        // 给定设备号注册
        result = register_chrdev_region(dev, 1, CHAR_DEV_DEVICE_NAME);//1是你请求的连续设备编号的总数
        printk("char_dev register_chrdev_region.\n");
    }
    else
    {
        // 动态分配设备号 
        result = alloc_chrdev_region(&dev, 0, 1, CHAR_DEV_DEVICE_NAME);//0是请求的第一个要用的次编号，它常常是 0
        printk("char_dev alloc_chrdev_region.\n");
        major = MAJOR(dev);
    }
    if(result < 0)//获取设备号失败返回
    {
        printk(KERN_WARNING "char_dev region fail.\n");
        return result;
    }
    char_dev_setup_cdev(&char_dev_cdev, 0, &char_dev_fops);
    printk("The major of the char_dev device is %d.\n", major);
    //==== 有中断的可以在此注册中断：request_irq，并要实现中断服务程序 ===//
    // 创建设备节点
    char_dev_class = class_create(THIS_MODULE,CHAR_DEV_CLASS_NAME);
    if (IS_ERR(char_dev_class))
    {
        printk("Err: failed in creating char_dev class.\n");
        return 0;
    }
    device_create(char_dev_class, NULL, dev, NULL, CHAR_DEV_NODE_NAME);
    charge_gpio_init();
    printk("char_dev device installed.\n");
    return 0;
}

// 设备注销 
static void char_dev_cleanup(void)
{
    device_destroy(char_dev_class,MKDEV(major, 0));
    class_destroy(char_dev_class);
    cdev_del(&char_dev_cdev);//字符设备的注销
    unregister_chrdev_region(MKDEV(major, 0), 1);//设备号的注销
    //========  有中断的可以在此注销中断：free_irq  ======//
    printk("char_dev device uninstalled.\n");
}

module_init(char_dev_init);//模块初始化接口
module_exit(char_dev_cleanup);//模块注销接口
//所有模块代码都应该指定所使用的许可证，该句不能省略，否则模块加载会报错
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("Driver Description");
