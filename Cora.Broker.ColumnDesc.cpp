/* Cora.Broker.ColumnDesc.cpp

   Copyright (C) 2000, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 14 April 2000
   Last Change: Thursday 11 April 2013
   Last Commit: $Date: 2013-04-11 11:10:37 -0600 (Thu, 11 Apr 2013) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.ColumnDesc.h"
#include "Cora.Broker.Defs.h"


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class ColumnDesc definitions
      ////////////////////////////////////////////////////////////

      ColumnDesc::ColumnDesc():
         data_type(CsiUInt1),
         modifying_command(0),
         string_len(1)
      { }


      ColumnDesc::ColumnDesc(ColumnDesc const &other):
         name(other.name),
         data_type(other.data_type),
         modifying_command(other.modifying_command),
         units(other.units),
         process(other.process),
         dimensions(other.dimensions),
         pieces(other.pieces),
         string_len(other.string_len)
      { }

   
      ColumnDesc::~ColumnDesc()
      { }

   
      ColumnDesc &ColumnDesc::operator =(ColumnDesc const &other)
      {
         name = other.name;
         description = other.description;
         data_type = other.data_type;
         modifying_command = other.modifying_command;
         units = other.units;
         process = other.process;
         dimensions = other.dimensions;
         pieces = other.pieces;
         string_len = other.string_len;
         return *this; 
      } // copy operator


      bool ColumnDesc::read(Csi::Messaging::Message *msg)
      {
         bool rtn;
         if(msg->getMsgType() == Cora::Broker::Messages::table_def_get_ack)
            rtn = read_old(msg);
         else if(msg->getMsgType() == Cora::Broker::Messages::extended_table_def_get_ack)
            rtn = read_extended(msg);
         else
            rtn = false;
         return rtn;
      } // read


      uint4 ColumnDesc::num_elements() const
      {
         uint4 rtn = 0;
         for(pieces_type::const_iterator pi = pieces.begin();
             pi != pieces.end();
             ++pi)
            rtn += pi->num_elements;
         return rtn; 
      } // num_elements


      bool ColumnDesc::read_old(Csi::Messaging::Message *msg)
      {
         bool rtn;
         uint4 num_dimensions;
         uint4 data_type;

         dimensions.clear();
         pieces.clear();
         if(msg->readWStr(name) && msg->readUInt4(data_type) &&
            msg->readUInt4(modifying_command) &&
            msg->readWStr(units) && msg->readWStr(process) &&
            msg->readUInt4(num_dimensions))
         {
            piece_type piece;
            
            this->data_type = static_cast<CsiDbTypeCode>(data_type);
            if(num_dimensions == 0)
            {
               piece.num_elements = piece.start_index = 1;
               rtn = true;
            }
            else if(num_dimensions == 1)
            {
               if(msg->readUInt4(piece.num_elements) &&
                  msg->readUInt4(piece.start_index))
               {
                  // we are going to treat arrays of ascii characters as scalars because so that the
                  // entire string can be selected as a single value.
                  if(piece.num_elements > 1 && data_type != CsiAscii && data_type != CsiAsciiZ)
                     dimensions.add_dimension(piece.num_elements + piece.start_index - 1);
                  rtn = true;
               }
               else
                  rtn = false;
            }
            else
               rtn = false;
            if(rtn)
               pieces.push_back(piece);
         }
         else
            rtn = false;
         return rtn;
      } // read_old


      bool ColumnDesc::read_extended(Csi::Messaging::Message *msg)
      {
         bool rtn = false;
         uint4 num_dimensions;
         uint4 data_type;

         dimensions.clear();
         pieces.clear();
         if(msg->readWStr(name) &&
            msg->readWStr(description) &&
            msg->readUInt4(data_type) &&
            msg->readUInt4(modifying_command) &&
            msg->readWStr(units) && msg->readWStr(process) &&
            msg->readUInt4(num_dimensions))
         {
            // read the list of dimensions in
            uint4 dim_size;
            rtn = true;
            this->data_type = static_cast<CsiDbTypeCode>(data_type);
            for(uint4 i = 0; rtn && i < num_dimensions; ++i)
            {
               if(msg->readUInt4(dim_size))
               {
                  if(data_type != CsiAscii && data_type != CsiAsciiZ)
                     dimensions.add_dimension(dim_size);
                  else if(i + 1 < num_dimensions)
                     dimensions.add_dimension(dim_size);
               }
               else
                  rtn = false;
            }

            // if the dimensions list was successfully read, we can now read in the list of pieces.
            uint4 num_pieces;
            if(rtn && msg->readUInt4(num_pieces))
            {
               piece_type piece;
               pieces.reserve(num_pieces);
               for(uint4 i = 0; rtn && i < num_pieces; ++i)
               {
                  uint4 start_index_len;
                  
                  if(msg->readUInt4(piece.num_elements) &&
                     msg->readUInt4(start_index_len))
                  {
                     // read the starting index
                     std::vector<uint4> start_index(start_index_len);
                     for(uint4 j = 0; rtn && j < start_index_len; ++j)
                        rtn = msg->readUInt4(start_index[j]);
                     if(rtn)
                     {
                        if(data_type == CsiAscii || data_type == CsiAsciiZ)
                        {
                           start_index.pop_back();
                           string_len = dim_size;
                           piece.num_elements /= dim_size;
                        }
                        piece.start_index = dimensions.to_offset(
                           start_index.begin(),
                           start_index.end());
                        pieces.push_back(piece);
                     }
                  }
                  else
                     rtn = false;
               }
            }
            else
               rtn = false;

            // finally, we need to handle the list of aliases. For now, we will ignore them but
            // eventually, this section will have to be filled in.
            uint4 num_aliases;
            if(rtn && msg->readUInt4(num_aliases))
            {
               // if there are aliases defined we will assert false here so that the code is
               // obviously broken when they are defined.
               if(num_aliases > 0)
                  assert(false);
            }
            else
               rtn = false;
         }
         return rtn;
      } // read_extended
   };
};
