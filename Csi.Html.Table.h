/* Csi.Html.Table.h

   Copyright (C) 2003, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 January 2003
   Last Change: Wednesday 11 July 2012
   Last Commit: $Date: 2012-07-12 08:46:37 -0600 (Thu, 12 Jul 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Html_Table_h
#define Csi_Html_Table_h

#include "Csi.Html.Tag.h"
#include "Csi.PolySharedPtr.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class TableRow
      //
      // Defines an HTML tag based object that will render one table row.  This
      // class overloads add_tag() so that a new cell tag is added.  It also
      // provides add_heading() which will add a TH element rather than a TD
      // element. 
      ////////////////////////////////////////////////////////////
      class TableRow: public Tag
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TableRow():
            Tag("tr", false)
         { }

         ////////////////////////////////////////////////////////////
         // add_tag
         //
         // Overloads add_tag to add a cell to the row
         ////////////////////////////////////////////////////////////
         virtual tag_handle add_tag(tag_handle tag)
         { 
            Tag *td = new Tag("td",true);
            td->add_tag(tag);
            Tag::add_tag(td);
            return tag;
         }

         ////////////////////////////////////////////////////////////
         // add_heading
         ////////////////////////////////////////////////////////////
         void add_heading(tag_handle tag)
         {
            Tag *th = new Tag("th",true);
            th->add_tag(tag);
            Tag::add_tag(th);
         }

         ////////////////////////////////////////////////////////////
         // add_last_cell_attribute
         //
         // Adds the named attribute on the last cell that was added to this row
         ////////////////////////////////////////////////////////////
         void add_last_cell_attribute(
            StrAsc const &name,
            StrAsc const &value)
         {
            if(!tags.empty())
               tags.back()->add_attribute(name,value);
         }
      };


      ////////////////////////////////////////////////////////////
      // class Table
      //
      // Defines a TAG derived class that renders an HTML table structure. The
      // application uses this component by creating an instance and adding one
      // row at a time. 
      ////////////////////////////////////////////////////////////
      class Table: public Tag
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Table():
            Tag("table")
         { }

         ////////////////////////////////////////////////////////////
         // add_tag
         //
         // Overloaded to add a tag to the last row added.  Will add a row if
         // there is none.
         ////////////////////////////////////////////////////////////
         typedef Csi::PolySharedPtr<Tag, TableRow> row_handle;
         virtual tag_handle add_tag(tag_handle tag)
         {
            row_handle last_row;
            if(tags.empty())
            {
               last_row.bind(new TableRow);
               add_row(last_row);
            }
            else
               last_row = tags.back();
            last_row->add_tag(tag);
            return tag;
         }

         ////////////////////////////////////////////////////////////
         // add_heading
         //
         // Adds a heading at the end of the last row
         ////////////////////////////////////////////////////////////
         void add_heading(tag_handle tag)
         {
            row_handle last_row;
            if(tags.empty())
            {
               last_row.bind(new TableRow);
               add_row(last_row);
            }
            else
               last_row = tags.back();
            last_row->add_heading(tag);
         }

         ////////////////////////////////////////////////////////////
         // add_row
         ////////////////////////////////////////////////////////////
         row_handle add_row(row_handle row)
         {
            Tag::add_tag(row.get_handle());
            return row;
         } 
         row_handle add_row()
         { return add_row(new TableRow); }

         ////////////////////////////////////////////////////////////
         // add_last_cell_attribute
         //
         // Adds an attribute to the last cell that was added
         ////////////////////////////////////////////////////////////
         void add_last_cell_attribute(
            StrAsc const &name,
            StrAsc const &value)
         {
            if(!tags.empty())
            {
               row_handle last_row(tags.back());
               last_row->add_last_cell_attribute(name,value);
            }
         }
      };
      typedef PolySharedPtr<Tag, Table> TableHandle;
   };
};


#endif
