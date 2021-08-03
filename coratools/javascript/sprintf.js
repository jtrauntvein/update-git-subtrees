/* sprintf.js

   Copyright (C) 2011, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 14 February 2011
   Last Change: Tuesday 02 June 2020
   Last Commit: $Date: 2020-06-02 11:59:10 -0600 (Tue, 02 Jun 2020) $
   Last Changed by: $Author: jon $

*/


function sprintf()
{
   var state_normal = 0;
   var state_flags = 1;
   var state_width = 2;
   var state_precision = 3;
   var state_conv = 4;   
   var format_spec = arguments[0].toString();
   var current_argument = 1;
   var pad_zero = false;
   var alternate = false;
   var add_sign = false;
   var left_justify = false;
   var width = 0;
   var precision = 6;
   var i = 0;
   var j = 0;
   var format_spec_len = format_spec.length;
   var state = state_normal;
   var rtn = String();
   var do_next_char = false;
   var ch;
   var scratch = String();
   var value;
   var locale_prototype = Number(1024.25).toLocaleString();
   var thousands_sep = locale_prototype.charAt(1);
   var decimal_point = locale_prototype.charAt(5);

   if(thousands_sep.localeCompare('.') !== 0 && thousands_sep.localeCompare(',') !== 0)
      thousands_sep = ',';
   if(decimal_point.localeCompare('.') !== 0 && decimal_point.localeCompare(',') !== 0)
      decimal_point = '.';
   while(i < format_spec_len)
   {
      ch = format_spec.charAt(i);
      do_next_char = true;
      if(state === state_normal)
      {
         if(ch === '%')
         {
            pad_zero = false;
            alternate = false;
            width = 0;
            precision = 6;
            add_sign = false;
            left_justify = false;
            state = state_flags;
         }
         else
            rtn += ch;
      }
      else if(state === state_flags)
      {
         switch(ch)
         {
         case '%':
            state = state_normal;
            rtn += '%';
            break;
            
         case '-':
            left_justify = true;
            break;
            
         case '+':
            add_sign = true;
            break;
            
         case '#':
            alternate = true;
            break;
            
         case ' ':
            // this doesn't seem to make any difference with boost::format()
            break;
            
         case '0':
            pad_zero = true;
            break;
            
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
         case '*':
            state = state_width;
            do_next_char = false;
            scratch = "";
            break;

         case '.':
            state = state_precision;
            scratch = "";
            break;

         default:
            state = state_conv;
            do_next_char = false;
            break;
         }
      }
      else if(state === state_width)
      {
         switch(ch)
         {
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            scratch += ch;
            break;

         case '*':
            width = Number(arguments[current_argument++]);
            state = state_precision;
            break;
            
         case '.':
            width = Number(scratch);
            scratch = "";
            state = state_precision;
            break;

         default:
            width = Number(scratch);
            do_next_char = false;
            state = state_conv;
            break;
         }
      }
      else if(state === state_precision)
      {
         switch(ch)
         {
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            scratch += ch;
            break;

         case '*':
            precision = Number(arguments[current_argument++]);
            state = state_conv;
            break;

         default:
            precision = Number(scratch);
            do_next_char = false;
            state = state_conv;
            break;
         }
      }
      else if(state === state_conv)
      {
         switch(ch)
         {
         case 'd':              // integer conversion
         case 'i':
         case 'u':
            value = Number(arguments[current_argument++]);
            if(isNaN(value))
               rtn += "NAN";
            else if(value === Infinity)
               rtn += "+INF";
            else if(value === -Infinity)
               rtn += "-INF";
            else
            {
               if(value < 0 || add_sign)
               {
                  if(value < 0)
                  {
                     rtn += "-";
                     value = -value;
                  }
                  else
                     rtn += "+";
                  --width;
               }
               scratch = value.toFixed(0);
               if(scratch.length < width)
               {
                  if(left_justify)
                  {
                     rtn += scratch;
                     for(j = scratch.length; j < width; ++j)
                        rtn += ' ';
                  }
                  else
                  {
                     for(j = scratch.length; j < width; ++j)
                     {
                        if(pad_zero)
                           rtn += '0';
                        else
                           rtn += ' ';
                     }
                     rtn += scratch;
                  }
               }
               else
                  rtn += scratch;
            }
            break;

         case 'o':              // octal conversion
            value = Number(arguments[current_argument++]);
            if(isNaN(value))
               rtn += "NAN";
            else if(value === Infinity)
               rtn += "+INF";
            else if(value === -Infinity)
               rtn += "-INF";
            else
            {
               if(alternate)
               {
                  rtn += '0';
                  --width;
               }
               scratch = value.toString(8);
               if(scratch.length < width)
               {
                  if(left_justify)
                  {
                     rtn += scratch;
                     for(j = scratch.length; j < width; ++j)
                        rtn += ' ';
                  }
                  else
                  {
                     for(j = scratch.length; j < width; ++j)
                     {
                        if(pad_zero)
                           rtn += '0';
                        else
                           rtn += ' ';
                     }
                     rtn += scratch;
                  }
               }
               else
                  rtn += scratch;
            }
            break;
            
         case 'x':
         case 'X':              // hex conversion
            value = Number(arguments[current_argument++]);
            if(isNaN(value))
               rtn += "NAN";
            else if(value === Infinity)
               rtn += "+INF";
            else if(value === -Infinity)
               rtn += "-INF";
            else
            {
               if(alternate)
               {
                  if(ch === 'x')
                     rtn += "0x";
                  else
                     rtn += "0X";
                  width -= 2;
               }
               scratch = value.toString(16);
               if(ch === 'X')
                  scratch = scratch.toUpperCase();
               if(scratch.length < width)
               {
                  if(left_justify)
                  {
                     rtn += scratch;
                     for(j = scratch.length; j < width; ++j)
                        rtn += ' ';
                  }
                  else
                  {
                     for(j = scratch.length; j < width; ++j)
                     {
                        if(pad_zero)
                           rtn += '0';
                        else
                           rtn += ' ';
                     }
                     rtn += scratch;
                  }
               }
               else
                  rtn += scratch;
            }
            break;

         case 'c':              // convert character encoding
            value = Number(arguments[current_argument++]);
            scratch = String.fromCharCode(value);
            if(scratch.length < width)
            {
               if(left_justify)
               {
                  rtn += scratch;
                  for(j = scratch.length; j < width; ++j)
                     rtn += ' ';
               }
               else
               {
                  for(j = scratch.length; j < width; ++j)
                     rtn += ' ';
                  rtn += scratch;
               }
            }
            else
               rtn += scratch;
            break;

         case 's':              // convert string
            value = String(arguments[current_argument++]);
            scratch = value;
            if(scratch.length < width)
            {
               if(left_justify)
               {
                  rtn += scratch;
                  for(j = scratch.length; j < width; ++j)
                     rtn += ' ';
               }
               else
               {
                  for(j = scratch.length; j < width; ++j)
                     rtn += ' ';
                  rtn += scratch;
               }
            }
            else
               rtn += scratch;
            break;

         case 'e':
         case 'E':
         case 'f':
         case 'g':
         case 'G':
         case 'n':
            if(ch === 'n')
               value = current_argument;
            else
               value = Number(arguments[current_argument++]);
            if(add_sign || value < 0)
            {
               if(value < 0)
               {
                  value = -value;
                  rtn += '-';
               }
               else
                  rtn += '+';
               --width;
            }
            scratch = sprintf.convert_value(value, ch, precision, thousands_sep, decimal_point);
            if(scratch.length < width)
            {
               if(left_justify)
               {
                  rtn += scratch;
                  for(j = scratch.length; j < width; ++j)
                     rtn += ' ';
               }
               else
               {
                  for(j = scratch.length; j < width; ++j)
                  {
                     if(pad_zero)
                        rtn += '0';
                     else
                        rtn += ' ';
                  }
                  rtn += scratch;
               }
            }
            else
               rtn += scratch;
            break;
            
         default:
            // @todo: throw an exception for an invalid format converter
            break;
         }
         state = state_normal;
      }
      if(do_next_char)
         ++i;
   }
   return rtn;
} // sprintf


