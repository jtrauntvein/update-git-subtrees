/* Csi.Html.Tag.cpp

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 3 May 2000
   Last Change: Tuesday 20 November 2012
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Html.Tag.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Tag definitions
      ////////////////////////////////////////////////////////////
      bool Tag::before_render(std::ostream &out)
      {
         bool rtn = true;
         if(tag_name.length() > 0)
         {
            if(!on_same_line)
               out << std::endl;
            out << '<' << tag_name;
            for(attributes_type::iterator ai = attributes.begin(); ai != attributes.end(); ++ai)
               out << ' ' << ai->first << "=\"" << ai->second << '\"';
            out << '>';
            if(!on_same_line)
               out << std::endl;
         }
         else
            rtn = false;
         return rtn;
      } // before_render

      
      bool Tag::before_render(std::wostream &out)
      {
         bool rtn = true;
         if(tag_name.length() > 0)
         {
            if(!on_same_line)
               out << std::endl;
            out << L'<' << tag_name;
            for(attributes_type::iterator ai = attributes.begin(); ai != attributes.end(); ++ai)
               out << L' ' << ai->first << L"=\"" << ai->second << L'\"';
            out << L'>';
            if(!on_same_line)
               out << std::endl;
         }
         else
            rtn = false;
         return rtn;
      } // before_render
      

      void Tag::render(std::ostream &out)
      {
         for(tags_type::iterator ti = tags.begin(); ti != tags.end(); ++ti)
         {
            if((*ti)->before_render(out))
            {
               (*ti)->render(out);
               (*ti)->after_render(out);
            }
         }
      } // render


      void Tag::render(std::wostream &out)
      {
         for(tags_type::iterator ti = tags.begin(); ti != tags.end(); ++ti)
         {
            if((*ti)->before_render(out))
            {
               (*ti)->render(out);
               (*ti)->after_render(out);
            }
         }
      } // render
      

      void Tag::after_render(std::ostream &out)
      {
         if(!on_same_line)
            out << std::endl;
         out << "</" << tag_name << '>';
         if(!on_same_line)
            out << std::endl;
      } // after_render


      void Tag::after_render(std::wostream &out)
      {
         if(!on_same_line)
            out << std::endl;
         out << L"</" << tag_name << L'>';
         if(!on_same_line)
            out << std::endl;
      } // after_render
   };
};
