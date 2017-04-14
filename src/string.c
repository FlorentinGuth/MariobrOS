#include "string.h"


int str_copy(string source, string dest)
{
  int pos;
  for (pos = 0; source[pos] != '\0'; pos++) {
    dest[pos] = source[pos];
  }
  dest[pos] = '\0';
  return pos + 1;
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
      case 'x': {
        write += int_to_string((char*) (buf+write), va_arg(param,int), 16) - 1;
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
