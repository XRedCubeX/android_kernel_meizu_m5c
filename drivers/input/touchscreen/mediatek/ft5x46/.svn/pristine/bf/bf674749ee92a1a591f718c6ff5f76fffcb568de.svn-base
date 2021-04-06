/*
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * * VERSION      	DATE			AUTHOR          Note
 *    1.0		  2013-7-16			Focaltech        initial  based on MTK platform
 *
 */
//#include <cust_eint.h>
//#include <cust_gpio_usage.h>
//#include <mach/mt_pm_ldo.h>
//#include <mach/mt_typedefs.h>
//#include "mt_boot.h"

#include <../../../misc/mediatek/include/mt-plat/mt_gpio.h>
#include <../../../misc/mediatek/include/mt-plat/mt_gpio_core.h>
//#include <../../../misc/mediatek/include/mt-plat/mt6755/include/mach/gpio_const.h>

#include <../../../misc/mediatek/include/mt-plat/mt_boot_common.h>



#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_irq.h>

#include "tpd.h"
#include "tpd_custom_fts.h"
#include <linux/input/mt.h>

#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>

//#include "pmic_api_ldo.h"
#include "focaltech_ex_fun.h"
#include "focaltech_ctl.h"
#include "focaltech_core.h"

unsigned int touch_irq = 0;
bool check_flag = false;
static struct task_struct *probe_thread;

extern struct tpd_device *tpd;
struct regulator *g_ReguVdd = NULL;
#define GTP_GPIO_AS_INT(pin) tpd_gpio_as_int(pin)
#define GTP_GPIO_OUTPUT(pin, level) tpd_gpio_output(pin, level)

/*result*/
#define TPD_OK 							0

/*coordinate register define begin*/
#define DEVICE_MODE 					0x00
#define GEST_ID 						0x01
#define TD_STATUS 						0x02
//point1 info from 0x03~0x08
//point2 info from 0x09~0x0E
//point3 info from 0x0F~0x14
//point4 info from 0x15~0x1A
//point5 info from 0x1B~0x20
/*coordinate register define end*/

/*max detect count in fuction probe*/
#define TPD_RESET_ISSUE_WORKAROUND
#define TPD_MAX_RESET_COUNT 			3


#define TRUE 1
#define FALSE 0

#define TPD_SUPPORT_I2C_DMA         1


extern struct tpd_device *tpd;

struct i2c_client *fts_i2c_client = NULL;
struct input_dev *fts_input_dev 			   =NULL;
//u8 *g_dma_buff_va = NULL;
//dma_addr_t g_dma_buff_pa = 0;

struct task_struct *thread = NULL;

static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DECLARE_WAIT_QUEUE_HEAD(init_waiter);
static DEFINE_MUTEX(i2c_access);



static irqreturn_t tpd_eint_interrupt_handler(int irq, void *dev_id);
// Start:Here maybe need port to different platform,like MT6575/MT6577
//extern void mt_eint_unmask(unsigned int line);
//extern void mt_eint_mask(unsigned int line);
//extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
//extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
//extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
// End:Here maybe need port to different platform,like MT6575/MT6577

static int tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info);
static int tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);


#if 0
#define __MSG_DMA_MODE__
#ifdef __MSG_DMA_MODE__
	u8 *g_dma_buff_va = NULL;
	dma_addr_t g_dma_buff_pa = 0;
#endif

#ifdef __MSG_DMA_MODE__

	 void msg_dma_alloct(void)
	{
		g_dma_buff_va = (u8 *)dma_alloc_coherent(&fts_input_dev->dev, 128, &g_dma_buff_pa, GFP_KERNEL);	// DMA size 4096 for customer
	    if(!g_dma_buff_va)
		{
	        	TPD_DMESG("[DMA][Error] Allocate DMA I2C Buffer failed!\n");
	    }
	}
	 void msg_dma_release(void){
		if(g_dma_buff_va)
		{
	     		dma_free_coherent(&fts_input_dev->dev, 128, g_dma_buff_va, g_dma_buff_pa);
	        	g_dma_buff_va = NULL;
	        	g_dma_buff_pa = 0;
			TPD_DMESG("[DMA][release] Allocate DMA I2C Buffer release!\n");
	    	}
	}
#endif
#endif


static int tpd_flag 					= 0;
static int tpd_halt					= 0;
static int point_num 					= 0;
static int p_point_num 					= 0;


//extern int tpd_mstar_status ;  // compatible mstar and ft6306 chenzhecong

