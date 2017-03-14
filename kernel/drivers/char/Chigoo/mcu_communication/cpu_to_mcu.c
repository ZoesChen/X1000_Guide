 /************************************************************ 
  Copyright (C),Chigoo Tech. Co., Ltd. 
  FileName: cpu_to_mcu.c  
      <author>  <time>   <version >   <desc>  
***********************************************************/ 
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <linux/poll.h>
#include<linux/cdev.h> 
#include <linux/delay.h>

#define ERR(fmt, args...) printk("<0>TIOINFO_ERROR:" fmt, ##args)

#define TINTERFACE_NUM           168
#define TINTERFACE_DEVNAME	    "mcu_dev"

static struct cdev char_dev_devs;// 定义一个cdev结构

static	int mcu_major;
static  int mcu_IRQ;

static struct class *mcu_gpio_dev_class; 
static unsigned int queue_flag;
DECLARE_WAIT_QUEUE_HEAD(location_queue);

#define CommTypeBits              4
#define CommCmdBits               8
#define CommDataBits              32
#define CommReturnBits            4

#define MCU_DATA0	GPIO_PC(26)
#define MCU_DATA1	GPIO_PC(27)
//#define MCU_DATA	GPIO_PD(00)
#define MCU_ACK     GPIO_PD(03)
#define MCU_ENA     GPIO_PD(01)
#define MCU_CLK     GPIO_PD(02)

//struct semaphore sem1;

static int    IrqCount = 0;
static u8     CommType = 0;
static u8     SendType = 0; 
static u32    AckStatues= 0; 
static int location_test = 10;

static void ReposeACK(void)
{
  if (AckStatues)
         gpio_set_value(MCU_ACK,0);
  else   gpio_set_value(MCU_ACK,1);
  AckStatues=!AckStatues; 
}

static void char_dev_setup_cdev(struct cdev *dev, int minor, struct file_operations *fops)
{
    int err, devno = MKDEV(mcu_major, minor);
    
    cdev_init(dev, fops);
    dev->owner = THIS_MODULE;
    dev->ops = fops;
    
    err = cdev_add(dev, devno, 1);
    if( err )
    {
        printk(KERN_NOTICE "Error %d adding char_dev %d\n", err, minor);
    }
}

static void mcu_gpio_init(void)
{  
	gpio_request(MCU_DATA1, "mcu_data1"); 
	printk("MCU_DATA1 = %d\n",MCU_DATA1);
	gpio_request(MCU_DATA0, "mcu_data1"); 
	gpio_request(MCU_ACK, "mcu_ack"); 
	gpio_request(MCU_ENA,"mcu_ena"); 
	gpio_request(MCU_CLK, "mcu_clk"); 

	gpio_direction_input(MCU_DATA0);
	gpio_direction_input(MCU_DATA1);
	gpio_direction_output(MCU_ACK,0);
	AckStatues=0;
	gpio_direction_input(MCU_ENA);
	gpio_direction_input(MCU_CLK);
	printk("mcu_interface_gpio_init\n");
}

static int mcu_open(struct inode *inode, struct file *filp)
{
    printk("mcu devive open\n");
    return 0;
}

static ssize_t mcu_read(struct file * file,char * buf,size_t count,loff_t * f_ops) 
{ 
    msleep(2000);
    queue_flag = 1;
    wake_up_interruptible(&location_queue);//
    /********唤醒  --test will in mcu_isr********/
    wait_event_interruptible(location_queue,queue_flag);
    copy_to_user(buf, &location_test,sizeof(int));
    queue_flag = 0;
    return 0; 
} 
 
static int mcu_close(struct inode *inode, struct file *filp)
{
    
	return 0;
}
static irqreturn_t mcu_isr(int irq, void *dev_id)
{
	int interrupt_num;
	interrupt_num = *(int*)dev_id;
	if (interrupt_num!=mcu_IRQ)
	{
		return IRQ_HANDLED;
	}     
    
    queue_flag = 1;
    wake_up_interruptible(&location_queue);//唤醒
//recive mcu data
    
  
  ReposeACK();
  return IRQ_HANDLED; 
}

unsigned int mcu_poll(struct file *file , struct poll_table_struct *wait)
{
    //获取struct xxx *dev = file
    unsigned int mask;
    printk("will poll wait...\n");
    poll_wait(file,&location_queue,wait);
    printk("poll wait end...\n");
    //阻塞
    if(queue_flag)
        mask = POLLIN | POLLRDNORM; //有数据可以读
    return mask;
}

static struct file_operations mcu_fops = {
	.owner  =   THIS_MODULE,
//    .probe  =   mcu_probe,
	.open   =   mcu_open,
	.read   =   mcu_read,
 	.release=   mcu_close,
 	.poll   =   mcu_poll,
};

static int __init	mcu_dev_init(void)
{
	int    ret;
	mcu_major = register_chrdev(TINTERFACE_NUM, TINTERFACE_DEVNAME,&mcu_fops);                                                                                                                                                            
	if (mcu_major < 0) 
	{
        	ERR("Failed to allocate major number.\n");
	        return -ENODEV;
    }
    char_dev_setup_cdev(&char_dev_devs, 0, &mcu_fops);
    printk("The major of the char_dev device is %d\n", mcu_major);
	//sema_init(&sem1, 0);
	mcu_gpio_init();
	mcu_IRQ = gpio_to_irq(MCU_CLK);
	disable_irq(mcu_IRQ);
	irq_set_irq_type(mcu_IRQ,IRQF_TRIGGER_FALLING);//IRQF_TRIGGER_RISING);
	ret = request_irq(mcu_IRQ, mcu_isr,IRQF_DISABLED, TINTERFACE_DEVNAME,&mcu_IRQ);
	if(ret != 0)
	{
		return -ENODEV;
	}
	mcu_gpio_dev_class=class_create(THIS_MODULE, TINTERFACE_DEVNAME); 
	if (IS_ERR(mcu_gpio_dev_class)==0)
	    device_create(mcu_gpio_dev_class,NULL, MKDEV(TINTERFACE_NUM, 0), NULL,TINTERFACE_DEVNAME); 
	else   
        ERR("Failed to Create  Dev=%s\n",TINTERFACE_DEVNAME);	
	printk("Chigoo Comm Successfully initialized module IRQ=%d\n",mcu_IRQ);
	return 0;
}
static void __exit	mcu_dev_cleanup(void)
{
	  device_destroy(mcu_gpio_dev_class, MKDEV(TINTERFACE_NUM, 0)); 
  	  class_destroy(mcu_gpio_dev_class);
      cdev_del(&char_dev_devs);
  	  unregister_chrdev(mcu_major, TINTERFACE_DEVNAME);	
}

module_init(mcu_dev_init);
module_exit(mcu_dev_cleanup);
MODULE_DESCRIPTION("Chigoo mcu communication driver");
MODULE_AUTHOR("Ozil");
MODULE_LICENSE("GPL");



