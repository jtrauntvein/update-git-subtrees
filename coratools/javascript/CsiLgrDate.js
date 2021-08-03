/* CsiLgrDate.js

   Copyright (C) 2010, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 July 2010
   Last Change: Tuesday 07 November 2017
   Last Commit: $Date: 2019-01-25 12:54:02 -0600 (Fri, 25 Jan 2019) $
   Last Changed by: $Author: jdevey $

*/

////////////////////////////////////////////////////////////
// class CsiLgrDate
//
// Defines an object that is capable of storing a datalogger time stamp with a
// resolution of milli-seconds.  Internally, this  class will maintain the
// date/time as milli-seconds elapsed since midnight 1 January 1990.
//
// This class has been adapted from the original DateTime class that was written by Wayne
// Campbell and uses concepts from the following articles:
//
//  Dr. Dobb's Journal #80, June 1983 
//     True Julian dates as used by astronomers take noon, 1 January 4713 BC as their base. We
//     will use the same base but base from midnight rather than noon.
//
//  Collected Algorithms from CACM - Algorithm 199
//     1 March 1900 = Julian Day 2415080 (noon based)
//     1 January 0000 = Julian Day 1721119
//     1 January 1970 = Julian Day 2440588
//     1 January 1980 = Julian day 2444240
//     1 January 1970 fell on a Thursday
//     The difference between 1 January 1990 and 1 January 1970 is 631,152,000
//     seconds or 7,305 days
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////
function CsiLgrDate()
{
   if(arguments.length > 0)
   {
      var arg1 = arguments[0];
      var arg1_type = typeof arg1;
      this.milliSecs = 0;
      if(arg1_type === "string")
      {
         this.milliSecs = CsiLgrDate.fromStr(arg1).milliSecs;
      }
      else if(arg1 instanceof Date)
      {
         this.milliSecs = arg1.getTime();
         this.milliSecs -= arg1.getTimezoneOffset() * CsiLgrDate.msecPerMin;
         this.milliSecs -= CsiLgrDate.unix_to_csi_diff * CsiLgrDate.msecPerDay;
      }
      else if(arg1 instanceof CsiLgrDate)
      {
         this.milliSecs = arg1.milliSecs;
      }
      else if(arg1_type === "number" || arg1 instanceof Number)
      {
         if(arguments.length === 1)
         {
            this.milliSecs = arg1;
         }
         else
         {
            var year = arg1;
            var month = 0;
            var day = 1;
            var hours = 0;
            var minutes = 0;
            var seconds = 0;
            var millis = 0;
            if(arguments.length > 1)
            {
               month = arguments[1];
            }
            if(arguments.length > 2)
            {
               month = arguments[2];
            }
            if(arguments.length > 3)
            {
               day = arguments[3];
            }
            if(arguments.length > 4)
            {
               hours = arguments[4];
            }
            if(arguments.length > 5)
            {
               minutes = arguments[5];
            }
            if(arguments.length > 6)
            {
               seconds = arguments[6];
            }
            if(arguments.length > 7)
            {
               millis = arguments[7];
            }
            this.setDate(year, month, day);
            this.setTime(hours, minutes, seconds, millis);
         }
      }
      else
      {
         this.milliSecs = 0;
      }
   }
   else
   {
      this.milliSecs = 0;
   }
}

CsiLgrDate.nsecPerUSec = 1000;
CsiLgrDate.nsecPerMSec = CsiLgrDate.nsecPerUSec * 1000;
CsiLgrDate.nsecPerSec = CsiLgrDate.nsecPerMSec * 1000;
CsiLgrDate.msecPerSec = 1000;
CsiLgrDate.msecPerMin = CsiLgrDate.msecPerSec * 60;
CsiLgrDate.msecPerHour = CsiLgrDate.msecPerMin * 60;
CsiLgrDate.msecPerDay = CsiLgrDate.msecPerHour * 24;
CsiLgrDate.msecPerWeek = CsiLgrDate.msecPerDay * 7;
CsiLgrDate.julDay0 = 1721119;
CsiLgrDate.julDay1970 = 2440588;
CsiLgrDate.unix_to_csi_diff = 7305;
CsiLgrDate.julDay1990 = CsiLgrDate.julDay1970 + CsiLgrDate.unix_to_csi_diff;