#ifdef TPD_HAVE_BUTTON
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

//#define VELOCITY_CUSTOM_FTXXXX
#ifdef VELOCITY_CUSTOM_FTXXXX
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

// for magnify velocity********************************************

#ifndef TPD_VELOCITY_CUSTOM_X
#define TPD_VELOCITY_CUSTOM_X 			10
#endif
#ifndef TPD_VELOCITY_CUSTOM_Y
#define TPD_VELOCITY_CUSTOM_Y 			10
#endif

#define TOUCH_IOC_MAGIC 				'A'

#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC,0)
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC,1)

static int g_v_magnify_x =TPD_VELOCITY_CUSTOM_X;
static int g_v_magnify_y =TPD_VELOCITY_CUSTOM_Y;




static int tpd_misc_open(struct inode *inode, struct file *file)
{

	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int tpd_misc_release(struct inode *inode, struct file *file)
{
	//file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
//static int adxl345_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd,
                               unsigned long arg)
{

	void __user *data;

	long err = 0;

#ifdef FTS_GESTURE
	unsigned int value = 0;
#endif

	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		FTS_DBG("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
	case TPD_GET_VELOCITY_CUSTOM_X:
		data = (void __user *) arg;
		if(data == NULL)
		{
			err = -EINVAL;
			break;
		}

		if(copy_to_user(data, &g_v_magnify_x, sizeof(g_v_magnify_x)))
		{
			err = -EFAULT;
			break;
		}
		break;

	case TPD_GET_VELOCITY_CUSTOM_Y:
		data = (void __user *) arg;
		if(data == NULL)
		{
			err = -EINVAL;
			break;
		}

		if(copy_to_user(data, &g_v_magnify_y, sizeof(g_v_magnify_y)))
		{
			err = -EFAULT;
			break;
		}
		break;



	default:
		FTS_DBG("tpd: unknown IOCTL: 0x%08x\n", cmd);
		err = -ENOIOCTLCMD;
		break;
	}

	return err;
}


static struct file_operations tpd_fops =
{
//	.owner = THIS_MODULE,
	.open = tpd_misc_open,
	.release = tpd_misc_release,
	.unlocked_ioctl = tpd_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice tpd_misc_device =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = TPD_NAME,
	.fops = &tpd_fops,
};

//**********************************************
#endif

struct touch_info
{
	int y[10];
	int x[10];
	int p[10];
	int id[10];
	int count;
};

static const struct i2c_device_id ftxxxx_tpd_id[] = {{TPD_NAME,0},{}};


static const struct of_device_id tpd_of_match[] = {
	{.compatible = "mediatek,cap_touch"},
	{},
};

static struct i2c_driver tpd_i2c_driver =
{
	.driver = {
		.name 	= TPD_NAME,
		.of_match_table = tpd_of_match,
	},
	.probe 		= tpd_probe,
	.remove 	= tpd_remove,
	.id_table 	= ftxxxx_tpd_id,
	.detect 	= tpd_detect,
};


static  void tpd_down(int x, int y, int p)
{
	//input_report_abs(tpd->dev, ABS_PRESSURE, p);
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 20);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	//FTS_DBG("D[%4d %4d %4d] ", x, y, p);

	/* track id Start 0 */
	input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p);
	input_mt_sync(tpd->dev);

#ifndef MT6572
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
#endif
	{
		tpd_button(x, y, 1);
	}

	TPD_EM_PRINT(x, y, x, y, p-1, 1);
}
#if 0
static  void tpd_up(int x, int y)
{
	//input_report_abs(tpd->dev, ABS_PRESSURE, 0);
	input_report_key(tpd->dev, BTN_TOUCH, 0);

	input_mt_sync(tpd->dev);
	TPD_EM_PRINT(x, y, x, y, 0, 0);

#ifndef MT6572
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
#endif
	{
		tpd_button(x, y, 0);
	}
}
#else
 static  void tpd_up(int x, int y,int *count)
{
	 input_report_key(tpd->dev, BTN_TOUCH, 0);
	 //printk("U[%4d %4d %4d] ", x, y, 0);
	 input_mt_sync(tpd->dev);
	 TPD_EM_PRINT(x, y, x, y, 0, 0);

	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
	{   
		tpd_button(x, y, 0); 
	}   		 
 }

