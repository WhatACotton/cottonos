#include "lib.h"

#include <stdio.h>

/* １文字受信 */
unsigned char co_getc(void)
{
    unsigned char c = getc(stdin);
    return (c == '\r') ? '\n' : c; // 改行コードを統一
}
int co_gets(unsigned char *buf)
{
    int i = 0;
    int rst_flg = 0;
    char *special_char;
    unsigned char c;
    do
    {
        c = co_getc();
        if (c == '\n')
        {
            c = '\0';
        }
        else if (c == '\b' || c == 127) // バックスペースまたはDELキー
        {
            if (i > 0)
            {
                i--;
                buf[i] = '\0';
                putc('\b', stdout); // カーソルを戻す
                putc(' ', stdout);  // 空白で上書き
                putc('\b', stdout); // 再度カーソルを戻す
            }
            continue;
        }
        else if (c == '\x1b' && (c = co_getc()) == '[') // エスケープシーケンス
        {
            c = co_getc(); // さらに次の文字を取得
            rst_flg = 1;
            if (c == 'A') // 上矢印キー
            {
                buf[i] = '\0'; // バッファをクリア
                special_char = "\x1b[A";
                continue;
            }
        }
        else
        {
            putc(c, stdout);
            buf[i++] = c;
        }
    } while (c);
    if (rst_flg)
    {
        memcpy(buf, special_char, strlen(special_char));
        i = strlen(special_char);
        buf[i] = '\0'; // バッファをクリア
    }
    return i - 1;
}
