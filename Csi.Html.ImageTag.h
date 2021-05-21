/* Csi.Html.ImageTag.h

   Copyright (C) 2000, 2007 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 09 May 2000
   Last Change: Thursday 11 January 2007
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Html_ImageTag_h
#define Html_ImageTag_h

#include "Csi.Html.Tag.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class ImageTag
      //
      // Represents an object that will format an image reference when rendered
      ////////////////////////////////////////////////////////////
      class ImageTag: public Tag
      {
      protected:
         ////////////////////////////////////////////////////////////
         // image_name
         ////////////////////////////////////////////////////////////
         StrAsc image_name;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ImageTag(StrAsc const &image_name_):
            image_name(image_name_)
         { }
         
         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ImageTag() { }
         
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
         { out << "<img src=\"" << image_name << "\">\n"; }
         virtual void render(std::wostream &out)
         { out << L"<img src=\"" << image_name << L"\">\n"; }
         
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
