/* CsiUnits.js

   Copyright (C) 2018, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 27 November 2018
   Last Change: Friday 17 May 2019
   Last Commit: $Date: 2019-05-17 12:18:37 -0600 (Fri, 17 May 2019) $
   Last Changed by: $Author: jon $

*/


/**
 * Defines an object that apecifies the names of source and destination units as well as a function
 * that will convert a number based on the source and dest units.
 *
 * @param {string} source_unit Specifies the name of the source unit.
 *
 * @param {string} dest_unit Specifis the name of the destination unit.
  */
function CsiUnitConverterBase(source_unit, dest_unit)
{
   if(source_unit !== undefined && dest_unit !== undefined)
   {
      this.source_unit = source_unit;
      this.dest_unit = dest_unit;
   }
}


/**
 * @return {number} Returns the value parameter with the unit conversion function applied.
 *
 * @param {number} Specifies the value to convert.
 */
CsiUnitConverterBase.prototype.convert = function(value)
{
   return value;
};



/**
 * Defines an object that performs a linear transformation (multiplier and offset) for
 * unit conversion.
 *
 * @param source_unit {string} Specifies the name of the source unit.
 *
 * @param dest_unit {string} Specifies the name of the dest unit.
 *
 * @param invert {boolean} Set to true if the conversion should be inverted.
 *
 * @param multiplier {number} Specifies the multiplier that should be applied.
 *
 * @param offset {number} Specifies the offset that should be applied.
 */
function CsiUnitConverterLinear(source_unit, dest_unit, invert, multiplier, offset)
{
   if(source_unit !== undefined)
   {
      CsiUnitConverterBase.call(this, source_unit, dest_unit);
      this.invert = invert;
      this.multiplier = multiplier;
      this.offset = offset;
   }
}
CsiUnitConverterLinear.prototype = new CsiUnitConverterBase();


CsiUnitConverterLinear.prototype.convert = function(value)
{
   var rtn;
   if(this.invert)
      rtn = (value - this.offset) / this.multiplier;
   else
      rtn = value * this.multiplier + this.offset;
   return rtn;
};


/**
 * Defines a JavaScript component that can be used to translate between similar units which are identified
 * by their case sensitive NIST abbreviations.
 */
function CsiUnits()
{
   var standard_conversions;
   var units = this;
   if(this instanceof CsiUnits)
   {
      this.converters = {};
      standard_conversions = [
         { source_unit: "m", dest_unit: "km", multiplier: 1E-3, offset: 0 },
         { source_unit: "m", dest_unit: "cm", multiplier: 1E2, offset: 0 },
         { source_unit: "m", dest_unit: "mm", multiplier: 1E3, offset: 0 },
         { source_unit: "m", dest_unit: "\u03bcm", multiplier: 1E6, offset: 0 },
         { source_unit: "m", dest_unit: "nm", multiplier: 1E9, offset: 0 },
         { source_unit: "m", dest_unit: "ft", multiplier: 3.28084, offset: 0 },
         { source_unit: "ft", dest_unit: "in", multiplier: 12.0, offset: 0 },
         { source_unit: "ft", dest_unit: "mi", multiplier: 0.000189394, offset: 0 },
         { source_unit: "ft\u00b2", dest_unit: "gal", multiplier: 0.133681, offset: 0 },
         { source_unit: "m\u00b2", dest_unit: "km\u00b2", multiplier: 1E-6, offset: 0 },
         { source_unit: "m\u00b2", dest_unit: "ha", multiplier: 1E-4, offset: 0 },
         { source_unit: "m\u00b2", dest_unit: "ft\u00b2", multiplier: 10.7639, offset: 0 },
         { source_unit: "m\u00b3", dest_unit: "ft\u00b3", multiplier: 35.3147, offset: 0 },
         { source_unit: "m\u00b3", dest_unit: "L", multiplier: 1E3, offset: 0 },
         { source_unit: "L", dest_unit: "mL", multiplier: 1E3, offset: 0 },
         { source_unit: "L", dest_unit: "\u2c98L", multiplier: 1E6, offset: 0 },
         { source_unit: "L", dest_unit: "gal", multiplier: 0.26, offset: 0 },
         { source_unit: "gal", dest_unit: "qt", multiplier: 4, offset: 0 },
         { source_unit: "gal", dest_unit: "pt", multiplier: 8, offset: 0 },
         { source_unit: "gal", dest_unit: "cup", multiplier: 15.7725, offset: 0 },
         { source_unit: "gal", dest_unit: "oz", multiplier: 128, offset: 0 },
         { source_unit: "g", dest_unit: "kg", multiplier: 1E-3, offset: 0 },
         { source_unit: "g", dest_unit: "mg", multiplier: 1E3, offset: 0 },
         { source_unit: "g", dest_unit: "\u03bcg", multiplier: 1E-6, offset: 0 },
         { source_unit: "m/s", dest_unit: "km/h", multiplier: 3.6, offset: 0 },
         { source_unit: "m/s", dest_unit: "mph", multiplier: 2.23694, offset: 0 },
         { source_unit: "mph", dest_unit: "MPH", multiplier: 1, offset: 0 },
         { source_unit: "mph", dest_unit: "mi/h", multiplier: 1, offset: 0 },
         { source_unit: "m/s", dest_unit: "c", multiplier: 299792458, offset: 0 },
         { source_unit: "mph", dest_unit: "ft/s", multiplier: 1.46667, offset: 0 },
         { source_unit: "degC", dest_unit: "\u00b0C", multiplier: 1, offset: 0 },
         { source_unit: "\u00b0C", dest_unit: "\u00b0F", multiplier: 1.8, offset: 32 },
         { source_unit: "degF", dest_unit: "\u00b0F", multiplier: 1, offset: 0 },
         { source_unit: "\u00b0C", dest_unit: "K", multiplier: 1.0, offset: 273.15 },
         { source_unit: "s", dest_unit: "\u03bcs", multiplier: 1E6, offset: 0 },
         { source_unit: "s", dest_unit: "ms", multiplier: 1E3, offset: 0 },
         { source_unit: "s", dest_unit: "ns", multiplier: 1E9, offset: 0 },
         { source_unit: "s", dest_unit: "min", multiplier: 1.0/60.0, offset: 0 },
         { source_unit: "s", dest_unit: "hr", multiplier: 1.0/3600.0, offset: 0 },
         { source_unit: "s", dest_unit: "day", multiplier: 1.0/86400.0, offset: 0 },
         { source_unit: "s", dest_unit: "week", multiplier: 1.0/604800.0, offset: 0 },
         { source_unit: "kPa", dest_unit: "Pa", multiplier: 1E3, offset: 0 },
         { source_unit: "kPa", dest_unit: "b", multiplier: 0.01, offset: 0 },
         { source_unit: "kPa", dest_unit: "mmHg", multiplier: 7.500616827, offset: 0 },
         { source_unit: "kPa", dest_unit: "psi", multiplier: 0.145038, offset: 0 },
         { source_unit: "kPa", dest_unit: "atm", multiplier: 0.00986923, offset: 0 },
         { source_unit: "kPa", dest_unit: "inHg", multiplier: 0.2953, offset: 0 },
         { source_unit: "KPa", dest_unit: "kPa", multiplier: 1, offset: 0 },
         { source_unit: "W/m^2", dest_unit: "W/m\u00b2", multiplier: 1, offset: 0 },
         { source_unit: "C", dest_unit: "\u00b0C", multiplier: 1, offset: 0 }
      ]
      standard_conversions.forEach(function(conversion) {
         units.add_converter(new CsiUnitConverterLinear(conversion.source_unit, conversion.dest_unit, false, conversion.multiplier, conversion.offset));
         units.add_converter(new CsiUnitConverterLinear(conversion.dest_unit, conversion.source_unit, true, conversion.multiplier, conversion.offset));
      });
   }
}


