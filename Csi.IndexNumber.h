/* Csi.IndexNumber.h

   Copyright (C) 1998, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Monday 20 December 1999
   Last Change: Monday 20 December 1999

*/

#ifndef Csi_IndexNumber_h
#define Csi_IndexNumber_h

#include <limits>

namespace Csi
{
   ////////// template class IndexNumber
   // Creates a class of number that is suitable for use in zero-based indexing. This type of class
   // is useful wherever unsigned integers are used but a maximum value needs to be specified.
   template <class T, T max_value>
   class IndexNumber
   {
   private:
      ////////// can_overflow
      // Set to true if addition operations can overflow
      // static bool can_overflow = (max_value >= std::numeric_limits<T>::max()/2);
      
   public:
      ////////// default constructor
      IndexNumber():
         value(0)
      { }

      ////////// value constructor
      typedef T value_type;
      IndexNumber(value_type const &value_) throw (std::out_of_range):
         value(value_)
      { if(value > max_value) throw std::out_of_range(); }

      ////////// copy constructor
      IndexNumber(IndexNumber const &other):
         value(other.value)
      { }

      ////////// copy operator
      IndexNumber &operator =(IndexNumber const &other) { value = other.value; return *this; }

      ////////// assignment operator
      IndexNumber &operator =(value_type const &value_) throw (std::out_of_range)
      { operator =(IndexNumber(value_)); }

      ////////// conversion operator
      operator value_type() const { return value; }

      //@group unary operators
      ////////// prefix increment
      IndexNumber &operator ++()
      {
         if(value == max_value)
            value = 0;
         else
            value++;
         return *this;
      } // prefix increment

      ////////// postfix increment
      IndexNumber operator ++(int)
      {
         IndexNumber rtn(*this);
         operator ++();
         return rtn;
      } // postfix increment

      ////////// prefix decrement
      IndexNumber &operator --()
      {
         if(value == 0)
            value = max_value;
         else
            value--;
         return *this;
      } // prefix decrement

      ////////// postfix decrement
      IndexNumber operator --(int)
      {
         IndexNumber rtn(*this);
         operator --();
         return rtn;
      } // postfix decrement
      //@endgroup

      //@group binary members
      //@endgroup
   private:
      ////////// value
      value_type value;
   };
};

#endif
