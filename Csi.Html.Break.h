/* Csi.Html.Break.h

   Copyright (C) 2007, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 March 2007
   Last Change: Tuesday 20 November 2012
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Html_Break_h
#define Csi_Html_Break_h

#include "Csi.Html.Tag.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Break
      ////////////////////////////////////////////////////////////
      class Break: public Tag
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Break()
         { }

         ////////////////////////////////////////////////////////////
         // before_render
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out)
         {
            out << "<br>\n";
            return true;
         }
         virtual bool before_render(std::wostream &out)
         {
            out << L"<br>\n";
            return true;
         }

         ////////////////////////////////////////////////////////////
         // after_render
         ////////////////////////////////////////////////////////////
         virtual void after_render(std::ostream &out)
         { }
         virtual void after_render(std::wostream &out)
         { }
      };
   };
};


#endif
