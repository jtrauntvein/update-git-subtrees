/* Csi.Html.Heading.h

   Copyright (C) 2000, 2007 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 6 May 2000
   Last Change: Thursday 11 January 2007
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Html_Heading_h
#define Csi_Html_Heading_h

#include "Csi.Html.Tag.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Heading
      ////////////////////////////////////////////////////////////
      class Heading: public Tag
      {
      public:
         enum heading_level_type
         {
            heading_1 = 1,
            heading_2 = 2,
            heading_3 = 3,
            heading_4 = 4,
            heading_5 = 5,
            heading_6 = 6
         };

      private:
         ////////////////////////////////////////////////////////////
         // heading_level
         ////////////////////////////////////////////////////////////
         heading_level_type heading_level;

         ////////////////////////////////////////////////////////////
         // contents
         ////////////////////////////////////////////////////////////
         StrAsc contents;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Heading(
            int heading_level_,
            StrAsc const &contents_):
            contents(contents_)
         {
            if(heading_level_ < heading_1 || heading_level_ > heading_6)
               throw std::invalid_argument("invalid heading level");
            heading_level = static_cast<heading_level_type>(heading_level_);
         }

         ////////////////////////////////////////////////////////////
         // before_render
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out)
         { out << "<h" << heading_level << '>'; return true; }
         virtual bool before_render(std::wostream &out)
         { out << L"<h" << heading_level << L'>'; return true; }

         ////////////////////////////////////////////////////////////
         // render
         ////////////////////////////////////////////////////////////
         virtual void render(std::ostream &out)
         { out << contents; }
         virtual void render(std::wostream &out)
         { out << contents; }

         ////////////////////////////////////////////////////////////
         // after_render
         ////////////////////////////////////////////////////////////
         virtual void after_render(std::ostream &out)
         { out << "</h" << heading_level << '>' << std::endl; }
         virtual void after_render(std::wostream &out)
         { out << L"</h" << heading_level << L'>' << std::endl; }

         ////////////////////////////////////////////////////////////
         // set_contents
         ////////////////////////////////////////////////////////////
         void set_contents(StrAsc const &contents_)
         { contents = contents_; }

         ////////////////////////////////////////////////////////////
         // get_contents
         ////////////////////////////////////////////////////////////
         StrAsc const &get_contents() const
         { return contents; }
      };
   };
};


#endif
