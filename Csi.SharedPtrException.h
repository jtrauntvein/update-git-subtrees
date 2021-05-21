/* Csi.SharedPtrException.h

   Copyright (C) 2014, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 21 July 2014
   Last Change: Monday 18 August 2014
   Last Commit: $Date: 2014-08-18 13:19:41 -0600 (Mon, 18 Aug 2014) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SharedPtrException_h
#define Csi_SharedPtrException_h

#include <stdexcept>
#include <typeinfo>
#include <assert.h>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // template class SharedPtrException
   ////////////////////////////////////////////////////////////
   template <class T>
   class SharedPtrException: public std::exception
   {
   private:
      ////////////////////////////////////////////////////////////
      // message
      ////////////////////////////////////////////////////////////
      char message[256];

      ////////////////////////////////////////////////////////////
      // message_len
      ////////////////////////////////////////////////////////////
      int message_len;

      ////////////////////////////////////////////////////////////
      // append_str
      ////////////////////////////////////////////////////////////
      void append_str(char const *s)
      {
         for(int i = 0; s[i] != 0 && message_len < sizeof(message) - 1; ++i)
            message[message_len++] = s[i];
      }
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      SharedPtrException():
         message_len(0)
      {
         append_str("attempt to reference null pointer of type ");
         append_str(typeid(T).name());
         message[message_len] = 0;
#ifdef DEBUG
         assert(false);
#endif
      }
      
      ////////////////////////////////////////////////////////////
      // what
      ////////////////////////////////////////////////////////////
      virtual char const *what() const throw ()
      { return message; }
   };
};


#endif
