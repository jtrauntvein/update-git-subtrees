/* TermStr.h

   Copyright (C) 1998, 2020 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Friday 18 September 1998
   Last Change: Monday 20 July 2020
   Last Commit: $Date: 2020-07-20 09:00:48 -0600 (Mon, 20 Jul 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef TermStr_h
#define TermStr_h

#include "DynBuff.h"
#include "Csi.MaxMin.h"
#include <string.h>
#include <vector>
#include <algorithm>


/**
 * Defines a template for a string terminated by a nul character.
 */
template <class T>
class TermStr: public DynBuff<T, 1>
{
protected:
   typedef DynBuff<T, 1> base_class_type;
   
public:
   /**
    * Default constructor
    */
   TermStr()
   { }

   /**
    * Creates a copy of another string.
    *
    * @param other Specifies the string to copy.
    */
   TermStr(TermStr const &other):
      base_class_type(other.storage, other.buff_len)
   { }

   /**
    * Copy from a nul teminated c string pointer.
    *
    * @param buff Specifies the begin address of the c string to copy.
    */
   TermStr(T const *buff):
      base_class_type(buff, termArrayLen(buff))
   { }

   /**
    * Copy from a character array with a specified length.
    *
    * @param buff Specifies the beginning of the character array.
    *
    * @param buff_len Specifies the number of characters to copy.
    */
   TermStr(T const *buff, size_t buff_len):
      base_class_type(buff, buff_len)
   { terminate(); }

   /**
    * Copy operator
    *
    * @param other Specifies the string to copy.
    */
   TermStr &operator =(TermStr const &other)
   { setContents(other.storage, other.buff_len); }

   /**
    * Nul terminated assignment operator.
    *
    * @param buff Specifies a nul terminated character array to copy.
    */
   TermStr &operator =(T const *buff)
   { setContents(buff, termStrLen(buff)); }

   //@group access methods

   /**
    * @return Returns the character at the specified position.
    *
    * @param i Specifies the position of the selected character.
    */
   T charAt(size_t i) const
   {
      if(i >= this->length())
         throw std::out_of_range("Invalid charAt subscript");
      return this->storage[i];
   }

   /**
    * @return Returns a nul terminated c string.
    */
   T const *c_str() const
   {
      static T empty = 0;
      T const *rtn = &empty;
      if(this->buff_len)
         rtn = this->storage;
      return rtn;
   }
   
   //@endgroup

   //@group extended manipulation routines

   /**
    * Append a character to this string.
    *
    * @param e Specifies the character to append.
    */
   void append(T const &e)
   { base_class_type::append(e); }

   /**
    * Append a character array with known length to this string.
    *
    * @param buff Specifies the beginning of the character array.
    *
    * @param len Specifies the number of characters to copy from buff.
    */
   void append(T const *buff, size_t len)
   { base_class_type::append(buff,len); }
   
   /**
    * Append a nul terminated character array to this string.
    *
    * @param buff Specifies the beginning of a nul terminated character array.
    */
   void append(T const *buff)
   { base_class_type::append(buff, termArrayLen(buff)); }

   /**
    * Append the contents of another string object.
    *
    * @param other Specifies the string to append.
    */
   void append(TermStr const &other) 
   { base_class_type::append(other); }

   /**
    * Inserts the specified nul terminated character array at the specified position.
    *
    * @param buff Specifies the nul terminated character array to insert.
    *
    * @param pos Specifies the position in this string where buff will be inserted.
    */
   void insert(T const *buff, size_t pos)
   { base_class_type::insert(buff, termArrayLen(buff), pos); }

   /**
    * Replaces the contents of this string with those of a nul terminated character array.
    *
    * @param buff Specifies the beginning of a nul terminated character array.
    */
   void setContents(T const *buff)
   { setContents(buff, termArrayLen(buff)); }

   /**
    * Replaces the contents of this string with those of another string object.
    *
    * @param other Specifies the string to copy into this string.
    */
   void setContents(TermStr const &other) 
   { base_class_type::setContents(other); }

