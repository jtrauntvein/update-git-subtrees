/* Cora.Broker.FSLReader.cpp

   Copyright (C) 2008, 2016 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Thursday Aug 23 2007
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header

#include "Cora.Broker.FSLReader.h"
#include "Csi.BuffStream.h"
#include "Csi.fstream.h"


namespace Cora
{
   namespace Broker
   {
      FSLReader::FSLReader()
      { }


      FSLReader::~FSLReader()
      { }


      void FSLReader::extract_labels(StrAsc const &fsl_file_name_)
      {
         try
         {
            fsl_file_name = fsl_file_name_;
            if(fsl_file_name.find(".dld") < fsl_file_name.length())
               extract_from_dld();
            else
               extract_from_fsl();
         }
         catch(std::exception &)
         {
            fsl_file_name.cut(0);
            throw;
         }
      } // extract_labels


      uint2 FSLReader::get_labels_sig()
      {
         FILE *in = Csi::open_file(fsl_file_name.c_str(), "rb");
         uint2 rtn = 0xAAAA;
         if(in)
         {
            rtn = Csi::calc_file_sig(in);
            fclose(in);
         }
         return rtn;
      } // get_labels_sig
      
      
      void FSLReader::extract_from_dld()
      {
         Csi::ifstream fin(fsl_file_name.c_str());

         // scan for a comment line that contains the begin FSL mark (#)
         StrAsc line;
         StrAsc fsl_contents;
         enum state_type
         {
            state_before_block,
            state_in_block,
            state_complete
         } state = state_before_block;

         while(fin && state != state_complete)
         {
            line.readLine(fin);
            if(state == state_before_block)
            {
               if(line.length() >= 2 && line[0] == ';' && line[1] == '%')
                  state = state_in_block;
            }
            else if(state == state_in_block)
            {
               if(line.length() > 0 && line[0] == ';')
               {
                  if(line.length() >= 2 && line[1] == '%')
                     state = state_complete;
                  else
                  {
                     fsl_contents += line;
                     fsl_contents += "\n";
                  }
               }
               else
                  state = state_complete;
            }
         }

         // use the contents that were extracted to read the tables
         record_descs.clear();
         if(state == state_complete && fsl_contents.length() > 0)
         {
            Csi::IBuffStream fsl_stream(fsl_contents.c_str(), fsl_contents.length());
            extract_record_descs(fsl_stream);
         }
      } // extract_from_dld


      void FSLReader::extract_from_fsl()
      {
         Csi::ifstream fin(fsl_file_name.c_str());
         extract_record_descs(fin);
      } // extract_from_fsl


      void FSLReader::extract_record_descs(std::istream &fsl_in)
      {
         StrAsc line;
         enum state_type
         {
            state_find_output,
            state_look_for_array_id,
            state_scan_column_names,
            state_error,
            state_complete
         } state = state_find_output;

         bool is_complete = false;
         do //Read through the fsl input
         {
            size_t pos;
            char *token_end;
            uint4 line_no;
            StrUni column_name;

            uint4 array_id = 0;
            record_description_handle record(new Cora::Broker::RecordDesc(L"Broker",L"Table"));

            while(fsl_in && state != state_complete && state != state_error)
            {
               // read the line and cut out any comment marker (see .dld)
               line.readLine(fsl_in);
               if(line.length() > 0 && line[0] == ';')
                  line.cut(0,1);

               if(line.find("Estimated Total Final Storage Locations") < line.length())
               {
                  is_complete = true;
                  record.clear();
                  break;
               }

               // process the line
               switch(state)
               {
                  case state_find_output:
                  {
                     pos = line.find(" Output_Table ");
                     if(pos < line.length())
                     {
                        line[pos] = 0;
                        state = state_look_for_array_id;
                     }
                     break;
                  }
                  case state_look_for_array_id:
                  {
                     line_no = strtoul(line.c_str(),&token_end,10);
                     if(line_no == 1)
                     {
                        array_id = strtoul(token_end,0,10);
                        record->table_name = token_end;
                        record->table_name.cut(0,1); //Remove space
                        record->table_name.cut(record->table_name.findRev(L" "));
                        state = state_scan_column_names;
                        ++line_no;
                     }
                     else
                        state = state_error;
                     break;
                  }   
                  case state_scan_column_names:
                  {
                     if(line.length() == 0)
                     {
                        state = state_complete;
                        record_descs[array_id] = record;
                     }
                     else if(strtoul(line.c_str(),&token_end,10) == line_no)
                     {
                        column_name = token_end + 1;
                        column_name.cut(column_name.find(L" "));
                        Csi::SharedPtr<Cora::Broker::ValueDesc> value_description(new Cora::Broker::ValueDesc);
                        value_description->name = column_name;
                        value_description->data_type = CsiIeee4;
                        value_description->modifying_cmd = 0;
                        record->values.push_back(value_description);
                        ++line_no;
                     }
                     else
                        state = state_error;
                     break;
                  }
               } //switch
            } //while

            //Restart looking for the next record
            state = state_find_output;
         } 
         while(!is_complete && fsl_in);
      } // extract
   };
};
