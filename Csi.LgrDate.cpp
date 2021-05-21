/* Csi.LgrDate.cpp

   Copyright (C) 1990, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 24 March 1997
   Last Change: Wednesday 31 March 2021
   Last Commit: $Date: 2021-03-31 13:05:30 -0600 (Wed, 31 Mar 2021) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#ifdef _WIN32
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>            // needed to access system time and locale information
#else
#include <sys/time.h>           // access to gettimeofday and tzset vars
#endif
#include <locale>
#include <iomanip>
#include <time.h>
#include <math.h>
#include "Csi.LgrDate.h"
#include "Csi.StrAscStream.h"
#include "Csi.StrUniStream.h"
#include "ListOf.h"
#include "truediv.h"
#include "CsiTypes.h"


Csi::LgrDate operator +(Csi::LgrDate const &t1, Csi::LgrDate const &t2)
{
   Csi::LgrDate rtn(t1);
   rtn += t2;
   return rtn;
} // operator

Csi::LgrDate operator -(Csi::LgrDate const &t1, Csi::LgrDate const &t2)
{
   Csi::LgrDate rtn(t1);
   rtn -= t2;
   return rtn;
} // operator

Csi::LgrDate operator *(Csi::LgrDate const &t1, Csi::LgrDate const &t2)
{
   Csi::LgrDate rtn(t1);
   rtn *= t2;
   return rtn;
} // operator

Csi::LgrDate operator /(Csi::LgrDate const &t1, Csi::LgrDate const &t2)
{
   Csi::LgrDate rtn(t1);
   rtn /= t2;
   return rtn;
} // operator

namespace Csi
{
   namespace
   {
      void monDayFromDayOfYear(int &mon,int &day,int dayOfYear,int year);
      void makeTokens(ListOf<StrAsc> &tokens,StrAsc &delimiters,char const *buff);
      void makeWebTokens(ListOf<StrAsc> &tokens,StrAsc &delimiters,char const *buff);
      int determineFormat(ListOf<StrAsc> &tokens);
      bool isNumeric(StrAsc const &token);
      int whichMonth(StrAsc const &token);
      LgrDate readFmt1(ListOf<StrAsc> &tokens);
      LgrDate readFmt2(ListOf<StrAsc> &tokens);
      LgrDate readFmt3(ListOf<StrAsc> &tokens);
      LgrDate readFmt4(ListOf<StrAsc> &tokens);
      LgrDate readFmt5(ListOf<StrAsc> &tokens);
      LgrDate readFmt6(ListOf<StrAsc> &tokens);
      LgrDate readFmt7(ListOf<StrAsc> &tokens);
      void readTime(
         ListOf<StrAsc> &tokens,
         int start,
         int &hour,
         int &min,
         int &sec,
         int &nsec);
      LgrDate convertComponents(
         int year,
         int month,
         int day,
         int hour,
         int minute,
         int sec,
         int nsec);
   };


   const int8 LgrDate::nsecPerUSec = 1000;
   const int8 LgrDate::nsecPerMSec = LgrDate::nsecPerUSec*1000;
   const int8 LgrDate::nsecPerSec = LgrDate::nsecPerMSec*1000;
   const int8 LgrDate::nsecPerMin = LgrDate::nsecPerSec*60;
   const int8 LgrDate::nsecPerHour = LgrDate::nsecPerMin*60;
   const int8 LgrDate::nsecPerDay = LgrDate::nsecPerHour*24;
   const int8 LgrDate::nsecPerWeek = LgrDate::nsecPerDay*7;
   const uint4 LgrDate::msecPerSec = 1000;
   const uint4 LgrDate::msecPerMin = msecPerSec*60;
   const uint4 LgrDate::msecPerHour = msecPerMin*60;
   const uint4 LgrDate::msecPerDay = msecPerHour*24;
   const uint4 LgrDate::msecPerWeek = msecPerDay*7;
   const uint4 LgrDate::julDay0 = 1721119;
   const uint4 LgrDate::julDay1970 = 2440588;
   uint4 const unix_to_csi_diff = 7305;
   const uint4 LgrDate::julDay1990 = julDay1970 + unix_to_csi_diff;
   LgrDate::SystemTimeCodeOpt LgrDate::systemTimeCode = LgrDate::system_local_na;
   int8 LgrDate::system_time_correction(0);

   namespace
   {
      uint4 const days_since_30_dec_1899 = 32874;
   };
   
   void LgrDate::setDate(Int4 year,uint4 month,uint4 day)
   {
      // we need to do input bounds checking on the parameters.  If any are out of bounds, we will
      // bind them to the closest boundary.
      uint4 month_lens[12] = {
         31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
      };
      if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
         month_lens[1] += 1;
      if(year < 1)
         year = 1;
      if(month < 1)
         month = 1;
      if(day < 1)
         day = 1;
      if(month > 12)
         month = 12;
      if(day > month_lens[month - 1])
         day = month_lens[month - 1];

      // preserve the time information
      uint4 hora,minuto,seg,nseg;
      toTime(hora,minuto,seg,nseg);

      // separate the century and year into century
      Int4 century,                // the century
         yearOfCent,               // the year into the century
         y,m;

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
      century = y/100;
      yearOfCent = y%100;
   
      // calculate the number of days since 1 January 0000
      Int4 days = ((146097*century)/4) + ((1461*yearOfCent)/4) + ((153*m + 2)/5) + day;

      // now add the offset to 1990
      days -= julDay1990 - julDay0;
      nanoSec = days*nsecPerDay + hora*nsecPerHour + minuto*nsecPerMin + seg*nsecPerSec + nseg; 
   } // setDate

   void LgrDate::setTime(uint4 hour,uint4 minute,uint4 second,uint4 nsec)
   {
      // perform range checking on the parameters
      if(hour > 24)
         hour = 24;
      if(minute > 59)
         minute = 59;
      if(second > 59)
         second = 59;
      if(nsec > nsecPerSec)
         nsec = nsecPerSec;
      
      // strip off the time information from the present stamp
      int8 q,r;

      truediv(q,r,nanoSec,nsecPerDay);
      nanoSec -= r;
      nanoSec += hour*nsecPerHour + minute*nsecPerMin + second*nsecPerSec + nsec;
   } // setTime

   void LgrDate::setNSec(uint4 nsec)
   {
      // strip off the nano-second information from the stamp
      int8 q,r;

      truediv(q,r,nanoSec,nsecPerSec);
      nanoSec = nanoSec - q + nsec;
   } // setNSec

   void LgrDate::toDate(Int4 &year,uint4 &month,uint4 &day) const
   {
      // express the stamp in terms of days since 1990
      int8 daysSince1990,r;

      truediv(daysSince1990,r,nanoSec,nsecPerDay);

      // calculate the days since 1 January 0000
      int4 clk = int4(julDay1990 - julDay0 + daysSince1990);
      year = (4*clk - 1)/146097L;
      clk = 4*clk - 1 - year*146097L;

      // strip off the year, month, and day
      Int4 d = clk/4;
      clk = (4*d + 3)/1461;
      d = 4*d + 3 - clk*1461;
      d = (d + 4)/4;
      month = (5*d - 3)/153;
      d = 5*d - 3 - month*153;
      day = (d + 5)/5;
      year = 100*year + clk;
      if(month < 10)
         month += 3;
      else
      {
         month -= 9;
         year++;
      } 
   } // toDate

   void LgrDate::toTime(uint4 &hour, uint4 &minute, uint4 &sec, uint4 &nsec) const
   {
      int8 temp, r; 

      truediv(temp,r,nanoSec,nsecPerSec); nsec = uint4(r);
      truediv(temp,r,temp,int8(60)); sec = uint4(r);
      truediv(temp,r,temp,int8(60)); minute = uint4(r);
      truediv(temp,r,temp,int8(24)); hour = uint4(r);
   } // toTime

   Int4 LgrDate::year() const
   {
      Int4 rtn;
      uint4 month,day;
      toDate(rtn,month,day);
      return rtn;
   } // year

   uint4 LgrDate::month() const
   {
      Int4 year;
      uint4 rtn,day;
      toDate(year,rtn,day);
      return rtn;
   } // month

   uint4 LgrDate::day() const
   {
      Int4 year;
      uint4 month,rtn;
      toDate(year,month,rtn);
      return rtn;
   } // day

   uint4 LgrDate::hour() const
   {
      uint4 rtn,min,sec,nsec;
      toTime(rtn,min,sec,nsec);
      return rtn;
   } // hour

   uint4 LgrDate::minute() const
   {
      uint4 hour,rtn,sec,nsec;
      toTime(hour,rtn,sec,nsec);
      return rtn;
   } // minute

   uint4 LgrDate::second() const
   {
      uint4 hour,min,rtn,nsec;
      toTime(hour,min,rtn,nsec);
      return rtn;
   } // second

   uint4 LgrDate::nsec() const
   {
      uint4 hour,min,sec,rtn;
      toTime(hour,min,sec,rtn);
      return rtn;
   } // nsec

   int LgrDate::dayOfWeek() const
   {
      uint4 days = uint4(nanoSec/nsecPerDay + julDay1990 - julDay0);
      return (int)(((days + 2)%7) + 1);
   } // dayOfWeek

   int LgrDate::dayOfYear() const
   {
      LgrDate yearStart;
      int rtn;
   
      yearStart.setDate(year());
      rtn = (int)((nanoSec - yearStart.nanoSec)/nsecPerDay) + 1;
      return rtn;
   } // dayOfYear

   double LgrDate::julianDate() const
   { 
      double rtn = 0.0;
      double y(year());
      double mon(month());
      double d(day());
      double h(hour());
      double min = minute();
      double s(static_cast<double>(second() + nsec() / nsecPerSec));

      //Formula to convert gregorian date to julian date
      rtn = d - 32075 + 1461 * (y + 4800 + (mon - 14) / 12) / 4 + 367 *
         (mon - 2 - (mon - 14) / 12 * 12) / 12 - 3 * ((y + 4900 + (mon - 14) / 12) / 100) / 4;
      rtn = floor(rtn);


      double time = (h + 12) / 24 + min / 1440 + s / 86400;
      rtn += time;

      return rtn;
   } // julianDate

   void LgrDate::make_tm(struct tm *dest) const
   {
      // split this stamp into its component parts
      int4 year;
      uint4 month, day, hour, minute, second, nsec;
      toDate(year,month,day);
      toTime(hour,minute,second,nsec);

      // now fill in the dest structure
      dest->tm_sec = static_cast<int>(second);
      dest->tm_min = static_cast<int>(minute);
      dest->tm_hour = static_cast<int>(hour);
      dest->tm_mday = static_cast<int>(day);
      dest->tm_mon = static_cast<int>(month - 1);
      dest->tm_year = static_cast<int>(year - 1900);
      if(dest->tm_year < 0)
         dest->tm_year = 0;
      dest->tm_wday = dayOfWeek() - 1;
      dest->tm_yday = dayOfYear() - 1;
      dest->tm_isdst = -1;
   } // make_tm

   time_t LgrDate::to_time_t(bool is_utc) const
   {
      time_t rtn;
      if(!is_utc)
      {
         struct tm broken_down;
         make_tm(&broken_down);
         rtn = ::mktime(&broken_down);
      }
      else
      {
         int8 seconds(nanoSec / nsecPerSec);
         rtn = static_cast<time_t>(seconds + (unix_to_csi_diff * 86400));
      }
      return rtn;
   } // to_time_t

   bool LgrDate::is_leap_year() const
   {
      bool rtn(false);
      int4 ano(year());
      if((ano % 4 == 0 && ano % 100 != 0) || ano % 400 == 0)
         rtn = true;
      return rtn;
   } // is_leap_year
   
   double LgrDate::to_variant() const
   {
      // previous code used windows system calls to convert a SYSTEMTIME structure to the
      // variant.  Unfortunately, this method winds up throwing away any sub-second resolution.
      // As a result, we will perform the conversion arithmetic ourselves.
      int8 days = get_nanoSec()/nsecPerDay + days_since_30_dec_1899;
      int8 nsecs_in_day = get_nanoSec() % nsecPerDay;
      return static_cast<double>(days) +
         (static_cast<double>(nsecs_in_day) / static_cast<double>(nsecPerDay));
   } // to_variant

   std::ostream &LgrDate::format(std::ostream &out, StrAsc const &fmt) const
   {
      // we will separate the values for formatting
      char lastCh = '\0';
      int exp;
      int divisor;
      int j;
      int4 ano;
      uint4 mes,dia,hora,minuto,segundo,nsegundo;
      struct tm std_time;
   
      toDate(ano, mes, dia);
      toTime(hora, minuto, segundo, nsegundo);
      make_tm(&std_time);

      // if the format string contains a "%#H" sequence that is not quoted and the hour and minute
      // for this time stamp are both zero
      if(hora == 0 && minuto == 0)
      {
         size_t flagged_hour_pos(fmt.find("%#H"));
         if(flagged_hour_pos < fmt.length())
         {
            StrAsc new_fmt(fmt);
            LgrDate new_date(get_nanoSec() - nsecPerDay);
            
            new_fmt.cut(flagged_hour_pos, 3);
            new_fmt.insert("24", flagged_hour_pos);
            new_date.format(out, new_fmt);
            return out;
         }
      }
      
      //@bug fix 29 October 1999 by Jon Trauntvein
      // We need to make sure that the stream state is set so that numbers are printed as decimal.
      out << std::dec;
      //@endbugfix

      // process the format string
      typedef std::ostreambuf_iterator<char, std::char_traits<char> > iterator_type;
      typedef std::time_put<char, iterator_type> time_put_type;
      time_put_type const &tp = std::use_facet<time_put_type>(out.getloc());

      for(size_t i = 0; i < fmt.length(); i++)
      {
         if(lastCh != '%' && fmt[i] != '%')
            out << fmt[i];
         else if(lastCh == '%')
         {
            bool flagged = fmt[i] == '#';
            if(flagged && i+1 < fmt.length() && fmt[i + 1] != 0)
               ++i;
            switch(fmt[i])
            {
            case 'a':
            case 'A':
            case 'b':
            case 'B':
            case 'c':
            case 'd':
            case 'H':
            case 'I':
            case 'j':
            case 'm':
            case 'M':
            case 'n':
            case 'p':
            case 'S':
            case 'U':
            case 'w':
            case 'W':
            case 'X':
            case 'y':
            case 'Y':
            case 'Z':
            {
               char format_buff[4];
               iterator_type begin(out);

               format_buff[0] = '%';
               if(flagged && fmt[i] != 'H')
               {
                  format_buff[1] = '#';
                  format_buff[2] = fmt[i] == 'n' ? 'x' : fmt[i];
                  format_buff[3] = 0;
               }
               else
               {
                  format_buff[1] = fmt[i] == 'n' ? 'x' : fmt[i];
                  format_buff[2] = 0;
               }
               tp.put(
                  begin,
                  out,
                  ' ',
                  &std_time,
                  format_buff,
                  format_buff + strlen(format_buff));
               break;
            }

            case 'z':
            {
               int8 offset = gmt_offset() / nsecPerMin;
               out << (offset / 60) << std::setw(2) << std::setfill('0') << (offset % 60);
               break;
            }
         
            case '1':              // tenths of seconds
            case '2':              // hundredths of seconds
            case '3':              // thousandths of seconds
            case '4':              // 1/10000 of seconds
            case '5':              // 1/100000 of seconds
            case '6':              // micro-seconds
            case '7':              // 1/10000000 of seconds
            case '8':              // 1/100000000 of seconds
            case '9':              // nano-seconds
            {
               std::locale old_locale(out.getloc());
               out.imbue(std::locale::classic());
               exp = 9 - (fmt[i] - '0');
               divisor = 1;
               for(j = 0; j < exp; j++)
                  divisor *= 10;
               out << std::right << std::setw(fmt[i] - '0')
                   << std::setfill('0') << (nsegundo/divisor);
               out.imbue(old_locale);
               break;
            }
            
            case 'x':
               if(nsegundo > 0)
               {
                  char sub_seconds[11];
#pragma warning(disable: 4996)
                  sprintf(sub_seconds,".%09u",nsegundo);
#pragma warning(default: 4996)
                  for(int digit = 9; digit >= 0 && sub_seconds[digit] == '0'; --digit)
                     sub_seconds[digit] = '\0';
                  out << sub_seconds;
               }
               break;

            case 'J':   //Julian Date
            {
               csiFloatToStream(out, julianDate(), 15);
               break;
            }
            
            case '%':
               out << '%';
               if(i + 1 < fmt.length())
               {
                  i++;
                  if(fmt[i] != '%')
                     out << fmt[i];
               }
               break;
            } // what do we need to do? 
         } // found an escape sequence

         lastCh = fmt[i]; 
      } // format string scan loop 
      return out;
   } // format

   std::wostream &LgrDate::format(std::wostream &out, StrUni const &fmt) const
   {
      wchar_t lastCh = L'\0';
      int exp;
      int divisor;
      int j;
      int4 ano;
      uint4 mes,dia,hora,minuto,segundo,nsegundo;
      struct tm std_time;
   
      // separate the values
      toDate(ano,mes,dia);
      toTime(hora,minuto,segundo,nsegundo);
      make_tm(&std_time);

      // if the format string contains a "%#H" sequence that is not quoted and the hour and minute
      // for this time stamp are both zero
      if(hora == 0 && minuto == 0)
      {
         size_t flagged_hour_pos(fmt.find(L"%#H"));
         if(flagged_hour_pos < fmt.length())
         {
            StrUni new_fmt(fmt);
            LgrDate new_date(get_nanoSec() - nsecPerDay);
            
            new_fmt.cut(flagged_hour_pos, 3);
            new_fmt.insert(L"24", flagged_hour_pos);
            new_date.format(out, new_fmt);
            return out;
         }
      }
      
      //@bug fix 29 October 1999 by Jon Trauntvein
      // We need to make sure that the stream state is set so that numbers are printed as decimal.
      out << std::dec;
      //@endbugfix

      // process the format string
      typedef std::ostreambuf_iterator<wchar_t, std::char_traits<wchar_t> > iterator_type;
      typedef std::time_put<wchar_t, iterator_type> time_put_type;
      time_put_type const &tp = std::use_facet<time_put_type>(out.getloc());

      for(size_t i = 0; i < fmt.length(); ++i)
      {
         if(lastCh != L'%' && fmt[i] != L'%')
            out << fmt[i];
         else if(lastCh == '%')
         {
            bool flagged = fmt[i] == '#';
            if(flagged && i+1 < fmt.length() && fmt[i + 1] != 0)
               ++i;
            switch(fmt[i])
            {
            case L'a':
            case L'A':
            case L'b':
            case L'B':
            case L'c':
            case L'd':
            case L'H':
            case L'I':
            case L'j':
            case L'm':
            case L'M':
            case L'n':
            case L'p':
            case L'S':
            case L'U':
            case L'w':
            case L'W':
            case L'X':
            case L'y':
            case L'Y':
            case L'Z':
            {
               wchar_t format_buff[4];
               iterator_type begin(out);

               format_buff[0] = L'%';
               if(flagged && fmt[i] != L'H')
               {
                  format_buff[1] = L'#';
                  format_buff[2] = fmt[i] == L'n' ? L'x' : fmt[i];
                  format_buff[3] = 0;
               }
               else
               {
                  format_buff[1] = fmt[i] == L'n' ? L'x' : fmt[i];
                  format_buff[2] = 0;
               }
               tp.put(
                  begin,
                  out,
                  L' ',
                  &std_time,
                  format_buff,
                  format_buff + wcslen(format_buff));
               break;
            }

            case L'z':
            {
               int8 offset = gmt_offset() / nsecPerMin;
               out << (offset / 60) << std::setw(2) << std::setfill(L'0') << (offset % 60);
               break;
            }
            
            case L'1':              // tenths of seconds
            case L'2':              // hundredths of seconds
            case L'3':              // thousandths of seconds
            case L'4':              // 1/10000 of seconds
            case L'5':              // 1/100000 of seconds
            case L'6':              // micro-seconds
            case L'7':              // 1/10000000 of seconds
            case L'8':              // 1/100000000 of seconds
            case L'9':              // nano-seconds
               {
                  std::locale old_locale(out.getloc());
                  out.imbue(std::locale::classic());
                  exp = 9 - (fmt[i] - L'0');
                  divisor = 1;
                  for(j = 0; j < exp; j++)
                     divisor *= 10;
                  out << std::right << std::setw(fmt[i] - L'0')
                      << std::setfill(L'0') << (nsegundo/divisor);
                  out.imbue(old_locale);
                  break;
               }
            
            case L'x':
               if(nsegundo > 0)
               {
                  wchar_t sub_seconds[11];
                  swprintf(
                     sub_seconds,
                     sizeof(sub_seconds),
                     L".%09u",
                     nsegundo);
                  for(int digit = 9; digit >= 0 && sub_seconds[digit] == L'0'; --digit)
                     sub_seconds[digit] = L'\0';
                  out << sub_seconds;
               }
               break;

            case 'J':   //Julian Date
            {
               csiFloatToStream(out, julianDate(), 15);
               break;
            }

            case L'%':
               out << L'%';
               if(i + 1 < fmt.length())
               {
                  i++;
                  if(fmt[i] != L'%')
                     out << fmt[i];
               }
               break;
            } // what do we need to do? 
         } // found an escape sequence

         lastCh = fmt[i]; 
      } // format string scan loop 
      return out;
   } // format

   char const *LgrDate::format(StrAsc &out, StrAsc const &fmt) const
   {
      OStrAscStream temp;

      format(temp, fmt);
      out += temp.str();
      return out.c_str();
   } // format

   wchar_t const *LgrDate::format(StrUni &out, StrUni const &fmt) const
   {
      OStrUniStream temp;

      format(temp, fmt);
      out += temp.str();
      return out.c_str();
   } // format

   std::ostream &LgrDate::format_custom_classic(
      std::ostream &out,
      uint4 options) const
   {
      // break out the values and adjust them for the 2400 at midnight flag
      int4 year;
      uint4 mes, dia, hour, minute, second, nsec;
      int day = dayOfYear();
      
      toDate(year,mes,dia);
      toTime(hour,minute,second,nsec);
      if((options & custom_midnight_is_2400) != 0)
      {
         if(hour == 0 && minute == 0)
         {
            hour = 24;
            --day;
            if(day == 0)
            {
               LgrDate yesterday;
               yesterday.setDate(--year,12,31);
               day = yesterday.dayOfYear();
            }
         }
      }

      // we will now output the values
      int values_count = 0;
      std::ios::fmtflags old_flags = out.flags();
      if((options & custom_use_year) != 0)
      {
         out << year;
         ++values_count; 
      }
      if((options & custom_use_jul_day) != 0)
      {
         if(values_count++ > 0)
            out << ",";
         out << day;
      }
      if((options & custom_use_hour_min) != 0)
      {
         if(values_count++ > 0)
            out << ",";
         out << hour
             << std::setfill('0') << std::setw(2) << minute;
      }
      if((options & custom_use_seconds) != 0)
      {
         if(values_count++ > 0)
            out << ",";
         out << second;
         if(nsec != 0)
         {
            char temp[11];
#pragma warning(disable: 4996)
            sprintf(temp,".%09u",nsec);
#pragma warning(default: 4996)
            for(int digit = 8; digit >= 0 && temp[digit] == '0'; --digit)
               temp[digit] = '\0';
            out << temp; 
         }
      }
      out.flags(old_flags);
      return out;
   } // format_custom_classic

   void LgrDate::format_http(std::ostream &out) const
   {
      format(out, "%a, %d %b %Y %H:%M:%S GMT");
   }

   void LgrDate::format_http(std::wostream &out) const
   {
      format(out, L"%a, %d %b %Y %H:%M:%S GMT");
   }

   void LgrDate::format_http(StrAsc &out) const
   {
      format(out, "%a, %d %b %Y %H:%M:%S GMT");
   }

   LgrDate LgrDate::from_http(char const *s)
   {
      LgrDate rtn;
      ListOf<StrAsc> tokens;
      StrAsc delimiters;
      makeWebTokens(tokens, delimiters, s);

      int month = 0, day = 0, year = 0, hour = 0, minute = 0, sec = 0, nsec = 0;
      if(tokens.get_count() == 8 && delimiters == "    :: ")
      {         
         // Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
         day = atoi(tokens[1].c_str());
         month = whichMonth(tokens[2].c_str()) + 1;
         year = atoi(tokens[3].c_str());
         readTime(tokens,4,hour,minute,sec,nsec);
         rtn = convertComponents(year,month,day,hour,minute,sec,nsec);
      }
      else if(tokens.get_count() == 8 && delimiters == " -- :: ")
      {         
         // Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
         day = atoi(tokens[1].c_str());
         month = whichMonth(tokens[2].c_str()) + 1;
         year = atoi(tokens[3].c_str());
         readTime(tokens,4,hour,minute,sec,nsec);
         rtn = convertComponents(year,month,day,hour,minute,sec,nsec);
      }
      else if(tokens.get_count() == 7 && delimiters == "   :: ")
      {
         // Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
         month = whichMonth(tokens[1].c_str()) + 1;
         day = atoi(tokens[2].c_str());
         year = atoi(tokens[6].c_str());
         readTime(tokens,3,hour,minute,sec,nsec);
         rtn = convertComponents(year,month,day,hour,minute,sec,nsec);
      }
      else
      {
         //Unknown web format so see if it is anything we can parse
         fromStr(s);
      }

      return rtn;
   }

   int8 LgrDate::gmt_offset() const
   {
      struct tm broken_down;
      time_t local;
      time_t gmt;
      int8 rtn(0);
      
      make_tm(&broken_down);
      local = ::mktime(&broken_down);
#ifdef _WIN32
      gmt = _mkgmtime(&broken_down);
#else
      gmt = timegm(&broken_down);
#endif
      rtn = (int8)(gmt - local) * nsecPerSec;
      return rtn;
   } // gmt_offset

   LgrDate LgrDate::fromStr(char const *s)
   {
      ListOf<StrAsc> tokens;
      StrAsc delimiters;
      int format;
      LgrDate rtn;

      makeTokens(tokens,delimiters,s);
      format = determineFormat(tokens);
      switch(format)
      {
      case 1: rtn = readFmt1(tokens); break;
      case 2: rtn = readFmt2(tokens); break;
      case 3: rtn = readFmt3(tokens); break;
      case 4: rtn = readFmt4(tokens); break;
      case 5: rtn = readFmt5(tokens); break;
      case 6: rtn = readFmt6(tokens); break;
      case 7: rtn = readFmt7(tokens); break;
      default:
         throw MsgExcept("Unknown date format");
      }
      return rtn;
   } // fromStr

   LgrDate LgrDate::local()
   {
      LgrDate rtn;
#ifdef _WIN32
      SYSTEMTIME sys;
      GetLocalTime(&sys);
      rtn.setDate(
         sys.wYear,
         sys.wMonth,
         sys.wDay);
      rtn.setTime(
         sys.wHour,
         sys.wMinute,
         sys.wSecond,
         sys.wMilliseconds*uint4(nsecPerMSec));
#else
      struct tm broken_down;
      struct timeval time_of_day;
      gettimeofday(&time_of_day,0);
      localtime_r(
         &time_of_day.tv_sec,
         &broken_down);
      rtn.setDate(
         broken_down.tm_year + 1900,
         broken_down.tm_mon + 1,
         broken_down.tm_mday);
      rtn.setTime(
         broken_down.tm_hour,
         broken_down.tm_min,
         broken_down.tm_sec,
         time_of_day.tv_usec * nsecPerUSec);
#endif
      return rtn;
   } // local

   LgrDate LgrDate::local_na()
   {
      LgrDate rtn = local();
      int8 adjustment = 0;;
#ifdef _WIN32
      TIME_ZONE_INFORMATION zone_info;
      uint4 rcd;

      rcd = GetTimeZoneInformation(&zone_info);
      if(rcd == TIME_ZONE_ID_DAYLIGHT)
         adjustment = zone_info.DaylightBias*nsecPerMin;
#else
      // there is no portable means of obtaining the daylight savings time bias directly.  We can,
      // however, obtain the local time, and, if daylight savings time is enabled, we can use mktime
      // to find out what the bias would be.
      struct timeval time_of_day;
      struct tm local_tm;

      gettimeofday(&time_of_day,0);
      localtime_r(
         &time_of_day.tv_sec,
         &local_tm);
      if(local_tm.tm_isdst > 0)
      {
         local_tm.tm_isdst = 0;
         adjustment = (mktime(&local_tm) - time_of_day.tv_sec) * nsecPerSec * -1;
      }
#endif
      return rtn + adjustment;
   } // local_na
   
   LgrDate LgrDate::gmt()
   {
      LgrDate rtn;
#ifdef _WIN32
      SYSTEMTIME sys;
      GetSystemTime(&sys);
      rtn.setDate(
         sys.wYear,
         sys.wMonth,
         sys.wDay);
      rtn.setTime(
         sys.wHour,
         sys.wMinute,
         sys.wSecond,
         sys.wMilliseconds*uint4(nsecPerMSec));
#else
      struct timeval time_of_day;
      struct tm broken_down;
      gettimeofday(&time_of_day,0);
      gmtime_r(
         &time_of_day.tv_sec,
         &broken_down);
      rtn.setDate(
         broken_down.tm_year + 1900,
         broken_down.tm_mon + 1,
         broken_down.tm_mday);
      rtn.setTime(
         broken_down.tm_hour,
         broken_down.tm_min,
         broken_down.tm_sec,
         time_of_day.tv_usec * nsecPerUSec);
#endif
      return rtn;
   } // gmt

   LgrDate LgrDate::system()
   {
      LgrDate rtn;
      switch(systemTimeCode)
      {
      case system_local:
         rtn = local();
         break;
         
      case system_local_na:
         rtn = local_na();
         break;
         
      case system_gmt:
         rtn = gmt();
         break;
      }
      rtn.nanoSec += system_time_correction;
      return rtn;
   } // system

   LgrDate LgrDate::logger(char const *buff,int len)
   {
      int year,dayOfYear,month,day,hour,minute,second;
      char temp[25];
      char *t = temp;
      char lastId;

      for(int i = 0; i < len; i++)
      {
         switch(buff[i])
         {
         case 'Y':
            lastId = 'Y';
            break;

         case 'D':
            lastId = 'D';
            *t = '\0';
            year = atoi(temp);
            t = temp;
            break;

         case 'T':
            lastId = 'T';
            *t = '\0';
            dayOfYear = atoi(temp);
            t = temp;
            break;

         case ':':
            if(lastId == 'T')
            {
               *t = '\0';
               hour = atoi(temp);
               t = temp;
               lastId = 'M';
            }
            else if(lastId == 'M')
            {
               *t = '\0';
               minute = atoi(temp);
               t = temp;
               lastId = 'S';
            }
            break;
         
         case 'C':
            *t = '\0';
            second = atoi(temp);
            t = temp;
            break;

         default:
            *t++ = buff[i];
            break;
         }
      } // scan for tokens

      // assume the year to be in the closest century
      if(year >= 50)
         year += 1900;
      else
         year += 2000;

      // now take the year and day of year to get the month and day of month
      if(dayOfYear == 0)
         dayOfYear = 1;
      monDayFromDayOfYear(month,day,dayOfYear,year);

      // initialise the return value
      LgrDate rtn;
      rtn.setDate(year,month,day);
      rtn.setTime(hour,minute,second);
      return rtn;
   } // logger

   LgrDate LgrDate::fStorage(Int4 year,int dayOfYear,int hourMin,double secs)
   {
      // fill in any defaults
      LgrDate rtn,now(local());
      if(year == 0) year = now.year();
      if(dayOfYear == 0) dayOfYear = now.dayOfYear();

      // separate the hours from minutes, seconds from fractional
      Int4 hour = hourMin/100;
      Int4 minute = hourMin%100;
      Int4 second = (Int4)secs;
      Int4 nsec;
   
      secs -= second;
      nsec = (Int4)(secs*nsecPerSec);

      // initialise the return value
      int month,day;
   
      monDayFromDayOfYear(month,day,dayOfYear,year); 
      rtn.setDate(year,month,day);
      rtn.setTime(hour,minute,second,nsec);
      return rtn;
   } // fStorage

   LgrDate LgrDate::from_variant(double variant_time)
   {
      // we first need to convert the date portion
      double integer_part, fractional_part;
      int8 rtn;
      fractional_part = modf(variant_time,&integer_part);
      rtn = (static_cast<int8>(integer_part) - days_since_30_dec_1899) * nsecPerDay;
      
      // we now need to deal with the fractional part (less than one day).  Since the variant type
      // can't handle any resolution greater than milliseconds, we will deal with those units.
      // Because of round-off error, we will need to round this as well.
      double ms, subms;
      subms = modf(fractional_part * msecPerDay, &ms);
      if (subms >= 0.5)
         ms += 1;

      //return the date portion + nsecs into day
      rtn += static_cast<int8>(ms * nsecPerMSec);
      return rtn;
   } // from_variant

   LgrDate LgrDate::from_time_t(uint4 secs_since_1970)
   {
      int8 rtn = (secs_since_1970 * nsecPerSec) - (unix_to_csi_diff * nsecPerDay);
      return rtn;
   } // from_time_t

   namespace
   {
      void monDayFromDayOfYear(int &mon,int &day,int dayOfYear,int year)
      {
         // array marks the beginning of each month
         int monthStarts[] = { 1,32,60,91,121,152,182,213,244,274,305,335 };
         int i; // index variable
      
         // adjust the months for leap year
         if((year%4 == 0 && year%100 != 0) || year%400 == 0)
            for(i = 2; i < 12; i++)
               monthStarts[i]++;
      
         // now search for the slot that dayOfYear falls under
         for(i = 0; i < 12; i++)
            if(dayOfYear < monthStarts[i])
               break;
      
         // assign the month and day
         mon = i;
         day = dayOfYear - monthStarts[i - 1] + 1;
      } // monDayFromDayOfYear
   
      void makeTokens(ListOf<StrAsc> &tokens,StrAsc &delimiters,char const *buff)
      {
         StrAsc token;
         char lastCh = '\0';
      
         for(int i = 0; buff[i] != '\0'; i++)
         {
            // skip multiple spaces
            if(buff[i] == ' ' && lastCh == ' ')
               continue;
            lastCh = buff[i];
         
            // break the token if a delimiter was found
            if(buff[i] == '/' ||
               buff[i] == '-' ||
               buff[i] == ' ' ||
               buff[i] == ';' ||
               buff[i] == ':' ||
               buff[i] == '.' ||
               buff[i] == 'T')  // add the "T" to deal with XML style date/time stamps
            {
               tokens.add(token);
               delimiters += buff[i];
               token = "";
            }
            // otherwise add to the current token
            else if(buff[i] != ',')
               token += buff[i]; 
         } // scan loop
         // add the final token if there is remainder
         if(token.length() > 0)
            tokens.add(token);
      } // makeTokens
   
      void makeWebTokens(ListOf<StrAsc> &tokens,StrAsc &delimiters,char const *buff)
      {
         StrAsc token;
         char lastCh = '\0';
      
         for(int i = 0; buff[i] != '\0'; i++)
         {
            // skip multiple spaces
            if(buff[i] == ' ' && lastCh == ' ')
               continue;
            lastCh = buff[i];
         
            // break the token if a delimiter was found
            if(buff[i] == '/' ||
               buff[i] == '-' ||
               buff[i] == ' ' ||
               buff[i] == ';' ||
               buff[i] == ':' ||
               buff[i] == '.')
            {
               tokens.add(token);
               delimiters += buff[i];
               token = "";
            }
            // otherwise add to the current token
            else if(buff[i] != ',')
               token += buff[i]; 
         } // scan loop

         // add the final token if there is remainder
         if(token.length() > 0)
            tokens.add(token);
      } // makeWebTokens

      int determineFormat(ListOf<StrAsc> &tokens)
      {
         int rtn = 0;
         if(tokens.get_count() >= 2 && isNumeric(tokens[0]) && tokens[0].length() <= 2 && 
            isNumeric(tokens[1]) && tokens[1].length() <= 2)
         {
#ifdef _WIN32
            // get the locale information
            char lcData[20];
            memset(lcData,'\0',sizeof(lcData));
            GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_IDATE,lcData,sizeof(lcData));
            if(lcData[0] == '0')
               rtn = 1;
            else if(lcData[0] == '1')
               rtn = 2;
#else
            rtn = 1;
#endif
         } // standard format
         else if(tokens.get_count() >= 1 && isNumeric(tokens[0]) &&
                 (tokens[0].length() == 6 || tokens[0].length() == 8))
            rtn = 3;                  // encoded date
         else if(tokens.get_count() >= 3 && isNumeric(tokens[0]) &&
                 tokens[0].length() == 4 &&
                 isNumeric(tokens[1]) && isNumeric(tokens[2]))
            rtn = 7;                  // encoded date
         else if(tokens.get_count() >= 2 && whichMonth(tokens[0]) >= 0)
            rtn = 4;
         else if(tokens.get_count() >= 2 && whichMonth(tokens[1]) >= 0)
            rtn = 5;
         else if(tokens.get_count() >= 1 && isNumeric(tokens[0]))
            rtn = 6;
         return rtn;
      } // determineFormat
   
      bool isNumeric(StrAsc const &token)
      {
         bool rtn = true;
      
         for(uint4 i = 0; i < token.length(); i++)
            if(token.charAt(i) < '0' || token.charAt(i) > '9')
            {
               rtn = false;
               break;
            }
         return rtn;
      } // isNumeric
   
      char const *MonthNames[] =
      {
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
         "dec",
         0
      };
   
      int whichMonth(StrAsc const &token)
      {
         // scan the list of month abbreviations to for the token
         int rtn = -1;
         for(int i = 0; MonthNames[i] != 0; i++)
         {
            if(token == MonthNames[i])
            {
               rtn = i;
               break;
            }
         }
         return rtn;
      } // whichMonth
   
      LgrDate readFmt1(ListOf<StrAsc> &tokens)
      {
         int month = 0,
            day = 0,
            year = 0,
            hour = 0,
            minute = 0,
            sec = 0,
            nsec = 0;
      
         month = atoi(tokens[0].c_str());
         day = atoi(tokens[1].c_str());
         year = atoi(tokens[2].c_str()); 
         readTime(tokens,3,hour,minute,sec,nsec);
         return convertComponents(year,month,day,hour,minute,sec,nsec);
      } // readFmt1
   
      LgrDate readFmt2(ListOf<StrAsc> &tokens)
      {
         int month = 0,
            day = 0,
            year = 0,
            hour = 0,
            minute = 0,
            sec = 0,
            nsec = 0;
      
         day = atoi(tokens[0].c_str());
         month = atoi(tokens[1].c_str());
         year = atoi(tokens[2].c_str());
         readTime(tokens,3,hour,minute,sec,nsec);
         return convertComponents(year,month,day,hour,minute,sec,nsec);
      } // readFmt2
   
      LgrDate readFmt3(ListOf<StrAsc> &tokens)
      {
         uint4 code = atol(tokens[0].c_str());
         int month = 0,
            day = 0,
            year = 0,
            hour = 0,
            minute = 0,
            sec = 0,
            nsec = 0;
      
         day = code%100; code /= 100;
         month = code%100; code /= 100;
         year = code;
         readTime(tokens,1,hour,minute,sec,nsec);
         return convertComponents(year,month,day,hour,minute,sec,nsec);
      } // readFmt3
   
      LgrDate readFmt4(ListOf<StrAsc> &tokens)
      {
         int month = 0,
            day = 0,
            year = 0,
            hour = 0,
            minute = 0,
            sec = 0,
            nsec = 0;
         int val = 0;
      
         month = whichMonth(tokens[0]) + 1;
         day = atoi(tokens[1].c_str());
         year = atoi(tokens[2].c_str());
         readTime(tokens,3,hour,minute,sec,nsec);
         return convertComponents(year,month,day,hour,minute,sec,nsec);
      } // readFmt4
   
      LgrDate readFmt5(ListOf<StrAsc> &tokens)
      {
         int month = 0,
            day = 0,
            year = 0,
            hour = 0,
            minute = 0,
            sec = 0,
            nsec = 0;
         int val = 0;
      
         day = atoi(tokens[0].c_str());
         month = whichMonth(tokens[1]) + 1;
         year = atoi(tokens[2].c_str());
         readTime(tokens,3,hour,minute,sec,nsec);
         return convertComponents(year,month,day,hour,minute,sec,nsec);
      } // readFmt5

      LgrDate readFmt6(ListOf<StrAsc> &tokens)
      {
         int hour = 0;
         int minute = 0;
         int sec = 0;
         int nsec = 0; 
      
         readTime(tokens,0,hour,minute,sec,nsec);
         return convertComponents(0,0,0,hour,minute,sec,nsec);
      } // readFmt6

      LgrDate readFmt7(ListOf<StrAsc> &tokens)
      {
         int month = 0,
            day = 0,
            year = 0,
            hour = 0,
            minute = 0,
            sec = 0,
            nsec = 0;
      
         year = atol(tokens[0].c_str());
         month = atol(tokens[1].c_str());;
         day = atol(tokens[2].c_str());
         readTime(tokens,3,hour,minute,sec,nsec);
         return convertComponents(year,month,day,hour,minute,sec,nsec);
      } // readFmt7

      void readTime(
         ListOf<StrAsc> &tokens,
         int start,
         int &hour,
         int &min,
         int &sec,
         int &nsec)
      {
         for(uint4 i = start; i < tokens.get_count(); i++)
         {
            switch(i - start)
            {
            case 0:
               hour = atoi(tokens[i].c_str());
               break;
               
            case 1:
               min = atoi(tokens[i].c_str());
               break;
               
            case 2:
               sec = atoi(tokens[i].c_str());
               break;
               
            case 3:
               if(tokens[i].length() > 0)
               {
                  if(tokens[i][0] == '.')
                     tokens[i].cut(0,1);
                  tokens[i].cut(9);
                  nsec = atoi(tokens[i].c_str());
                  for(int j = 9 - (int)tokens[i].length(); j > 0; --j)
                     nsec *= 10;
               }
               break;

            case 4:
               if(tokens[i] == "PM")
                  hour += 12;
               break;
            }
         } 
      } // readTime

      LgrDate convertComponents(
         int year,
         int month,
         int day,
         int hour,
         int minute,
         int sec,
         int nsec)
      {
         // round the year
         if(year < 100 && year >= 50)
            year += 1900;
         else if(year < 100 && year < 50)
            year += 2000;
      
         // form the return value
         LgrDate rtn;
      
         rtn.setDate(year,month,day);
         rtn.setTime(hour,minute,sec,nsec);
         return rtn;
      } // convertComponents
   }; 
};

