/* Csi.Units.cpp

   Copyright (C) 2018, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 16 November 2018
   Last Change: Wednesday 13 March 2019
   Last Commit: $Date: 2019-03-13 13:25:17 -0600 (Wed, 13 Mar 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Units.h"
#include <deque>
#include <set>
#include <limits>


namespace Csi
{
   namespace Units
   {
      namespace
      {
         struct LinearConverter: public ConverterBase
         {
            double const multiplier;
            double const offset;
            bool const inverse;
            
            LinearConverter(
               StrUni const &source,
               StrUni const &dest,
               bool const inverse_,
               double multiplier_,
               double offset_):
               ConverterBase(source, dest),
               multiplier(multiplier_),
               offset(offset_),
               inverse(inverse_)
            { }

            virtual double convert(double val) const
            {
               double rtn = std::numeric_limits<double>::quiet_NaN();
               if(!inverse)
                  rtn = val * multiplier + offset;
               else
                  rtn = (val - offset) / multiplier;
               return rtn;
            }
         };


         /**
          * Defines the set of linear conversions that we can support.  This list will be terminated
          * when both the source and dest strings are empty.
          */
         struct supported_linear_conversion_type
         {
            char const *source;
            char const *dest;
            double multiplier;
            double offset;
         } supported_linear_conversions[] =
         {
            { "m", "km", 1E-3, 0 },
            { "m", "cm", 1E2, 0 },
            { "m", "mm", 1E3, 0 },
            { "m", "\xce\xbcm", 1E6, 0 },
            { "m", "nm", 1E9, 0 },
            { "m", "ft", 3.28084, 0 },
            { "ft", "in", 12.0, 0 },
            { "ft", "yd", 3, 0 },
            { "ft", "mi", 0.000189394, 0 },
            { "ft\xc2\xb3", "gal", 0.133681, 0 },
            { "m\xc2\xb2", "km\xc2\xb2", 1E-6, 0 },
            { "m\xc2\xb2", "ha", 1E-4, 0 },
            { "m\xc2\xb2", "ft\xc2\xb2", 10.7639, 0 },
            { "m\xc2\xb3", "ft\xc2\xb3", 35.3147, 0 },
            { "m\xc2\xb3", "L", 1E3, 0 },
            { "m\xc2\xb3", "mL", 1E6, 0 },
            { "m\xc2\xb3", "\xe2\xbcL", 1E6, 0 },
            { "L", "gal", 0.26, 0 },
            { "gal", "qt", 4, 0 },
            { "gal", "pt", 8, 0 },
            { "gal", "cup", 15.7725, 0 },
            { "gal", "oz", 128, 0 },
            { "g", "kg", 1E-3, 0 },
            { "g", "mg", 1E3, 0 },
            { "g", "\xce\xbcg", 1E6, 0 },
            { "m/s", "km/h", 3.6, 0 },
            { "m/s", "MPH", 2.23694, 0 },
            { "MPH", "mph", 1, 0 },
            { "MPH", "mi/h", 1, 0 },
            { "m/s", "c", 299792458, 0 },
            { "MPH", "ft/s", 1.46667, 0 },
            { "degC", "\xc2\xb0\x43", 1, 0 },
            { "C", "\xc2\xb0\x43", 1, 0 },
            { "degF", "\xc2\xb0\x46", 1, 0 },
            { "\xc2\xb0\x43", "\xc2\xb0\x46", 1.8, 32 },
            { "\xc2\xb0\x43", "K", 1.0, 273.15 },
            { "s", "ms", 1E3, 0 },
            { "s", "\xce\xbcs", 1E6, 0 },
            { "s", "ns", 1E9, 0 },
            { "s", "min", 1.0/60.0, 0 },
            { "s", "hr", 1.0/3600.0, 0 },
            { "s", "day", 1.0/86400.0, 0 },
            { "s", "week", 1.0/604800.0, 0 },
            { "kPa", "Pa", 1E3, 0 },
            { "kPa", "b", 0.01, 0 },
            { "kPa", "mmHg", 7.500616827, 0 },
            { "kPa", "psi", 0.145038, 0 },
            { "kPa", "atm", 0.00986923, 0 },
            { "kPa", "inHg", 0.2953, 0 },
            { "W/m^2", "W/m\xc2\xb2", 1, 0 },
            { 0, 0, 0, 0 }
         };
      };
      


      Converter::Converter()
      {
         for(size_t i = 0; supported_linear_conversions[i].source != 0; ++i)
         {
            add_converter(
               new LinearConverter(
                  supported_linear_conversions[i].source,
                  supported_linear_conversions[i].dest,
                  false,
                  supported_linear_conversions[i].multiplier,
                  supported_linear_conversions[i].offset));
            add_converter(
               new LinearConverter(
                  supported_linear_conversions[i].dest,
                  supported_linear_conversions[i].source,
                  true,
                  supported_linear_conversions[i].multiplier,
                  supported_linear_conversions[i].offset));
         }
      } // constructor


      Converter::~Converter()
      { converters.clear(); }


      double Converter::convert(double val, StrUni const &source_units, StrUni const &dest_units) const
      {
         typedef std::deque<converter_handle> chain_type;
         typedef std::pair<StrUni, chain_type> stack_elem_type;
         typedef std::deque<stack_elem_type> stack_type;
         typedef std::set<StrUni, ConverterHelpers::compare_no_case> visited_units_type;
         double rtn(val);
         chain_type selected_chain;
         
         if(source_units.compare(dest_units, true) != 0)
         {
            // there may not be a direct conversion from the source to the destination units so we
            // might have to perform a recursive search.  We will do this by using a stack and building
            // a chain of converters that can be called to perform the conversion
            stack_type stack;
            visited_units_type visited_units;

            stack.push_back(stack_elem_type(source_units, chain_type()));
            visited_units.insert(source_units);
            while(!stack.empty() && selected_chain.empty())
            {
               stack_elem_type elem(stack.front());
               converters_type::const_iterator ci(converters.find(elem.first));
               stack.pop_front();
               if(ci != converters.end())
               {
                  // there is a chance we can find the direct conversion on this step using the
                  // find() method.  if that doesn't work, will will have to push all of the
                  // previously unvisited conversions at this level onto the stack.
                  unit_converters_type::const_iterator uci(ci->second.find(dest_units));
                  if(uci != ci->second.end())
                  {
                     selected_chain = elem.second;
                     selected_chain.push_back(uci->second);
                     break;
                  }
                  else
                  {
                     for(uci = ci->second.begin(); uci != ci->second.end(); ++uci)
                     {
                        if(visited_units.find(uci->first) == visited_units.end())
                        {
                           stack_elem_type next_elem(uci->first, elem.second);
                           next_elem.second.push_back(uci->second);
                           stack.push_back(next_elem);
                           visited_units.insert(uci->first);
                        }
                     }
                  }
               }
            }
         }

         // we will now run through the chain of conversions needed.
         for(chain_type::const_iterator ci = selected_chain.begin(); ci != selected_chain.end(); ++ci)
            rtn = (*ci)->convert(rtn);
         return rtn;
      } // convert
   };
};
