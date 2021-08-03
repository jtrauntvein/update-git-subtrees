/* Cora.Broker.ValueTypes.cpp

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 06 December 2002
   Last Change: Tuesday 03 July 2012
   Last Commit: $Date: 2012-11-13 15:45:49 -0600 (Tue, 13 Nov 2012) $ 
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.ValueTypes.h"
#include "Csi.Json.h"


namespace Cora
{
   namespace Broker
   {
      namespace ValueTypes
      {
         ////////////////////////////////////////////////////////////
         // class VAscii definitions
         ////////////////////////////////////////////////////////////
         void VAscii::read_json(Csi::Json::ValueBase *json)
         {
            using namespace Csi::Json;
            if(json->get_type() == value_string)
            {
               String *string(static_cast<String *>(json));
               read_text(string->get_value().c_str());
            }
         } // read_json

         
         void VAscii::format(
            std::ostream &out, 
            bool overload_stream_attribs,
            char const *time_format,
            CustomCsvOptions const *custom_options,
            bool specials_as_numbers,
            bool optimise_without_locale,
            bool translate_strings) const
         {
            char *chars = static_cast<char *>(storage);
            for(uint4 i = 0; i < max_val_len && chars[i] != 0; ++i)
            {
               if(translate_strings)
               {
                  if(chars[i] == '\r' || chars[i] == '\n')
                     out << ' ';
                  else if(chars[i] == '\"')
                     out << '\'';
                  else
                     out << chars[i];
               }
               else
                  out << chars[i];
            }
         } // format (narrow stream)

         
         void VAscii::format(
            std::wostream &out,
            bool overload_stream_attribs,
            wchar_t const *time_format,
            CustomCsvOptions const *options,
            bool specials_as_numbers,
            bool optimise_without_locale,
            bool translate_strings) const
         {
            char *chars = static_cast<char *>(storage);
            uint4 len = 0;
            StrUni temp;
            
            for(uint4 i = 0; i < max_val_len && chars[i] != 0; ++i)
               ++len;
            temp.append_utf8(chars, len);
            if(translate_strings)
            {
               for(size_t i = 0; i < temp.length(); ++i)
               {
                  if(temp[i] == '\r' || temp[i] == '\n')
                     out << L' ';
                  else if(temp[i] == '\"')
                     out << L'\'';
                  else
                     out << temp[i];
               }
            }
            else
               out << temp;
         } // format (wide stream)


         void VAscii::format_json(std::ostream &out)
         {
            out << "\""
                << Csi::Json::format_string(static_cast<char const *>(storage), max_val_len)
                << "\"";
         } // format_json
         
         
         ////////////////////////////////////////////////////////////
         // class VStamp definitions
         ////////////////////////////////////////////////////////////
         void VStamp::read_json(Csi::Json::ValueBase *json)
         {
            using namespace Csi::Json;
            if(json->get_type() == value_string)
            {
               String *string(static_cast<String *>(json));
               read_text(string->get_value().c_str());
            }
            else
               throw std::invalid_argument("invalid value data type");
         } // read_json

         
         Csi::LgrDate VStamp::get_value() const
         {
            Csi::LgrDate rtn;
            if(description->data_type == CsiLgrDate || description->data_type == CsiLgrDateLsf)
            {
               int8 val;
               memcpy(&val,storage,sizeof(val));
               if((Csi::is_big_endian() && description->data_type == CsiLgrDateLsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiLgrDate))
                  Csi::reverse_byte_order(&val,sizeof(val));
               rtn = val;
            }
            else if(description->data_type == CsiNSec)
               rtn = csiNSecToLgrDate(storage);
            else if (description->data_type == CsiNSecLsf)
               rtn = csiNSecLsfToLgrDate(storage);
            else if(description->data_type == CsiSec)
               rtn = csiSecToLgrDate(storage);
            else if(description->data_type == CsiUSec)
               rtn = csiUSecToLgrDate(storage);
            return rtn;
         } // get_value


         void VStamp::set_value(Csi::LgrDate const &value)
         {
            if(description->data_type == CsiLgrDate ||
               description->data_type == CsiLgrDateLsf)
            {
               int8 val = value.get_nanoSec();
               memcpy(storage,&val,sizeof(val));
               if((Csi::is_big_endian() && description->data_type == CsiLgrDateLsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiLgrDate))
                  Csi::reverse_byte_order(storage,sizeof(val));
            }
            else if(description->data_type == CsiNSec ||
                    description->data_type == CsiNSecLsf)
            {
               int4 sec = value.get_sec();
               uint4 nsec = value.nsec();
               byte *bytes = static_cast<byte *>(storage);
               memcpy(bytes,&sec,sizeof(sec));
               memcpy(bytes + 4,&nsec,sizeof(nsec));
               if((Csi::is_big_endian() && description->data_type == CsiNSecLsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiNSec))
               {
                  Csi::reverse_byte_order(bytes,sizeof(sec));
                  Csi::reverse_byte_order(bytes + 4,sizeof(nsec));
               }
            }
            else if(description->data_type == CsiSec)
            {
               int4 sec = value.get_sec();
               memcpy(storage,&sec,sizeof(sec));
               if(Csi::is_big_endian())
                  Csi::reverse_byte_order(storage,sizeof(sec));
            }
            else if(description->data_type == CsiUSec)
            {
               int8 usec = value.get_nanoSec() / 10000;
               byte *bytes = static_cast<byte *>(storage);
               for(int i = 0; i < 6; ++i)
               {
                  bytes[i] = static_cast<byte>(usec & 0xFF);
                  usec >>= 8;
               }
            }
         } // set_value


         void VStamp::set_to_null()
         {
            if(description->data_type == CsiLgrDate ||
               description->data_type == CsiLgrDateLsf ||
               description->data_type == CsiNSec ||
               description->data_type == CsiNSecLsf)
            {
               int8 *val = static_cast<int8 *>(storage);
               *val = 0; 
            }
            else if(description->data_type == CsiSec)
            {
               int4 *val = static_cast<int4 *>(storage);
               *val = 0;
            }
            else if(description->data_type == CsiUSec)
            {
               byte *val = static_cast<byte *>(storage);
               memset(val, 0, 6); 
            }
         } // set_to_null


         ////////////////////////////////////////////////////////////
         // class VBool definitions
         ////////////////////////////////////////////////////////////
         void VBool::read_json(Csi::Json::ValueBase *json)
         {
            using namespace Csi::Json;
            if(json->get_type() == value_bool)
            {
               byte *bytes(static_cast<byte *>(storage));
               Boolean *boolean(static_cast<Boolean *>(json));
               if(boolean->get_value())
                  bytes[0] = 0xFF;
               else
                  bytes[0] = 0x00;
            }
         } // read_json
         

         ////////////////////////////////////////////////////////////
         // class VBool2 definitions
         ////////////////////////////////////////////////////////////
         void VBool2::read_json(Csi::Json::ValueBase *json)
         {
            using namespace Csi::Json;
            if(json->get_type() == value_bool)
            {
               uint2 *bytes(static_cast<uint2 *>(storage));
               Boolean *boolean(static_cast<Boolean *>(json));
               if(boolean->get_value())
                  bytes[0] = 0xFFFF;
               else
                  bytes[0] = 0x0000;
            }
         } // read_json


         ////////////////////////////////////////////////////////////
         // class VBool8 definitions
         ////////////////////////////////////////////////////////////
         void VBool8::read_json(Csi::Json::ValueBase *json)
         {
            using namespace Csi::Json;
            if(json->get_type() == value_string)
            {
               byte *value_byte(static_cast<byte *>(storage));
               String *value_str(static_cast<String *>(json));
               StrAsc const &temp(value_str->get_value());
               *value_byte = 0;
               for(size_t i = 0; i < temp.length() && i < 8; ++i)
               {
                  if(i > 0)
                     *value_byte = (*value_byte) << 1;
                  if(temp[i] == '1')
                     *value_byte |= 1;
               }
            }
         } // read_json

         
         ////////////////////////////////////////////////////////////
         // class VBool4 definitions
         ////////////////////////////////////////////////////////////
         void VBool4::read_json(Csi::Json::ValueBase *json)
         {
            using namespace Csi::Json;
            if(json->get_type() == value_bool)
            {
               uint4 *bytes(static_cast<uint4 *>(storage));
               Boolean *boolean(static_cast<Boolean *>(json));
               if(boolean->get_value())
                  bytes[0] = 0xFFFFFFFF;
               else
                  bytes[0] = 0x00000000;
            }
         } // read_json
         
         
         namespace
         {
            ////////////////////////////////////////////////////////////
            // template function read_json_number
            ////////////////////////////////////////////////////////////
            template<class T>
            void read_json_number(Value *value, Csi::Json::ValueBase *json)
            {
               if(json->get_type() == Csi::Json::value_number)
               {
                  Csi::Json::Number *number(static_cast<Csi::Json::Number *>(json));
                  T *val(static_cast<T *>(value->get_pointer()));
                  *val = static_cast<T>(number->get_value());
               }
               else if(json->get_type() == Csi::Json::value_string)
               {
                  Csi::Json::String *string(static_cast<Csi::Json::String *>(json));
                  T *val(static_cast<T *>(value->get_pointer()));
                  *val = static_cast<T>(
                     csiStringToFloat(
                        string->get_value().c_str(), std::locale::classic()));
               }
               else
                  throw std::invalid_argument("cannot convert json value to numeric");
            }
         };
         
         
         ////////////////////////////////////////////////////////////
         // class VSignedByte definitions
         ////////////////////////////////////////////////////////////
         void VSignedByte::read_json(Csi::Json::ValueBase *json)
         { read_json_number<char>(this, json); }
         
         
         ////////////////////////////////////////////////////////////
         // class VByte definitions
         ////////////////////////////////////////////////////////////
         void VByte::read_json(Csi::Json::ValueBase *json)
         { read_json_number<byte>(this, json); }
         
         
         ////////////////////////////////////////////////////////////
         // class VInt2 definitions
         ////////////////////////////////////////////////////////////
         void VInt2::read_json(Csi::Json::ValueBase *json)
         { read_json_number<int2>(this, json); }
         
         
         ////////////////////////////////////////////////////////////
         // class VUInt2 definitions
         ////////////////////////////////////////////////////////////
         void VUInt2::read_json(Csi::Json::ValueBase *json)
         { read_json_number<uint2>(this, json); }
         
         
         ////////////////////////////////////////////////////////////
         // class VInt4 definitions
         ////////////////////////////////////////////////////////////
         void VInt4::read_json(Csi::Json::ValueBase *json)
         { read_json_number<int4>(this, json); }
         
         
         ////////////////////////////////////////////////////////////
         // class VUInt4 definitions
         ////////////////////////////////////////////////////////////
         void VUInt4::read_json(Csi::Json::ValueBase *json)
         { read_json_number<uint4>(this, json); }
         
         
         ////////////////////////////////////////////////////////////
         // class VDouble definitions
         ////////////////////////////////////////////////////////////
         void VDouble::read_json(Csi::Json::ValueBase *json)
         { read_json_number<double>(this, json); }
         
         
         ////////////////////////////////////////////////////////////
         // class VFloat definitions
         ////////////////////////////////////////////////////////////
         void VFloat::read_json(Csi::Json::ValueBase *json)
         { read_json_number<float>(this, json); }
         
         
         ////////////////////////////////////////////////////////////
         // class VInt8 definitions
         ////////////////////////////////////////////////////////////
         void VInt8::read_json(Csi::Json::ValueBase *json)
         { read_json_number<int8>(this, json); }
      };
   };
};
