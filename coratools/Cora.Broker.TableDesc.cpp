/* Cora.Broker.TableDesc.cpp

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Friday 14 April 2000
   Last Change: Tuesday 30 May 2000
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.TableDesc.h"
#include "NetMessage.h"

namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class TableDesc definitions
      ////////////////////////////////////////////////////////////

      TableDesc::TableDesc(StrUni const &name_):
         name(name_),
         num_records(0),
         original_num_records(0)
      { }

   
      TableDesc::TableDesc():
         num_records(0),
         original_num_records(0)
      { }

   
      TableDesc::TableDesc(TableDesc const &other):
         name(other.name),
         interval(other.interval),
         num_records(other.num_records),
         original_num_records(other.original_num_records),
         columns(other.columns)
      { }

   
      TableDesc &TableDesc::operator =(TableDesc const &other)
      {
         name = other.name;
         interval = other.interval;
         num_records = other.num_records;
         original_num_records = other.original_num_records;
         columns = other.columns;
         return  *this;
      } // copy operator


      TableDesc::~TableDesc()
      { }


      bool TableDesc::read(NetMessage *msg)
      {
         bool rtn;
         uint4 num_columns;

         if(msg->readInt8(interval) && msg->readUInt4(num_records) &&
            msg->readUInt4(original_num_records) && msg->readUInt4(num_columns))
         {
            rtn = true;
            for(uint4 i = 0; i < num_columns && rtn; ++i)
            {
               column_desc_handle column(new ColumnDesc);
               rtn = column->read(msg);
               if(rtn)
                  columns.push_back(column);
            }
         }
         else
            rtn = false;
         return rtn;
      } // read


      bool TableDesc::find_column_desc(column_desc_handle &desc, StrUni const &column_name) const
      {
         desc.bind(0);
         for(const_iterator ci = columns.begin();
             desc.get_rep() == 0 && ci != columns.end();
             ++ci)
            if((*ci)->get_name() == column_name)
               desc = *ci;
         return desc.get_rep() != 0;
      } // find_column_desc
   };
};
