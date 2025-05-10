#!/bin/bash

xxd -i workspace/os/cottonos_os/Debug/cottonos_os.elf > workspace/os/cottonos_os/Debug/cottonos_os.h

sed -i 's/unsigned char workspace_os_cottonos_os_Debug_cottonos_os_elf\[\] = {/unsigned char os_elf[] = {/g' workspace/os/cottonos_os/Debug/cottonos_os.h

cp workspace/os/cottonos_os/Debug/cottonos_os.h workspace/bootloader/cottonos_bootloader/src/