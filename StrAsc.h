/* StrAsc.h

   Copyright (C) 1998, 2014 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Friday 18 September 1998
   Last Change: Wednesday 10 December 2014
   Last Commit: $Date: 2014-12-10 15:41:44 -0600 (Wed, 10 Dec 2014) $ 
   Committed by: $Author: jon $
   
*/

#ifndef StrAsc_h
#define StrAsc_h

#include <iosfwd>
#include "TermStr.h"

////////// class StrAsc
// Provides dynamically sized storage for nul-terminated ASCII strings. Will guarantee the presence
// of the ASCII terminator at the end of the string at all times
class StrAsc: public TermStr<char>
{
public:
   ////////// default constructor
   StrAsc();

   ////////// copy constructor
   StrAsc(StrAsc const &other);

   ////////// array constructor
   StrAsc(char const *buff);

   ////////////////////////////////////////////////////////////
   // fixed length buffer constructor
   ////////////////////////////////////////////////////////////
   StrAsc(char const *buff, size_t buff_len);

   ////////// copy operator
   StrAsc &operator =(StrAsc const &other);

   ////////// copy array operator
   StrAsc &operator =(char const *buff);

   //@group Ascii File input operations
   ////////// readLine
   // Reads a line of text in from the current position in the file until end of file or the next
   // end of line is encountered.
   void readLine(FILE *in);

   ////////// readLine
   // Reads a line of text from an input stream object rather than a file handle
   void readLine(std::istream &in);

   ////////// readLine
   // Reads a token from the input stream. Skips preceding whitespace and reads until whitespace or
   // the end of file is encountered. Functions similarly to scanf("%s")
   void readToken(std::istream &in);
   //@endgroup

   //group Manipulation
   ////////// append array
   StrAsc &operator +=(char const *s)
   { append(s); return *this; }

   ////////// append other
   StrAsc &operator +=(StrAsc const &other)
   { append(other.c_str(),other.length()); return *this; }

   ////////// append element
   StrAsc &operator +=(char ch)
   { append(ch); return *this; }

   ////////// encodeHex
   // Encodes the bytes specified in the array into hexadecimal numbers
   void encodeHex(void const *buff, size_t len, bool use_sep = true);
   //@endgroup
};

////////// stream input operator
std::istream &operator >>(std::istream &in, StrAsc &buff);

////////// stream output operator
std::ostream &operator <<(std::ostream &out, StrAsc const &buff);

////////////////////////////////////////////////////////////
// wide stream output
////////////////////////////////////////////////////////////
std::wostream &operator <<(std::wostream &out, StrAsc const &buff);

////////// binary concatenation
inline StrAsc operator +(StrAsc const &s1, StrAsc const &s2)
{ StrAsc rtn(s1); rtn += s2; return rtn; }

#endif
