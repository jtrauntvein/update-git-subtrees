/* Csi.Html.WideText.h

   Copyright (C) 2000, 2007 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 09 May 2000
   Last Change: Thursday 11 January 2007
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Html_WideText_h
#define Html_WideText_h

#include "StrUni.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class WideText
      //
      // Defines an HTML object that can render a unicode string
      ////////////////////////////////////////////////////////////
      class WideText: public Html::Tag
      {
      protected:
         ////////////////////////////////////////////////////////////
         // content
         ////////////////////////////////////////////////////////////
         StrUni content;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         WideText(
            StrUni const &content_,
            StrAsc const &tag_name = ""):
            Tag(tag_name),
            content(content_)
         { }

         ////////////////////////////////////////////////////////////
         // add_attribute
         ////////////////////////////////////////////////////////////
         virtual void add_attribute(
            StrAsc const &name,
            StrAsc const &value)
         { throw invalid_tag_operation(); }

         ////////////////////////////////////////////////////////////
         // add_tag
         ////////////////////////////////////////////////////////////
         virtual tag_handle add_tag(tag_handle tag)
         { throw invalid_tag_operation(); }

         ////////////////////////////////////////////////////////////
         // before_render
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out)
         {
            if(tag_name.length())
               Tag::before_render(out);
            return true;
         }
         virtual bool before_render(std::wostream &out)
         {
            if(tag_name.length())
               Tag::before_render(out);
            return true;
         }

         ////////////////////////////////////////////////////////////
         // render
         ////////////////////////////////////////////////////////////
         virtual void render(std::ostream &out)
         { out << content; }
         virtual void render(std::wostream &out)
         { out << content; }

         ////////////////////////////////////////////////////////////
         // after_render
         ////////////////////////////////////////////////////////////
         virtual void after_render(std::ostream &out)
         {
            if(tag_name.length())
               Tag::after_render(out);
         }
         virtual void after_render(std::wostream &out)
         {
            if(tag_name.length()) 
               Tag::after_render(out);
         }
         
         ////////////////////////////////////////////////////////////
         // set_content
         ////////////////////////////////////////////////////////////
         virtual void set_content(StrUni const &content_)
         { content = content_; }
      };
   };
};


#endif
