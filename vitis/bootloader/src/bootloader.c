#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "lib.h"
#include "ff.h"
#include "cottonos_os.h"
#include "elf.h"
#include <stdlib.h> // malloc and free
#include <string.h>

#define BUFFER_SIZE 1024

void copy_last_command(char *buf, char *last_command)
{
    memset(last_command, 0, sizeof(last_command));
    strcpy(last_command, buf);
}

void buf_rst(char *buf, size_t size)
{
    memset(buf, 0, size);
}
void print_banner()
{
    printf("\r\n             __    __                                     \r\n");
    printf("            /\\ \\__/\\ \\__                                  \r\n");
    printf("  ___    ___\\ \\ ,_\\ \\ ,_\\   ___     ___     ___     ____  \r\n");
    printf(" /'___\\ / __`\\ \\ \\/\\ \\ \\/  / __`\\ /' _ `\\  / __`\\  /',__\\ \r\n");
    printf("/\\ \\__//\\ \\L\\ \\ \\ \\_\\ \\ \\_/\\ \\L\\ \\/\\ \\/\\ \\/\\ \\L\\ \\/\\__, `\\\r\n");
    printf("\\ \\____\\ \\____/\\ \\__\\\\ \\__\\ \\____/\\ \\_\\ \\_\\ \\____/\\/\\____/ \r\n");
    printf(" \\/____/\\/___/  \\/__/ \\/__/\\/___/  \\/_/\\/_/\\/___/  \\/___/  \r\n");
    printf("                                                          \r\n");
}

static int dump(unsigned char *buf, long size) // Change type to unsigned char *
{
    long i;

    if (size < 0)
    {
        printf("no data.\n");
        return -1;
    }
    for (i = 0; i < size; i++)
    {
        if ((i & 0xf) == 15)
        {
            printf("\n");
        }
        else
        {
            if ((i & 0xf) == 7)
                printf(" ");
            printf(" ");
        }
    }
    printf("\n");

    return 0;
}

static void wait()
{
    volatile long i;
    for (i = 0; i < 300000; i++)
        ;
}

FRESULT read_data_to_entry_point(const char *filename, void **entry_point, size_t read_size)
{
    FATFS fs;
    FIL fil;
    UINT br;
    FRESULT res;

    // Mount SD card
    if ((res = f_mount(&fs, "", 1)) != FR_OK)
    {
        xil_printf("f_mount failed: %d\n", res);
        return res;
    }

    // Open file (read mode)
    if ((res = f_open(&fil, filename, FA_READ)) != FR_OK)
    {
        xil_printf("f_open failed: %d\n", res);
        return res;
    }

    // Allocate memory for stack area
    unsigned char *buffer = (unsigned char *)malloc(read_size); // Changed to unsigned char *

    size_t total_read = 0;

    while (total_read < read_size)
    {
        size_t to_read = (read_size - total_read < BUFFER_SIZE) ? (read_size - total_read) : BUFFER_SIZE;

        if ((res = f_read(&fil, buffer + total_read, to_read, &br)) != FR_OK)
        {
            xil_printf("Error: %d\n", res);
            break;
        }

        total_read += br;
        if (br < to_read)
        {
            break;
        }
    }

    // Close file
    f_close(&fil);

    // Set the allocated memory address to entry_point
    *entry_point = buffer;

    return FR_OK; // Success
}

FRESULT write_binary_to_sdcard(const char *filename, const uint8_t *data, size_t size)
{
    FATFS fs;
    FIL fil;
    UINT bw;
    FRESULT res;

    // Mount file system
    if ((res = f_mount(&fs, "", 1)) != FR_OK)
    {
        xil_printf("f_mount failed: %d\n", res);
        return res;
    }

    // Open file (binary write mode)
    if ((res = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_APPEND)) != FR_OK)
    {
        xil_printf("f_open failed: %d\n", res);
        return res;
    }

    // Write data
    if ((res = f_write(&fil, data, size, &bw)) != FR_OK)
    {
        xil_printf("f_write failed: %d\n", res);
        f_close(&fil);
        return res;
    }

    // Close file
    f_close(&fil);
    return FR_OK; // Success
}

