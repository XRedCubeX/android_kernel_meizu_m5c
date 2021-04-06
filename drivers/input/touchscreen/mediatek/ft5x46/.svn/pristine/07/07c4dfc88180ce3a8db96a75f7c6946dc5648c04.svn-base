#include $(srctree)/drivers/misc/mediatek/Makefile.custom

# Linux driver folder
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/pmic/include/


obj-y	+=  focaltech_driver.o
obj-y	+=  focaltech_ex_fun.o
obj-y	+=  focaltech_ctl.o
obj-y	+=  focaltech_flash.o
