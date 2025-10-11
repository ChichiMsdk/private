#ifndef CM_STRING_C
#define CM_STRING_C

#include <cm_macro_defs.c>
#include <cm_types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#if (CM_WINDOWS)

#pragma warning(push, 0)
#include <shlwapi.h>
#pragma warning(pop)

i64
write_file(HANDLE h, void* buffer, u32 length)
{
  DWORD written = 0;
  BOOL  value   = WriteFile(h, buffer, length, &written, NULL);
  if (!value) return -1;
  return written;
}

i32
message_box(char* msg)
{
	DWORD written = 0;
	u32 size = (u32)strlen(msg);
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), msg, size, &written, NULL);
  i32 value = 1;
	value = MessageBox(NULL, msg, NULL, MB_OK);
	return value;
}

#elif (OS_LINUX) || (OS_MAC)
#include <unistd.h>

i32
write_file(i32 fd, void* buffer, ui32 length)
{
  return write(fd, buffer, length);
}

i32
message_box(MU char* msg)
{
  return -1;
}

#endif // CM_WINDOWS

char*
cstrchr(char* str, char c)
{
  i32 i = 0;
  i32 len = (i32) strlen(str);
  while (i < len)
  {
    if (str[i] == c)
      return str + i;
    i++;
  }
  return NULL;
}

bool
str_ncmp_impl(char* src, ...)
{
  va_list args;
  va_start(args, src);
  char* cmp;
  while ((cmp = va_arg(args, char*)))
  {
    if (!strcmp(src, cmp))
    {
      return true;
    }
  }
  va_end(args);
  return false;
}

#define str_ncmp(str, ...) str_ncmp_impl(str, __VA_ARGS__, NULL) 

global u8 utf8_class[32] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

static UnicodeDecode
utf8_decode(u8 *str, u64 max)
{
  u8            byte       = str[0];
  u8            byte_class = utf8_class[byte >> 3];
  UnicodeDecode result     = {1, _u32max};
  switch (byte_class)
  {
    case 1:
    {
      result.codepoint = byte;
    }break;
    case 2:
    {
      if (2 < max)
      {
        u8 cont_byte = str[1];
        if (utf8_class[cont_byte >> 3] == 0)
        {
          result.codepoint  = (byte & _BITMASK5) << 6;
          result.codepoint |= (cont_byte & _BITMASK6);
          result.inc = 2;
        }
      }
    }break;
    case 3:
    {
      if (2 < max)
      {
        u8 cont_byte[2] = {str[1], str[2]};
        if (utf8_class[cont_byte[0] >> 3] == 0 &&
            utf8_class[cont_byte[1] >> 3] == 0)
        {
          result.codepoint  = (byte & _BITMASK4) << 12;
          result.codepoint |= ((cont_byte[0] & _BITMASK6) << 6);
          result.codepoint |= (cont_byte[1] & _BITMASK6);
          result.inc = 3;
        }
      }
    }break;
    case 4:
    {
      if (3 < max)
      {
        u8 cont_byte[3] = {str[1], str[2], str[3]};
        if (utf8_class[cont_byte[0] >> 3] == 0 &&
            utf8_class[cont_byte[1] >> 3] == 0 &&
            utf8_class[cont_byte[2] >> 3] == 0)
        {
          result.codepoint = (byte & _BITMASK3) << 18;
          result.codepoint |= ((cont_byte[0] & _BITMASK6) << 12);
          result.codepoint |= ((cont_byte[1] & _BITMASK6) <<  6);
          result.codepoint |=  (cont_byte[2] & _BITMASK6);
          result.inc = 4;
        }
      }
    }
  }
  return(result);
}

static S32
s32_from_s8(S8 from, u32 *str32)
{
  S32 result = {0};
  if (from.len == 0) return result;

  u8  *ptr_from = (u8*)from.str;
  u64 size      = 0;
  u8  *opl      = ptr_from + from.len;
  UnicodeDecode consume;
  for(; ptr_from < opl; ptr_from += consume.inc)
  {
    consume = utf8_decode(ptr_from, opl - ptr_from);
    str32[size] = consume.codepoint;
    size += 1;
  }
  str32[size] = 0;
  result.str  = str32;
  result.len  = size;
  return result;
}

static i32
buflen_until_int(void* buf, i32 c)
{
  i32 len = 0;
  char* str = (char*)buf;
  while (str[len] != c)
  {
    len++;
  }
  return len;
}

static size_t
wstrlen(wchar_t* wstr)
{
	size_t i;

	i = 0;
	while (wstr[i]) i++;
	return i;
}

/* NOTE: Returns len - 1 */
static uint64_t
wchar_to_char(wchar_t *src, char* dest, uint64_t dest_len)
{
  uint64_t i;
  wchar_t cp;

  i = 0;
  while (src[i] && i < (dest_len - 1))
	{
    cp = src[i];
    if (cp < 128) dest[i] = (char)cp;
    else
		{
      dest[i] = '?';
			// NOTE: lead surrogate, skip the next codepoint (trail)
      if (cp >= 0xD800 && cp <= 0xDBFF) i++;
    }
    i++;
  }
  dest[i] = 0;
  return i - 1;
}

static i32
buflen_until_int_n(void* buf, i32 c, i32 size)
{
  i32 len = 0;
  char* str = (char*)buf;
  while (len < size && str[len] != c)
  {
    len++;
  }
  return len;
}

static size_t
c_strlen(char* str)
{
  size_t len = 0;
  while (str[len])
  {
    len++;
  }
  return len;
}