#endif
static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
{

	int i = 0;
//	int ret=0;
	char data[128] = { 0 };
	unsigned short high_byte, low_byte;
	char reg[1]={0};
	u8 report_rate = 0;
	p_point_num = point_num;
	if (tpd_halt) {
		TPD_DMESG("tpd_touchinfo return ..\n");
		return false;
	}
	mutex_lock(&i2c_access);

	reg[0] = 0x00;
	//fts_i2c_Read(fts_i2c_client, reg, 1, data, 64);
	i2c_smbus_read_i2c_block_data(fts_i2c_client, 0x00, 8, &(data[0]));
	i2c_smbus_read_i2c_block_data(fts_i2c_client, 0x08, 8, &(data[8]));
	i2c_smbus_read_i2c_block_data(fts_i2c_client, 0x10, 8, &(data[16]));
	i2c_smbus_read_i2c_block_data(fts_i2c_client, 0x18, 8, &(data[24]));
	i2c_smbus_read_i2c_block_data(fts_i2c_client, 0x20, 1, &(data[32]));

	i2c_smbus_read_i2c_block_data(fts_i2c_client, 0x88, 1, &report_rate);
	mutex_unlock(&i2c_access);

	/*get the number of the touch points */

	point_num = data[2] & 0x0f;

	for (i = 0; i < point_num; i++) {
		cinfo->p[i] = data[3 + 6 * i] >> 6;	/*event flag */
		cinfo->id[i] = data[3 + 6 * i + 2] >> 4;	/*touch id*/
		/*get the X coordinate, 2 bytes */
		high_byte = data[3 + 6 * i];
		high_byte <<= 8;
		high_byte &= 0x0f00;
		low_byte = data[3 + 6 * i + 1];
		cinfo->x[i] = high_byte | low_byte;
		high_byte = data[3 + 6 * i + 2];
		high_byte <<= 8;
		high_byte &= 0x0f00;
		low_byte = data[3 + 6 * i + 3];
		cinfo->y[i] = high_byte | low_byte;
	}

	//printk(" tpd cinfo->x[0] = %d, cinfo->y[0] = %d, cinfo->p[0] = %d\n", cinfo->x[0], cinfo->y[0], cinfo->p[0]);
	return TRUE;

};

static int touch_event_handler(void *unused)
{
	struct touch_info cinfo, pinfo;
	int i=0;

	//struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	//sched_setscheduler(current, SCHED_RR, &param);

	//u8 state;

	do
	{
		//mt_eint_unmask(CUST_EINT_TOUCH_INT_NUM);
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible(waiter,tpd_flag!=0);

		tpd_flag = 0;

		set_current_state(TASK_RUNNING);



		if (tpd_touchinfo(&cinfo, &pinfo))
		{
			//FTS_DBG("point_num = %d\n",point_num);
			TPD_DEBUG_SET_TIME;
			#if 1
			if(point_num >0)
			{
				for(i =0; i<point_num; i++)
				{
					tpd_down(cinfo.x[i], cinfo.y[i], cinfo.id[i]);
				}
				input_sync(tpd->dev);
			}
			else
			{
				tpd_up(cinfo.x[0], cinfo.y[0],0);
				//FTS_DBG("release --->\n");
				//input_mt_sync(tpd->dev);
				input_sync(tpd->dev);
			}
			#else
			fts_report_value(&cinfo);
			#endif
		}

		if(tpd_mode==12)
		{
			//power down for desence debug
			//power off, need confirm with SA
			//ctp_power_off();
			msleep(20);
		}
	}
	while(!kthread_should_stop());

	return 0;
}

static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info)
{
	strcpy(info->type, TPD_DEVICE);
	return 0;
}


static irqreturn_t tpd_eint_interrupt_handler(int irq, void *dev_id)
{
	//printk("TPD interrupt has been triggered\n");
	tpd_flag = 1;
	wake_up_interruptible(&waiter);
	return IRQ_HANDLED;
}


static int tpd_irq_registration(void)
{
	struct device_node *node = NULL;
	int ret = 0;
	u32 ints[2] = { 0, 0 };

	//printk("Device Tree Tpd_irq_registration!\n");

	node = of_find_matching_node(node, touch_of_match);
	if (node) {
		of_property_read_u32_array(node, "debounce", ints, ARRAY_SIZE(ints));
		gpio_set_debounce(ints[0], ints[1]);

		touch_irq = irq_of_parse_and_map(node, 0);
		//printk("Device ft5x46_int_type = %d!\n", ft5x46_int_type);
		
			ret =request_irq(touch_irq, (irq_handler_t) tpd_eint_interrupt_handler, IRQF_TRIGGER_FALLING,"TOUCH_PANEL-eint", NULL);
			if (ret > 0) {
				ret = -1;
				TPD_DMESG("tpd request_irq IRQ LINE NOT AVAILABLE!.\n");
			}
		
	} else {
		TPD_DMESG("tpd request_irq can not find touch eint device node!.\n");
		ret = -1;
	}
	//printk("[%s]irq:%d, debounce:%d-%d, ret=%d:\n", __func__, touch_irq, ints[0], ints[1],ret);
	return ret;
}

