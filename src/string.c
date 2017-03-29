#include "string.h"


void str_copy(string source, string dest)
{
  int pos;
  for (pos = 0; source[pos] != '\0'; pos++) {
    dest[pos] = source[pos];
  }
  dest[pos] = '\0';
}

/* inline string int_to_string(int n) */
/* { */
/*   // Computes the length to allocate the string */
/*   int len = 0, rem = n; */
/*   do { */
/*     len++; */
/*     rem /= 10; */
/*   } while (rem != 0); // Works with n = 0 too! */

/*   char str[len + 1]; */
/*   for (int i = len - 1; i >= 0; i--) { */
/*     str[i] = n % 10 + '0'; */
/*     n /= 10; */
/*   } */
/*   str[len] = '\0'; */

/*   return str; */
/* } */
