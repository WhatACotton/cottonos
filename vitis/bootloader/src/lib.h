#ifndef _LIB_H_INCLUDED_
#define _LIB_H_INCLUDED_
void *co_memset(void *b, int c, long len);
void *co_memcpy(void *dst, const void *src, long len);
int co_memcmp(const void *b1, const void *b2, long len);
int co_strlen(const char *s);
char *co_strcpy(char *dst, const char *src);
int co_strcmp(const char *s1, const char *s2);
int co_strncmp(const char *s1, const char *s2, int len);

int co_putc(unsigned char c);
int co_puts(unsigned char *str);
unsigned char co_getc(void);
int co_gets(unsigned char *buf);
int putxval(unsigned long value, int column);

#endif