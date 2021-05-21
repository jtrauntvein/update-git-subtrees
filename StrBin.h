/* StrBin.h

   Copyright (C) 1998, 2017 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Wednesday 16 September 1998
   Last Change: Monday 06 November 2017
   Last Commit: $Date: 2017-11-06 13:42:32 -0600 (Mon, 06 Nov 2017) $ 
   Committed by: $Author: jon $
   
*/

#ifndef StrBin_h
#define StrBin_h

#include "CsiTypeDefs.h"
#include "DynBuff.h"


/**
 * Declares a dynamically expandable buffer suitable for storing binary data.
 */
class StrBin: public DynBuff<byte, 0>
{
public:
   /**
    * Default constructor will create an empty string.
    */
   StrBin()
   { }

   /**
    * Copy constructor
    */
   typedef DynBuff<byte, 0> base_class_type;
   StrBin(StrBin const &other):
      base_class_type(other)
   { }

   ////////////////////////////////////////////////////////////
   // memory copy constructor
   ////////////////////////////////////////////////////////////
   /**
    * Construct from memory contents.
    *
    * @param buff  Specifies the beginning memory location to copy.
    *
    * @param Specifies the number of bytes to copy from the beginning memory location.
    */
   StrBin(void const *buff, size_t len)
   { base_class_type::setContents((byte const *)buff,len); }

   /**
    * Copy operator
    */
   StrBin &operator =(StrBin const &other)
   { setContents(other.getContents(),other.length()); return *this; }

   /**
    * @return Returns a pointer to the start of the storage buffer owned by this object.
    */
   char const *getContents() const 
   { return (char const *)base_class_type::getContents(); }

   /**
    * Sets the contents of this object's storage buffer to be equal to the provided buffer and
    * length.
    *
    * @param buff  Specifies the beginning address of the buffer to be copied.
    *
    * @param len  Specifies the number of bytes to copy.
    */
   void setContents(void const *buff, size_t len)
   { base_class_type::setContents((byte const *)buff,len); }

   //@group: Manipulation

   /**
    * Inserts the data specified by buff into the specified position of the memory held by this
    * object.
    *
    * @param buff  Specifies the beginning pointer to the memory to be copied.
    *
    * @param len Specifies the number of bytes to insert.
    *
    * @param pos Specifies the position in this buffer where the memory is to be inserted.
    */
   void insert(void const *buff, size_t len, size_t pos)
   { base_class_type::insert((byte const *)buff,len,pos); }

   /**
    * Copies the specified memory contents to the buffer owned by this object.
    *
    * @param buff Specifies the beginning memory address to copy.
    *
    * @param len Specifies the number of bytes to copy.
    */
   void append(void const *buff, size_t len)
   { base_class_type::append((byte const *)buff,len); }

   /**
    * Appends the specified character to the buffer owned by this object.
    *
    * @param e Specifies the data to append.
    */
   void append(byte e)
   { append(&e,1); }

   /**
    * Replaces the memory within the buffer owned by this object with the contents of the specified
    * buffer.
    *
    * @param buff Specifies the beginning memory address to copy.
    *
    * @param len Specifies the number of bytes to copy.
    *
    * @param pos Specifies the starting position within this object's buffer.
    */
   void replace(void const *buff, size_t len, size_t pos)
   { base_class_type::replace((byte const *)buff,len,pos); }

   /**
    * Sets the length of the buffer owned bu this object to the specifies length and fills that
    * memory with zeroes.
    *
    * @param len Specifies the new length of this buffer.
    */
   void clear(size_t len)
   {
      reserve(len);
      buff_len = len;
      if(len > 0)
         memset(storage, 0, len);
   }
   
   // @endgroup:
   

   //@group searching

   /**
    * Searches for the specified pattern of bytes within the buffer owned by this object.
    *
    * @return Returns the byte offset of the beginning of the pattern or a value greater or equal to
    * the buffer's length if the pattern was not found.
    *
    * @param pattern Specifies the beginning memory address of the search pattern.
    *
    * @param len Specifies the number of bytes in the pattern
    *
    * @param start_pos  Specifies the offset within this object's buffer where the search will
    * begin.
    *
    * @param case_sensitive  Set to true if the pattern matching should be case sensitive.
    */
   size_t find(
      void const *pattern,
      size_t len,
      size_t start_pos = 0,
      bool case_sensitive = true) const
   {
      return base_class_type::find(
         static_cast<byte const *>(pattern), len, start_pos, case_sensitive);
   }

   /**
    * Searches for the specified pattern closest to the end of the buffer owned by this object.
    *
    * @return Returns the offset of the match location or a value greater or equal to the buffer's
    * length if the pattern was not found.
    *
    * @param buff Specifies the starting address of the pattern.
    *
    * @param len Specifies the number of bytes in the pattern.
    *
    * @param case_sensitive Set to true if the pattern matching should be case sensitive.
    */
   size_t findRev(void const *pattern, size_t len, bool case_sensitive = true) const
   {
      return base_class_type::findRev(
         static_cast<byte const *>(pattern), len, buff_len, case_sensitive);
   }
   
   // @endgroup:
   
   /**
    * @return Compares the buffer owned by this object with the buffer owned by another. Returns 0
    * if both buffers are equal, a negative value if this buffer sorts less than the other, and a
    * positive value if this buffer sorts greater than the other.
    *
    * @param other  Specifes the other object to compare.
    *
    * @param case_sensitive Set to true if the case of buffer characters matters when being
    * compared.
    */
   int compare(StrBin const &other, bool case_sensitive = true) const
   { return base_class_type::compare(other, case_sensitive); }

   // @group: comparison operators
   
   bool operator ==(StrBin const &other) const
   { return buff_len == other.buff_len && compare(other) == 0; }

   bool operator !=(StrBin const &other) const
   { return buff_len != other.buff_len || compare(other) != 0; }

   bool operator <(StrBin const &other) const
   { return compare(other) != 0; }

   bool operator <=(StrBin const &other) const
   { return compare(other) <= 0; }

   bool operator >(StrBin const &other) const
   { return compare(other) > 0; }

   bool operator >=(StrBin const &other) const
   { return compare(other) >= 0; }

   // @endgroup:
};


/**
 * Overload of the addition operator to concatenate two binary strings.
 */
inline StrBin operator +(StrBin const &s1, StrBin const &s2)
{ StrBin rtn(s1); rtn += s2; return rtn; }

#endif