   /**
    * Replaces the contents of this string with the content of a character array with known length.
    *
    * @param buff Specifies the beginning of the character array to copy.
    *
    * @param len Specifies the number of characters to copy.
    */
   void setContents(T const *buff, size_t len) 
   {
      // since this is a terminated string, we will not copy anything past a
      // terminator in the source string
      for(size_t i = 0; i < len; ++i)
      {
         if(buff[i] == 0)
         {
            len = i;
            break;
         }
      }
      base_class_type::setContents(buff,len);
   } 
   //@endgroup

   // Extended comparison against terminated arrays

   /**
    * Compares this string against the specified nul terminated character array.
    *
    * @param buff Specifies the beginning of a nul terminated character array.
    *
    * @param case_sensitive Set to false if all compared characters are first converted to their
    * upper case equivalents before being compared.
    *
    * @return Returns a negative value if buff is less than this string, zero if buff is equal, or a
    * positive value if buff is greater than this string.
    */
   int compare_array(T const *buff, bool case_sensitive = false) const
   { return base_class_type::compare(buff, termArrayLen(buff), case_sensitive); }

   /**
    * Compares this string against a nul terminated character array for equality.  Cases of
    * individual characters will be ignored.
    *
    * @param buff Specifies a nul terminated character array.
    *
    * @return Returns true if the upper case version of buff is equal to the upper case value of
    * this string.
    */
   bool operator ==(T const *buff) const
   { return compare_array(buff) == 0; }

   /**
    * Compares this string against a nul terminated character array for inequality.  Cases of
    * individual characters will be ignored.
    *
    * @param buff Specifies a nul terminated character array.
    *
    * @return Returns true if the upper case version of buff is not equal to the upper case value of
    * this string.
    */
   bool operator !=(T const *buff) const
   { return compare_array(buff) != 0; }

   /**
    * Compares this string against a nul terminated character array for inequality.  Cases of
    * individual characters will be ignored.
    *
    * @param buff Specifies a nul terminated character array.
    *
    * @return Returns true if the upper case version of buff is greater than the upper case value of
    * this string.
    */
   bool operator >(T const *buff) const
   { return compare_array(buff) > 0; }

   /**
    * Compares this string against a nul terminated character array for inequality.  Cases of
    * individual characters will be ignored.
    *
    * @param buff Specifies a nul terminated character array.
    *
    * @return Returns true if the upper case version of buff is greater than or equal to the upper
    * case value of this string.
    */
   bool operator >=(T const *buff) const
   { return compare_array(buff) >= 0; }

   /**
    * Compares this string against a nul terminated character array for inequality.  Cases of
    * individual characters will be ignored.
    *
    * @param buff Specifies a nul terminated character array.
    *
    * @return Returns true if the upper case version of buff is less than the upper case
    * value of this string.
    */
   bool operator <(T const *buff) const
   { return compare_array(buff) < 0; }

   /**
    * Compares this string against a nul terminated character array for inequality.  Cases of
    * individual characters will be ignored.
    *
    * @param buff Specifies a nul terminated character array.
    *
    * @return Returns true if the upper case version of buff is less than or equal to the upper case
    * value of this string.
    */
   bool operator <=(T const *buff) const
   { return compare_array(buff) <= 0; }

   //@group Self comparison operations

   /**
    * Compares the content of this string against that of the other string.
    *
    * @param other Specifies the other string to compare.
    *
    * @param case_sensitive Set to false if individual characters are to be converted to their upper
    * case equavalents before they are compared.
    *
    * @return Returns zero if the strings are equal, a negative number if this string is less than
    * other, or a positive value if this string is greater than other.
    */
   int compare(TermStr const &other, bool case_sensitive) const
   {
      return base_class_type::compare(other, case_sensitive);
   }
   
   // These operators have to be declared here because overloading is not supported between scopes

   /**
    * Compares this string against another for equality.
    *
    * @param other Specifies another string to compare.
    *
    * @return Returns true if the upper case version of this string is equal to the upper case
    * version of the other string.
    */
   bool operator ==(TermStr const &other) const
   { return this->length() == other.length() && compare(other, false) == 0; }

   /**
    * Compares this string against another for inequality.
    *
    * @param other Specifies another string to compare.
    *
    * @return Returns true if the upper case version of this string is not equal to the upper case
    * version of the other string.
    */
   bool operator !=(TermStr const &other) const
   { return this->length() != other.length() || compare(other, false) != 0; }

