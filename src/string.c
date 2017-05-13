#include "string.h"
#include "malloc.h"
#include "printer.h"
#include "utils.h"


int str_copy(string source, string dest)
{
  int pos;
  for (pos = 0; source[pos] != '\0'; pos++) {
    dest[pos] = source[pos];
  }
  dest[pos] = '\0';
  return pos + 1;
}

bool str_cmp(string a, string b)
{
  int pos;
  for (pos = 0; a[pos] != '\0' && b[pos] != '\0' && a[pos] == b[pos]; pos++);
  return a[pos] == b[pos];
}

void str_fill(string s, char c, int length)
{
  if (length) {
    for (int pos = 0; pos < length; pos++) {
      s[pos] = c;
    }
  } else {
    for (int pos = 0; s[pos] != '\0'; pos++) {
      s[pos] = c;
    }
  }
}

unsigned int str_length(string s)
{
  int pos = 0;
  for (; s[pos] != '\0'; pos++);
  return pos;
}


list_t str_split(string s, char c, bool empty)
{
  char buffer[128];
  int pos = 0;
  list_t res = 0;
  /* TODO: handle buffer overflow */

  for (int i = 0; s[i] != '\0'; i++) {
    if (s[i] == c && !(pos == 0 && !empty)) {
      /* Split! */
      buffer[pos] = '\0';
      string word = mem_alloc(pos + 1);
      str_copy(buffer, word);
      push(&res, (u_int32)word);
      pos = 0;
    } else {
      if (s[i] != c) {
        buffer[pos] = s[i];
        pos++;
      }
    }
  }

  /* Flush last buffer */
  if (!(pos == 0 && !empty)) {
    buffer[pos] = '\0';
    string word = mem_alloc(pos + 1);
    str_copy(buffer, word);
    push(&res, (u_int32)word);
    pos = 0;
  }

  reverse(&res);
  return res;
}

string str_cat(string a, string b)
{
  unsigned int a_len = str_length(a), b_len = str_length(b);
  string c = (string)mem_alloc(a_len + b_len + 1);  /* Don't forget '\0' */
  str_copy(a, c);
  str_copy(b, c+a_len);
  return c;
}



char digit_to_char(int digit)
{
  if (digit <= 9) {
    return '0' + digit;
  } else {
    return 'A' + digit - 10;
  }
}

int int_to_string(char str[], int num, int base)
{
  int i, rem, len = 0, n;

  n = num;
  if (n < 0) {
    len += 1;
    n = -num;
  }
  do {
    len++;
    n /= base;
  } while (n != 0);

  if (str) {
    n = num;
    int finish = len;
    if (num < 0) {
      n = -num;
      str[0] = '-';
      finish -= 1;
    }
    for (i = 0; i < finish; i++) {
      rem = n % base;
      n = n / base;

      str[len - (i + 1)] = digit_to_char(rem);
    }

    str[len] = '\0';
  }

  return len + 1;  /* Account for '\0' */
}

int u_int_to_string(char str[], unsigned int num, int base)
{
  unsigned int i, rem, len = 0, n;

  n = num;
  do {
    len++;
    n /= base;
  } while (n != 0);

  if (str) {
    for (i = 0; i < len; i++) {
      rem = num % base;
      num = num / base;

      str[len - (i + 1)] = digit_to_char(rem);
    }

    str[len] = '\0';
  }

  return len + 1;  /* Account for '\0' */
}

int format_to_string(char buf[], char s[], va_list* ap)
{
  va_list param = *ap;

  int read  = 0;
  int write = 0;

  while(s[read]!='\0') {
    if(s[read]=='%') {
      read++;
      switch(s[read]) {

      case 'd': {
        write += int_to_string((char*) (buf+write), va_arg(param,int), 10) - 1;
        break; }
      case 'u': {
        write += u_int_to_string((char*) (buf+write), va_arg(param,unsigned int), 10) - 1;
        break; }
      case 'x': {
        buf[write]   = '0';
        buf[write+1] = 'x';
        write += 2;
        write += u_int_to_string((char*) (buf+write), va_arg(param,u_int32), 16) - 1;
        break; }
      case 'X': {
        buf[write]   = '0';
        buf[write+1] = 'x';
        write += 2;
        u_int32 hex = va_arg(param,u_int32);
        u_int32 min_length = va_arg(param,u_int32);
        u_int32 length = u_int_to_string(NULL, hex, 16) - 1;
        for (u_int32 i = 0; i < max(0, min_length - length); i++) {
          buf[write] = '0';
          write++;
        }
        write += u_int_to_string((char*) (buf+write), hex, 16) - 1;
        break; }
      case 'c': {
        buf[write]=va_arg(param,int); write++; break; }
      case 's': {
        write+= str_copy(va_arg(param,string), (char*) (buf+write)) - 1;
        break; }
      default: { buf = "Invalid format string"; return 22; }
      }
    } else {
      buf[write] = s[read];
      write++;
    }
    read++;
  }
  buf[write] = '\0';

  return write + 1;
}
