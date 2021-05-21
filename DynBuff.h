/* DynBuff.h

   Copyright (C) 1998, 2020 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Wednesday 16 September 1998
   Last Change: Monday 02 March 2020
   Last Commit: $Date: 2020-03-02 16:14:48 -0600 (Mon, 02 Mar 2020) $
   Committed by: $Author: jon $
   
*/

#ifndef DynBuff_h
#define DynBuff_h

#include <stdexcept>
#include <cctype>
#include <cwctype>
#include <stdio.h>
#include <new>
#include <string.h>
#include "CsiTypeDefs.h"


/**
 * Declare a type of exception that will be thrown when performing file I/O operations with DynBuff
 * derived objects.
 */
class fileio_error: public std::exception
{
public:
   virtual const char *what() const throw ()
   { return "File I/O error"; }
};


/**
 * Declares a dynamically expandable base component for string types.
 */
template <class T, size_t spec_additional>
class DynBuff
{
public:
   /**
    * Default Constructor
    */
   DynBuff():
      buff_len(0),
      alloc_len(sizeof(fixed_storage) / sizeof(T)),
      storage(fixed_storage)
   { memset(fixed_storage, 0, sizeof(fixed_storage)); }

   /**
    * Copy constructor
    */
   DynBuff(DynBuff const &other):
      buff_len(0),
      alloc_len(sizeof(fixed_storage) / sizeof(T)),
      storage(fixed_storage)
   {
      memset(fixed_storage, 0, sizeof(fixed_storage));
      setContents(other.storage,other.buff_len);
   } // copy constructor
   
   /**
    * Construct from any array with specified length.
    *
    * @param buff Specifies the start of the array to copy.
    *
    * @param len Specifies the number of elements to copy.
    */
   DynBuff(T const *buff, size_t len):
      buff_len(0),
      alloc_len(sizeof(fixed_storage) / sizeof(T)),
      storage(fixed_storage)
   {
      memset(fixed_storage, 0, sizeof(fixed_storage));
      setContents(buff,len);
   } // array constructor

   /**
    * Destructor
    */
   virtual ~DynBuff()
   {
      if(storage && storage != fixed_storage)
         delete[] storage;
   }

   /**
    * Copy operator
    */
   DynBuff &operator =(DynBuff const &other)
   {
      setContents(other.storage, other.buff_len);
      return *this;
   }

   // @group: Storage access methods
   
   /**
    * Returns the pointer at the beginning of the storage buffer for this string.
    */
   T const *getContents() const
   { return storage; }

   /**
    * Returns a non-const pointer to the beginning of the storage buffer.
    */
   T *getContents_writable()
   { return storage; }

   /**
    * Replaces the content of this buffer with the content of the specified array.
    *
    * @param buff Specifies the start of the array to copy.
    *
    * @param len Specifies the number of elements to copy.
    */
   void setContents(T const *buff, size_t len)
   {
      // if the source buffer is the same as this storage buffer, we are being passed our own stroage
      // for initialisation,  since the two values are already equal, none of the regular copying needs
      // to be done.
      if(buff != storage)
      {
         if(buff && len > 0)
         {
            grow(len);
            copyElems(storage,buff,len);
            buff_len = len;
            terminate();
         }
         else
            cut(0);
      }
   }

   /**
    * Replaces the contents of this buffer with the contents of the other buffer.
    *
    * @param other Specifies the buffer to copy.
    */
   void setContents(DynBuff const &other)
   { setContents(other.storage,other.buff_len); }

   /**
    * Fills this buffer with the specified value.
    *
    * @param e Specifies the value to initialise.
    *
    * @param cnt Specifies the number of elements to add.
    */
   void fill(T const &e, size_t cnt)
   {
      cut(0);
      reserve(cnt);
      for(buff_len = 0; buff_len < cnt; buff_len++)
         storage[buff_len] = e;
      terminate();
   }

   /**
    * Pads this buffer with the specified element value until the total size of this buffer is at
    * least the specified number in length.  This will have no effect if this buffer's length is
    * already greater than or equal to the specified length.
    *
    * @param e Specifies the valeu to write.
    *
    * @param cnt Specifies the  maximum number of elements to pad.
    */
   void pad(T const &e, size_t cnt)
   {
      reserve(cnt);
      while(buff_len < cnt)
         storage[buff_len++] = e;
      terminate();
   }

