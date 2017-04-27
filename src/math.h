#ifndef MATH_H
#define MATH_H
/**
 * @name floor_ratio -
 * @param a -
 * @param b -
 * @return The lower integer part of a / b
 */
unsigned int floor_ratio(unsigned int a, unsigned int b);
/**
 * @name ceil_ratio -
 * @param a -
 * @param b -
 * @return The upper integer part of a / b
 */
unsigned int ceil_ratio(unsigned int a, unsigned int b);

/**
 * @name floor_multiple -
 * @param n -
 * @param k -
 * @return The highest multiple of k lesser than or equal to n
 */
unsigned int floor_multiple(unsigned int n, unsigned int k);
/**
 * @name ceil_multiple -
 * @param n -
 * @param k -
 * @return The smallest multiple of k greater than or equal to n
 */
unsigned int ceil_multiple(unsigned int n, unsigned int k);

#endif