CsiLgrDate.leap_year = function (year)
{ return ((year % 4 === 0 && year % 100 !== 0) || year % 400 === 0); };

////////////////////////////////////////////////////////////
// setDate
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.setDate = function ()
{
   // interpret the arguments.  We will perform input bounds checking while
   // doing this
   var year = 1990;
   var month = 1;
   var day = 1;
   var month_lens = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
   if(arguments.length >= 1)
   {
      year = Number(arguments[0]);
   }
   if(CsiLgrDate.leap_year(year))
   {
      month_lens[1] += 1;
   }
   if(arguments.length >= 2)
   {
      month = Number(arguments[1]);
      if(month < 1)
      {
         month = 1;
      }
      if(month > 12)
      {
         month = 12;
      }
   }
   if(arguments.length >= 3)
   {
      day = Number(arguments[2]);
      if(day < 1)
      {
         day = 1;
      }
      if(day > month_lens[month - 1])
      {
         day = month_lens[month - 1];
      }
   }

   // preserve the time information and separate the century and year
   var time = this.toTime();
   var century;
   var year_of_century;
   var y, m;
   if(month >= 3)
   {
      m = month - 3;
      y = year;
   }
   else
   {
      m = month + 9;
      y = year - 1;
   }
   century = Math.floor(y / 100);
   year_of_century = y % 100;

   // we can now calculate the number of days since 1 January 0000
   var days_cent = Math.floor((146097 * century) / 4);
   var days_ano = Math.floor((1461 * year_of_century) / 4);
   var days_mes = Math.floor((153 * m + 2) / 5);
   var days = days_cent + days_ano + days_mes + day;
   days -= CsiLgrDate.julDay1990 - CsiLgrDate.julDay0;
   this.milliSecs = days * CsiLgrDate.msecPerDay +
         time.hour * CsiLgrDate.msecPerHour +
         time.minute * CsiLgrDate.msecPerMin +
         time.second * CsiLgrDate.msecPerSec +
         time.msec;
};

////////////////////////////////////////////////////////////
// truediv
////////////////////////////////////////////////////////////
CsiLgrDate.truediv = function (numerator, denominator)
{
   var quotient = numerator / denominator;
   var remainder = numerator % denominator;
   if(remainder < 0)
   {
      --quotient;
      remainder += denominator;
   }
   return {
      "quotient": Math.floor(quotient),
      "remainder": Math.floor(remainder)
   };
};

////////////////////////////////////////////////////////////
// setTime
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.setTime = function ()
{
   // process the parameters
   var hour = 0;
   var minute = 0;
   var second = 0;
   var msec = 0;

   if(arguments.length >= 1)
   {
      hour = arguments[0];
      if(hour > 24)
      {
         hour = 24;
      }
   }
   if(arguments.length >= 2)
   {
      minute = arguments[1];
      if(minute > 59)
      {
         minute = 59;
      }
   }
   if(arguments.length >= 3)
   {
      second = arguments[2];
      if(second > 59)
      {
         second = 59;
      }
   }
   if(arguments.length >= 4)
   {
      msec = arguments[3];
      if(msec > CsiLgrDate.msecPerSec)
      {
         msec = CsiLgrDate.msecPerSec;
      }
   }

   // we now need to strip off the current time of the timestamp.  We can then add the new values
   var qr = CsiLgrDate.truediv(this.milliSecs, CsiLgrDate.msecPerDay);
   this.milliSecs -= qr.remainder;
   this.milliSecs += hour * CsiLgrDate.msecPerHour +
         minute * CsiLgrDate.msecPerMin +
         second * CsiLgrDate.msecPerSec +
         msec;
};

////////////////////////////////////////////////////////////
// setMSec
//
// Sets the milli-seconds within the seconds leaving the rest of the date
// and time alone.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.setMSec = function (msec)
{
   var qr = CsiLgrDate.truediv(this.milliSecs, CsiLgrDate.msecPerSec);
   this.milliSecs = this.milliSecs - qr.quotient + msec;
};