   /**
    * Compares this string against another for inequality.
    *
    * @param other Specifies another string to compare.
    *
    * @return Returns true if the upper case version of this string is less than the upper case
    * version of the other string.
    */
   bool operator <(TermStr const &other) const
   { return compare(other, false) < 0; }

   /**
    * Compares this string against another for inequality.
    *
    * @param other Specifies another string to compare.
    *
    * @return Returns true if the upper case version of this string is less than or equal to the
    * upper case version of the other string.
    */
   bool operator <=(TermStr const &other) const
   { return compare(other, false) <= 0; }

   /**
    * Compares this string against another for inequality.
    *
    * @param other Specifies another string to compare.
    *
    * @return Returns true if the upper case version of this string is greater than the upper case
    * version of the other string.
    */
   bool operator >(TermStr const &other) const
   { return compare(other, false) > 0; }

   /**
    * Compares this string against another for inequality.
    *
    * @param other Specifies another string to compare.
    *
    * @return Returns true if the upper case version of this string is greater than or equal to the
    * upper case version of the other string.
    */
   bool operator >=(TermStr const &other) const
   { return compare(other, false) >= 0; }

   //@endgroup
   //@endgroup

   //@group Pattern match against terminated arrays

   /**
    * Searches this string for the first position of the specified substring.
    *
    * @return Returns the position of the substring if found or a value greater than or equal to the
    * string length if the substring is not present.
    *
    * @param pattern Specifies a nul terminated array that gives the pattern to find.
    *
    * @param startPos Specifies the starting position in this string at which the pattern will be
    * searched.
    *
    * @param case_sensitive Set to false if the characters from this string and th characters from
    * the pattern will be first converted to upper case before being compared.
    */
   size_t find(
      T const *pattern,
      size_t startPos = 0,
      bool case_sensitive = false) const
   { return base_class_type::find(pattern, termArrayLen(pattern), startPos, case_sensitive); }
   
   /**
    * Searches this string for the last position of the specified substring.
    *
    * @return Returns the position of the substring if found or a value greater than or equal to the
    * string length if the substring is not present.
    *
    * @param pattern Specifies a nul terminated array that gives the pattern to find.
    *
    * @param startPos Specifies the starting position in this string at which the pattern will be
    * searched.
    *
    * @param case_sensitive Set to false if the characters from this string and th characters from
    * the pattern will be first converted to upper case before being compared.
    */
   size_t findRev(
      T const *pattern,
      size_t start_pos = 0xffffffff,
      bool case_sensitive = false) const
   { return base_class_type::findRev(pattern, termArrayLen(pattern), start_pos, case_sensitive); }

   /**
    * Searches this string for the last position of the specified substring.
    *
    * @return Returns the position of the substring if found or a value greater than or equal to the
    * string length if the substring is not present.
    *
    * @param pattern Specifies a nul terminated array that gives the pattern to find.
    *
    * @param startPos Specifies the starting position in this string at which the pattern will be
    * searched.
    *
    * @param case_sensitive Set to false if the characters from this string and th characters from
    * the pattern will be first converted to upper case before being compared.
    */
   size_t rfind(
      T const *pattern,
      size_t start_pos = 0xffffffff,
      bool case_sensitive = false) const
   { return findRev(pattern, start_pos, case_sensitive); }

   /**
    * @return Returns true if this string begins with the specified search pattern.
    *
    * @param pattern Specifies the search pattern.
    *
    * @param case_sensitive Set to true if the pattern should be exactly matched.
    */
   bool begins_with(T const *pattern, bool case_sensitive = false) const
   {
      size_t pos(find(pattern, 0, case_sensitive));
      return pos == 0 && this->length() > 0;
   }

   /**
    * @return Returns true if this string ends with the specified search pattern.
    *
    * @param pattern Specifies the search pattern.
    *
    * @param case_sensitive Set to true if the pattern should be matched exactly.
    */
   bool ends_with(T const *pattern, bool case_sensitive = false) const
   {
      size_t pattern_len(termArrayLen(pattern));
      size_t pos(base_class_type::find(pattern, pattern_len, case_sensitive));
      return pos + pattern_len == this->length();
   }

