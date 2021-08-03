/* Csi.PakBus.Bmp5TableDef.h

   Copyright (C) 2014, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 11 February 2014
   Last Change: Thursday 26 February 2015
   Last Commit: $Date: 2015-02-26 11:59:38 -0600 (Thu, 26 Feb 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_Bmp5TableDef_h
#define Csi_PakBus_Bmp5TableDef_h

#include "Csi.PakBus.Bmp5Message.h"
#include "Csi.ArrayDimensions.h"
#include "Cora.Broker.ValueName.h"
#include "CsiTypes.h"
#include "trace.h"
#include <deque>


namespace Csi
{
   namespace PakBus
   {
      namespace Bmp5TableDefHelpers
      {
         /**
          * Defines the meta-data for a table field.
          */
         class FieldDef
         {
         public:
            /**
             * Default Constructor
             */
            FieldDef():
               data_type(LgrUnknown),
               begin_index(1),
               piece_size(1),
               read_only(false)
            { }

            /**
             * Copy constructor
             *
             * @param other The field from which to copy
             */
            FieldDef(FieldDef const &other):
               data_type(other.data_type),
               name(other.name),
               processing(other.processing),
               units(other.units),
               description(other.description),
               begin_index(other.begin_index),
               array_dimensions(other.array_dimensions),
               read_only(other.read_only),
               piece_size(other.piece_size)
            { }

            /**
             * Destructor
             */
            virtual ~FieldDef()
            { }

            /**
             * Copy operator
             *
             * @param other The field from which to copy.
             * @return Returns a reference to this object
             */
            FieldDef &operator =(FieldDef const &other)
            {
               data_type = other.data_type;
               read_only = other.read_only;
               name = other.name;
               processing = other.processing;
               units = other.units;
               description = other.description;
               begin_index = other.begin_index;
               piece_size = other.piece_size;
               array_dimensions = other.array_dimensions;
               return *this;
            }

            /**
             * @return Returns the field data type code.
             */
            byte get_data_type() const
            { return data_type; }

            /**
             * @return Returns true if this field is read-only.
             */
            bool get_read_only() const
            { return read_only; }

            /**
             * @return Returns the field name (without array subscripts.
             */
            StrUni const &get_name() const
            { return name; }

            /**
             * @return Returns the field processing string.
             */
            StrUni const &get_processing() const
            { return processing; }

            /**
             * @return Returns the field units string.
             */
            StrUni const &get_units() const
            { return units; }

            /**
             * @return the description string for this field.
             */
            StrUni const &get_description() const
            { return description; }

            /**
             * @return Returns the one based row major linear offset of this field.
             */
            uint4 get_begin_index() const
            { return begin_index; }

            /**
             * @return Returns the number of array element of this field.
             */
            uint4 get_piece_size() const
            { return piece_size; }

            /**
             * @return Returns the one based row major linear offset of this field.
             */
            uint4 get_end_index() const
            { return begin_index + piece_size - 1; }

            /**
             * @return Returns the array dimensions for this field.
             */
            ArrayDimensions const &get_array_dimensions() const
            { return array_dimensions; }

            /**
             * Reads the meta-data for this field from the provided message.
             *
             * @param in  A BMP5 message with its read index positioned at the data type for the field.
             * @return Returns true if the fields were read or false if the data type was zero.
             */
            bool read(Bmp5Message &in)
            {
               bool rtn(false);
               data_type = in.readByte();
               if(data_type != 0)
               {
                  uint4 dim;
                  StrAsc temp;
                  read_only = ((data_type & 0x80) != 0);
                  data_type &= 0x7f;
                  in.readAsciiZ(temp);
                  name = temp;
                  in.movePast(1); // reserved
                  in.readAsciiZ(temp);
                  processing = temp;
                  in.readAsciiZ(temp);
                  units = temp;
                  in.readAsciiZ(temp);
                  description = temp;
                  begin_index = in.readUInt4();
                  piece_size = in.readUInt4();
                  array_dimensions.clear();
                  while((dim = in.readUInt4()) != 0)
                     array_dimensions.add_dimension(dim);
                  rtn = true;
               }
               return rtn;
            }

         private:
            /**
             * Specifies the field data type.
             */
            byte data_type;

            /**
             * Specifies whether this field is read only.
             */
            bool read_only;
            
            /**
             * Specifies the field name.
             */
            StrUni name;

            /**
             * Specifies the field processing string.
             */
            StrUni processing;

            /**
             * Specifies the field units string.
             */
            StrUni units;

            /**
             * Specifies the description string.
             */
            StrUni description;

            /**
             * Specifies the row-major one based index for this piece.
             */
            uint4 begin_index;

            /**
             * Specifies the number of array elements in this piece.
             */
            uint4 piece_size;

            /**
             * Specifies the array dimensions for this piece.
             */
            ArrayDimensions array_dimensions;
         };
      };

      
      /**
       * This class represents the information derived from BMP5 table definition file formats as
       * sent by the CR1000.
       */
      class Bmp5TableDef
      {
      public:
         /**
          * Default constructor.
          */
         Bmp5TableDef():
            time_into(0),
            table_interval(0),
            time_type(LgrUnknown)
         { }

         /**
          * Copy constructor
          *
          * @param other Specifies the table def object from which we will copy fields.
          */
         Bmp5TableDef(Bmp5TableDef const &other):
            name(other.name),
            time_type(other.time_type),
            table_size(other.table_size),
            time_into(other.time_into),
            table_interval(other.table_interval),
            fields(other.fields)
         { }

         /**
          * Copy operator
          *
          * @param other Specifies the table def object from which we will copy fields.
          * @return Returns a reference to this object.
          */
         Bmp5TableDef &operator =(Bmp5TableDef const &other)
         {
            name = other.name;
            time_type = other.time_type;
            table_size = other.table_size;
            time_into = other.time_into;
            table_interval = other.table_interval;
            fields = other.fields;
            return *this;
         }

         /**
          * @return Returns the table name
          */
         StrUni const &get_name() const
         { return name; }

         /**
          * @return Returns the data type code for record time stamps.
          */
         byte get_time_type() const
         { return time_type; }

         /**
          * @return Returns the number of records allocated for the table.
          */
         uint4 get_table_size() const
         { return table_size; }

         /**
          * @return Returns the time into the table interval at which records will be stored in
          * units of nanoseconds.
          */
         int8 get_time_into() const
         { return time_into; }

         /**
          * @return Returns the table interval in nanoseconds.
          */
         int8 get_table_interval() const
         { return table_interval; }

         /**
          * @return Returns the begin iterator for the fields list.
          */
         typedef Bmp5TableDefHelpers::FieldDef field_type;
         typedef std::deque<field_type> fields_type;
         typedef fields_type::iterator iterator;
         typedef fields_type::const_iterator const_iterator;
         iterator begin()
         { return fields.begin(); }
         const_iterator begin() const
         { return fields.begin(); }

         /**
          * @return Returns the end iterator for the fields list.
          */
         iterator end()
         { return fields.end(); }
         const_iterator end() const
         { return fields.end(); }

         /**
          * @return Returns the number of fields in this table.
          */
         typedef fields_type::size_type size_type;
         size_type size() const
         { return fields.size(); }

         /**
          * @return Returns true if the fields list is empty.
          */
         bool empty() const
         { return fields.empty(); }

         /**
          * Clears the field list.
          */
         void clear()
         { fields.clear(); }

         /**
          * Initialises this table definition from the provided message.
          *
          * @param in Specifies the message buffer.  The read index of this buffer should be set to
          * the start of the table name.
          */
         void read(Bmp5Message &in)
         {
            uint4 sec, nsec;
            field_type field;
            StrAsc temp;

            in.readAsciiZ(temp);
            name = temp;
            table_size = in.readUInt4();
            time_type = in.readByte();
            sec = in.readUInt4();
            nsec = in.readUInt4();
            time_into = sec * LgrDate::nsecPerSec + nsec;
            sec = in.readUInt4();
            nsec = in.readUInt4();
            table_interval = sec * LgrDate::nsecPerSec + nsec;
            fields.clear();
            while(field.read(in))
               fields.push_back(field);
         }

      private:
         /**
          * Specifies the name of this table.
          */
         StrUni name;

         /**
          * Specifies the timestamp data type code.
          */
         byte time_type;

         /**
          * Specifies the number of records allocated for this table.
          */
         uint4 table_size;

         /**
          * Specifies the time offset into the interval for storing records in this table in units
          * of nanoseconds.
          */
         int8 time_into;

         /**
          * Specifies the interval for storing records in this table in units of nanoseconds.
          */
         int8 table_interval;

         /**
          * Specifies the list of fields for this table.
          */
         fields_type fields;
      };


      /**
       * Reads the table definitions from the specified file name.
       *
       * @param table_defs  Specifies a container of table definitions.  This function will add
       * table definitions to this list as they are read.
       *
       * @param tdf_file_name  Specifies the name and path of a file that contains the table
       * definitions.
       */
      void read_table_defs(std::deque<Bmp5TableDef> &table_defs, StrUni const &tdf_file_name);


      /**
       * Defines a predicate that can be used to determine if a table def with the specified name
       * exists.
       */
      class table_has_name
      {
      public:
         StrUni name;
         table_has_name(StrUni const &name_):
            name(name_)
         { }

         bool operator ()(Bmp5TableDef &table) const
         { return table.get_name() == name; }
      };


      /**
       * Defines a predicate that can be used to determine if a field has the specified name (with
       * optional array dimensions.
       */
      class field_has_name
      {
      public:
         Cora::Broker::ValueName name;
         field_has_name(StrUni const &name_):
            name(name_.c_str())
         { }

         typedef Bmp5TableDefHelpers::FieldDef field_type;
         bool operator ()(field_type const &field) const
         {
            bool rtn(false);
            if(field.get_name() == name.get_column_name())
            {
               ArrayDimensions dims(field.get_array_dimensions());
               uint4 offset(dims.to_offset(name.begin(), name.end()));
               if(offset >= field.get_begin_index() && offset <= field.get_end_index())
                  rtn = true;
            }
            return rtn;
         }
      };
   };
};


#endif