sprintf.convert_value = function (value, format, precision, thousands_sep, decimal_point, trim_trailing_zeroes)
{
   var rtn = "";
   var temp = value;
   var separator_pos;
   var array;

   if(trim_trailing_zeroes === undefined)
      trim_trailing_zeroes = true;
   if(isNaN(value))
      rtn += "NAN";
   else if(value === Infinity)
      rtn += "+INF";
   else if(value === -Infinity)
      rtn = "-INF";
   else
   {
      switch(format)
      {
      case 'f':
         rtn = value.toFixed(precision);
         break;
         
      case 'e':
      case 'E':
         rtn = value.toExponential(precision);
         if(format === 'E')
            rtn = rtn.toUpperCase();
         break;
         
      case 'g':
      case 'G':
         if((!trim_trailing_zeroes && Math.abs(value) < 1E-4) || Math.abs(value) > 1E5)
            rtn = value.toExponential(precision - 1);
         else
            rtn = value.toPrecision(precision);
         if(format === 'G')
            rtn = rtn.toUpperCase();
         if(trim_trailing_zeroes)
            rtn = sprintf.trim_trailing_zeroes(rtn);
         break;
      }
      
      // we need to replace the decimal point with the locale decimal point character
      if(decimal_point.localeCompare('.') !== 0)
         rtn = rtn.replace(".", decimal_point);
      
      // we also need to insert thousands separators 
      if(value >= 1000)
      {
         temp = value;
         array = Csi.string_to_array(rtn);
         separator_pos = rtn.indexOf(decimal_point);
         if(separator_pos < 0)
            separator_pos = rtn.length;
         separator_pos -= 3;
         while(temp >= 1000 && separator_pos > 0)
         {
            array.splice(separator_pos, 0, thousands_sep);
            separator_pos -= 3;
            temp /= 1000;
         }
         rtn = array.join("");
      }
   }
   return rtn;
};


sprintf.trim_trailing_zeroes = function (s)
{
   // we need to trim any zeroes trailing the decimal point but
   // preceding the exponent. 
   var array = Csi.string_to_array(s);
   var exp_pos = s.search(/[eE][+\-].\d*$/);
   var dec_point_pos = s.search(/\.\d*0*/);
   var i;

   if(exp_pos < 0)
      exp_pos = array.length;
   if(dec_point_pos >= 0)
   {
      for(i = exp_pos - 1; i >= dec_point_pos; --i)
      {
         if(array[i] === '0' || array[i] === '.')
            array[i] = '';
         else
            break;
      }
   }

   // trim off the exponent if it has a value of zero. 
   if(exp_pos < s.length)
   {
      var delete_exp = true;
      for(i = exp_pos; i < array.length; ++i)
      {
         if(array[i] !== 'e' && array[i] !== 'E' && array[i] !== '+' &&
            array[i] !== '-' && array[i] !== '0')
         {
            delete_exp = false;
            break;
         }
      }
      if(delete_exp)
      {
         for(i = exp_pos; i < array.length; ++i)
            array[i] = '';
      }
   }
   return array.join("");
};