   /**
    * @return Returns the number of elements stored in this buffer.
    */
   size_t getLen() const { return buff_len; }
   size_t length() const { return buff_len; }

   /**
    * @return Returns the element at the specified subscript.
    *
    * @param i Specifies the subscript.
    */
   T &operator [](size_t i)
   {
      if(i >= buff_len)
         throw std::out_of_range("subscript out of range");
      return storage[i];
   }
   T const &operator [](size_t i) const 
   {
      if(i >= buff_len)
         throw std::out_of_range("subscript out of range");
      return storage[i];
   }

   // @endgroup:

   //@group Manipulation routines
   
   /**
    * Appends the content of the specified array to this buffer.
    *
    * @param buff Specifies the start of the array to be copied.
    *
    * @param len Specifies the number of elements to copy.
    */
   void append(T const *buff, size_t len)
   {
      grow(len);
      copyElems(storage + buff_len,buff,len);
      buff_len += len;
      terminate();
   }

   /**
    * Appends the contents of the other buffer to this buffer.
    *
    * @param buff Specifies the buffer to copy.
    */
   void append(DynBuff const &buff)
   { append(buff.storage,buff.buff_len); }

   /**
    * Appends the specified element to this buffer.
    */
   void append(T const &element)
   { append(&element,1); }

   /**
    * Replaces the elements specified at the given starting position and range with the content of
    * the specified array.
    *
    * @param buff Specifies the buffer to insert.
    *
    * @param len Specifies the length of the buffer.
    *
    * @param pos Specifies the position where the replacement should take place.
    */
   void replace(T const *buff, size_t len, size_t pos)
   {
      // check the bounds of the elements that should be replaced
      if(pos + len > buff_len)
         throw std::out_of_range("Invalid replacement range");
      copyElems(storage + pos,buff,len);
   }

   /**
    * Inserts the specified array contents at the specified position.
    *
    * @param buff Specifies the start of the source array.
    *
    * @param len Specifies the number of elements in the source array.
    *
    * @param pos Specifies the position where the source array will be inserted.
    */
   void insert(T const *buff, size_t len, size_t pos)
   {
      // first check the bounds for the starting position
      if(pos > buff_len)
         throw std::bad_alloc();
      
      // check to see if this operation can decay into an append
      if(pos == buff_len)
      {
         append(buff,len);
         return;
      }
      
      // At this point, we know that space has to be opened up in the buffer.
      grow(len);
      for(size_t i = buff_len; i >= pos; i--)
      {
         storage[i + len] = storage[i];
         if(i == pos)              // This check is made because size_t is an unsigned type
            break;
      }
      copyElems(storage + pos,buff,len);
      buff_len += len;
      terminate();
   }

   /**
    * Removes the content of this buffer starting at the specified offset up to the specified
    * length.
    *
    * @param start Specifies the starting position of data to be cut.
    *
    * @param len Specifies the maxumum number of elements that will be removed.
    */
   void cut(size_t start, size_t len)
   {
      // we do not throw a bounds exception in this method if the positions fall outside of the buffer
      // bounds because the method has fulfilled its goal if these positons are out of bound
      if(start >= buff_len || len == 0)
         return;
      if(start + len > buff_len)
         len = buff_len - start;
      
      // copy the remnants of the buffer to the beginning of the marked block
      copyElems(storage + start, storage + start + len,buff_len - (start + len));
      buff_len -= len;
      terminate();
   }

   /**
    * Removes the content of this buffer from the specified position to the end of the string.
    *
    * @param start Specifies the position to start cutting.
    */
   void cut(size_t start)
   {
      if(start >= 0 && start < buff_len)
         buff_len = start;
      terminate();
   }

   /**
    * Reverses the elements of this buffer in place.
    */
   void reverse()
   {
      T elem;
      size_t i, j;
      for(i = 0, j = buff_len - 1; buff_len > 0 && i < j; i++, j--)
      {
         elem = storage[i];
         storage[i] = storage[j];
         storage[j] = elem;
      }
      terminate();
   }