static int tpd_registration(void *client)
{
	s32 err = 0;
	s32 idx = 0;
	//printk("%s\n",__func__);


	
	check_flag = true;
	wake_up_interruptible(&init_waiter);
	
	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	if (IS_ERR(thread)) {
		err = PTR_ERR(thread);
		TPD_DMESG(" failed to create kernel thread: %d\n", err);
	}
	if (tpd_dts_data.use_tpd_button) {
		for (idx = 0; idx < tpd_dts_data.tpd_key_num; idx++)
			input_set_capability(tpd->dev, EV_KEY, tpd_dts_data.tpd_key_local[idx]);
	}

	GTP_GPIO_AS_INT(GTP_INT_PORT);

	msleep(50);
	/* EINT device tree, default EINT enable */
	tpd_irq_registration();

	return 0;
}


static int tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
//	int retval = TPD_OK;
	int err = 0;
//	int reset_count = 0;
	u8 data=0x0;
	const char *vdd_name = "vtouch";
	s32 nRetVal = 0;
//	u8 chip_id, i;
//	u8 report_rate = 8;
/*20150911, TPV-Mobile: Eric, MP7551{*/
/*Mask */
//reset_proc:

/*20150911, TPV-Mobile: Eric, MP7551}*/

	fts_i2c_client = client;
	//fts_input_dev=tpd->dev;
	TPD_DMESG("[FTS] enter tpd_probe...\r\n ");
	
	/*sangfei jinfeng changed start 20161022*/
	//pmic_ldo_vldo28_sw_en(1);
	g_ReguVdd = regulator_get(tpd->tpd_dev, vdd_name);
    tpd->reg = g_ReguVdd;

    err = regulator_set_voltage(g_ReguVdd, 2800000, 2800000); 
    if (err)
    {
        TPD_DMESG("Could not set to 2800mv.\n");
    }	
	nRetVal = regulator_enable(g_ReguVdd);
	if (nRetVal)
	{
	    TPD_DMESG( "regulator_enable(g_ReguVdd) failed. nRetVal=%d\n", nRetVal);
	}
	/*sangfei jinfeng changed end 20161022*/
	
	msleep(10);
	GTP_GPIO_OUTPUT(GTP_RST_PORT, 1);
	//printk("GPIO10: dir=0x%x, mode=0x%x, out=0x%x\n", __raw_readl(ioremap(0x10005000,4)),__raw_readl(ioremap(0x10005310,4)), __raw_readl(ioremap(0x10005100,4)));
	msleep(10);
	GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);
	//printk("GPIO10: dir=0x%x, mode=0x%x, out=0x%x\n", __raw_readl(ioremap(0x10005000,4)),__raw_readl(ioremap(0x10005310,4)), __raw_readl(ioremap(0x10005100,4)));
	msleep(10);
	GTP_GPIO_OUTPUT(GTP_RST_PORT, 1);
	//printk("GPIO10: dir=0x%x, mode=0x%x, out=0x%x\n", __raw_readl(ioremap(0x10005000,4)), __raw_readl(ioremap(0x10005310,4)), __raw_readl(ioremap(0x10005100,4)));
	msleep(10);

	
	i2c_smbus_read_i2c_block_data(client, 0x88, 1, &data);
	TPD_DMESG("fts_i2c_read 0x88=%x\n",data);

	/*alloclate DMA*/
	#if 0
	g_dma_buff_va= (u8 *)dma_alloc_coherent(&tpd->dev->dev,128,&g_dma_buff_pa,GFP_KERNEL);
	memset(g_dma_buff_va, 0, 128);
	#endif
	//msg_dma_alloct();
	tpd_load_status = 1;

	probe_thread = kthread_run(tpd_registration, (void *)client, "tpd_probe");
	if (IS_ERR(probe_thread)) {
		err = PTR_ERR(probe_thread);
		printk(" failed to create probe thread: %d\n", err);
		return err;
	}
	TPD_DMESG("tpd_i2c_probe start.wait_event_interruptible\n");
	wait_event_interruptible(init_waiter, check_flag == true);
	TPD_DMESG("tpd_i2c_probe end.wait_event_interruptible\n");

