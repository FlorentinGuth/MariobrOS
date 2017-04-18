/** string.h:
 *  Manipulations on strings.
 */

#ifndef STRING_H
#define STRING_H

#include <stdarg.h>

/** string:
 *  The type of a string.
 *  It is a pointer towards an array of characters, terminated by '\0'.
 *  Each time the length changes, one needs to reallocate a new string.
 */
typedef char* string;


/** str_copy:
 *  Copies a string into another. Be aware that the destination should be longer
 *  than the source, and that no memory freeing will be done.
 *
 *  @param source The string to be copied
 *  @param dest   The string to be overwritten
 *  @return       The length of the copied string
 */
int str_copy(string source, string dest);


/** int_to_string:
 *  Computes the representation of an integer.
 *  Usage: char str[int_to_string(0, n, b)]; int_to_string(str, n, b);
 *
 *  @param str  The buffer which will receive the null-terminated output (if non-null)
 *  @param num  The integer to transform into a string
 *  @param base The base of the representation
 *  @return     The length of the representation
 */
int int_to_string(char str[], int num, int base);

/** u_int_to_string:
 *  Same as before, but with an unsigned int.
 */
int u_int_to_string(char str[], unsigned int num, int base);

/** format_to_string:
 *  printf-like function that fills a buffer with the formatted string
 *
 *  @param buf  The buffer to fill
 *  @param s    The format string
 *  @param ap   The arguments of the format string
 *  @return     The length of the string written to the buffer
 */
int format_to_string(char buf[], char s[], va_list* ap);

#endif
