
/*
DONT TOUCH THIS LINES
Partialy reversed by zoggn@HakonTI 2021
*/
#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#include <mt-plat/mt_gpio.h>
#include <mach/mt_pmic.h>
#include <mt-plat/upmu_common.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h> 
#include <platform/mt_pmic.h>
#include <string.h>
#endif

#ifndef CONFIG_FPGA_EARLY_PORTING
#if defined(CONFIG_MTK_LEGACY)
#include <cust_i2c.h>
#endif
#endif

#ifdef BUILD_LK
#define LCD_DEBUG  printf
#else
#define LCD_DEBUG  printk
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

//#define LCM_ID (0x98)

#ifdef BUILD_LK
#define GPIO_TPS65132_ENN   GPIO_LCD_BIAS_ENN_PIN
#define GPIO_TPS65132_ENP   GPIO_LCD_BIAS_ENP_PIN
#endif

#define  LCM_DSI_CMD_MODE	0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)				lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)												lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)							lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)												lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   					lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   
#define dsi_swap_port(swap)				lcm_util.dsi_swap_port(swap)	//wqtao.add for build error.

#ifndef BUILD_LK
#define set_gpio_lcd_enp(cmd) lcm_util.set_gpio_lcd_enp_bias(cmd)
#define set_gpio_lcd_enn(cmd) lcm_util.set_gpio_lcd_enn_bias(cmd)
//#define set_gpio_lcd_power_enable(cmd) lcm_util.set_gpio_lcd_power_enable(cmd)
#endif

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

#ifdef BUILD_LK
#define TPS65132_SLAVE_ADDR_WRITE  0x7C
static struct mt_i2c_t tps65132_i2c;