int main()
{
    const char *filename = "example.bin";

    static char buf[16];
    static char last_command[16] = "";
    int os_loaded = 0;
    size_t data_size = sizeof(os_elf);
    unsigned char *entry_point; // Changed to unsigned char *
    void (*f)(void);

    init_platform();
    unsigned char *loadbuf = NULL;
    extern unsigned char __buffer_start;
    FRESULT res;

    xil_printf("Write Binary To SD Card\n\r");
    xil_printf("data_size: %d\n\r", data_size);

    // Write binary data
    res = write_binary_to_sdcard(filename, os_elf, data_size);
    if (res == FR_OK)
    {
        xil_printf("Binary data written successfully to %s\r\n", filename);
    }
    else
    {
        xil_printf("Error writing data: %d\n", res);
    }

    print_banner();
    xil_printf("cottonos bootloader\n\r");
    xil_printf("cottonos bootloader version 0.1\n\r");
    xil_printf("build date %s %s\n\r", __DATE__, __TIME__);
    printf("\r\nType commands (Enter exit to stop):\n");
    while (1)
    {
        printf("cottonos_console> ");
        co_gets((unsigned char *)buf); // Cast to unsigned char *
        if (!strcmp(buf, "\x1b[A"))    // Up arrow key
        {
            strcpy(buf, last_command);
            printf("%s", buf);
        }
        if (!strcmp(buf, "load"))
        {
            os_loaded = 1;
            loadbuf = (unsigned char *)&__buffer_start;
            copy_last_command(buf, last_command);
            buf_rst(buf, sizeof(buf));
            printf("\nLoad Command received.\n");
            FRESULT res = read_data_to_entry_point(filename, (void **)&loadbuf, data_size);
            if (res != FR_OK)
            {
                printf("Load error!\n");
            }
            else
            {
                printf("Load success.\n");
            }
        }
        else if (!strcmp(buf, "\0"))
        {
            printf("\n");
            buf_rst(buf, sizeof(buf));
        }
        else if (!strcmp(buf, "dump"))
        {
            copy_last_command(buf, last_command);
            buf_rst(buf, sizeof(buf));
            printf("\nDump Command received.\n");
            dump(loadbuf, data_size); // Dump the data from entry point
        }
        else if (!strcmp(buf, "run"))
        {
            if (!os_loaded)
            {
                printf("Please load the binary first.\n");
                continue;
            }
            copy_last_command(buf, last_command);
            buf_rst(buf, sizeof(buf));
            printf("\nRun Command received.\n");

            entry_point = co_elf_load((unsigned char *)loadbuf); // Cast to unsigned char *
            printf("entry_point: %p\n", entry_point);
            if (entry_point == NULL)
            {
                printf("Error loading ELF file.\n");
                continue;
            }
            f = (void (*)(void))entry_point;
            f(); // Pass control to the loaded program
            printf("Run success.\n");
            wait();
            break;
        }
        else if (!strcmp(buf, "exit"))
        {
            printf("\nExiting...\n");
            break;
        }
        else if (!strcmp(buf, "help") || !strcmp(buf, "?"))
        {
            printf("\nAvailable commands:\n");
            printf("  load   - Load binary data from SD card\n");
            printf("  dump   - Dump loaded data\n");
            printf("  run    - Run loaded program\n");
            printf("  exit   - Exit the program\n");
            printf("  help   - Show this help message\n");
            copy_last_command(buf, last_command);
            buf_rst(buf, sizeof(buf));
        }
        else if (!strcmp(buf, "q"))
        {
            printf("\nQuit Command received.\n");
            break;
        }
        else
        {
            printf("\nunknown command: ");
            printf("%s", buf);
            printf("\n");
            buf_rst(buf, sizeof(buf));
        }
    }
    xil_printf("\nFinished receiving characters.\r\n");
    cleanup_platform();
    return 0;
}