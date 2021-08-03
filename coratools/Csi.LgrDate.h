/* Csi.LgrDate.h

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 21 March 1997
   Last Change: Wednesday 31 March 2021
   Last Commit: $Date: 2021-03-31 13:05:30 -0600 (Wed, 31 Mar 2021) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_LgrDate_h
#define Csi_LgrDate_h

#include <iosfwd>
#include "CsiTypeDefs.h"
#include "StrAsc.h"
#include "StrUni.h"


namespace Csi
{
   /**
    * Defines an object that is capable of storing any time stamp or time interval produced by
    * Campbell Scientific dataloggers. The internal format for this timestamp is the offset in
    * nano-seconds since midnight, 1 January 1990. This class overloads standard arithmetic
    * operators.
    *
    * This class has been adapted from the original DateTime class that was written by Wayne
    * Campbell and uses concepts from the following articles:
    *
    *  Dr. Dobb's Journal #80, June 1983 
    *     True Julian dates as used by astronomers take noon, 1 January 4713 BC as their base. We
    *     will use the same base but base from midnight rather than noon.
    *
    *  Collected Algorithms from CACM - Algorithm 199
    *     1 March 1900 = Julian Day 2415080 (noon based)
    *     1 January 0000 = Julian Day 1721119
    *     1 January 1970 = Julian Day 2440588
    *     1 January 1980 = Julian day 2444240
    *     1 January 1970 fell on a Thursday
    *     The difference between 1 January 1990 and 1 January 1970 is 631,152,000
    *     seconds or 7,305 days
    */
   class LgrDate 
   {
   public:
      /**
       * Constructor
       * @param nanoSec_ Optionally specifies an interval that represents the interval between the
       * date and 1 January 1990
       * @param other Specifies a date object to copy.
       */
      LgrDate():
         nanoSec(0)
      { }
      LgrDate(int8 nanoSec_):
         nanoSec(nanoSec_)
      { }
      LgrDate(LgrDate const &other):
         nanoSec(other.nanoSec)
      { }

      /**
       * Destructor
       */
      virtual ~LgrDate()
      { }

      /**
       * Copy operator
       */
      LgrDate &operator =(LgrDate const &other)
      {
         nanoSec = other.nanoSec;
         return *this;
      }

      /**
       * @return Overloads the comparison operator to compare the internal intervals.  Returns true
       * of false depending upon the values and the equality operator.
       */
      bool operator >(LgrDate const &other) const
      { return nanoSec > other.nanoSec ? true : false; }
      bool operator >=(LgrDate const &other) const
      { return nanoSec >= other.nanoSec ? true : false; }
      bool operator <(LgrDate const &other) const
      { return nanoSec < other.nanoSec ? true : false; }
      bool operator <=(LgrDate const &other) const
      { return nanoSec <= other.nanoSec ? true : false; }
      bool operator ==(LgrDate const &other) const
      { return nanoSec == other.nanoSec ? true : false; }
      bool operator !=(LgrDate const &other) const
      { return nanoSec != other.nanoSec ? true : false; }

      /**
       * Overloads the increment by operator to add the interval given by the second object.
       */
      LgrDate &operator +=(LgrDate const &other)
      { 
         nanoSec += other.nanoSec;
         return *this;
      }

      /**
       * Overloads the decrement by operator.
       */
      LgrDate &operator -=(LgrDate const &other)
      {
         nanoSec -= other.nanoSec;
         return *this;
      }

      /**
       * Overloads the multiply by operator.
       */
      LgrDate &operator *=(LgrDate const &other)
      {
         nanoSec *= other.nanoSec;
         return *this;
      }
   
      /**
       * Overloads the divide by operator.
       */
      LgrDate &operator /=(LgrDate const &other)
      {
         nanoSec /= other.nanoSec;
         return *this;
      } 

      /**
       * Sets the parameters of the date portion of this time stamp.
       *
       * @param year Specifies the year.
       * @param month Optionally specifies the month with January being a value of 1.
       * @param day Optionally specifies the day of the month (first day is one).
       */
      void setDate(Int4 year,uint4 month = 1,uint4 day = 1);

      /**
       * Sets the time portion of the time stamp without touching the date portion.
       *
       * @param hour Specifies the hour of day as 0 <= hour <= 24
       * @param minute Specifies the minute into the hour as 0 <= minute < 60
       * @param second Specifies the seconds into the minute as 0 <= second < 60
       * @param nsec Specifies the nanoseconds into the second as 0 <= nsec < 1E9
       */
      void setTime(uint4 hour,uint4 minute = 0,uint4 second = 0,uint4 nsec = 0);

      /**
       * Sets the nanoseconds into the second without touching any of the other time stamp
       * parameters.
       *
       * @param nsec Specifies the nanoseconds into the second as 0 <= nsec < 1E9
       */
      void setNSec(uint4 nsec);

      /**
       * Extracts date parameters to the supplied reference parameters.
       *
       * @param year Reference to the variable that will receive the year.
       * @param month Reference to the variable that will receive the month
       * @param day Reference to the variable that will receive the day.
       */
      void toDate(Int4 &year,uint4 &month,uint4 &day) const;

      /**
       * Extracts the time parameters to the supplied reference parameters.
       *
       * @param hour Reference to the variable that will receive the hour.
       * @param minute Reference to the variable that will receive the minutes into the hour
       * @param sec Reference to the variable that will receive the seconds into the minute
       * @param nsec Reference to the variable that will receive the nanoseconds into second.
       */
      void toTime(uint4 &hour,uint4 &minute,uint4 &sec,uint4 &nsec) const;

      /**
       * @return Returns the year for this time stamp.
       */
      int4 year() const;

      /**
       * Returns the month into the year for this time stamp such that 1 <= month() <= 12
       */
      uint4 month() const;

      /**
       * @return Returns the day of month such that 1 <= day() <= 31
       */
      uint4 day() const;

      /**
       * @return Returns the hour into the day such that 0 <= hour() <= 24
       */
      uint4 hour() const;

      /**
       * @return Returns the minutes into the hour such that 0 <= minute() < 60
       */
      uint4 minute() const;

      /**
       * @return Returns the seconds into the minute such that 0 <= second() < 60.
       */
      uint4 second() const;

      /**
       * @return Returns the nanoseconds into the second such that 0 <= nsec() < 1E9.
       */
      uint4 nsec() const;

      /**
       * @return Returns the day of the week encoded as an integer such that 0 < dayOfWeek() <= 7
       */
      int dayOfWeek() const;

      /**
       * @return Returns the day of the year (julian day) as an integer such that 0 < dayOfYear() <=
       * 366
       */
      int dayOfYear() const;

      /**
       * @return The Julian date (JD) is a continuous count of days from 1 January 4713 BC (= -4712
       * January 1), Greenwich mean noon (= 12h UT). For example, AD 1978 January 1, 0h UT is JD
       * 2443509.5 and AD 1978 July 21, 15h UT, is JD 2443711.125. See:
       * http://aa.usno.navy.mil/data/docs/JulianDate.php
       */
      double julianDate() const;

      /**
       * Fills in the provided structure with information derived from this time stamp.
       *
       * @param dest Specifies the target structure.
       */
      void make_tm(struct tm *dest) const;

      /**
       * @return Returns the epoch time (seconds since January 1 1970 UTC).
       *
       * @param is_utc Set to true (default of false) if this date is already UTC.
       */
      time_t to_time_t(bool is_utc = false) const;
   
      /**
       * Specifies the number of nanoseconds in a microsecond.
       */
      static const int8 nsecPerUSec;

      /**
       * Specifies the number of nanoseconds in a millisecond.
       */
      static const int8 nsecPerMSec;

      /**
       * Specifies the number of nanoseconds in a second.
       */
      static const int8 nsecPerSec;

      /**
       * Specifies the number of nanoseconds in a minute.
       */
      static const int8 nsecPerMin;
   
      /**
       * Specifies the number of nanoseconds in an hour.
       */
      static const int8 nsecPerHour;
   
      /**
       * Specifies the number of nanoseconds in a day.
       */
      static const int8 nsecPerDay;

      /**
       * Specifies the number of nanoseconds in a week.
       */
      static const int8 nsecPerWeek;
   
      /**
       * Specifies the number of milliseconds in a second.
       */
      static const uint4 msecPerSec;
      
      /**
       * Specifies the number of milliseconds in a minute.
       */
      static const uint4 msecPerMin;

      /**
       * Specifies the number of milliseconds in an hour.
       */
      static const uint4 msecPerHour;

      /**
       * Specifies the number of milliseconds in a day.
       */
      static const uint4 msecPerDay;

      /**
       * Specifies the number of milliseconds in a week.
       */
      static const uint4 msecPerWeek;

      /**
       * Specifies the julian date for midnight 1 January 0000.
       */
      static const uint4 julDay0;

      /**
       * Specifies the julian date for midnight 1 January 1970 (base date for unix time stamps).
       */
      static const uint4 julDay1970; 

      /**
       * Specifies the julian date for midnight 1 January 1990 (base date for CSI time stamps).
       */
      static const uint4 julDay1990; 

      /**
       * @return Returns the nanosecond interval represented by this object.
       */
      int8 get_nanoSec() const
      { return nanoSec; }

      /**
       * @return Returns the interval represented by this object in units of milliseconds.
       */
      int8 get_millSec() const
      { return nanoSec / nsecPerMSec; }

      /**
       * @return Returns the interval represented by this object in units of seconds.
       */
      int4 get_sec() const
      { return (int4)(nanoSec / nsecPerSec); }

      /**
       * @return Returns true if the year represented by this object falls on a leap year.
       */
      bool is_leap_year() const;

      /**
       * @return Converts the date and time represented by this object into a VARIANT DATE format
       * (days since 30 December 1899) as used by microsloth libraries.
       */
      double to_variant() const;

      /**
       * Formats the time according to the specifications given in the format string. This
       * specification using a subset of codes supported by the strftime function. The following
       * codes are recognised in the format string:
       *
       *   %a  -  abbreviated weekday name according to locale
       *   %A  -  full weekday name according to locale
       *   %b  -  abbreviated month name according to locale
       *   %B  -  Full month name according to locale
       *   %c  -  Local date and time representation (Short Version)
       *   %#c -  Local date and time representation (Long Version)
       *   %d  -  day of month, two spaces, rights justified, padded with zero
       *   %H  -  hours into the day, two spaces right justified, padded with zero
       *   %#H -  same as %H except that, if both the hour and minute are zero, the hour will be
       *          reported as 24 on the previous day.
       *   %I  -  Hour with 12 hour clock, two spaces right justified, padded with zero
       *   %j  -  Day of year, three spaces right justified, padded with zero
       *   %J  -  Julian Date
       *   %m  -  numeric month, two spaces right justified, padded with zero
       *   %M  -  minutes into the hour, two spaces, rights justified, padded with zero
       *   %p  -  local equivalent of "AM" or "PM" specifier
       *   %S  -  seconds into the minute, two spaces, right justified, padded with zero
       *   %U  -  week number of the year (Sunday being the first day of the week)
       *   %w  -  day of week as an integer, one space
       *   %W  -  week number of the year (Monday being the first day of the week)
       *   %y  -  years into century, two spaces, rights justified, padded with zero
       *   %Y  -  year as an integer
       *   %1  -  tenths of seconds, one space
       *   %2  -  hundredths of seconds, two spaces, rights justified, padded with zero
       *   %3  -  thousands of seconds, three spaces, right justified, padded with zero
       *   %4  -  1/10000 of second, four spaces, right justified, padded with zero
       *   %5  -  1/100000 of second, five spaces, right justified, padded with zero
       *   %6  -  micro-seconds, six spaces, right justified, padded with zero
       *   %7  -  1/10000000 of second, seven spaces, right justified, padded with zero
       *   %8  -  1/100000000 of seconds, eight spaces, right justified, padded with zero
       *   %9  -  nano-seconds, nine spaces, right justified, padded with zero
       *   %x  -  prints the sub-second resolution of the stamp with a preceding period with no padding
       *   %X  -  local time representation
       *   %n  -  local date representation (%x conflicts with previous usage)
       *   %Z  -  Time zone name
       *   %%  -  Prints the '%' character
       *
       * @param out Specifies the output stream or string.
       * @param fmt Specifies format string.
       */
      std::ostream &format(std::ostream &out, StrAsc const &fmt) const;
      std::wostream &format(std::wostream &out, StrUni const &fmt) const;
      char const *format(StrAsc &out, StrAsc const &fmt) const;
      wchar_t const *format(StrUni &out, StrUni const &fmt) const;
      
      /**
       * @return Formats the time stamp as if it is part of a classic datalogger comma
       * separated file time stamp.  The options parameter should be composed
       * by bitwise ORing the custom_classic_flags values.  Each value output
       * will be followed by a comma.  If no values are output, no characters
       * will be output either. 
       *
       * @param out Specifies the output stream.
       * @param Specifies the options controlling the format.
       */
      enum custom_classic_flags
      {
         custom_use_seconds  = 0x01,
         custom_use_hour_min = 0x02,
         custom_use_jul_day  = 0x04,
         custom_use_year     = 0x08,
         custom_midnight_is_2400 = 0x10
      };
      std::ostream &format_custom_classic(std::ostream &out, uint4 options) const;

      /**
       * @return Returns the offset of the time stamp represented by this object (assuming local
       * time) snd the UTC time for the same instance.
       */
      int8 gmt_offset() const;

      /**
       * @return Returns time stamp from a string that can take on any of the following formats:
       *    1)  mm-dd-yy[yy] [hh[:mn[:ss[.sf]]]] for LOCALE_IDATE == 0
       *    2)  dd-mm-yy[yy] [hh[:mn[:ss[.sf]]]] for LOCALE_IDATE == 1
       *    3)  [yy]yymmdd [hh[:mn[:ss[.sf]]]]         
       *    4)  dd mon[,] yy[yy] [hh[:mn[:ss[.sf]]]]
       *    5)  mon dd, yy[yy] [hh[:mn[:ss[.sf]]]]
       *    6)  hh[:mn[:ss[.sf]]]
       *    7)  yyyy-mm-dd [hh[:mn[:ss[.sf]]]]         
       *
       * When the century is not specified, the century will be rounded based upon the decade.
       */
      static LgrDate fromStr(char const *s);

      /**
       * @return Returns a time stamp from a string that represents date and time that can be found
       * in an HTTP header.  The supported formats include:
       *
       * Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
       * Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
       * Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
       */
      static LgrDate from_http(char const *s);

      /**
       * Formats this time stamp to the specidfied string or stream  in a format that can be used in
       * an HTTP header.
       */
      void format_http(std::ostream &out) const;
      void format_http(std::wostream &out) const;
      void format_http(StrAsc &out) const;

      /**
       * @return Returns the current local time adjusted for time zone and daylight savings.
       */
      static LgrDate local();

      /**
       * @return Returns the current local time adjusted for time zone but not for daylight savings
       * time.
       */
      static LgrDate local_na();

      /**
       * @return Returns the current UTC system time.
       */
      static LgrDate gmt();

      /**
       * @return Returns the time stamp represented by a classic logger's file storage buffer.
       *
       * @param buff Specifies the start of the buffer.
       * @param len Specifies the length if the buffer in bytes.
       */
      static LgrDate logger(char const *buff, int len);

      /**
       * @return Returns the time stamp represented by the given parameters derived from classic
       * datalogger final storage (P77) output.
       *
       * @param year Specifies the year given by the logger.
       * @param dayOfYear Specifies the julian day given by the datalogger.
       * @param hourMin Specifies the encoded hour/minute time given by the datalogger.
       * @param secs Specifies the seconds given by the datalogger.
       */
      static LgrDate fStorage(Int4 year, int dayOfYear, int hourMin, double secs);

      /**
       * @return Returns a time stamp initialised by the system clock and the static systemTimeCode
       * option.  Based upon that code, the value of local(), local_no(), or gmt() will be
       * returned.  If the system_time_correction value is set, this will be applied to the returned
       * value.
       */
      enum SystemTimeCodeOpt
      {
         system_local = 2,
         system_local_na = 1,
         system_gmt = 3,
      };
      static LgrDate system();

      /**
       * @param val Sets the code that will determine the type of clock returned by system().
       */
      static void set_systemTimeCode(SystemTimeCodeOpt val)
      { systemTimeCode = val; }

      /**
       * @return Returns the code that determines the type of clock returned by system().
       */
      static SystemTimeCodeOpt get_systemTimeCode()
      { return systemTimeCode; }

      /**
       * @return Returns the correction interval that will be applied to time stamps returned by the
       * system() static method.
       */
      static int8 get_system_time_correction()
      { return system_time_correction; }

      /**
       * @param value Specifies the interval, in nanoseconds that should be applied to all time
       * stamps returned by the static system() method.
       */
      static void set_system_time_correction(int8 value)
      { system_time_correction = value; }

      /**
       * @return Returns the time stamp represented by the given VARIANT interval.
       */
      static LgrDate from_variant(double variant_time);

      /**
       * @return Returns the time stamp represented by the given interval interpreted to be a UTC
       * time offset since 1 January 1970 (the Unix epoch).
       */
      static LgrDate from_time_t(uint4 secs_since_1970);

   private:
      /**
       * Specifies the interval, in units of nano seconds, represented by this object.
       */
      int8 nanoSec;

      /**
       * Specifies the selector for he systm() time function.
       */
      static SystemTimeCodeOpt systemTimeCode;

      /**
       * Specifies the correction, in nanoseconds, that will be applied to all time values returned
       * by system().
       */
      static int8 system_time_correction;
   }; 
};


/**
 * Overloads the stream insertion operators to format the time stamp.
 */
inline std::ostream &operator <<(std::ostream &out, Csi::LgrDate const &d)
{ return d.format(out,"%Y%m%d %H:%M:%S%x"); }
inline std::wostream &operator <<(std::wostream &out, Csi::LgrDate const &d)
{ return d.format(out,L"%Y%m%d %H:%M:%S%x"); }


//@group Binary Date Operators
Csi::LgrDate operator +(Csi::LgrDate const &t1, Csi::LgrDate const &t2);
Csi::LgrDate operator -(Csi::LgrDate const &t1, Csi::LgrDate const &t2);
Csi::LgrDate operator *(Csi::LgrDate const &t1, Csi::LgrDate const &t2);
Csi::LgrDate operator /(Csi::LgrDate const &t1, Csi::LgrDate const &t2); 
//@endgroup


#endif