   /**
    * Searches this string for a possible substring using a modified version of Levenshtein's
    * algorithm (see
    * http://ginstrom.com/scribbles/2007/12/01/fuzzy-substring-matching-with-levenshtein-distance-in-python/).
    *
    * @return Returns the total number of edits that would need to be made.  Returns a negative
    * value if the pattern is empty or the position of the substring if the length of the substring
    * is one.
    *
    * @param pattern Specifies a nul terminated character array that specified the pattern to search
    * for.
    *
    * @param case_sensitive Set to true if characters from this string and the substring will be
    * first converted to upper case .
    */
   int fuzzy_find(T const *pattern, bool case_sensitive = false) const
   {
      int rtn(-1);
      size_t hlen(this->length());
      size_t nlen(this->termArrayLen(pattern));
      if(hlen > 0)
      {
         if(nlen > 1)
         {
            std::vector<int> row1(hlen + 1, 0);
            for(size_t i = 0; i < nlen; ++i)
            {
               std::vector<int> row2(1, i + 1);
               for(size_t j = 0; j < hlen; ++j)
               {
                  int const cost(
                     this->ecompare(
                        pattern[i],
                        this->storage[j],
                        case_sensitive) != 0);
                  row2.push_back(
                     Csi::csimin(
                        row1[j + 1] + 1,
                        Csi::csimin(row2[j] + 1, row1[j] + cost)));
               }
               row1.swap(row2);
            }
            rtn = *std::min_element(row1.begin(), row1.end());
         }
         else
            rtn = find(pattern, 0, case_sensitive) < hlen ? 0 : hlen;
      }
      return rtn;
   } 
  
   //@endgroup

   /**
    * Replaces up to max_count instances of the specified substring with the specified replacement
    * string.
    *
    * @param pattern Specifies a nul terminated array that specifies the substring that is to be
    * replaced.
    *
    * @param replacement Specifies a nul terminated array that specified the string with which
    * instances of the substring will be replaced.
    *
    * @param start_pos Specifies the starting position in this string to start searching for
    * potential substrings.
    *
    * @param max_count Specifies the maximum number of replacements that will be permitted.
    *
    * @param case_sensitive Set to false if characters from this string and characters from the
    * pattern will be converted to upper case before being compared.
    *
    * @return Returns the number of substitutions that were made.
    */
   size_t replace(
      T const *pattern,
      T const *replacement,
      size_t start_pos = 0,
      size_t max_count = 0xFFFFFFFF,
      bool case_sensitive = false)
   {
      size_t rtn = 0;
      size_t replace_pos;
      size_t pattern_len = termArrayLen(pattern);
      size_t replacement_len = termArrayLen(replacement);
      
      while((replace_pos = this->find(pattern, start_pos, case_sensitive)) < this->buff_len &&
            max_count > 0)
      {
         this->cut(replace_pos,pattern_len);
         if(replacement_len > 0)
            base_class_type::insert(
               replacement,
               replacement_len,
               replace_pos);
         start_pos = replace_pos + replacement_len;
         ++rtn;
         --max_count;
      }
      return rtn;
   }

protected:
   /**
    * Ensures that the character just after the string length will be set to nul.
    */
   virtual void terminate()
   {
      if(this->storage)
         this->storage[this->buff_len] = 0;
   }

   /**
    * @return Returns 1 in order to ensure that the storage buffer has room for the final
    * terminator.
    */
   virtual size_t specAdditional() const
   { return 1; }

   /**
    * Implements the method that will copy one array to another.
    */
   virtual void copyElems(T *dest, T const *src, size_t len) const
   {
      if(len > 0)
         memmove(dest, src, len * sizeof(T));
   }

   /**
    * Searches the specified buffer for the presence of the nul terminator.
    *
    * @param buff Specifies a nul terminated character array.
    *
    * @return Returns the position of the nul terminator.
    */
   size_t termArrayLen(T const *buff) const
   {
      size_t rtn;
      for(rtn = 0; buff != 0 && buff[rtn] != 0; rtn++)
         0;
      return rtn;
   }
};

#endif