static int tps65132_write_bytes(kal_uint8 addr, kal_uint8 value)
{
	kal_uint32 ret_code = I2C_OK;
	kal_uint8 write_data[2];
	kal_uint16 len;

	write_data[0]= addr;
	write_data[1] = value;

	tps65132_i2c.id = I2C1;
	/* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
	tps65132_i2c.addr = (TPS65132_SLAVE_ADDR_WRITE >> 1);
	tps65132_i2c.mode = ST_MODE;
	tps65132_i2c.speed = 100;
	len = 2;

	ret_code = i2c_write(&tps65132_i2c, write_data, len);
	LCD_DEBUG("%s: i2c_write: ret_code: %d\n", __func__, ret_code);
	return ret_code;
}
#endif

#define REGFLAG_DELAY		0xFFFC
#define REGFLAG_END_OF_TABLE	0xFFFD   /* END OF REGISTERS MARKER */

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
	
	for(i = 0; i < count; i++) 
	{   
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd)
		{
			case REGFLAG_DELAY :
				MDELAY(table[i].count);
				break;
			case REGFLAG_END_OF_TABLE :
				break;
			/*case 0xd9 :
				table[i].para_list[0] = vcom;
				vcom +=2;
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				break;*/
			default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

static struct LCM_setting_table lcm_initialization_setting[] = {
    { 0xE0, 0x01, {0x00}},
    { 0xE1, 0x01, {0x93}},
    { 0xE2, 0x01, {0x65}},
    { 0xE3, 0x01, {0xF8}},
    { 0x80, 0x01, {0x03}},
    { 0x70, 0x01, {0x10}},
    { 0x71, 0x01, {0x13}},
    { 0x72, 0x01, {0x06}},
    { 0x75, 0x01, {0x03}},
    { 0xE0, 0x01, {0x04}},
    { 0x09, 0x01, {0x10}},
    { 0x2B, 0x01, {0x2B}},
    { 0x2D, 0x01, {0x03}},
    { 0x2E, 0x01, {0x44}},
    { 0xE0, 0x01, {0x01}},
    { 0x01, 0x00, {0x00}},
    { 0x01, 0x01, {0xD6}},
    { 0x03, 0x01, {0x00}},
    { 0x04, 0x01, {0xEE}},
    { 0x17, 0x01, {0x00}},
    { 0x18, 0x01, {0xEF}},
    { 0x19, 0x01, {0x01}},
    { 0x1A, 0x01, {0x00}},
    { 0x1B, 0x01, {0xEF}},
    { 0x1C, 0x01, {0x01}},
    { 0x1F, 0x01, {0x6B}},
    { 0x20, 0x01, {0x24}},
    { 0x21, 0x01, {0x24}},
    { 0x22, 0x01, {0x4E}},
    { 0x37, 0x01, {0x05}},
    { 0x38, 0x01, {0x04}},
    { 0x39, 0x01, {0x08}},
    { 0x3A, 0x01, {0x12}},
    { 0x3C, 0x01, {0x78}},
    { 0x3D, 0x01, {0xFF}},
    { 0x3E, 0x01, {0xFF}},
    { 0x3F, 0x01, {0x7F}},
    { 0x40, 0x01, {0x04}},
    { 0x41, 0x01, {0xA0}},
    { 0x43, 0x01, {0x0F}},
    { 0x44, 0x01, {0x12}},
    { 0x45, 0x01, {0x46}},
    { 0x55, 0x01, {0x0C}},
    { 0x56, 0x01, {0x01}},
    { 0x57, 0x01, {0x69}},
    { 0x58, 0x01, {0x0A}},
    { 0x59, 0x01, {0x2A}},
    { 0x5A, 0x01, {0x2B}},
    { 0x5B, 0x01, {0x13}},
    { 0x5D, 0x01, {0x57}},
    { 0x5E, 0x01, {0x4A}},
    { 0x5F, 0x01, {0x3F}},
    { 0x60, 0x01, {0x36}},
    { 0x61, 0x01, {0x36}},
    { 0x62, 0x01, {0x28}},
    { 0x63, 0x01, {0x2F}},
    { 0x64, 0x01, {0x1B}},
    { 0x65, 0x01, {0x34}},
    { 0x66, 0x01, {0x33}},
    { 0x67, 0x01, {0x32}},
    { 0x68, 0x01, {0x4F}},
    { 0x69, 0x01, {0x3C}},
    { 0x6A, 0x01, {0x43}},
    { 0x6B, 0x01, {0x35}},
    { 0x6C, 0x01, {0x31}},
    { 0x6D, 0x01, {0x24}},
    { 0x6E, 0x01, {0x15}},
    { 0x6F, 0x01, {0x09}},
    { 0x70, 0x01, {0x57}},
    { 0x71, 0x01, {0x4A}},
    { 0x72, 0x01, {0x3F}},
    { 0x73, 0x01, {0x36}},
    { 0x74, 0x01, {0x36}},
    { 0x75, 0x01, {0x28}},
    { 0x76, 0x01, {0x2F}},
    { 0x77, 0x01, {0x1B}},
    { 0x78, 0x01, {0x34}},
    { 0x79, 0x01, {0x33}},
    { 0x7A, 0x01, {0x32}},
    { 0x7B, 0x01, {0x4F}},
    { 0x7C, 0x01, {0x3C}},
    { 0x7D, 0x01, {0x43}},
    { 0x7E, 0x01, {0x35}},
    { 0x7F, 0x01, {0x31}},
    { 0x80, 0x01, {0x24}},
    { 0x81, 0x01, {0x15}},
    { 0x82, 0x01, {0x09}},
    { 0xE0, 0x01, {0x02}},
    { 0x00, 0x01, {0x1E}},
    { 0x01, 0x01, {0x1F}},
    { 0x02, 0x01, {0x57}},
    { 0x03, 0x01, {0x58}},
    { 0x04, 0x01, {0x48}},
    { 0x05, 0x01, {0x4A}},
    { 0x06, 0x01, {0x44}},
    { 0x07, 0x01, {0x46}},
    { 0x08, 0x01, {0x40}},
    { 0x09, 0x01, {0x1F}},
    { 0x0A, 0x01, {0x42}},
    { 0x0B, 0x01, {0x1F}},
    { 0x0C, 0x01, {0x1F}},
    { 0x0D, 0x01, {0x1F}},
    { 0x0E, 0x01, {0x1F}},
    { 0x0F, 0x01, {0x1F}},
    { 0x10, 0x01, {0x1F}},
    { 0x11, 0x01, {0x1F}},
    { 0x12, 0x01, {0x1F}},
    { 0x13, 0x01, {0x1F}},
    { 0x14, 0x01, {0x1F}},
    { 0x15, 0x01, {0x1F}},
    { 0x16, 0x01, {0x1E}},
    { 0x17, 0x01, {0x1F}},
    { 0x18, 0x01, {0x57}},
    { 0x19, 0x01, {0x58}},
    { 0x1A, 0x01, {0x49}},
    { 0x1B, 0x01, {0x4B}},
    { 0x1C, 0x01, {0x45}},
    { 0x1D, 0x01, {0x47}},
    { 0x1E, 0x01, {0x41}},
    { 0x1F, 0x01, {0x1F}},
    { 0x20, 0x01, {0x43}},
    { 0x21, 0x01, {0x1F}},
    { 0x22, 0x01, {0x1F}},
    { 0x23, 0x01, {0x1F}},
    { 0x24, 0x01, {0x1F}},
    { 0x25, 0x01, {0x1F}},
    { 0x26, 0x01, {0x1F}},
    { 0x27, 0x01, {0x1F}},
    { 0x28, 0x01, {0x1F}},
    { 0x29, 0x01, {0x1F}},
    { 0x2A, 0x01, {0x1F}},
    { 0x2B, 0x01, {0x1F}},
    { 0x2C, 0x01, {0x1F}},
    { 0x2D, 0x01, {0x1E}},
    { 0x2E, 0x01, {0x57}},
    { 0x2F, 0x01, {0x58}},
    { 0x30, 0x01, {0x47}},
    { 0x31, 0x01, {0x45}},
    { 0x32, 0x01, {0x4B}},
    { 0x33, 0x01, {0x49}},
    { 0x34, 0x01, {0x43}},
    { 0x35, 0x01, {0x1F}},
    { 0x36, 0x01, {0x41}},
    { 0x37, 0x01, {0x1F}},
    { 0x38, 0x01, {0x1F}},
    { 0x39, 0x01, {0x1F}},
    { 0x3A, 0x01, {0x1F}},
    { 0x3B, 0x01, {0x1F}},
    { 0x3C, 0x01, {0x1F}},
    { 0x3D, 0x01, {0x1F}},
    { 0x3E, 0x01, {0x1F}},
    { 0x3F, 0x01, {0x1F}},
    { 0x40, 0x01, {0x1F}},
    { 0x41, 0x01, {0x1F}},
    { 0x42, 0x01, {0x1F}},
    { 0x43, 0x01, {0x1E}},
    { 0x44, 0x01, {0x57}},
    { 0x45, 0x01, {0x58}},
    { 0x46, 0x01, {0x46}},
    { 0x47, 0x01, {0x44}},
    { 0x48, 0x01, {0x4A}},
    { 0x49, 0x01, {0x48}},
    { 0x4A, 0x01, {0x42}},
    { 0x4B, 0x01, {0x1F}},
    { 0x4C, 0x01, {0x40}},
    { 0x4D, 0x01, {0x1F}},
    { 0x4E, 0x01, {0x1F}},
    { 0x4F, 0x01, {0x1F}},
    { 0x50, 0x01, {0x1F}},
    { 0x51, 0x01, {0x1F}},
    { 0x52, 0x01, {0x1F}},
    { 0x53, 0x01, {0x1F}},
    { 0x54, 0x01, {0x1F}},
    { 0x55, 0x01, {0x1F}},
    { 0x56, 0x01, {0x1F}},
    { 0x57, 0x01, {0x1F}},
    { 0x58, 0x01, {0x40}},
    { 0x59, 0x01, {0x00}},
    { 0x5A, 0x01, {0x00}},
    { 0x5B, 0x01, {0x30}},
    { 0x5C, 0x01, {0x05}},
    { 0x5D, 0x01, {0x30}},
    { 0x5E, 0x01, {0x01}},
    { 0x5F, 0x01, {0x02}},
    { 0x60, 0x01, {0x30}},
    { 0x61, 0x01, {0x01}},
    { 0x62, 0x01, {0x02}},
    { 0x63, 0x01, {0x03}},
    { 0x64, 0x01, {0x5E}},
    { 0x65, 0x01, {0x35}},
    { 0x66, 0x01, {0x0D}},
    { 0x67, 0x01, {0x73}},
    { 0x68, 0x01, {0x09}},
    { 0x69, 0x01, {0x03}},
    { 0x6A, 0x01, {0x5E}},
    { 0x6B, 0x01, {0x08}},
    { 0x6C, 0x01, {0x00}},
    { 0x6D, 0x01, {0x00}},
    { 0x6E, 0x01, {0x00}},
    { 0x6F, 0x01, {0x88}},
    { 0x70, 0x01, {0x00}},
    { 0x71, 0x01, {0x00}},
    { 0x72, 0x01, {0x06}},
    { 0x73, 0x01, {0x7B}},
    { 0x74, 0x01, {0x00}},
    { 0x75, 0x01, {0xBC}},
    { 0x76, 0x01, {0x00}},
    { 0x77, 0x01, {0x0D}},
    { 0x78, 0x01, {0x21}},
    { 0x79, 0x01, {0x00}},
    { 0x7A, 0x01, {0x00}},
    { 0x7B, 0x01, {0x00}},
    { 0x7C, 0x01, {0x00}},
    { 0x7D, 0x01, {0x03}},
    { 0x7E, 0x01, {0x7B}},
    { 0xE0, 0x01, {0x00}},
    { 0xE6, 0x01, {0x02}},
    { 0xE7, 0x01, {0x0F}},
    { 0xE0, 0x01, {0x00}},
    { 0x11, 0x01, {0x00}},

	{REGFLAG_DELAY, 120, {} },
	{0x29, 0x01, {0x00} },
	{REGFLAG_DELAY, 5, {} },
	{0x35, 1, {0x00} },
    {0xE0, 1, {0x00} },
    {REGFLAG_DELAY, 2, {} },

	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	{REGFLAG_DELAY, 10, {}},
    {0x28,1,{0x00}},   // Display off 
	
	{REGFLAG_DELAY, 20, {}},
	
	{0x10,1,{0x00}},   // Enter Sleep mode 
	
	{REGFLAG_DELAY, 120, {}},

  	{REGFLAG_END_OF_TABLE, 0x00, {}},
};



// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{

	memset(params, 0, sizeof(LCM_PARAMS));
	
  params->dsi.vertical_backporch = 12;
  params->dsi.vertical_frontporch = 24;
  params->dsi.horizontal_sync_active = 30;
  params->dsi.horizontal_backporch = 60;
  params->dsi.horizontal_frontporch = 66;
  params->dsi.ssc_range = 0;
  params->dsi.PLL_CLOCK = 212;
  params->type = 2;
  params->dsi.data_format.format = 2;
  params->dsi.PS = 2;
  params->dsi.noncont_clock_period = 2;
  params->width = 720;
  params->dsi.horizontal_active_pixel = 720;
  params->height = 1280;
  params->dsi.vertical_active_line = 1280;
  params->dsi.mode = 1;
  params->dsi.ssc_disable = 1;
  params->dsi.noncont_clock = 1;
  params->dsi.esd_check_enable = 1;
  params->dsi.customization_esd_check_enable = 0;
  params->dsi.LANE_NUM = 4;
  params->dsi.vertical_sync_active = 4;
  params->dsi.HS_TRAIL = 6;	


}

static void lcm_init_power(void)
{
	int ret = 0;
	
	//I2C to Bias chip  
	unsigned char cmd  = 0x00;
	unsigned char data = 0xFF;
	LCD_DEBUG("[ili9881c-dj]: %s, line%d\n", __func__, __LINE__);
	pmic_set_register_value(PMIC_RG_VGP1_EN,1);
	MDELAY(2);

#ifdef BUILD_LK
	mt_set_gpio_mode(GPIO_TPS65132_ENP, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_TPS65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_TPS65132_ENP, GPIO_OUT_ONE);
#else
	lcd_bais_enp_enable(1);
#endif
	MDELAY(2);

	//I2C to Bias VSP
       cmd  = 0x00;
	data = 0x0F; //5.5   0x14: 6v
	ret = tps65132_write_bytes(cmd, data);
       LCD_DEBUG("add VSP ret=%d\n", ret);

#ifdef BUILD_LK
	mt_set_gpio_mode(GPIO_TPS65132_ENN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_TPS65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_TPS65132_ENN, GPIO_OUT_ONE);
#else
	lcd_bais_enn_enable(1);
#endif
	MDELAY(2);
        
	//I2C to Bias VSN
	cmd  = 0x01;
	data = 0x0F; //5.5v  0x14:6v
	ret = tps65132_write_bytes(cmd, data);
	LCD_DEBUG("add VSN ret=%d\n", ret);
}

static void lcm_suspend_power(void)
{
	SET_RESET_PIN(0);
	MDELAY(5);
#ifdef BUILD_LK
	mt_set_gpio_mode(GPIO_TPS65132_ENN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_TPS65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_TPS65132_ENN, GPIO_OUT_ZERO);
#else
	lcd_bais_enn_enable(0);
#endif
	MDELAY(5);

#ifdef BUILD_LK
	mt_set_gpio_mode(GPIO_TPS65132_ENP, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_TPS65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_TPS65132_ENP, GPIO_OUT_ZERO);
#else
	lcd_bais_enp_enable(0);
#endif

//	pmic_set_register_value(PMIC_RG_VGP1_EN,0);

}

static void lcm_resume_power(void)
{
	lcm_init_power();
}

static void lcm_init(void)
{
	SET_RESET_PIN(1);
	MDELAY(2);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120); 

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	LCD_DEBUG("[ili9881c-dj]: %s, line%d\n", __func__, __LINE__);
}

static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

	LCD_DEBUG("[ili9881c-dj]: %s, line%d\n", __func__, __LINE__);
}


static void lcm_resume(void)
{
//	lcm_init();

	SET_RESET_PIN(1);
	MDELAY(2);   //10
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);  //50

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	LCD_DEBUG("[ili9881c-dj]: %s, line%d\n", __func__, __LINE__);
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

//	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
//	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#endif

static unsigned int lcm_compare_id(void)
{
	return 1;
}

LCM_DRIVER jd9365_dsi_vdo_holitech_hd720_lcm_drv = 
{
	.name			= "jd9365_dsi_vdo_holitech_hd720",
	.set_util_funcs		= lcm_set_util_funcs,
	.get_params		= lcm_get_params,
	.init			= lcm_init,
};

