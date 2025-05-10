#include "lib.h"

#include <stdio.h>

void *co_memset(void *b, int c, long len)
{
    char *p;
    for (p = b; len > 0; len--)
        *(p++) = c;
    return b;
}

void *co_memcpy(void *dst, const void *src, long len)
{
    char *d = dst;
    const char *s = src;
    for (; len > 0; len--)
        *(d++) = *(s++);
    return dst;
}

int co_memcmp(const void *b1, const void *b2, long len)
{
    const char *p1 = b1, *p2 = b2;
    for (; len > 0; len--)
    {
        if (*p1 != *p2)
            return (*p1 > *p2) ? 1 : -1;
        p1++;
        p2++;
    }
    return 0;
}

int co_strlen(const char *s)
{
    int len;
    for (len = 0; *s; s++, len++)
        ;
    return len;
}

char *co_strcpy(char *dst, const char *src)
{
    char *d = dst;
    for (;; dst++, src++)
    {
        *dst = *src;
        if (!*src)
            break;
    }
    return d;
}

int co_strcmp(const char *s1, const char *s2)
{
    while (*s1 || *s2)
    {
        if (*s1 != *s2)
            return (*s1 > *s2) ? 1 : -1;
        s1++;
        s2++;
    }
    return 0;
}

int co_strncmp(const char *s1, const char *s2, int len)
{
    while ((*s1 || *s2) && (len > 0))
    {
        if (*s1 != *s2)
            return (*s1 > *s2) ? 1 : -1;
        s1++;
        s2++;
        len--;
    }
    return 0;
}

/* １文字送信 */
int co_putc(unsigned char c)
{
    if (c == '\n')
        putchar('\r');
    putchar(c);
    fflush(stdout);
    return c;
}

/* １文字受信 */
unsigned char co_getc(void)
{
    unsigned char c = getc(stdin);
    c = (c == '\r') ? '\n' : c;
    co_putc(c); /* エコー・バック */

    return c;
}

/* 文字列送信 */
int co_puts(unsigned char *str)
{
    while (*str)
        co_putc(*(str++));
    return 0;
}

/* 文字列受信 */
int co_gets(unsigned char *buf)
{
    int i = 0;
    unsigned char c;
    do
    {
        c = co_getc();
        if (c == '\n')
            c = '\0';
        buf[i++] = c;
    } while (c);
    return i - 1;
}
int co_putxval(unsigned long value, int column)
{
    char buf[9];
    char *p;

    p = buf + sizeof(buf) - 1;
    *(p--) = '\0';

    if (!value && !column)
        column++;

    while (value || column)
    {
        *(p--) = "0123456789abcdef"[value & 0xf];
        value >>= 4;
        if (column)
            column--;
    }

    co_puts(p + 1);

    return 0;
}
