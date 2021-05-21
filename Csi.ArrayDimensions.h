/* Db.ArrayDimensions.h

   Copyright (C) 2001, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 16 January 2001
   Last Change: Tuesday 20 August 2019
   Last Commit: $Date: 2019-08-28 16:43:36 -0600 (Wed, 28 Aug 2019) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Db_ArrayDimensions_h
#define Db_ArrayDimensions_h

#include <assert.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include "CsiTypeDefs.h"


namespace Csi
{
   /**
    * Defines an object that represents the dimensions of an array as a vector of unsigned
    * integers.  Implements methods that convert between subscript sets and one-based row-major
    * indices.
    */
   class ArrayDimensions
   {
      /**
       * Specifies the set of dimension sizes maintained by this object.
       */
   public:
      typedef std::vector<uint4> dims_type;
   private:
      dims_type dims;
      
   public:
      /**
       * Default constructor (represents a scalar)
       */
      ArrayDimensions()
      { }

      /**
       * Construct this object from a range of iterators interpeted as unsigned integers.
       *
       * @param first Specifies the start of the range.
       *
       * @param last Specifies the point beyond the end of the range.
       */
      template <class fi_type>
      ArrayDimensions(fi_type first, fi_type last)
      { std::copy(first,last,std::back_inserter(dims)); }

      /**
       * Destructor
       */
      ~ArrayDimensions()
      { }
      
      /**
       * @return Returns the calculated one-based row-major offset referred to by the range of array
       * subscripts provided in the parameter list.
       *
       * @param first Specifies the start of the range of subscripts.
       *
       * @param last Specifies the position beyond the end of the range of subscripts.
       */
      template <class fi_type>
      uint4 to_offset(fi_type first, fi_type last) const
      {
         uint4 rtn = 0;
         if(first != last)
         {
            uint4 weight = 1;
            fi_type subscript = last - 1;
            
            for(dims_type::size_type i = dims.size(); i > 0; --i)
            {
               rtn += (*subscript - 1)*weight;
               weight *= dims[i - 1];
               if(subscript != first)
                  subscript--;
            }
         }
         return rtn + 1;
      } // to_offset

      /**
       * Calculates the one-based subscripts associated with the specified one-based row-major
       * offset based upon dimension sizes.
       *
       * @param dest Specifies the container to which the subscripts will be added.
       *
       * @param offset Specifies the one-based row-major offset to convert.
       */
      template <class dest_type>
      void to_index(dest_type dest, uint4 offset) const
      {
         uint4 weight = 1;
         dest_type d = dest;
         for(dims_type::size_type i = dims.size(); i > 0; --i)
         {
            *d++ = ((offset - 1)/weight)%dims[i - 1] + 1;
            weight *= dims[i - 1];
         }
         std::reverse(dest,d);
      } // to_index

      /**
       * @return Returns true if the specified range of subscripts can be contained within the
       * array dimensions.
       *
       * @param first Specifies the start of the range of subscripts.
       *
       * @param last Specifies the point beyond the last of the range of subscripts.
       */
      template <class fi_type>
      bool verify_index(fi_type first, fi_type last) const
      {
         bool rtn = true;
         fi_type subscript = first;
         dims_type::const_iterator di;
         
         for(di = dims.begin();
             rtn && subscript != last && di != dims.end();
             ++subscript, ++di)
         {
            if(*subscript < 1 || *subscript > *di)
               rtn = false;
         }
         if(rtn && (di != dims.end() || subscript != last))
            rtn = false;
         return rtn;
      }

      /**
       * @return Returns the total number of elements that the array can contain.  This value is the
       * product of the size of all dimensions.
       */
      uint4 array_size() const
      {
         uint4 rtn = 1;
         for(dims_type::const_iterator i = dims.begin();
             i != dims.end();
             ++i)
            rtn *= (*i);
         return rtn;
      } // array_size

      /**
       * Adds a new dimension to the end of the vector maintained.
       *
       * @param dim_size Specifies the size of the dimension to be added.
       */
      void add_dimension(uint4 dim_size)
      {
         if(dim_size > 0)
            dims.push_back(dim_size);
      } // add_dimension

      /**
       * Sets the size of the dimension at the specified position.
       *
       * @param dim_size Specifies the new size.
       *
       * @param pos Specifies the dimension position.
       */
      void set_dimension(uint4 dim_size, uint4 pos)
      {
         while(pos >= dims.size())
            dims.push_back(0);
         dims[pos] = dim_size;
      }

      /**
       * @return Returns the dimension at the specified position or zero if there is no such
       * dimension.
       *
       * @param pos Specifies the position.
       */
      uint4 get_dimension(uint4 pos)
      {
         uint4 rtn(0);
         if(pos < dims.size())
            rtn = dims[pos];
         return rtn;
      }
      
      /**
       * Removes all dimension sizes.
       */
      void clear()
      { dims.clear(); }


      // @group: comparison operator
      
      /**
       * @return Returns tru if this set of dimensions is equal to the other.
       *
       * @param other Specifies the set of dimensions to compare.
       */
      bool operator ==(ArrayDimensions const &other) const
      { return dims == other.dims; }

      /**
       * @return Returns true if this set of dimensions is not equal to the other.
       *
       * @param other Specifies the set of dimensions to compare.
       */
      bool operator !=(ArrayDimensions const &other) const
      { return dims != other.dims; }

      // @endgroup:


      // @group: dims access methods
      
      /**
       * @return Returns the iterator to the first of the dimensions.
       */
      typedef dims_type::const_iterator const_iterator;
      const_iterator begin() const { return dims.begin(); }

      /**
       * @return Returns the iterator beyond the last of the dimensions.
       */
      const_iterator end() const { return dims.end(); }

      /**
       * @return Returns a reverse iterator to the last of the dimensions.
       */
      typedef dims_type::const_reverse_iterator const_reverse_iterator;
      const_reverse_iterator rbegin() const { return dims.rbegin(); }

      /**
       * @return Returns a reverse iterator beyond the position of the first of the dimensions.
       */
      const_reverse_iterator rend() const { return dims.rend(); }

      /**
       * @return Returns true if this set is empty.
       */
      bool empty() const
      { return dims.empty(); }

      /**
       * @return Returns the number of dimensions.
       */
      dims_type::size_type size() const
      { return dims.size(); }

      /**
       * @return Returns a reference to the first dimension.
       */
      uint4 const &front() const
      { return dims.front(); }
      uint4 &front()
      { return dims.front(); }

      /**
       * @return Returns a reference to the last dimension.
       */
      uint4 const &back() const
      { return dims.back(); }
      uint4 &back()
      { return dims.back(); }

      /**
       * @return Overloads the subscript operator to reference the dimension at the specified
       * position.
       */
      uint4 operator [](dims_type::size_type pos) const
      { return dims[pos]; }
      
      // @endgroup:
   };
};

#endif
