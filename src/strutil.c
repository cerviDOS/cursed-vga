#include <string.h>

#include "strutil.h"
int is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\12';
}

char* ltrim(char* str)
{
    while (*str && is_whitespace(*str)) str++;
    return str;
}

char* rtrim(char* str)
{
    char* end = str + strlen(str);
    while(*str && is_whitespace(*--end));
    *(end+1) = '\0';
    return str;
}

char* trim(char* str)
{
    return rtrim(ltrim(str));
}


