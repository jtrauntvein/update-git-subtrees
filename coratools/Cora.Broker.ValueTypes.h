/* Cora.Broker.ValueTypes.h

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 20 March 2000
   Last Change: Thursday 09 March 2017
   Last Commit: $Date: 2020-02-06 19:41:32 -0600 (Thu, 06 Feb 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_ValueTypes_h
#define Cora_Broker_ValueTypes_h

#include "Cora.Broker.Value.h"
#include "Csi.Messaging.Message.h"
#include "Csi.ByteOrder.h"
#include "Csi.Utils.h"
#include "Csi.MaxMin.h"
#include <iomanip>
#include <sstream>
#include <limits>
#include <stdlib.h>


#ifdef max
#undef max
#undef min
#endif


namespace Cora
{
   namespace Broker
   {
      namespace ValueTypes
      {
         namespace
         {
            template <class value_type>
            void write_tob1_value(StrBin &buffer, value_type const &value)
            {
               value_type temp = value;
               if(Csi::is_big_endian())
                  Csi::reverse_byte_order(&temp,sizeof(temp));
               buffer.append(&temp,sizeof(temp));
            }
         };

         
         ////////////////////////////////////////////////////////////
         // class VAscii
         ////////////////////////////////////////////////////////////
         class VAscii: public Value
         {
         protected:
            ////////////////////////////////////////////////////////////
            // max_val_len
            ////////////////////////////////////////////////////////////
            uint4 max_val_len;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VAscii(desc_handle &desc):
               Value(desc),
               max_val_len(1)
            { }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               uint4 text_len = (uint4)strlen(text);
               memset(storage,0,max_val_len);
               memcpy(
                  storage,
                  text,
                  Csi::csimin((uint4)text_len, max_val_len - 1));
            }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return max_val_len; }

            ////////////////////////////////////////////////////////////
            // format (narrow)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out, 
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *custom_options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const;

            ////////////////////////////////////////////////////////////
            // format (wide)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const;

            ////////////////////////////////////////////////////////////
            // format_json
            ////////////////////////////////////////////////////////////
            virtual void format_json(std::ostream &out);
            
            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "CHAR(" << max_val_len << ")"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"CHAR(" << max_val_len << L")"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            {
               VAscii *rtn = new VAscii(description);
               rtn->max_val_len = max_val_len;
               rtn->combined_with_adjacent_values = combined_with_adjacent_values;
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // combine_with_adjacent_values
            ////////////////////////////////////////////////////////////
            virtual bool combine_with_adjacent_values(desc_handle &other_desc)
            {
               bool rtn = false;
               if(!other_desc->array_address.empty() &&
                  other_desc->array_address.back() != 1)
               {
                  ++max_val_len;
                  rtn = combined_with_adjacent_values = true;
               }
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // quote_when_formatting
            ////////////////////////////////////////////////////////////
            virtual bool quote_when_formatting(
               CustomCsvOptions const *options) const
            { return true; }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return true; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               char *v = static_cast<char *>(storage);
               *v = 0;
            }
         };
         

         ////////////////////////////////////////////////////////////
         // class VBool
         ////////////////////////////////////////////////////////////
         class VBool: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VBool(desc_handle &desc): Value(desc)
            { }
            
            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               byte *bytes = static_cast<byte *>(storage);
               if(text[0] == 'f' || text[0] == 'F')
                  bytes[0] = 0;
               else if(text[0] == 't' || text[0] == 'T')
                  bytes[0] = 0xff;
               else
               {
                  int4 temp = strtol(text, 0, 10);
                  if(temp == 0)
                     bytes[0] = 0;
                  else
                     bytes[0] = 0xFF;
               }
            }
            
            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Overloads the base class version to return true or false.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               byte const *bytes(static_cast<byte const *>(storage));
               Csi::Json::BooleanHandle rtn(new Csi::Json::Boolean(bytes[0] ? true : false));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *custom_options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               byte *bytes = static_cast<byte *>(storage);
               if(bytes[0] != 0)
                  out << -1;
               else
                  out << 0;
            }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *custom_options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               byte *bytes = static_cast<byte *>(storage);
               if(bytes[0] != 0)
                  out << -1;
               else
                  out << 0;
            }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               byte *bytes = static_cast<byte *>(storage);
               bool val = bytes[0] ? true : false;
               if(val)
                  dest = -1.0;
               else
                  dest = 0.0;
               return true;
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 1; }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "VARCHAR(1)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"VARCHAR(1)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            {
               VBool *rtn = new VBool(description);
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return true; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            { *(static_cast<byte *>(storage)) = 0; }

            ////////////////////////////////////////////////////////////
            // format_json
            ////////////////////////////////////////////////////////////
            virtual void format_json(std::ostream &out)
            {
               double val(0.0);
               to_float(val);
               if(val == 0)
                  out << "false";
               else
                  out << "true";
            }
         };


         ////////////////////////////////////////////////////////////
         // class VBool2
         ////////////////////////////////////////////////////////////
         class VBool2: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VBool2(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               uint2 *val = static_cast<uint2 *>(storage);
               if(text[0] == 'f' || text[0] == 'F')
                  *val = 0;
               else if(text[0] == 't' || text[0] == 'T')
                  *val = 0xffff;
               else
               {
                  int4 temp = strtol(text, 0, 10);
                  *val = (temp ? 0xffff : 0);
               }
            }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Overloads the base class version to return a boolean value.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               uint2 const *val((uint2 const *)storage);
               Csi::Json::BooleanHandle rtn(new Csi::Json::Boolean(*val ? true : false));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // format (normal stream)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *custom_options = 0,
               bool specials_as_number = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               uint2 *val = static_cast<uint2 *>(storage);
               out << (*val != 0 ? true : false);
            }

            ////////////////////////////////////////////////////////////
            // format (wide stream)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               uint2 *val = static_cast<uint2 *>(storage);
               out << (*val != 0 ? true : false);
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 2; }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "DECIMAL(5,0)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"DECIMAL(5,0)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VBool2(description); }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            {
               uint2 *val = static_cast<uint2 *>(storage);
               buffer.append(*val ? 0xFF : 0x00);
            }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               uint2 *val = static_cast<uint2 *>(storage);
               *val = 0;
            }

            ////////////////////////////////////////////////////////////
            // format_json
            ////////////////////////////////////////////////////////////
            virtual void format_json(std::ostream &out)
            {
               double val(0.0);
               to_float(val);
               if(val == 0)
                  out << "false";
               else
                  out << "true";
            }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               uint2 const *val(static_cast<uint2 const *>(storage));
               if(val)
                  dest = -1;
               else
                  dest = 0;
               return true;
            }
         };


         ////////////////////////////////////////////////////////////
         // class VBool4
         ////////////////////////////////////////////////////////////
         class VBool4: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VBool4(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               uint4 *val = static_cast<uint4 *>(storage);
               if(text[0] == 'f' || text[0] == 'F')
                  *val = 0;
               else if(text[0] == 't' || text[0] == 'T')
                  *val = 0xffffffff;
               else
               {
                  int4 temp = strtol(text, 0, 10);
                  *val = (temp ? 0xffffffff : 0);
               }
            }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Overloads the base class version to return a boolean value.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               uint4 const *val((uint4 const *)storage);
               Csi::Json::BooleanHandle rtn(new Csi::Json::Boolean(*val ? true : false));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // format (normal stream)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               uint4 *val = static_cast<uint4 *>(storage);
               out << (*val != 0 ? -1 : 0);
            }

            ////////////////////////////////////////////////////////////
            // format (wide stream)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               uint4 *val = static_cast<uint4 *>(storage);
               out << (*val != 0 ? -1 : 0);
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 4; }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "DECIMAL(9,0)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"DECIMAL(9,0)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VBool4(description); }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            {
               uint4 *val = static_cast<uint4 *>(storage);
               buffer.append(*val ? 0xFF : 0x00);
            }

             ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               uint4 *val = static_cast<uint4 *>(storage);
               *val = 0;
            }

            ////////////////////////////////////////////////////////////
            // format_json
            ////////////////////////////////////////////////////////////
            virtual void format_json(std::ostream &out)
            {
               double val(0.0);
               to_float(val);
               if(val == 0)
                  out << "false";
               else
                  out << "true";
            }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               uint4 const *val(static_cast<uint4 const *>(storage));
               dest = (*val ? -1 : 0);
               return true;
            }
         };


         ////////////////////////////////////////////////////////////
         // class VBool8
         ////////////////////////////////////////////////////////////
         class VBool8: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VBool8(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               byte *val(static_cast<byte *>(storage));
               *val = 0;
               for(int i = 0;
                   i < 8 && text[i] != 0 && (text[i] == '1' || text[i] == '0');
                   ++i)
               {
                  if(i > 0)
                     *val = (*val) << 1;
                  (*val) |= (text[i] == '1' ? 1 : 0);
               }
            }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               byte const *val(static_cast<byte const *>(storage));
               byte temp(*val);
               for(int i = 0; i < 8; ++i)
               {
                  byte mask(1 << i);
                  if((temp & mask) != 0)
                     out << "1";
                  else
                     out << "0";
               }
            }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               byte const *val(static_cast<byte const *>(storage));
               byte temp(*val);
               for(int i = 0; i < 8; ++i)
               {
                  byte mask(1 << i);
                  if((temp & mask) != 0)
                     out << L"1";
                  else
                     out << L"0";
               }
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 1; }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "Char(8)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"Char(8)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VBool8(description); }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            {
               byte const *val(static_cast<byte const *>(storage));
               buffer.append(val, 1);
            }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               byte *val(static_cast<byte *>(storage));
               *val = 0;
            }

            ////////////////////////////////////////////////////////////
            // format_json
            ////////////////////////////////////////////////////////////
            virtual void format_json(std::ostream &out)
            {
               out << "\"";
               format(out);
               out << "\"";
            }

            ////////////////////////////////////////////////////////////
            // quote_when_formatting
            ////////////////////////////////////////////////////////////
            virtual bool quote_when_formatting(CustomCsvOptions const *options) const
            { return true; }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return true; }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               byte const *val(static_cast<byte const *>(storage));
               dest = *val;
               return true;
            }
         };


         ////////////////////////////////////////////////////////////
         // class VSignedByte
         ////////////////////////////////////////////////////////////
         class VSignedByte: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VSignedByte(desc_handle &desc): Value(desc) { }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               char *val = static_cast<char *>(storage);
               *val = atoi(text);
            }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               char *val = static_cast<char *>(storage);
               out << static_cast<int>(*val);
            }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               char *val = static_cast<char *>(storage);
               out << static_cast<int>(*val);
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 1; }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "VARCHAR(1)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"VARCHAR(1)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VSignedByte(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               char *val = static_cast<char *>(storage);
               dest = static_cast<double>(*val);
               return true;
            }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               char *val = static_cast<char *>(storage);
               *val = 0;
            }
         };

         
         ////////////////////////////////////////////////////////////
         // class VByte
         ////////////////////////////////////////////////////////////
         class VByte: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VByte(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               byte *val = static_cast<byte *>(storage);
               *val = static_cast<byte>(strtoul(text,0,10));
            }
               
            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               byte *val = static_cast<byte *>(storage);
               out << static_cast<uint2>(*val);
            }

            ////////////////////////////////////////////////////////////
            // format (wide version)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               byte *val = static_cast<byte *>(storage);
               out << static_cast<uint2>(*val);
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 1; }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const { out << "VARCHAR(1)"; }
            virtual void format_ldep_type(std::wostream &out) const { out << L"VARCHAR(1)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VByte(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               byte *val = static_cast<byte *>(storage);
               dest = static_cast<double>(*val);
               return true;
            }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               byte *val = static_cast<byte *>(storage);
               *val = 0;
            }
         };


         ////////////////////////////////////////////////////////////
         // class VInt2
         ////////////////////////////////////////////////////////////
         class VInt2: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VInt2(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // get_value
            ////////////////////////////////////////////////////////////
            int2 get_value() const
            {
               int2 rtn;
               memcpy(&rtn,storage,sizeof(rtn));
               if((Csi::is_big_endian() && description->data_type == CsiInt2Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiInt2))
                  Csi::reverse_byte_order(&rtn,sizeof(rtn));
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // set_value
            ////////////////////////////////////////////////////////////
            void set_value(int2 value)
            {
               memcpy(storage,&value,sizeof(value));
               if((Csi::is_big_endian() && description->data_type == CsiUInt2Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiUInt2))
                  Csi::reverse_byte_order(storage,sizeof(value));
            }
            
            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            { set_value(static_cast<uint2>(strtoul(text,0,10))); }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 2; }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); } 

            ////////////////////////////////////////////////////////////
            // format (wide version)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); } 

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "DECIMAL(5,0)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"DECIMAL(5,0)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VInt2(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               dest = static_cast<double>(get_value());
               return true;
            }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            { write_tob1_value(buffer,get_value()); }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return description->data_type == CsiInt2Lsf; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               int2 *val = static_cast<int2 *>(storage);
               *val = 0;
            }
         };
         

         ////////////////////////////////////////////////////////////
         // class VUInt2
         ////////////////////////////////////////////////////////////
         class VUInt2: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VUInt2(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // get_value
            ////////////////////////////////////////////////////////////
            uint2 get_value() const
            {
               uint2 rtn;
               memcpy(&rtn,storage,sizeof(rtn));
               if((Csi::is_big_endian() && description->data_type == CsiUInt2Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiUInt2))
                  Csi::reverse_byte_order(&rtn,sizeof(rtn));
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // set_value
            ////////////////////////////////////////////////////////////
            void set_value(uint2 value)
            {
               memcpy(storage,&value,sizeof(value));
               if((Csi::is_big_endian() && description->data_type == CsiUInt2Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiUInt2))
                  Csi::reverse_byte_order(storage,sizeof(value));
            }
            
            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            { set_value(static_cast<uint2>(strtoul(text,0,10))); }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 2; }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); } 

            ////////////////////////////////////////////////////////////
            // format (wide version)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); } 

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "DECIMAL(5,0)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"DECIMAL(5,0)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VUInt2(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               dest = static_cast<double>(get_value());
               return true;
            }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            { write_tob1_value(buffer,get_value()); }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return description->data_type == CsiUInt2Lsf; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               uint2 *val = static_cast<uint2 *>(storage);
               *val = 0;
            }
         };

         
         ////////////////////////////////////////////////////////////
         // class VUInt4
         ////////////////////////////////////////////////////////////
         class VUInt4: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VUInt4(desc_handle &desc): Value(desc) { }

            ////////////////////////////////////////////////////////////
            // get_value
            ////////////////////////////////////////////////////////////
            uint4 get_value() const
            {
               uint4 val;
               memcpy(&val,storage,sizeof(val));
               if((description->data_type == CsiUInt4Lsf && Csi::is_big_endian()) ||
                  (description->data_type == CsiUInt4 && !Csi::is_big_endian()))
                  Csi::reverse_byte_order(&val,sizeof(val));
               return val;
            }

            ////////////////////////////////////////////////////////////
            // set_value
            ////////////////////////////////////////////////////////////
            void set_value(uint4 val)
            {
               memcpy(storage,&val,sizeof(val));
               if((description->data_type == CsiUInt4Lsf && Csi::is_big_endian()) ||
                  (description->data_type == CsiUInt4 && !Csi::is_big_endian()))
                  Csi::reverse_byte_order(storage,sizeof(val));
            } 

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            { set_value(strtoul(text,0,10)); }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 4; }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); } 

            ////////////////////////////////////////////////////////////
            // format (wide version)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); } 

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "DECIMAL(10,0)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"DECIMAL(10,0)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VUInt4(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               dest = static_cast<double>(get_value());
               return true;
            }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            { write_tob1_value(buffer,get_value()); }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return description->data_type == CsiUInt4Lsf; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               uint4 *val = static_cast<uint4 *>(storage);
               *val = 0;
            }
         };

         
         ////////////////////////////////////////////////////////////
         // class VInt4
         ////////////////////////////////////////////////////////////
         class VInt4: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VInt4(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // get_value
            ////////////////////////////////////////////////////////////
            int4 get_value() const
            {
               int4 rtn;
               memcpy(&rtn,storage,sizeof(rtn));
               if((Csi::is_big_endian() && description->data_type == CsiInt4Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiInt4))
                  Csi::reverse_byte_order(&rtn,sizeof(rtn));
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // set_value
            ////////////////////////////////////////////////////////////
            void set_value(int4 value)
            {
               memcpy(storage,&value,sizeof(value));
               if((Csi::is_big_endian() && description->data_type == CsiInt4Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiInt4))
                  Csi::reverse_byte_order(storage,sizeof(value));
            }
            
            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            { set_value(strtol(text,0,10)); }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 4; }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); }

            ////////////////////////////////////////////////////////////
            // format (wide version)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "DECIMAL(10,0)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"DECIMAL(10,0)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VInt4(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               dest = static_cast<double>(get_value());
               return true;
            }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            { write_tob1_value(buffer,get_value()); }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return description->data_type == CsiInt4Lsf; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               int4 *val = static_cast<int4 *>(storage);
               *val = 0;
            }
         };

         
         ////////////////////////////////////////////////////////////
         // class VDouble
         ////////////////////////////////////////////////////////////
         class VDouble: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VDouble(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // get_value
            ////////////////////////////////////////////////////////////
            double get_value() const
            {
               byte temp[8];
               bool do_reverse =
                  (Csi::is_big_endian() && description->data_type == CsiIeee8Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiIeee8);
               memcpy(temp, storage, sizeof(temp));
               return Csi::build_double(temp, do_reverse);
            }

            ////////////////////////////////////////////////////////////
            // set_value
            ////////////////////////////////////////////////////////////
            void set_value(double value)
            {
               memcpy(storage,&value,sizeof(value));
               if((Csi::is_big_endian() && description->data_type == CsiIeee8Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiIeee8))
                  Csi::reverse_byte_order(storage,sizeof(value));
            }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               set_value(
                  csiStringToFloat(text, std::locale::classic()));
            }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 8; }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { 
               if(!overload_stream_attribs)
                  csiFloatToStream(
                     out,
                     get_value(),
                     15,
                     true,
                     15,
                     false,
                     specials_as_numbers,
                     optimise_without_locale);
               else
                  out << get_value(); 
            }

            ////////////////////////////////////////////////////////////
            // format (wide version)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { 
               if(!overload_stream_attribs)
               {
                  std::ostringstream temp;
                  if(!optimise_without_locale)
                     temp.imbue(out.getloc());
                  csiFloatToStream(
                     temp,
                     get_value(),
                     15,
                     true,
                     15,
                     false,
                     specials_as_numbers,
                     optimise_without_locale);
                  out << temp.str().c_str();
               }
               else
                  out << get_value(); 
            }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "FLOAT"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"FLOAT"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VDouble(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            {
               dest = static_cast<double>(get_value());
               return true;
            }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            { write_tob1_value(buffer,get_value()); }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            { set_value(std::numeric_limits<double>::quiet_NaN()); }
         };

         
         ////////////////////////////////////////////////////////////
         // class VFloat
         ////////////////////////////////////////////////////////////
         class VFloat: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VFloat(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // get_value
            ////////////////////////////////////////////////////////////
            float get_value() const
            {
               byte temp[4];
               bool do_reverse =
                  (Csi::is_big_endian() && description->data_type == CsiIeee4Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiIeee4);
               memcpy(temp, storage, sizeof(temp));
               return Csi::build_float(temp, do_reverse);
            }

            ////////////////////////////////////////////////////////////
            // set_value
            ////////////////////////////////////////////////////////////
            void set_value(float value)
            {
               memcpy(storage,&value,sizeof(value));
               if((Csi::is_big_endian() && description->data_type == CsiIeee4Lsf) ||
                  (!Csi::is_big_endian() && description->data_type == CsiIeee4))
                  Csi::reverse_byte_order(storage,sizeof(value));
            }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               set_value(
                  static_cast<float>(
                     csiStringToFloat(text, std::locale::classic())));
            } 

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 4; }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               if(!overload_stream_attribs)
                  csiFloatToStream(
                     out,
                     get_value(),
                     7,
                     true,
                     7,
                     false,
                     specials_as_numbers,
                     optimise_without_locale);
               else
                  out << get_value();
            }

            ////////////////////////////////////////////////////////////
            // format (wide)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               if(!overload_stream_attribs)
               {
                  std::ostringstream temp;
                  if(!optimise_without_locale)
                     temp.imbue(out.getloc());
                  csiFloatToStream(
                     temp,
                     get_value(),
                     7,
                     true,
                     7,
                     false,
                     specials_as_numbers,
                     optimise_without_locale);
                  out << temp.str().c_str();
               }
               else
                  out << get_value();
            }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "FLOAT"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"FLOAT"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VFloat(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            { dest = get_value(); return true; }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            { write_tob1_value(buffer,get_value()); }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return description->data_type == CsiIeee4Lsf; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            { set_value(std::numeric_limits<float>::quiet_NaN()); }
         };


         ////////////////////////////////////////////////////////////
         // class VFs2
         ////////////////////////////////////////////////////////////
         class VFs2: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VFs2(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 2; }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               csiFloatToStream(
                  out,
                  csiFs2ToDouble(storage),
                  5,
                  true,
                  5,
                  false,
                  specials_as_numbers,
                  optimise_without_locale);
            }

            ////////////////////////////////////////////////////////////
            // format (wide stream)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               std::ostringstream temp;
               if(!optimise_without_locale)
                  temp.imbue(out.getloc());
               csiFloatToStream(
                  temp,
                  csiFs2ToDouble(storage),
                  5,
                  true,
                  5,
                  false,
                  specials_as_numbers,
                  optimise_without_locale);
               out << temp.str().c_str();
            }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "FLOAT"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"FLOAT"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VFs2(description); }

            ////////////////////////////////////////////////////////////
            // to_float
            ////////////////////////////////////////////////////////////
            virtual bool to_float(double &dest) const
            { dest = csiFs2ToFloat(storage); return true; }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            { buffer.append(storage,2); }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return true; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               byte *val = static_cast<byte *>(storage);
               val[0] = 0x9f;
               val[1] = 0xfe;
            }

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            {
               double temp = csiStringToFloat(text, std::locale::classic());
               doubleToCsiFs2(storage, temp);
            }

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }
         };

         
         ////////////////////////////////////////////////////////////
         // VStamp
         ////////////////////////////////////////////////////////////
         class VStamp: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VStamp(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // get_value
            ////////////////////////////////////////////////////////////
            Csi::LgrDate get_value() const;

            ////////////////////////////////////////////////////////////
            // set_value
            ////////////////////////////////////////////////////////////
            void set_value(Csi::LgrDate const &value);

            ////////////////////////////////////////////////////////////
            // read_text
            ////////////////////////////////////////////////////////////
            virtual void read_text(char const *text)
            { 
               Csi::LgrDate timestamp;
               try
               {
                  timestamp = Csi::LgrDate::fromStr(text);
               }
               catch(std::exception &)
               { }

               set_value(timestamp); 
            }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Overloads the base class to return the formatted timestamp.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               Csi::Json::DateHandle rtn(new Csi::Json::Date(get_value()));
               return rtn.get_handle();
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return csiTypeLen(description->data_type); }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            {
               if(options && !options->get_embedded_as_string())
               {
                  get_value().format_custom_classic(
                     out,
                     options->get_embedded_format_flags());
               }
               else
               {
                  if(time_format)
                     get_value().format(out,time_format);
                  else
                     get_value().format(out, "%Y-%m-%d %H:%M:%S%x");
               }
            }

            ////////////////////////////////////////////////////////////
            // format (wide)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { 
               if( time_format )
                  get_value().format(out,time_format);
               else
                  get_value().format(out,L"%Y-%m-%d %H:%M:%S%x");
            }

            ////////////////////////////////////////////////////////////
            // format_json
            ////////////////////////////////////////////////////////////
            virtual void format_json(std::ostream &out)
            { get_value().format(out, "\"%Y-%m-%dT%H:%M:%S%x\""); }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "TIMESTAMP"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"TIMESTAMP"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VStamp(description); }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            {
               Csi::LgrDate stamp(get_value());
               write_tob1_value(buffer,stamp.get_sec());
               write_tob1_value(buffer,stamp.nsec());
            }

            ////////////////////////////////////////////////////////////
            // quote_when_formatting
            ////////////////////////////////////////////////////////////
            virtual bool quote_when_formatting(
               CustomCsvOptions const *options) const
            {
               bool rtn = true;
               if(options && !options->get_embedded_as_string())
                  rtn = false;
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // is_tob1_native
            ////////////////////////////////////////////////////////////
            virtual bool is_tob1_native() const
            { return description->data_type == CsiNSecLsf; }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null();
         };


         ////////////////////////////////////////////////////////////
         // class VInt8
         ////////////////////////////////////////////////////////////
         class VInt8: public Value
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VInt8(desc_handle &desc):
               Value(desc)
            { }

            ////////////////////////////////////////////////////////////
            // read_json
            ////////////////////////////////////////////////////////////
            virtual void read_json(Csi::Json::ValueBase *json);

            /**
             * @return Returns a JSON number.
             */
            virtual Csi::Json::ValueHandle write_json()
            {
               double val;
               to_float(val);
               Csi::Json::NumberHandle rtn(new Csi::Json::Number(val));
               return rtn.get_handle();
            }
            
            ////////////////////////////////////////////////////////////
            // get_value
            ////////////////////////////////////////////////////////////
            int8 get_value() const
            {
               int8 rtn;
               memcpy(&rtn,storage,sizeof(rtn));
               if((!Csi::is_big_endian() && description->data_type == CsiInt8) ||
                  (Csi::is_big_endian() && description->data_type == CsiInt8Lsf))
                  Csi::reverse_byte_order(&rtn,sizeof(rtn));
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // set_value
            ////////////////////////////////////////////////////////////
            void set_value(int8 value)
            {
               memcpy(storage,&value,sizeof(value));
               if((!Csi::is_big_endian() && description->data_type == CsiInt8) ||
                  (Csi::is_big_endian() && description->data_type == CsiInt8Lsf))
                  Csi::reverse_byte_order(storage,sizeof(value));
            }

            ////////////////////////////////////////////////////////////
            // get_pointer_len
            ////////////////////////////////////////////////////////////
            virtual uint4 get_pointer_len() const
            { return 8; }
            
            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::ostream &out,
               bool overload_stream_attribs = false,
               char const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); }

            ////////////////////////////////////////////////////////////
            // format (wide)
            ////////////////////////////////////////////////////////////
            virtual void format(
               std::wostream &out,
               bool overload_stream_attribs = false,
               wchar_t const *time_format = 0,
               CustomCsvOptions const *options = 0,
               bool specials_as_numbers = false,
               bool optimise_without_locale = false,
               bool translate_strings = false) const
            { out << get_value(); }

            ////////////////////////////////////////////////////////////
            // format_ldep_type
            ////////////////////////////////////////////////////////////
            virtual void format_ldep_type(std::ostream &out) const
            { out << "DECIMAL(20,0)"; }
            virtual void format_ldep_type(std::wostream &out) const
            { out << L"DECIMAL(20,0)"; }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual Value *clone()
            { return new VInt8(description); }

            ////////////////////////////////////////////////////////////
            // write_tob1
            ////////////////////////////////////////////////////////////
            virtual void write_tob1(StrBin &buffer)
            {
               int8 val8(get_value());
               int4 value(static_cast<int4>(get_value()));

               if(val8 > std::numeric_limits<int4>::max())
                  value = std::numeric_limits<int4>::max();
               else if(val8 < -std::numeric_limits<int4>::max())
                  value = -std::numeric_limits<int4>::max();
               write_tob1_value(buffer, value);
            }

            ////////////////////////////////////////////////////////////
            // set_to_null
            ////////////////////////////////////////////////////////////
            virtual void set_to_null()
            {
               int8 *val = static_cast<int8 *>(storage);
               *val = 0;
            }
         };
      };
   };
};
#endif
