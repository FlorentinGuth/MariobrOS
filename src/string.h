/** string.h:
 *  Manipulations on strings.
 */

#ifndef STRING_H
#define STRING_H

/** string:
 *  The type of a string.
 *  It is a pointer towards an array of characters, terminated by '\0'.
 *  Each time the length changes, one needs to reallocate a new string.
 */
typedef char* string;


/** int_to_string:
 *  Computes the base 10 representation of an integer.
 *  Note: this function is inlined to be able to return an allocated string.
 *  TODO: make GCC understand that it's OK to return a local variable address.
 *
 *  @param n The integer to transform into a string
 *  @return  The representation of the given integer
 */
// inline string int_to_string(int n);

#endif