////////////////////////////////////////////////////////////
// toDate
//
// Breaks the date down into the year, month, and day components.  These
// values will be returned as an object.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.toDate = function ()
{
   // we will express the stamp in terms of days since 1990
   var qr = CsiLgrDate.truediv(this.milliSecs, CsiLgrDate.msecPerDay);

   // we can now calulate the number of days since 1 January 0000 and strip off the century
   var clk = CsiLgrDate.julDay1990 - CsiLgrDate.julDay0 + qr.quotient;
   var year = Math.floor((4 * clk - 1) / 146097);
   var month;
   var day;

   clk = 4 * clk - 1 - year * 146097;

   // we can now strip off the year into the century, month, and day
   var d = Math.floor(clk / 4);

   clk = Math.floor((4 * d + 3) / 1461);
   d = 4 * d + 3 - clk * 1461;
   d = Math.floor((d + 4) / 4);
   month = Math.floor((5 * d - 3) / 153);
   d = 5 * d - 3 - month * 153;
   day = Math.floor((d + 5) / 5);
   year = 100 * year + clk;
   if(month < 10)
   {
      month += 3;
   }
   else
   {
      month -= 9;
      ++year;
   }
   return {
      "year": year,
      "month": month,
      "day": day
   };
};

////////////////////////////////////////////////////////////
// toTime
//
// Breaks down the time portion into hours, minutes, seconds, and milli-seconds
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.toTime = function ()
{
   var qr = CsiLgrDate.truediv(this.milliSecs, CsiLgrDate.msecPerSec);
   var rtn = {
      "hour": 0,
      "minute": 0,
      "second": 0,
      "msec": qr.remainder
   };

   qr = CsiLgrDate.truediv(qr.quotient, 60);
   rtn.second = qr.remainder;
   qr = CsiLgrDate.truediv(qr.quotient, 60);
   rtn.minute = qr.remainder;
   qr = CsiLgrDate.truediv(qr.quotient, 24);
   rtn.hour = qr.remainder;
   return rtn;
};


CsiLgrDate.prototype.julianDate = function ()
{ 
   var rtn = 0.0;

   var y = this.year();
   var mon = this.month();
   var d = this.day();
   var h = this.hour();
   var min = this.minute();
   var s = this.second() + this.msec() / CsiLgrDate.msecPerSec;

   //Formula to convert gregorian date to julian date
   rtn = d - 32075 + 1461 * (y + 4800 + (mon - 14) / 12) / 4 + 367 *
      (mon - 2 - (mon - 14) / 12 * 12) / 12 - 3 * ((y + 4900 + (mon - 14) / 12) / 100) / 4;
   rtn = Math.floor(rtn);


   var time = (h + 12) / 24 + min / 1440 + s / 86400;
   rtn += time;

   return rtn;
};


////////////////////////////////////////////////////////////
// year
//
// Returns the year for this stamp.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.year = function ()
{ return this.toDate().year; };

////////////////////////////////////////////////////////////
// month
//
// Returns the month of the year for this stamp.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.month = function ()
{ return this.toDate().month; };

////////////////////////////////////////////////////////////
// day
//
// Returns the day of the month for this stamp
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.day = function ()
{ return this.toDate().day; };

////////////////////////////////////////////////////////////
// hour
//
// Returns the hour of the day for this stamp.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.hour = function ()
{ return this.toTime().hour; };

////////////////////////////////////////////////////////////
// minute
//
// Returns the minutes into the hour for this stamp.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.minute = function ()
{ return this.toTime().minute; };

////////////////////////////////////////////////////////////
// second
//
// Returns the seconds into the minute for this stamp.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.second = function ()
{ return this.toTime().second; };

////////////////////////////////////////////////////////////
// msec
//
// Returns the milliseconds into the second for this stamp.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.msec = function ()
{ return this.toTime().msec; };

////////////////////////////////////////////////////////////
// dayOfWeek
//
// Returns day of the week for this stamp such that 0 < dayOfWeek <= 7
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.dayOfWeek = function ()
{
   var days = Math.floor(
      this.milliSecs / CsiLgrDate.msecPerDay + CsiLgrDate.julDay1990 - CsiLgrDate.julDay0);
   return ((days + 2) % 7) + 1;
};

////////////////////////////////////////////////////////////
// dayOfYear
//
// Returns the day of the year for this stamp such that 0 < dayOfYear() <= 366.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.dayOfYear = function ()
{
   var year_start = new CsiLgrDate();
   year_start.setDate(this.year());
   return Math.floor(
      ((this.milliSecs - year_start.milliSecs) / CsiLgrDate.msecPerDay) + 1);
};

