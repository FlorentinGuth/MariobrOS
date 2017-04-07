#include "string.h"


void str_copy(string source, string dest)
{
  int pos;
  for (pos = 0; source[pos] != '\0'; pos++) {
    dest[pos] = source[pos];
  }
  dest[pos] = '\0';
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
