#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "lib.h"
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

int main()
{
    const char *filename = "example.bin";

    static char buf[16];
    static char last_command[16] = "";

    init_platform();
    unsigned char *loadbuf = NULL;
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
        if (!strcmp(buf, "\0"))
        {
            printf("\n");
            buf_rst(buf, sizeof(buf));
        }
        else if (!strcmp(buf, "dump"))
        {
            copy_last_command(buf, last_command);
            buf_rst(buf, sizeof(buf));
            printf("\nDump Command received.\n");
        }
        else if (!strcmp(buf, "exit"))
        {
            printf("\nExiting...\n");
            break;
        }
        else if (!strcmp(buf, "help") || !strcmp(buf, "?"))
        {
            printf("\nAvailable commands:\n");
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