////////////////////////////////////////////////////////////
// make_date
//
// Generates a Javascript Date object from this timestamp.  
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.make_date = function ()
{
   var date = this.toDate();
   var time = this.toTime();
   return new Date(
      date.year, date.month - 1, date.day,
      time.hour, time.minute, time.second, time.msec);
};

////////////////////////////////////////////////////////////
// getTime
//
// Converts our epoch into the epoch used for the JavaScript Date class.  
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.getTime = function ()
{ return this.make_date().getTime(); };

////////////////////////////////////////////////////////////
// fromStr
//
// Attempts to parse a time stamp from a string. 
////////////////////////////////////////////////////////////
CsiLgrDate.fromStr = function (s)
{
   var rtn;
   var tokens = CsiLgrDate.make_tokens(s);
   var format = CsiLgrDate.determine_format(tokens);
   switch(format)
   {
      case 1:
         rtn = CsiLgrDate.read_format1(tokens);
         break;

      case 2:
         rtn = CsiLgrDate.read_format2(tokens);
         break;

      case 3:
         rtn = CsiLgrDate.read_format3(tokens);
         break;

      case 4:
         rtn = CsiLgrDate.read_format4(tokens);
         break;

      case 5:
         rtn = CsiLgrDate.read_format5(tokens);
         break;

      case 6:
         rtn = CsiLgrDate.read_format6(tokens);
         break;

      case 7:
         rtn = CsiLgrDate.read_format7(tokens);
         break;

      default:
         rtn = new CsiLgrDate();
         rtn.milliSecs = NaN;
         break;
   }
   return rtn;
};

////////////////////////////////////////////////////////////
// gmt
//
// Constructs a new CsiLgrDate using the current system time in GMT.
////////////////////////////////////////////////////////////
CsiLgrDate.gmt = function ()
{
   var now = new Date();
   return new CsiLgrDate(now.getTime() - CsiLgrDate.unix_to_csi_diff * CsiLgrDate.msecPerDay);
};

////////////////////////////////////////////////////////////
// local
//
// Constructs a new CsiLgrDate using the current system time and local time zone
////////////////////////////////////////////////////////////
CsiLgrDate.local = function ()
{ return new CsiLgrDate(new Date()); };