force_inline size_t
string_len(S8 str)
{
  return str.len;
}

force_inline S8
make_string(char* str)
{
  return (S8){.str = str, .len = c_strlen(str)};
}
/* 
 * WARN: 
 *       This is unsafe and should be used with trusted input only.
 *       1) 0 to INT_MAX for positive sign
 *       2) Either string is empty or only contains '0' to 9' char
 */ 
static i32
fast_atoi(char* str)
{
  i32 val  = 0;
  i32 sign = 1;
  if (*str++ == '-')
  {
    sign *= -1;
  }
  while(*str)
  {
    val = val * 10 + (*str++ - '0');
  }
  return val * sign;
}

static void
cm_itoa(i64 nb, char* buffer, u32 buf_size)
{
  memset(buffer, 0, buf_size);
  u32 i       = 0;
  u64 size    = 0;
  char nums[] = "0123456789";
  if (nb < 0) { buffer[i] = '-'; nb *= -1; }

  u64 tmp    = nb;
  while (tmp > 0) { tmp/= 10; size++;}
  if (size >= buf_size) return;
  size--;
  if (nb == 0)
  {
    buffer[size - i] = '0';
    return ;
  }
  while (nb > 0)
  {
    buffer[size - i] = nums[(nb % 10)];
    nb /= 10;
    i++;
  }
}

static i32
fast_atoi_n(char* str, i32 size)
{
  i32 val  = 0;
  i32 sign = 1;
  i32 i    = 0;
  if (i < size && str[i] == '-')
  {
    sign *= -1;
    i++;
  }
  while(i < size && (str[i] >= '0' && str[i] <= '9'))
  {
    val = val * 10 + (str[i] - '0');
    i++;
  }
  return val * sign;
}
#define atoi(str) fast_atoi_n((str), (i32)c_strlen((str)))

/* NOTE: This is chatgpt */
static void
reverse(char *str, i32 length)
{
  i32 start = 0;
  i32 end = length - 1;
  while (start < end)
  {
    char temp = str[start];
    str[start] = str[end];
    str[end] = temp;
    start++;
    end--;
  }
}

static char*
ftoa(f32 num, char *str, i32 afterpoint)
{
  // Handle negative numbers
  if (num < 0)
  {
    *str++ = '-';
    num *= -1;
  }
  // Extract integer part
  i32 intPart = (i32)num;
  f32 floatPart = num - intPart;
  // Convert integer part to string
  char intStr[20]; // Buffer for integer part
  i32 i = 0;
  if (intPart == 0)
  {
    intStr[i++] = '0';
  }
  else
  {
    while (intPart)
    {
      intStr[i++] = (intPart % 10) + '0';
      intPart /= 10;
    }
  }
  reverse(intStr, i);
  for (i32 j = 0; j < i; j++)
  {
    *str++ = intStr[j];
  }
  // Process fractional part
  if (afterpoint > 0)
  {
    *str++ = '.'; // Add decimal point
    for (i32 j = 0; j < afterpoint; j++)
    {
      floatPart *= 10;
      i32 fractionalDigit = (i32)floatPart;
      *str++ = (char)fractionalDigit + '0';
      floatPart -= fractionalDigit;
    }
  }
  // Null-terminate the string
  *str = '\0';
  return str;
}


#define PRINT_BUFFER_SIZE 10 * 1024

/* WARN: No float in the format ! */
static void
debug_printf_c(char* fmt, ...)
{
  /*
   * NOTE: error LNK2019: unresolved external symbol __chkstk
   *       caused when stack size is `too big`
   * https://www.basicinputoutput.com/2014/08/the-case-of-mysterious-chkstk.html
   */
  char buffer[PRINT_BUFFER_SIZE];
  memset(buffer, 0, PRINT_BUFFER_SIZE);
  va_list args;
  va_start(args, fmt);
  i32 len = wvnsprintf(buffer, PRINT_BUFFER_SIZE, fmt, args);
  if (len >= PRINT_BUFFER_SIZE) MessageBox(NULL, "LEN TOO BIG OUTPUTDEBUG", NULL, MB_OK);
  va_end(args);
  OutputDebugString(buffer);
}

#ifdef NO_CRT_LINKED
/* WARN: No float in the format ! */
static i64
fprintf(HANDLE h, char* fmt, ...)
{
  char buffer[PRINT_BUFFER_SIZE];
  memset(buffer, 0, PRINT_BUFFER_SIZE);
  va_list args;
  va_start(args, fmt);
  i32 len = wvnsprintf(buffer, PRINT_BUFFER_SIZE, fmt, args);
  if (len >= PRINT_BUFFER_SIZE) MessageBox(NULL, "LEN TOO BIG FPRINTF", NULL, MB_OK);
  va_end(args);
  return write_file(h, buffer, len);
}

/* WARN: No float in the format ! */
static i64
printf(char* fmt, ...)
{
  char buffer[PRINT_BUFFER_SIZE];
  memset(buffer, 0, PRINT_BUFFER_SIZE);
  va_list args;
  va_start(args, fmt);
  i32 len = wvnsprintf(buffer, PRINT_BUFFER_SIZE, fmt, args);
  if (len >= PRINT_BUFFER_SIZE) MessageBox(NULL, "LEN TOO BIG PRINTF", NULL, MB_OK);
  va_end(args);
  return write_file(STDOUT(), buffer, len);
}

#endif

static i64
print_string(S8 string)
{
  return write_file(STDOUT(), string.str, (DWORD)string.len);
}
#endif // CM_STRING_C
