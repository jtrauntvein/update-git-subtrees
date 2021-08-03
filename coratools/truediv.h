/* truediv.h

Defines a template function for true integer division

Copyright (C) 1998, Campbell Scientific, Inc.

Version 1.00
  Written by: Jon Trauntvein
  Date Begun: Thursday 26 February 1998

*/

#ifndef truediv_h
#define truediv_h

////////// truediv
// Calculates the quotient and remainder of an integer following the mathematical definition of the
// division operator with the following rules:
//   q*d + r = n
//   0 <= r < d
template<class T>
void truediv(T &quotient, T &remainder, T numerator, T denominator)
{
   quotient = numerator/denominator;
   remainder = numerator%denominator;
   if(remainder < 0)
   {
      quotient--;
      remainder += denominator;
   }
} /* truediv */

#endif