/**
 * Adds the specified converter object to the set of supported conversions.
 *
 ' @param {CsiConverterBase} converter Specifies the object that will do the conversion.
 */
CsiUnits.prototype.add_converter = function(converter)
{
   var unit_converter;
   if(this.converters.hasOwnProperty(converter.source_unit))
      unit_converter = this.converters[converter.source_unit];
   else
   {
      unit_converter = { };
      this.converters[converter.source_unit] = unit_converter;
   }
   unit_converter[converter.dest_unit] = converter;
};


/**
 * @return {number} Returns the value after the required set of units conversions have been applied.
 *
 * @param {number} val Specifies the value to convert.
 *
 * @param {string} source_units Specifies the source units for the value.
 *
 * @param {string} dest_units Specifies the destination units for the value.
 */
CsiUnits.prototype.convert = function(val, source_units, dest_units)
{
   var rtn = val;
   var selected_chain = null;
   var stack = [];
   var visited = new Set();
   var stack_elem;
   var current_source;
   var units = this;
   
   if(source_units != dest_units)
   {
      // there may not be a direct conversion from the source unit to the dest unit so we might have
      // to search recursively.  We will do this by using a stack and forming a chain of converter
      // objects that will be able to make the conversion.
      stack.push({unit: source_units, chain: [] });
      visited.add(source_units);
      while(stack.length > 0 && selected_chain === null)
      {
         stack_elem = stack.shift();
         if(this.converters.hasOwnProperty(stack_elem.unit))
         {
            current_source = this.converters[stack_elem.unit];
            if(current_source.hasOwnProperty(dest_units))
            {
               selected_chain = stack_elem.chain;
               selected_chain.push(current_source[dest_units]);
               break;
            }
            else
            {
               $.each(current_source, function(key, converter) {
                  var next_elem;
                  if(!visited.has(key))
                  {
                     visited.add(key);
                     next_elem = { unit: key, chain: stack_elem.chain.slice() };
                     next_elem.chain.push(converter);
                     stack.push(next_elem);
                  }
               });
            }
         }
      }
   }

   // we can now apply the chain of converters.
   if(selected_chain)
   {
      selected_chain.forEach(function(converter) {
         rtn = converter.convert(rtn);
      });
   }
   return rtn;
};