   /**
    * Extracts a substring from this buffer.
    *
    * @param dest Specifies the buffer to which the substring will be written.
    *
    * @param start Specifies the starting location of the substring.
    *
    * @param len Specifies the maximum number of bytes to copy.
    */
   void sub(DynBuff &dest, size_t start, size_t len) const
   {
      dest.cut(0);
      if(start < buff_len)
      {
         if(start + len > buff_len)
            len = buff_len - start;
         dest.setContents(storage + start,len);
      }
   }

   /**
    * Transforms this buffer to all upper case characters.
    */
   void to_upper()
   {
      for(size_t i = 0; i < buff_len; ++i)
         storage[i] = std::toupper(storage[i]);
   }

   /**
    * Transaforms the content in this buffer to all lower case characters.
    */
   void to_lower()
   {
      for(size_t i = 0; i < buff_len; ++i)
         storage[i] = std::tolower(storage[i]);
   }

   // @endgroup:

   
   // @group: Concatenation operators
   
   /**
    * Overloads the operator to concatenate an element.
    */
   DynBuff &operator +=(T const &e)
   { append(e);  return *this;   }

   /**
    * Overloads the operator to concatenate the contents of the other buffer.
    */
   DynBuff &operator +=(DynBuff const &other)
   { append(other); return *this; }
   // @endgroup: operators

   // @group: Search routines

   /**
    * @return Returns the results of comparing characters.
    *
    * @param e1 Specifies the first character to compare.
    *
    * @param e2 Specifies the second character to compare.
    *
    * @param case_sensitive Set to true if both characters should be transformed to upper case
    * before being compared.
    *
    * @return Returns zero if the characters are equal, -1 id e1 < e2, or +1 if e1 > e2.
    */
   static int ecompare(char e1, char e2, bool case_sensitive)
   {
      int rtn = 0;
      if(case_sensitive)
      {
         if(e1 == e2)
            rtn = 0;
         else if(e1 < e2)
            rtn = -1;
         else
            rtn = 1;
      }
      else
      {
         if(std::toupper(e1) == std::toupper(e2))
            rtn = 0;
         else if(std::toupper(e1) < std::toupper(e2))
            rtn = -1;
         else
            rtn = 1;
      }
      return rtn;
   }
   static int ecompare(wchar_t e1, wchar_t e2, bool case_sensitive)
   {
      int rtn(0);
      if(case_sensitive)
      {
         if(e1 == e2)
            rtn = 0;
         else if(e1 < e2)
            rtn = -1;
         else
            rtn = 1;
      }
      else
      {
         wchar_t u1(std::towupper(e1));
         wchar_t u2(std::towupper(e2));
         if(u1 == u2)
            rtn = 0;
         else if(u1 < u2)
            rtn = -1;
         else
            rtn = 1;
      }
      return rtn;
   }
   static int ecompare(unsigned char e1, unsigned char e2, bool case_sensitive)
   {
      int rtn(0);
      if(case_sensitive)
      {
         if(e1 == e2)
            rtn = 0;
         else if(e1 < e2)
            rtn = -1;
         else
            rtn = 1;
      }
      else
      {
         if(std::toupper(e1) == std::toupper(e2))
            rtn = 0;
         else if(std::toupper(e1) < std::toupper(e2))
            rtn = -1;
         else
            rtn = 1;
      }
      return rtn;
   }

   /**
    * Searches forward in the buffer for the first occurrence of the specified pattern after the
    * specified starting position.
    *
    * @return Returns the position of the located sub-string in the buffer or a value greater than
    * the length if the substring wes not found.
    *
    * @param pattern Specifies the start of the pattern buffer.
    *
    * @param len Specifies the length of the pattern buffer.
    *
    * @param start Specifies the location to start the search.
    *
    * @param case_sensitive Set to true if all pattern and buffer content characters should be
    * converted to upper case before being compare.
    */
   size_t find(T const *pattern, size_t len, size_t start = 0, bool case_sensitive = false) const
   {
      // if the pattern is empty or longer than this buffer, no match can exist
      if(len == 0 || start + len > buff_len)
         return buff_len;
      
      // use a naive algorithm to check for the pattern at each position in the buffer
      size_t rtn = buff_len;
      for(size_t i = start; i + len <= buff_len; i++)
      {
         size_t j;
         for(j = 0; j < len && ecompare(storage[i + j], pattern[j], case_sensitive) == 0; j++)
            0;
         if(j >= len)
         {
            rtn = i;
            break;
         }
      }
      return rtn;
   }

