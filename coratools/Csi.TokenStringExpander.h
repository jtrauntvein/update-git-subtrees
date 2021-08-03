/* Csi.TokenExpander.h

   Copyright (C) 2001, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Wednesday 30 May 2001
   Last Change: Wednesday 30 May 2001
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Csi_TokenExpander_h
#define Csi_TokenExpander_h

#include "StrAsc.h"
#include "Csi.SharedPtr.h"
#include <list>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class TokenExpander
   ////////////////////////////////////////////////////////////
   class TokenExpander
   {
   public:
      ////////////////////////////////////////////////////////////
      // expand
      //
      // Derived classes should overload this method to examine the token and write the expansion
      // for a matched token. The return value will be non-zero if the token was replaced and should
      // indicate the number of characters from toke_start were used. If the token was not
      // replaced, the return value should be zero.
      //////////////////////////////////////////////////////////// 
      virtual size_t expand(StrAsc &destination, char const *token_start) = 0;
   };


   ////////////////////////////////////////////////////////////
   // class TokenStringExpander
   //
   // Defines a concrete class that can accept an arbitrary number of TokenExpander based
   // objects. This class defines a method, expand(), that takes a source string and copies its
   // contents to a destination buffer with all of the recognised tokens expanded. The application
   // can add an arbitrary number of expanders by invoking add_expander().
   //
   // This class will use the percent sign character ('%') as the marker for a token. The
   // character(s) following the marker will be passed to the set of expanders. By default, all "%%"
   // sequences will be replaced with a single perecent sign. If there is no expansion for a token,
   // the percent sign and subsequent characters will be inserted into the destination.
   ////////////////////////////////////////////////////////////
   class TokenStringExpander
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      TokenStringExpander();

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      ~TokenStringExpander();

      ////////////////////////////////////////////////////////////
      // add_expander
      //
      // Adds an expander object to the set managed by this object. The next time that expand() is
      // called, the expander will be consulted. The order in which expanders is added is
      // significant. The first expander added will always have first crack at expanding tokens. 
      ////////////////////////////////////////////////////////////
      typedef Csi::SharedPtr<TokenExpander> expander_handle;
      void add_expander(expander_handle &expander);

      ////////////////////////////////////////////////////////////
      // expand
      //
      // Using the list of expanders currently defined for this object, this method will copy the
      // source buffer to the destination buffer while expanding recognised tokens. The return value
      // will be the pointer to the beginning of the destination buffer.
      ////////////////////////////////////////////////////////////
      char const *expand(StrAsc &destination, char const *source);

   private:
      ////////////////////////////////////////////////////////////
      // expanders
      ////////////////////////////////////////////////////////////
      typedef std::list<expander_handle> expanders_type;
      expanders_type expanders;
   };


   ////////////////////////////////////////////////////////////
   // class CharToStringExpander
   //
   // Implements a common expansion where a single character is replaced with a statically specified
   // string.
   ////////////////////////////////////////////////////////////
   class CharToStringExpander: public TokenExpander
   {
   private:
      ////////////////////////////////////////////////////////////
      // identifier
      //
      // The character identifier following the '%' sign that will trigger the replacement
      ////////////////////////////////////////////////////////////
      char identifier;

      ////////////////////////////////////////////////////////////
      // replacement
      //
      // The string that will be replaced
      ////////////////////////////////////////////////////////////
      StrAsc replacement;

   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      CharToStringExpander(char identifier_, StrAsc const &replacement_):
         identifier(identifier_),
         replacement(replacement_)
      { }

      ////////////////////////////////////////////////////////////
      // expand
      ////////////////////////////////////////////////////////////
      virtual size_t expand(StrAsc &destination, char const *source)
      {
         size_t rtn = 0;
         if(*source == identifier)
         {
            rtn = 1;
            destination += replacement;
         }
         return rtn;
      }
   };
};


#endif
