/* CrBasic.Constants.h

   Copyright (C) 2013, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 20 March 2013
   Last Change: Monday 25 March 2013
   Last Commit: $Date: 2013-03-25 15:09:01 -0600 (Mon, 25 Mar 2013) $
   Last Changed by: $Author: jon $

*/

#ifndef CrBasic_Constants_h
#define CrBasic_Constants_h

#include "StrAsc.h"
#include "CsiTypeDefs.h"
#include <list>


namespace CrBasic
{
   ////////////////////////////////////////////////////////////
   // struct Constant
   //
   // Defines a structure that holds information parsed from a CRBasic program. 
   ////////////////////////////////////////////////////////////
   struct Constant
   {
   public:
      ////////////////////////////////////////////////////////////
      // name
      //
      // Specifies the name of this constant.
      ////////////////////////////////////////////////////////////
      StrAsc name;

      ////////////////////////////////////////////////////////////
      // value
      // Specifies the value parsed for this constant
      ////////////////////////////////////////////////////////////
      StrAsc value;

      ////////////////////////////////////////////////////////////
      // start_pos
      //
      // Specifies the position from the original stream start offset where the
      // declaration for this constant began.
      ////////////////////////////////////////////////////////////
      int8 start_pos;

      ////////////////////////////////////////////////////////////
      // length
      //
      // Specifies the number of bytes occupied by this constant from its start
      // position to its end. 
      ////////////////////////////////////////////////////////////
      int8 length;

      ////////////////////////////////////////////////////////////
      // has_changed
      ////////////////////////////////////////////////////////////
      bool has_changed;

      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      Constant(
         StrAsc const &name_ = "",
         StrAsc const &value_ = "",
         int8 start_pos_ = -1,
         int8 length_ = -1):
         name(name_),
         value(value_),
         start_pos(start_pos_),
         length(length_),
         has_changed(false)
      { }

      ////////////////////////////////////////////////////////////
      // copy constructor
      ////////////////////////////////////////////////////////////
      Constant(Constant const &other):
         name(other.name),
         value(other.value),
         start_pos(other.start_pos),
         length(other.length),
         has_changed(other.has_changed)
      { }

      ////////////////////////////////////////////////////////////
      // copy operator
      ////////////////////////////////////////////////////////////
      Constant &operator = (Constant const &other)
      {
         name = other.name;
         value = other.value;
         start_pos = other.start_pos;
         length = other.length;
         has_changed = other.has_changed;
         return *this;
      }
   };


   ////////////////////////////////////////////////////////////
   // parse_constants
   //
   // Parses the program contained in the input stream, in, and outputs the
   // constants to the constants list.
   ////////////////////////////////////////////////////////////
   typedef std::list<Constant> constants_type;
   void parse_constants(constants_type &constants, std::istream &in);

   
   ////////////////////////////////////////////////////////////
   // output_constants
   //
   // Writes the program using the original source (provided by an input
   // stream), the list of constants to be output, and an output stream.
   ////////////////////////////////////////////////////////////
   void output_constants(
      std::ostream &out,
      std::istream &in,
      constants_type const &constants);
};


#endif