   /**
    * Searches backward in the buffer for this first occurrence of the specified pattern before the
    * given start position.
    *
    * @return Returns the position of the located substring or a value greater than or equal to the
    * string length if the substring was not found.
    *
    * @param pattern Specifies the start of the pattern to search.
    *
    * @param len Specifies the length of the patterm array.
    *
    * @param start_pos Specifies the starting position of the search.
    *
    * @param case_sensitive Set to true if the pattern characters and buffer characters should be
    * converted to upper case before comparison.
    */
   size_t findRev(
      T const *pattern,
      size_t len,
      size_t start_pos = 0xffffffff,
      bool case_sensitive = false) const
   {
      // There can be no match if the pattern will not fit into the buffer or has no length
      if(start_pos > buff_len)
         start_pos = buff_len;
      if(len == 0 || len > start_pos)
         return buff_len;
      
      // search for the pattern
      size_t rtn = buff_len;
      for(size_t i = start_pos - len; i >= 0; i--)
      {
         size_t j;
         for(j = 0; j < len && ecompare(storage[i + j], pattern[j], case_sensitive) == 0; j++)
            0;
         if(j >= len)
         {
            rtn = i;
            break;
         }
         if(i == 0)
            break;
      }
      return rtn;
   }
   
   // @endgroup:

   /**
    * Ensures that the buffer has an allocation of at least the specified size.
    *
    * @param req Specifies the minimum allocation requirement.
    */
   void reserve(size_t req)
   {
      size_t alloc_spec = req + spec_additional;
      if(alloc_spec > alloc_len)
         grow(req - buff_len + 1);
   }

   // @group: Comparison
   
   /**
    * Compares the contents of this buffer with the specified array.
    *
    * @return Returns zero if this buffer contents and the array order the same, a positive value if
    * this buffer orders greater than the array, and a negative value if this buffer orders less
    * than the array.
    *
    * @param buff Specifies the start of the comparison buffer.
    *
    * @param len Specifies the length of the comparison buffer.
    *
    * @param case_sensitive Set to true if the characters from this buffer and the comparison array
    * should be converted to upper case before being compared.
    */
   int compare(T const *buff, size_t len, bool case_sensitive) const
   {
      // cover the case when one or more of the lengths is zero
      int rtn = 0;
      if(buff_len == 0 && len != 0)
         rtn = -1;
      else if(buff_len != 0 && len == 0)
         rtn = 1;
      else if(buff_len == 0 && len == 0)
         rtn = 0;                  // empty strings are equal
      else
      {
         // compare over the minumum length
         size_t min_len = buff_len;
         if(min_len > len)
            min_len = len;
         for(size_t i = 0; i < min_len && rtn == 0; i++)
            rtn = ecompare(storage[i], buff[i], case_sensitive);
         if(len != buff_len && rtn == 0)
         {
            if(buff_len < len)
               rtn = -1;
            else
               rtn = +1;
         }
      }
      return rtn;
   }

   /**
    * Compares the contents of this buffer against the content of the other buffer.
    *
    * @return Returns zero if this buffer orders the same as the other buffer, a positive value if
    * this buffer orders greater than the other buffer, or a negative value if this buffer orders
    * less than the other buffer.
    *
    * @param other Specifies the buffer to compare.
    *
    * @param case_sensitive Set to true if this buffer characters and the other buffer characters
    * should be converted to upper case before being compared.
    */
   int compare(DynBuff<T, spec_additional> const &other, bool case_sensitive) const
   { return compare(other.storage, other.buff_len, case_sensitive); }

   // @endgroup:

   // @group: File I/O operations

