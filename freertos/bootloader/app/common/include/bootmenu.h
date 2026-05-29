#ifndef __BOOTMENU_H__
#define __BOOTMENU_H__

#ifndef BOOTMENU_AUTOBOOT_DELAY_TIME
#define BOOTMENU_AUTOBOOT_DELAY_TIME                2
#endif

#define BOOTMEMU_STR_BOOT_SYSTEM_VIA_FLASH          "1.Boot system code via Flash."
#define BOOTMEMU_STR_XMODEM_UPGRADE_BOOT_SYSTEM     "2.Upload RTOS to Flash via Xmodem/Xmodem1K then boot system."
#define BOOTMEMU_STR_SHELL                          "3.Bootloader console."

typedef enum
{
    BOOTMENU_ENTRY_BOOT_SYSTEM_VIA_FLASH =0,
    BOOTMENU_ENTRY_XMODEM_UPGRADE_BOOT_SYSTEM = 1,
    BOOTMEMU_ENTRY_SHELL = 2,
    BOOTMENU_ENTRY_LAST,
} BOOTMENU_ENTRY_T;

void bootmenu_show(int delay);
int get_boot_menu_select(void);


#endif /* __BOOTMENU_H__ */
