/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "../include/ansi.h"
#include "../include/bootmenu.h"
#include "air_ver.h"
#include "string.h"
#include "timer.h"
#include "uart.h"
#if defined(AIR_SUPPORT_ACE)
#include "util.h"
#endif

#define MENU_ITEMS (BOOTMENU_ENTRY_LAST)

const char *menu[MENU_ITEMS] = {
    "Boot system code via Flash.",
    "Upload RTOS to Flash via Xmodem/Xmodem1K then boot system.",
    "Bootloader console."
};

int boot_menu_select = BOOTMENU_ENTRY_BOOT_SYSTEM_VIA_FLASH;

int get_boot_menu_select(void) {
    return boot_menu_select;
}

void print_menu(int selected) {
    int i = 0;
    printf(ANSI_DECTCEM_DISABLE);
    printf(ANSI_ED_ALL);
    printf(ANSI_CUP(1, 1));
    printf("*** Bootloader SPI NOR-%s (system size %d.%d MB)***\n\n", AIR_VER_BOOTLOADER, SystemSize/S_1M, ((SystemSize%S_1M)/S_1K));
    for (i = 0; i < MENU_ITEMS; i++) {
        if (i == selected) {
            printf(ANSI_COLOR_REVERSE);
        }

        printf("    %d.%s\n", i + 1, menu[i]);
        printf(ANSI_COLOR_RESET);
    }
    printf("\n\n");

    printf("Press UP/DOWN to move or Press 1~%d to choose, ENTER to select", MENU_ITEMS);
}

void change_menu(int selected) {
    int i = 0;

    for (i = 0; i < MENU_ITEMS; i++) {
        if (i == selected) {
            printf(ANSI_COLOR_REVERSE);
        }
        printf(ANSI_CUP((2 + i + 1), 1));
        printf(ANSI_EL_ALL);
        printf("    %d.%s\n", i + 1, menu[i]);
        printf(ANSI_COLOR_RESET);
    }
}

void bootmenu_show(int delay)
{
    int i = 0, c = 0, running = 1, esc = 0;
#if defined(AIR_SUPPORT_ACE)
    unsigned int val;
#endif

    print_menu(boot_menu_select);
    while(running)
    {
        if (delay > 0) {
            printf(ANSI_CUP((2 + MENU_ITEMS + 2), 1));
            printf(ANSI_EL_ALL);
            printf("    Hit any key to stop autoboot: %d\n", delay);
            for (i = 0; i < 100; ++i) {
#if defined(AIR_SUPPORT_ACE)
                val = io_read32(0x10200538);
                if (val > 0) {
                    break;
                }
#endif

                if (!serial_tstc()) {
                    air_wdog_kick();
                    delay1ms(10);
                    continue;
                }

                delay = -1;
                printf(ANSI_CUP(7, 1));
                printf(ANSI_EL_ALL);
                break;
            }
        }
        else {
            if (delay == 0) {
                if (serial_tstc()) {
                    delay = -1;
                }
                else {
                    running = 0;
                    break;
                }
            }
            else
            {
                while (!serial_tstc()) {
                    air_wdog_kick();
                    delay1ms(1);
                }
            }
        }

#if defined(AIR_SUPPORT_ACE)
        val = io_read32(0x10200538);
        if (val > 0) {
            io_write32(0x10005010, 0xace);
            boot_menu_select = BOOTMEMU_ENTRY_SHELL;
            running = 0;
            break;
        }
#endif

        if (delay > 0) {
            delay--;
        }
        else {
            c = serial_inc();
            if (c == '\e') { // ESC sequence
                esc = 1;
                continue;
            }

            if (c == '[') { // [
                if (esc == 1) {
                    esc = 2;
                }
                else {
                    esc = 0;
                }
                continue;
            }

            if (esc == 2) {
                esc = 0;
                switch(c) {
                    case 'A': // up
                        boot_menu_select = (boot_menu_select - 1 + MENU_ITEMS) % MENU_ITEMS;
                        change_menu(boot_menu_select);
                        break;
                    case 'B': // down
                        boot_menu_select = (boot_menu_select + 1) % MENU_ITEMS;
                        change_menu(boot_menu_select);
                        break;
                }
            }

            if (c >= '1' && c <= '0' + MENU_ITEMS) {
                boot_menu_select = c - '1';
                change_menu(boot_menu_select);
            }

            if (c == '\r') {
                running = 0;
            }
        }
    }

    printf(ANSI_DECTCEM_ENABLE);
    printf(ANSI_ED_ALL);
    printf(ANSI_CUP(1, 1));
}

