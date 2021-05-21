/* Classic.FinalStorage.Table.cpp

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 20 December 2006
   Last Change: Friday 14 April 2017
   Last Commit: $Date: 2017-04-14 17:10:44 -0600 (Fri, 14 Apr 2017) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.ClassicFinalStorageTable.h"
#include "Cora.Broker.ValueDesc.h"
#include "Csi.Utils.h"
#include <iostream>
#include <algorithm>
#include <map>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class ClassicFinalStorageTable definitions
      ////////////////////////////////////////////////////////////
      ClassicFinalStorageTable::ClassicFinalStorageTable():
         array_id(0)
      { }


      ClassicFinalStorageTable::ClassicFinalStorageTable(
         ClassicFinalStorageTable const &other):
         array_id(other.array_id),
         name(other.name),
         column_names(other.column_names)
      { }


      ClassicFinalStorageTable::~ClassicFinalStorageTable()
      { }


      ClassicFinalStorageTable &ClassicFinalStorageTable::operator =(
         ClassicFinalStorageTable const &other)
      {
         array_id = other.array_id;
         name = other.name;
         column_names = other.column_names;
         return *this;
      } // copy operator


      bool ClassicFinalStorageTable::extract(std::istream &in)
      {
         bool rtn;
         StrAsc line;
         enum state_type
         {
            state_find_output,
            state_look_for_array_id,
            state_scan_column_names,
            state_error,
            state_complete
         } state = state_find_output;
         size_t pos;
         char *token_end;
         uint4 line_no;
         StrUni column_name;

         column_names.clear();
         while(in && state != state_complete && state != state_error)
         {
            // read the line and cut out any comment marker
            line.readLine(in);
            if(line.length() > 0 && line[0] == ';')
               line.cut(0,1);

            // process the line
            switch(state)
            {
            case state_find_output:
               pos = line.find(" Output_Table ");
               if(pos < line.length())
               {
                  line[pos] = 0;
                  name = line.c_str();
                  state = state_look_for_array_id;
               }
               break;

            case state_look_for_array_id:
               line_no = strtoul(line.c_str(),&token_end,10);
               if(line_no == 1)
               {
                  array_id = strtoul(token_end,0,10);
                  state = state_scan_column_names;
                  ++line_no;
               }
               else
                  state = state_error;
               break;
               
            case state_scan_column_names:
               if(line.length() == 0)
                  state = state_complete;
               else if(strtoul(line.c_str(),&token_end,10) == line_no)
               {
                  column_name = token_end + 1;
                  column_name.cut(column_name.find(L" "));
                  column_names.push_back(column_name);
                  ++line_no;
               }
               else
                  state = state_error;
               break;
            }
         }
         rtn = (state == state_complete);
         return rtn;
      } // extract


      Csi::SharedPtr<RecordDesc> ClassicFinalStorageTable::make_table_def()
      {
         // Any attempt to guess the table size is bound to be wrong for at least a minority of
         // scenarios. Rather than trying to outguess the user, we will assign an arbitrary size
         // which can be changed by a client at a later time using the data broker table resize
         // transaction.
         Csi::SharedPtr<RecordDesc> rtn(new RecordDesc(L"",name));
         uint4 order = 0;
         typedef std::map<StrUni, uint4> previous_names_type;
         previous_names_type previous_names;
         StrUni column_name;

         for(const_iterator ci = column_names.begin(); ci != column_names.end(); ++ci)
         {
            previous_names_type::iterator pni = previous_names.find(*ci);
            Csi::SharedPtr<ValueDesc> value_desc(new ValueDesc);
            
            if(pni == previous_names.end())
            {
               previous_names[*ci] = 0;
               column_name = ci->c_str();
            }
            else
            {
               wchar_t temp[25];
               
               column_name = ci->c_str();
               column_name.append(L"~");
               swprintf(temp,sizeof(temp),L"%u",pni->second + 1);
               column_name.append(temp);
               pni->second += 1;
            }
            value_desc->name = column_name;
            value_desc->data_type = CsiIeee4;
            value_desc->modifying_cmd = 0;
            rtn->values.push_back(value_desc);
         }
         return rtn;
      } // make_table_def
   };
};