#ifdef FTS_APK_DEBUG
	fts_create_apk_debug_channel(client);
#endif

#ifdef TPD_SYSFS_DEBUG
	fts_create_sysfs(client);
#endif

#ifdef FTS_CTL_IIC
	if (ft_rw_iic_drv_init(client) < 0)
	{
		TPD_DMESG("%s:[FTS] create fts control iic driver failed\n",__func__);
	}
#endif


	printk("tpd_probe OK \n");
	return 0;

}

static int tpd_remove(struct i2c_client *client)
{

//	fts_dma_buffer_deinit();
	// msg_dma_release();
	FTS_DBG("[FTS] TPD removed\n");

	return 0;
}

static int tpd_local_init(void)
{
//	TPD_DMESG("FTS I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);

	if(i2c_add_driver(&tpd_i2c_driver)!=0)
	{
		TPD_DMESG("FTS unable to add i2c driver.\n");
		return -1;
	}
	if(tpd_load_status == 0)
	{
		TPD_DMESG("FTS add error touch panel driver.\n");
		i2c_del_driver(&tpd_i2c_driver);
		return -1;
	}


	TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);
	tpd_type_cap = 1;
	return 0;
}


static void tpd_resume( struct device *h)
{
	int err = 0;
	TPD_DMESG("TPD resume start...\n");
	
	/*sangfei jinfeng changed start 20161022*/
	//pmic_ldo_vldo28_sw_en(1);
	err = regulator_enable(g_ReguVdd);
	/*sangfei jinfeng changed end 20161022*/
	
	msleep(10);
	GTP_GPIO_OUTPUT(GTP_RST_PORT, 1);
	//printk("GPIO10: dir=0x%x, mode=0x%x, out=0x%x\n", __raw_readl(ioremap(0x10005000,4)),__raw_readl(ioremap(0x10005310,4)), __raw_readl(ioremap(0x10005100,4)));
	msleep(10);
	GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);
	//printk("GPIO10: dir=0x%x, mode=0x%x, out=0x%x\n", __raw_readl(ioremap(0x10005000,4)),__raw_readl(ioremap(0x10005310,4)), __raw_readl(ioremap(0x10005100,4)));
	msleep(10);
	GTP_GPIO_OUTPUT(GTP_RST_PORT, 1);
	//printk("GPIO10: dir=0x%x, mode=0x%x, out=0x%x\n", __raw_readl(ioremap(0x10005000,4)), __raw_readl(ioremap(0x10005310,4)), __raw_readl(ioremap(0x10005100,4)));
	msleep(10);
	tpd_halt = 0;
	enable_irq(touch_irq);
	TPD_DMESG("tpd resume end.\n");

}

static void tpd_suspend(struct device *h )
{
	u8 reg_val=0x3;
	int err = 0;
	TPD_DMESG("FT5x46 suspend start...");
	
	tpd_halt = 1;
	disable_irq(touch_irq);

	i2c_smbus_write_i2c_block_data(fts_i2c_client, 0xA5, 1, &reg_val);
	
	/*sangfei jinfeng changed start 20161022*/
	//pmic_ldo_vldo28_sw_en(0);
	err = regulator_disable(g_ReguVdd);
	/*sangfei jinfeng changed end 20161022*/
	

	TPD_DMESG("TPD enter sleep done\n");
}

static struct tpd_driver_t tpd_device_driver =
{
	.tpd_device_name = TPD_NAME,
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
#else
	.tpd_have_button = 0,
#endif
};

static int __init tpd_driver_init(void)
{
	TPD_DMESG("FT5x46 touch panel driver init\n");
	//i2c_register_board_info(0, &ftxxxx_i2c_tpd, 1);
	tpd_get_dts_info();
	if(tpd_driver_add(&tpd_device_driver) < 0)
	{
		TPD_DMESG("add FT5x46 driver failed\n");
	}
	return 0;
}

static void __exit tpd_driver_exit(void)
{
	TPD_DMESG("MediaTek FTXXXX touch panel driver exit\n");
	tpd_driver_remove(&tpd_device_driver);
}



module_init(tpd_driver_init);
module_exit(tpd_driver_exit);


