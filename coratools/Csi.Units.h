/* Csi.Units.h

   Copyright (C) 2018, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 16 November 2018
   Last Change: Tuesday 27 November 2018
   Last Commit: $Date: 2018-11-27 11:18:29 -0600 (Tue, 27 Nov 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Units_h
#define Csi_Units_h
#include "StrUni.h"
#include "Csi.SharedPtr.h"
#include <map>
#include <exception>


namespace Csi
{
   namespace Units
   {
      /**
       * Defines the base class for object that will convert from one unit to another.
       */
      class ConverterBase
      {
      protected:
         /**
          * Specifies the designation for the source unit.
          */
         StrUni const source_unit;

         /**
          * Specifies the designation for the destination unit.
          */
         StrUni const dest_unit;

      public:
         /**
          * Constructor
          *
          * @param source Specifies the source unit string.
          *
          * @param dest Specifies the dest unit string.
          */
         ConverterBase(StrUni const &source, StrUni const &dest):
            source_unit(source),
            dest_unit(dest)
         { }

         /**
          * @return Returns the source units for this converter.
          */
         StrUni const &get_source_unit() const
         { return source_unit; }

         /**
          * @return Returns the destination units for this converter.
          */
         StrUni const &get_dest_unit() const
         { return dest_unit; }

         /**
          * Destructor
          */
         virtual ~ConverterBase()
         { }

         /**
          * @return Returns the value in the destination units.
          *
          * @param val Specifies the value in the source units.
          */
         virtual double convert(double val) const = 0;
      };


      namespace ConverterHelpers
      {
         /**
          * Defines a comparator for unicode strings that uses case sensitive conversions
          */
         struct compare_no_case
         {
            bool operator() (StrUni const &s1, StrUni const &s2) const
            { return s1.compare(s2, true) < 0; }
         };
      };


      /**
       * Defines a component that organises a collection of unit converters and provides methods to
       * convert units as well as query what conversions are available for a given source.
       */
      class Converter
      {
      public:
         /**
          * Specifies the type of the collection of converters avaialble for a single unit.
          */
         typedef Csi::SharedPtr<ConverterBase> converter_handle;
         typedef std::map<StrUni, converter_handle, ConverterHelpers::compare_no_case> unit_converters_type;
         typedef std::map<StrUni, unit_converters_type, ConverterHelpers::compare_no_case> converters_type;
      private:
         converters_type converters;

      public:
         /**
          * Constructor
          */
         Converter();

         /**
          * Destructor
          */
         virtual ~Converter();

         /**
          * @return Returns the specified value converted from the specified source units to the
          * specified destination units.
          *
          * @param val Specifies the value to be converted.
          *
          * @param source_units Specifies the name of the source units.
          *
          * @param dest_units Specifies the name of the dest units.
          */
         double convert(double val, StrUni const &source_units, StrUni const &dest_units) const;

         /**
          * @param Specifis the conversion to add.
          */
         void add_converter(converter_handle converter)
         {
            converters_type::iterator uci(converters.find(converter->get_source_unit()));
            if(uci == converters.end())
               uci = converters.insert(std::make_pair(converter->get_source_unit(), unit_converters_type())).first;
            uci->second.insert(std::make_pair(converter->get_dest_unit(), converter));
         }
      };
   };
};


#endif