////////////////////////////////////////////////////////////
// format
//
// Formats the date and time according to the strfime() type format
// string.  The following set of codes are recognised:
//
//   %a  -  abbreviated weekday name according to locale
//   %A  -  full weekday name according to locale
//   %b  -  abbreviated month name according to locale
//   %B  -  Full month name according to locale
//   %c  -  Local date and time representation (Short Version)
//   %#c -  Local date and time representation (Long Version)
//   %d  -  day of month, two spaces, rights justified, padded with zero
//   %H  -  hours into the day, two spaces right justified, padded with zero
//   %#H -  hours into the day similar to %H.  If the time is between 0:00:00
//          and 0:00:59, the hour will be formatted as 24 and the previous day
//          will be used. 
//   %I  -  Hour with 12 hour clock, two spaces right justified, padded with zero
//   %j  -  Day of year, three spaces right justified, padded with zero
//   %m  -  numeric month, two spaces right justified, padded with zero
//   %M  -  minutes into the hour, two spaces, rights justified, padded with zero
//   %p  -  local equivalent of "AM" or "PM" specifier
//   %S  -  seconds into the minute, two spaces, right justified, padded with zero
//   %U  -  week number of the year (Sunday being the first day of the week)
//   %w  -  day of week as an integer, one space
//   %W  -  week number of the year (Monday being the first day of the week)
//   %y  -  years into century, two spaces, rights justified, padded with zero
//   %Y  -  year as an integer
//   %1  -  tenths of seconds, one space
//   %2  -  hundredths of seconds, two spaces, rights justified, padded with zero
//   %3  -  thousands of seconds, three spaces, right justified, padded with zero
//   %4  -  1/10000 of second, four spaces, right justified, padded with zero
//   %5  -  1/100000 of second, five spaces, right justified, padded with zero
//   %6  -  micro-seconds, six spaces, right justified, padded with zero
//   %7  -  1/10000000 of second, seven spaces, right justified, padded with zero
//   %8  -  1/100000000 of seconds, eight spaces, right justified, padded with zero
//   %9  -  nano-seconds, nine spaces, right justified, padded with zero
//   %x  -  prints the sub-second resolution of the stamp with a preceding period with no padding
//   %X  -  local time representation
//   %n  -  local date representation (%x conflicts with previous usage)
//   %Z  -  Time zone name
//   %%  -  Prints the '%' character
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.format = function (spec)
{
   // break the date into its component parts. 
   var abbrev_days = [
      "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat"];
   var full_days = [
      "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
   var abbrev_months = [
      "Jan", "Feb", "Mar", "Apr", "May", "June", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
   var full_months = [
      "January", "February", "March", "April", "May", "June",
      "July", "August", "September", "October", "November", "December"];
   var rtn = "";
   var last_char = 0;
   var len = spec.length;
   var time_zone_pos;
   var temp;
   var date = this.toDate();
   var time = this.toTime();
   var jsdate = this.make_date();
   var year_start;
   var week1_start_day;
   var this_day;
   var weeks;
   var gmt_offset = null;
   
   // if the hour represents midnight, we may need to adjust this date and print hours as 24.
   if(time.hour === 0 && time.minute === 0)
   {
      // search the format string for an instance of "%#H", if present, we will replace it
      // with 24 and reformat with the previous days date.
      var flagged_day_pos = spec.search(/%#H/);
      if(flagged_day_pos > 0)
      {
         var new_date = new CsiLgrDate(this.milliSecs - CsiLgrDate.msecPerDay);
         return new_date.format(spec.replace(/%#H/, "24"));
      }
   }

   var i;
   for(i = 0; i < len; ++i)
   {
      if(last_char !== '%' && spec.charAt(i) !== '%')
      {
         rtn += spec.charAt(i);
      }
      else if(last_char === '%')
      {
         var flagged = (spec.charAt(i) === '#');
         if(flagged && i + 1 < spec.length)
         {
            ++i;
         }
         switch(spec.charAt(i))
         {
         case '%':
            rtn += '%';
            if(i + 1 < len)
            {
               ++i;
               rtn += spec.charAt(i);
            }
            break;
            
         case 'a':
            rtn += abbrev_days[this.dayOfWeek() - 1];
            break;
            
         case 'A':
            rtn += full_days[this.dayOfWeek() - 1];
            break;
            
         case 'b':
            rtn += abbrev_months[date.month - 1];
            break;
            
         case 'B':
            rtn += full_months[date.month - 1];
            break;
            
         case 'c':
            if(flagged)
            {
               temp = jsdate.toLocaleString(
                  [],
                  {
                     weekday:"long",
                     year:"numeric",
                     month:"long",
                     day:"numeric",
                     hour:"numeric",
                     minute:"numeric",
                     second:"numeric"
                  });
               time_zone_pos = temp.lastIndexOf(" GMT");
               if(time_zone_pos >= 0)
                  rtn += temp.slice(0, time_zone_pos);
               else
                  rtn += temp;
            }
            else
            {
               temp = jsdate.toLocaleString(
                  [],
                  {
                     year:"numeric",
                     month:"numeric",
                     day:"numeric",
                     hour:"numeric",
                     minute:"numeric",
                     second:"numeric"
                     });
               time_zone_pos = temp.lastIndexOf(" GMT");
               if(time_zone_pos >= 0)
                  rtn += temp.slice(0, time_zone_pos);
               else
                  rtn += temp;
            }
            break;
            
         case 'Z':
            temp = jsdate.toTimeString();
            time_zone_pos = temp.lastIndexOf(" GMT");
            if(time_zone_pos >= 0)
               rtn += temp.slice(time_zone_pos, temp.length);
            break;
            
         case 'z':
            gmt_offset = this.gmt_offset() / CsiLgrDate.msecPerMin;
            rtn += (gmt_offset / 60);
            rtn += CsiLgrDate.pad_zero(gmt_offset % 60, 2);
            break;
            
         case 'd':
            rtn += CsiLgrDate.pad_zero(date.day, 2);
            break;
            
         case 'H':
            rtn += CsiLgrDate.pad_zero(time.hour, 2);
            break;
            
         case 'I':
            temp = time.hour % 12;
            if(temp === 0)
            {
               temp = 12;
            }
            rtn += CsiLgrDate.pad_zero(temp, 2);
            break;
            
         case 'j':
            rtn += CsiLgrDate.pad_zero(this.dayOfYear(), 3);
            break;

         case 'J':
            rtn += sprintf("%.10g", this.julianDate());
            break;

         case 'm':
            rtn += CsiLgrDate.pad_zero(date.month, 2);
            break;
            
         case 'M':
            rtn += CsiLgrDate.pad_zero(time.minute, 2);
            break;
            
         case 'n':
            rtn += jsdate.toLocaleDateString();
            break;
            
         case 'p':
            if(time.hour >= 12)
            {
               rtn += "PM";
            }
            else
            {
               rtn += "AM";
            }
            break;
            
         case 'S':
            rtn += CsiLgrDate.pad_zero(time.second, 2);
            break;
            
         case 'U':
            year_start = new CsiLgrDate(date.year, 1, 1);
            week1_start_day = year_start.dayOfWeek() - 1;
            this_day = this.dayOfYear();
            weeks = Math.ceil((this_day + week1_start_day) / 7) - 1;
            rtn += weeks.toString();
            break;
            
         case 'W':
            year_start = new CsiLgrDate(date.year, 1, 1);
            week1_start_day = year_start.dayOfWeek() - 1;
            week1_start_day = (week1_start_day + 6) % 7; // Only difference between Sunday start (%U) and Monday start (%W)
            this_day = this.dayOfYear();
            weeks = Math.ceil((this_day + week1_start_day) / 7) - 1;
            rtn += weeks.toString();
            break;
            
         case 'w':
            rtn += this.dayOfWeek() - 1;
            break;
            
         case 'X':
            rtn += jsdate.toLocaleTimeString();
            break;
            
         case 'y':
            rtn += CsiLgrDate.pad_zero(date.year % 100, 2);
            break;
            
         case 'Y':
            rtn += CsiLgrDate.pad_zero(date.year, 4);
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
            var digits = spec[i] - '0';
            var exponent = 9 - digits;
            var divisor = 1;
            var j;
            for(j = 0; j < exponent; ++j)
            {
               divisor *= 10;
            }
            rtn += CsiLgrDate.pad_zero(
               Math.floor((time.msec * CsiLgrDate.nsecPerMSec) / divisor),
               digits);
            break;
            
         case 'x':
            if(time.msec > 0)
            {
               temp = time.msec.toString();
               rtn += ".";
               if(temp.length === 1)
               {
                  rtn += "00";
               }
               else if(temp.length === 2)
               {
                  rtn += "0";
               }
               rtn += temp;
            }
            break;
         }
      }
      last_char = spec.charAt(i);
   }
   return rtn;
};


////////////////////////////////////////////////////////////
// gmt_offset
//
// Returns the offset between this time and GMT in units of milliseconds.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.gmt_offset = function()
{
   var date = this.make_date();
   return -date.getTimezoneOffset() * CsiLgrDate.msecPerMin;
};


////////////////////////////////////////////////////////////
// toString
//
// Overloads the regular toString to format the date
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.toString = function ()
{ return this.format("%Y-%m-%d %H:%M:%S%x"); };

////////////////////////////////////////////////////////////
// valueOf
//
// Overrides the regular valueOf to return the internal representation.
////////////////////////////////////////////////////////////
CsiLgrDate.prototype.valueOf = function ()
{ return this.milliSecs; };

////////////////////////////////////////////////////////////
// make_tokens
//
// Implements the algorithm  that will perform the lexical scanning for parsing
// date/time strings.
////////////////////////////////////////////////////////////
CsiLgrDate.make_tokens = function (buff)
{
   var token = "";
   var rtn = [];
   var last_ch = 0;

   var i;
   for(i = 0; i < buff.length; ++i)
   {
      // skip multiple spaces
      if(buff[i] === ' ' && last_ch === ' ')
      {
         continue;
      }
      last_ch = buff.charAt(i);

      // break the token if a delimiter is found
      switch(buff.charAt(i))
      {
         case '/':
         case '-':
         case ' ':
         case ';':
         case ':':
         case '.':
         case 'T':
            rtn.push(token);
            token = "";
            break;

         case ',':
            break;

         default:
            token += buff.charAt(i);
            break;
      }
   }

   // add the final token if there is any remainder
   if(token.length > 0)
   {
      rtn.push(token);
   }
   return rtn;
};

////////////////////////////////////////////////////////////
// is_numeric
//
// Evaluates whether the string specified consists entirely of numeric
// characters.
////////////////////////////////////////////////////////////
CsiLgrDate.is_numeric = function (s)
{
   var rtn = true;
   var i;
   for(i = 0; rtn && i < s.length; ++i)
   {
      if(s[i] < '0' || s[i] > '9')
      {
         rtn = false;
      }
   }
   return rtn;
};

////////////////////////////////////////////////////////////
// which_month
//
// Resolves the english name of a month or its abbreviation to a month code such that
// 0 <= which_month() < 12,
////////////////////////////////////////////////////////////
CsiLgrDate.which_month = function (s_)
{
   var rtn = -1;
   var s = s_.toLowerCase();
   var abbrev_month_names = [
      "jan",
      "feb",
      "mar",
      "apr",
      "may",
      "jun",
      "jul",
      "aug",
      "sep",
      "oct",
      "nov",
      "dec"];
   var full_month_names = [
      "january",
      "february",
      "march",
      "april",
      "may",
      "june",
      "july",
      "august",
      "september",
      "october",
      "november",
      "december"];

   var i;
   for(i = 0; rtn < 0 && i < full_month_names.length; ++i)
   {
      if(s.localeCompare(abbrev_month_names[i]) === 0 ||
         s.localeCompare(full_month_names[i]) === 0)
      {
         rtn = i;
      }
   }
   return rtn;
};

////////////////////////////////////////////////////////////
// determine_format
//
// Based upon the tokens that were generated from make_tokens(), this method
// will determine the specific format, if any, that the date/time string is
// using.  
////////////////////////////////////////////////////////////
CsiLgrDate.determine_format = function (tokens)
{
   var rtn = 0;
   if(tokens.length >= 2 &&
      CsiLgrDate.is_numeric(tokens[0]) &&
      tokens[0].length <= 2 &&
      CsiLgrDate.is_numeric(tokens[1]) &&
      tokens[1].length <= 2)
   {
      rtn = 1;
   }
   else if(tokens.length >= 1 &&
           CsiLgrDate.is_numeric(tokens[0]) &&
           (tokens[0].length === 6 || tokens[0].length === 8))
   {
      rtn = 3;
   }
   else if(tokens.length >= 3 &&
           CsiLgrDate.is_numeric(tokens[0]) &&
           tokens[0].length === 4 &&
           CsiLgrDate.is_numeric(tokens[1]) &&
           CsiLgrDate.is_numeric(tokens[2]))
   {
      rtn = 7;
   }
   else if(tokens.length >= 2 &&
           CsiLgrDate.which_month(tokens[0]) >= 0)
   {
      rtn = 4;
   }
   else if(tokens.length >= 2 && CsiLgrDate.which_month(tokens[1]) >= 0)
   {
      rtn = 5;
   }
   else if(tokens.length >= 1 && CsiLgrDate.is_numeric(tokens[0]))
   {
      rtn = 6;
   }
   return rtn;
};

////////////////////////////////////////////////////////////
// read_time
//
// Parses the hours, minutes, seconds, and nsec from the time string. 
// //////////////////////////////////////////////////////////
CsiLgrDate.read_time = function (tokens, start)
{
   var rtn = {};
   rtn.hour = 0;
   rtn.minute = 0;
   rtn.second = 0;
   rtn.nsec = 0;
   var i, j;
   for(i = 0; i < tokens.length; ++i)
   {
      switch(i - start)
      {
      case 0:
         rtn.hour = tokens[i];
         break;
         
      case 1:
         rtn.minute = tokens[i];
         break;
         
      case 2:
         rtn.second = tokens[i];
         break;
         
      case 3:
         rtn.nsec = tokens[i];
         for(j = 9 - tokens[i].length; j > 0; --j)
         {
            rtn.nsec *= 10;
         }
         break;

      case 4:
         if(tokens[i] === "PM" || tokens[i] === "pm")
            rtn.hour += 12;
         break;
      }
   }
   return rtn;
};

////////////////////////////////////////////////////////////
// convert_components
//
// Converts the specified components into a LgrDate
////////////////////////////////////////////////////////////
CsiLgrDate.convert_components = function (year, month, day, hour, minute, second, nsec)
{
   if(typeof year === 'string')
      year = parseInt(year);

   if(typeof month === 'string')
      month = parseInt(month);

   if(typeof day === 'string')
      day = parseInt(day);

   if(typeof hour === 'string')
      hour = parseInt(hour);

   if(typeof minute === 'string')
      minute = parseInt(minute);

   if(typeof second === 'string')
      second = parseInt(second);

   if(typeof nsec === 'string')
      nsec = parseInt(nsec);

   var rtn = new CsiLgrDate();
   if(year < 100 && year >= 50)
   {
      year += 1900;
   }
   else if(year < 100 && year < 50)
   {
      year += 2000;
   }
   rtn.setDate(year, month, day);
   rtn.setTime(hour, minute, second, Math.floor(nsec / CsiLgrDate.nsecPerMSec));
   return rtn;
};

////////////////////////////////////////////////////////////
// read_format1
////////////////////////////////////////////////////////////
CsiLgrDate.read_format1 = function (tokens)
{
   var month = tokens[0];
   var day = tokens[1];
   var year = tokens[2];
   var time = CsiLgrDate.read_time(tokens, 3);
   return CsiLgrDate.convert_components(year, month, day, time.hour, time.minute, time.second, time.nsec);
};

////////////////////////////////////////////////////////////
// read_format2
////////////////////////////////////////////////////////////
CsiLgrDate.read_format2 = function (tokens)
{
   var day = tokens[0];
   var month = tokens[1];
   var year = tokens[2];
   var time = CsiLgrDate.read_time(tokens, 3);
   return CsiLgrDate.convert_components(year, month, day, time.hour, time.minute, time.second, time.nsec);
};

////////////////////////////////////////////////////////////
// read_format3
////////////////////////////////////////////////////////////
CsiLgrDate.read_format3 = function (tokens)
{
   var code = tokens[0];
   var day = Math.floor(code % 100);
   code /= 100;
   var month = Math.floor(code % 100);
   code /= 100;
   var year = Math.floor(code);
   var time = CsiLgrDate.read_time(tokens, 1);
   return CsiLgrDate.convert_components(year, month, day, time.hour, time.minute, time.second, time.nsec);
};

////////////////////////////////////////////////////////////
// read_format4
////////////////////////////////////////////////////////////
CsiLgrDate.read_format4 = function (tokens)
{
   var month = CsiLgrDate.which_month(tokens[0]) + 1;
   var day = tokens[1];
   var year = tokens[2];
   var time = CsiLgrDate.read_time(tokens, 3);
   return CsiLgrDate.convert_components(year, month, day, time.hour, time.minute, time.second, time.nsec);
};

////////////////////////////////////////////////////////////
// read_format5
////////////////////////////////////////////////////////////
CsiLgrDate.read_format5 = function (tokens)
{
   var day = tokens[0];
   var month = CsiLgrDate.which_month(tokens[1]) + 1;
   var year = tokens[2];
   var time = CsiLgrDate.read_time(tokens, 3);
   return CsiLgrDate.convert_components(year, month, day, time.hour, time.minute, time.second, time.nsec);
};

////////////////////////////////////////////////////////////
// read_format6
////////////////////////////////////////////////////////////
CsiLgrDate.read_format6 = function (tokens)
{
   var time = CsiLgrDate.read_time(tokens, 0);
   return CsiLgrDate.convert_components(0, 0, 0, time.hour, time.minute, time.second, time.nsec);
};

////////////////////////////////////////////////////////////
// read_format7
////////////////////////////////////////////////////////////
CsiLgrDate.read_format7 = function (tokens)
{
   var year = tokens[0];
   var month = tokens[1];
   var day = tokens[2];
   var time = CsiLgrDate.read_time(tokens, 3);
   return CsiLgrDate.convert_components(year, month, day, time.hour, time.minute, time.second, time.nsec);
};

////////////////////////////////////////////////////////////
// pad_zero
//
// Right justifies a number with preceding zeroes. 
////////////////////////////////////////////////////////////
CsiLgrDate.pad_zero = function (val, places)
{
   var temp = val.toString();
   var rtn = "";
   var i;
   for(i = 0; temp.length + i < places; ++i)
   {
      rtn += "0";
   }
   rtn += temp;
   return rtn;
};


////////////////////////////////////////////////////////////
// csi_max_timestamp
//
// Evaluates two timestamps and returns the value that is greatest. 
////////////////////////////////////////////////////////////
CsiLgrDate.max = function (time1, time2)
{
   var rtn = new CsiLgrDate(time1);
   if(time2.milliSecs > time1.milliSecs)
   {
      rtn = new CsiLgrDate(time2);
   }
   return rtn;
};

