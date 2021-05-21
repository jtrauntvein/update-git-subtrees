/* Csi.Html.DateTag.h

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 09 May 2000
   Last Change: Tuesday 20 November 2012
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Html_DateTag_h
#define Csi_Html_DateTag_h

#include "Html.Tag.h"
#include "../coratools/LgrDate.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class DateTag
      //
      // Represents an object that will format a time stamp when rendered
      ////////////////////////////////////////////////////////////
      class DateTag: public Tag
      {
      protected:
         ////////////////////////////////////////////////////////////
         // stamp
         ////////////////////////////////////////////////////////////
         LgrDate stamp;

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         StrAsc format;
      
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DateTag(LgrDate const &stamp_, char const *format_ = "%Y%m%d %H:%M:%S%x"):
            stamp(stamp_),
            format(format_)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DateTag() { }

         ////////////////////////////////////////////////////////////
         // before_render
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out)
         { return true; }
         virtual bool before_render(std::wostream &out)
         { return true; }
         
         ////////////////////////////////////////////////////////////
         // render
         ////////////////////////////////////////////////////////////
         virtual void render(std::ostream &out)
         { stamp.format(out,format.c_str()); }
         virtual void render(std::wostream  &out)
         { stamp.format(out, format.c_str()); }

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