   /**
    * Reads up to the specified number of elements from the specified file.
    *
    * @param in Specifies the file handle to read.
    *
    * @param len Specifies the maximum number of values to read.
    */
   void readFile(FILE *in, size_t len)
   {
      reserve(len);
      buff_len = fread(storage,sizeof(T),len,in);
      terminate();
   }

   /**
    * Reads the number of elements to be read from the file as a four byte binary integer (stored in
    * the local machine order) and then reads that number of elements from the file.
    *
    * @param in Specifies the file handle to read.
    */
   void readFile(FILE *in)
   {
      // read the length
      uint4 bytes;
      if(fread(&bytes, sizeof(bytes), 1, in) == 0)
         throw fileio_error();
      
      // now try to read the rest of the file
      size_t new_len = bytes/sizeof(T);
      
      reserve(new_len);
      if(new_len && fread(storage,sizeof(T),new_len,in) != new_len)
         throw fileio_error();
      buff_len = new_len;
      terminate();
   }

   /**
    * Writes the number of elements in this buffer to the file as a four byte integer expressed in
    * the native machine order followed by the contents of this buffer.
    *
    * @param out Specifies the file handle to which the data will be written.
    */
   void writeFile(FILE *out) const
   {
      // write the number of bytes required for storing the buffer
      uint4 bytes = buff_len*sizeof(T);
      if(fwrite(&bytes,sizeof(bytes),1,out) == 0)
         throw fileio_error();
      
      // write the elements to the file
      if(buff_len && fwrite(storage,sizeof(T),buff_len,out) != buff_len)
         throw fileio_error();
   }

   // @endgroup:

   /**
    * @return Returns the first element of this buffer or zero if this buffer is empty.
    */
   T first() const
   {
      T rtn = 0;
      if(buff_len >= 1)
         rtn = storage[0];
      return rtn;
   }

   /**
    * @return Returns the last element of this buffer or zero if this buffer is empty.
    */
   T last() const
   {
      T rtn = 0;
      if(buff_len > 0)
         rtn = storage[buff_len - 1];
      return rtn;
   }

protected:
   /**
    * Can be overloaded in a derived class to add any required termination characters beyond the
    * buffer length.
    */
   virtual void terminate()
   { }

   /**
    * @return Returns the required amount of data required beyond the array buffer needed for
    * termination.
    */
   virtual size_t specAdditional() const
   { return 0; }

   /**
    * Allows a derived class to control the method for moving data between arrays.  For native data
    * types, this can generally be done faster using calls like memcpy.  This version performs the
    * "safe" algorithm of simply copying each element of the buffers one ata time.
    *
    * @param dest Specifies the start of the destination buffer.
    *
    * @param src Specifies the start of th source buffer.
    *
    * @param len Specifies the number of elements to copy.
    */
   virtual void copyElems(T *dest, T const *src, size_t len) const
   {
      for(size_t i = 0; i < len; i++)
         dest[i] = src[i];
   }

protected:
   /**
    * Specifies the actual number of elements stored in this buffer.
    */
   size_t buff_len;

   /**
    * Specifies the number of bytes that have been allocated for this buffer.
    */
   size_t alloc_len;

   /**
    * Reference to the current storage location for this buffer.
    */
   T *storage;

   /**
    * Specifies a fixed array that will be used to storing small strings.
    */
   T fixed_storage[16];

   /**
    * Implements the algorithm needed to increase the allocation.
    */
   void grow(size_t delta)
   {
      // if there is enough space, we can return without doing anything
      if(delta == 0 || delta + buff_len + spec_additional <= alloc_len)
         return;
      
      // Determine what the new size will be and allocate a new buffer
      size_t additional = alloc_len;
      T *new_storage;
      
      if(delta + spec_additional > additional)
         additional = delta + spec_additional;
      new_storage = new T[alloc_len + additional];
      if(new_storage == 0)
         throw std::bad_alloc();   // Just in case Micro$loth sets it to null instead of throwing
      if(storage)
      {
         copyElems(new_storage,storage,buff_len);
         if(storage != fixed_storage)
            delete[] storage;
      }
      storage = new_storage;
      alloc_len = alloc_len + additional;
      terminate();
   }
};


#endif
