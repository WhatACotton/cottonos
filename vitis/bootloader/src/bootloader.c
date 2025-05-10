#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "lib.h"
#include "ff.h"
// #include "program.h"
#include "cottonos_os.h"
#include "elf.h"
#include <stdlib.h> // mallocとfreeを使用するために必要

#define BUFFER_SIZE 1024

static int dump(char *buf, long size)
{
    long i;

    if (size < 0)
    {
        co_puts("no data.\n");
        return -1;
    }
    for (i = 0; i < size; i++)
    {
        co_putxval(buf[i], 2);
        if ((i & 0xf) == 15)
        {
            co_puts("\n");
        }
        else
        {
            if ((i & 0xf) == 7)
                co_puts(" ");
            co_puts(" ");
        }
    }
    co_puts("\n");

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
    xil_printf("Reading data from %s to entry point %p\r\n", filename, *entry_point);
    FATFS fs;
    FIL fil;
    UINT br;
    FRESULT res;

    // SDカードをマウント
    if ((res = f_mount(&fs, "", 1)) != FR_OK)
    {
        xil_printf("f_mount failed: %d\n", res);
        return res;
    }

    // ファイルをオープン（読み込みモード）
    if ((res = f_open(&fil, filename, FA_READ)) != FR_OK)
    {
        xil_printf("f_open failed: %d\n", res);
        return res;
    }

    // スタック領域のmemoryを確保

    char *buffer = (char *)malloc(read_size);

    size_t total_read = 0;

    while (total_read < read_size)
    {
        size_t to_read = (read_size - total_read < BUFFER_SIZE) ? (read_size - total_read) : BUFFER_SIZE;

        if ((res = f_read(&fil, buffer + total_read, to_read, &br)) != FR_OK)
        {
            if (res != FR_OK)
            {
                xil_printf("Error: %d\n", res);
                switch (res)
                {
                case FR_NO_FILE:
                    xil_printf("File not found.\n");
                    break;
                case FR_NO_PATH:
                    xil_printf("Path not found.\n");
                    break;
                case FR_INVALID_NAME:
                    xil_printf("Invalid file name.\n");
                    break;
                // 他のエラーコードにも対応可能
                default:
                    xil_printf("Unknown error.\n");
                    xil_printf("Error code: %d\n", res);
                    break;
                }
            }
        }

        total_read += br;

        xil_printf("Read %d bytes, total read: %d bytes\r\n", br, total_read);
        if (br < to_read)
        {
            break;
        }
    }

    // ファイルをクローズ
    f_close(&fil);

    // 確保したメモリのアドレスをentry_pointに設定
    *entry_point = buffer;

    return FR_OK; // 正常終了
}

FRESULT write_binary_to_sdcard(const char *filename, const uint8_t *data, size_t size)
{
    FATFS fs;    // FatFsのファイルシステムオブジェクト
    FIL fil;     // FatFsのファイルオブジェクト
    UINT bw;     // 書き込んだバイト数
    FRESULT res; // 戻り値

    // ファイルシステムをマウント
    if ((res = f_mount(&fs, "", 1)) != FR_OK)
    {
        xil_printf("f_mount failed: %d\n", res);
        return res;
    }

    // ファイルをオープン（バイナリ書き込みモード）
    if ((res = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_APPEND)) != FR_OK)
    {
        xil_printf("f_open failed: %d\n", res);
        return res;
    }

    // データを書き込む
    if ((res = f_write(&fil, data, size, &bw)) != FR_OK)
    {
        xil_printf("f_write failed: %d\n", res);
        f_close(&fil);
        return res;
    }

    // ファイルをクローズ
    f_close(&fil);
    return FR_OK; // 正常終了
}
int main()
{
    const char *filename = "example.bin"; // 書き込むファイル名

    static char buf[16];
    size_t data_size = sizeof(os_elf);
    char *entry_point;
    void (*f)(void);

    init_platform();
    unsigned char *loadbuf = NULL;
    extern unsigned char __buffer_start;
    FRESULT res;

    xil_printf("Write Binary To SD Card\n\r");
    xil_printf("data_size: %d\n\r", data_size);

    // バイナリデータを書き込む
    res = write_binary_to_sdcard(filename, os_elf, data_size);
    if (res == FR_OK)
    {
        xil_printf("Binary data written successfully to %s\r\n", filename);
    }
    else
    {
        xil_printf("Error writing data: %d\n", res);
    }

    xil_printf("Enter characters (Enter exit to stop):\r\n");
    while (1)
    {
        co_puts("cottonos_load> ");
        co_gets(buf);

        if (!co_strcmp(buf, "load"))
        {
            co_puts("Load Command received.\n");
            loadbuf = (unsigned char *)&__buffer_start; // __buffer_startのアドレスを取得
            FRESULT res = read_data_to_entry_point(filename, (void **)&loadbuf, data_size);
            if (res != FR_OK)
            {
                co_puts("Load error!\n");
            }
            else
            {
                co_puts("Load success.\n");
                co_puts("Load data start.\n");
                co_puts("Load data end.\n");
            }
        }
        else if (!strcmp(buf, "dump"))
        {
            co_puts("Dump Command received.\n");
            dump(loadbuf, data_size); // エントリーポイントからデータをダンプ
        }
        else if (!strcmp(buf, "run"))
        {
            co_puts("Run Command received.\n");

            entry_point = co_elf_load(loadbuf); // ELFファイルを読み込む

            f = (void (*)(void))entry_point;
            f(); /* ここで，ロードしたプログラムに処理を渡す */
            co_puts("Run success.\n");
            co_puts("Run data start.\n");
            co_puts("Run data end.\n");
            wait();
        }
        else if (!co_strcmp(buf, "q"))
        {
            co_puts("Quit Command received.\n");
            break;
        }
        else if (!co_strcmp(buf, "exit"))
        {
            co_puts("Exiting ...\n");
            break;
        }
        else
        {
            co_puts("unknown.\n");
        }
    }
    xil_printf("Finished receiving characters.\r\n");
    cleanup_platform();
    return 0;
}
