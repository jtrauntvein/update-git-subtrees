/* Csi.LogByte.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 11 February 2000
   Last Change: Wednesday 20 July 2016
   Last Commit: $Date: 2016-07-21 15:54:36 -0600 (Thu, 21 Jul 2016) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.LogByte.h"
#include "StrBin.h"
#include "trace.h"
#include <iomanip>
#include <algorithm>
#include <fstream>


namespace Csi
{
   namespace
   {
      ////////////////////////////////////////////////////////////
      // class EvWr
      //
      // Defines the class of event that will be posted when LogByte::wr() is invoked
      ////////////////////////////////////////////////////////////
      class EvWr: public Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // buff
         ////////////////////////////////////////////////////////////
         StrBin buff;

         ////////////////////////////////////////////////////////////
         // is_input
         ////////////////////////////////////////////////////////////
         bool is_input;

         ////////////////////////////////////////////////////////////
         // stamp
         ////////////////////////////////////////////////////////////
         LgrDate stamp;

         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;
         
      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         EvWr(
            EventReceiver *receiver,
            void const *buff_,
            uint4 buff_len_,
            bool is_input_):
            Event(event_id,receiver),
            buff(buff_,buff_len_),
            is_input(is_input_),
            stamp(LgrDate::system())
         { }

      public:
         ////////////////////////////////////////////////////////////
         // create
         ////////////////////////////////////////////////////////////
         static EvWr *create(
            EventReceiver *receiver,
            void const *buff,
            uint4 buff_len,
            bool is_input)
         { return new EvWr(receiver,buff,buff_len,is_input); }
      }; 


      uint4 const EvWr::event_id = Event::registerType("LogByte::EvWr");


      ////////////////////////////////////////////////////////////
      // class EvBreak
      //
      // Defines the class of event that will be posted when LogByte::force_break() is invoked
      ////////////////////////////////////////////////////////////
      class EvBreak: public Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;

         ////////////////////////////////////////////////////////////
         // annotation
         ////////////////////////////////////////////////////////////
         StrAsc annotation;
         
      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         EvBreak(EvReceiver *receiver, char const *annotation_):
            Event(event_id,receiver),
            annotation(annotation_)
         { }

      public:
         ////////////////////////////////////////////////////////////
         // create
         ////////////////////////////////////////////////////////////
         static EvBreak *create(EvReceiver *receiver, char const *annotation)
         { return new EvBreak(receiver,annotation); }
      };


      uint4 const EvBreak::event_id = Event::registerType("Csi::LogByte::EvBreak");


      ////////////////////////////////////////////////////////////
      // class EvEnabledChange
      //
      // Defines the class of event that will be posted when LogByte::setEnabled() is called
      ////////////////////////////////////////////////////////////
      class EvEnabledChange: public Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;

         ////////////////////////////////////////////////////////////
         // is_enabled
         ////////////////////////////////////////////////////////////
         bool is_enabled;

      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         EvEnabledChange(EvReceiver *receiver, bool is_enabled_):
             Event(event_id,receiver),
             is_enabled(is_enabled_)
         { }

      public:
         ////////////////////////////////////////////////////////////
         // create
         ////////////////////////////////////////////////////////////
         static EvEnabledChange *create(EvReceiver *receiver, bool is_enabled)
         { return new EvEnabledChange(receiver,is_enabled); }
      };


      uint4 const EvEnabledChange::event_id =
      Event::registerType("Csi::LogByte::EvEnabledChange");


      ////////////////////////////////////////////////////////////
      // class BreakRecord
      //
      // Defines the class of record that will be logged when a break occurs
      ////////////////////////////////////////////////////////////
      class BreakRecord: public LogRecord
      {
         ////////////////////////////////////////////////////////////
         // annotation
         ////////////////////////////////////////////////////////////
         StrAsc annotation;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         BreakRecord(StrAsc const &annotation_):
            annotation(annotation_)
         { }
            
         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const
         {
            out << "\r\n\r\n";
            if(annotation.length())
               out << annotation << "\r\n";
            out << "B r e a k --------------------------------------------------\r\n";
         } // format

         ////////////////////////////////////////////////////////////
         // formatReq
         ////////////////////////////////////////////////////////////
         virtual uint4 formatReq() const
         { return 80 + (uint4)annotation.length() + 1; }
      };
   };

   
   ////////////////////////////////////////////////////////////
   // class LogByte definitions
   ////////////////////////////////////////////////////////////
   uint4 const LogByte::ev_log_recs_available =
   Event::registerType("LogByte::ev_log_recs_available");


   LogByte::LogByte(
      char const *path,
      char const *file_name):
      LogBaler(path,file_name),
      client(0),
      disable_send(false)
   { }


   LogByte::~LogByte()
   { flush(); }


   void LogByte::wr(void const *buff, uint4 buff_len, bool is_input)
   {
      try
      {
         if(buff_len > 0)
         {
            EvWr *ev = EvWr::create(this,buff,buff_len,is_input);
            ev->post();
         }
      }
      catch(Event::BadPost &)
      { trace("\"Csi::LogByte::wr\",\"post failed\""); }
   } // wr


   void LogByte::setEnable(bool is_enabled)
   {
      try
      {
         Event *ev = EvEnabledChange::create(this,is_enabled);
         ev->post();
      }
      catch(Event::BadPost &)
      { trace("\"Csi::LogByte::setEnable\",\"post failed\""); }
   } // setEnable


   void LogByte::force_break(char const *annotation)
   {
      try
      {
         Event *ev = EvBreak::create(this,annotation);
         ev->post();
      }
      catch(Event::BadPost &)
      { trace("\"Csi::LogByte::force_break\",\"post failed\""); }
   } // force_break;


   uint4 LogByte::pop_history(LogByte::Record recs[], uint4 max_records)
   {
      uint4 rtn = max_records;

      if(history.size() < max_records)
         rtn = (uint4)history.size();
      for(uint4 i = 0; i < rtn; i++)
      {
         recs[i] = history.front();
         history.pop_front();
      }
      disable_send = false;
      return rtn;
   } // pop_history


   void LogByte::set_client(EvReceiver *client_)
   {
      try
      {
         client = client_;
         disable_send = false;
         if(client && !history.empty())
         {
            Event *ev = Event::create(ev_log_recs_available,client);
            ev->post();
            disable_send = true;
         }
      }
      catch(Event::BadPost &)
      { trace("\"Csi::LogByte::set_client\",\"post failed\""); }
   } // set_client


   void LogByte::check_remaining()
   {
      if(accumulator.length)
      {
         LgrDate elapsed(LgrDate::system() - accumulator.stamp);
         if(elapsed.get_nanoSec() > 300*LgrDate::nsecPerMSec)
            flush();
      }
   } // check_remaining


   void LogByte::on_log_baled(char const *new_path)
   {
      if(output != 0)
      {
         LgrDate::system().format(
            *output,
            "New File:  %Y-%m-%d %H:%M:%S%x\r\n\r\n");
      }
   } // on_log_baled


   void LogByte::flush()
   {
      // we only need to do somthing if the accumulator has data
      if(accumulator.length)
      {
         // add the record to the history only if it is enabled
         if(client != 0)
         {
            // notify the client
            history.push_back(accumulator);
            try
            {
               if(!disable_send)
               {
                  Event *ev = Event::create(ev_log_recs_available,client);
                  ev->post();
                  disable_send = true;
               }
            }
            catch(Event::BadPost &)
            {
               client = 0;
               history.clear();
               trace("\"Csi::LogByte::flush\",\"post failed\"");
            }
         }

         // write the accumulator data and initialise the accumulator
         LogBaler::wr(accumulator);
         accumulator.initialise();
      }
   } // flush


   void LogByte::receive(SharedPtr<Event> &ev)
   {
      typedef EvWr wr_event_type;
      typedef EvBreak break_event_type;
      typedef EvEnabledChange enabled_change_type;

      if(ev->getType() == wr_event_type::event_id)
      {
         wr_event_type *wr_event = static_cast<wr_event_type *>(ev.get_rep());
         byte const *buff = reinterpret_cast<byte const *>(wr_event->buff.getContents());;
         
         for(uint4 i = 0; i < wr_event->buff.length(); ++i)
         {
            // send each byte to the accumulator. Flush it whenever it fills up
            if(!accumulator.add_byte(buff[i],wr_event->is_input,wr_event->stamp))
            {
               flush();
               accumulator.add_byte(buff[i],wr_event->is_input,wr_event->stamp);
            }
         }
      }
      else if(ev->getType() == break_event_type::event_id)
      {
         break_event_type *event = static_cast<break_event_type *>(ev.get_rep()); 
         flush();
         LogBaler::wr(BreakRecord(event->annotation));
      }
      else if(ev->getType() == enabled_change_type::event_id)
      {
         enabled_change_type *ev_chg = static_cast<enabled_change_type *>(ev.get_rep());
         flush();
         LogBaler::setEnable(ev_chg->is_enabled);
      }
   } // receive


   ////////////////////////////////////////////////////////////
   // class LogByte::Record definitions
   ////////////////////////////////////////////////////////////
   LogByte::Record::Record():
      is_input(false),
      length(0)
   { }


   LogByte::Record::Record(Record const &other):
      is_input(other.is_input),
      length(other.length),
      stamp(other.stamp)
   { memcpy(data,other.data,sizeof(data)); }


   LogByte::Record::~Record()
   { }


   LogByte::Record &LogByte::Record::operator =(Record const &other)
   {
      is_input = other.is_input;
      length = other.length;
      stamp = other.stamp;
      memcpy(data,other.data,sizeof(data));
      return *this;
   } // copy operator


   void LogByte::Record::initialise()
   { length = 0; }


   bool LogByte::Record::add_byte(byte val, bool is_input_, LgrDate const &stamp_)
   {
      bool rtn;
      if(length)
      {
         if(length < sizeof(data))
         {
            if(is_input == is_input_)
            {
               int8 elapsed = (stamp_.get_nanoSec() - stamp.get_nanoSec())/LgrDate::nsecPerMSec;
               if(elapsed >= 0 && elapsed < 300)
                  rtn = true;
               else
                  rtn = false;
            }
            else
               rtn = false;
         }
         else
            rtn = false;
       }
      else
      {
         is_input = is_input_;
         stamp = stamp_;
         rtn = true;
      }
      if(rtn)
         data[length++] = val;
      return rtn;
   } // add_byte


   void LogByte::Record::format(std::ostream &out) const
   {
      // format the time
      stamp.format(out,"%H:%M:%S.%3");
      
      // output the direction code
      out << ' ' << (is_input ? 'R' : 'T') << " ";
      
      // output the hex dump of each character
      int i;
      for(i = 0; i < length; i++)
         out << std::hex << std::setw(2) << std::setfill('0') 
             << static_cast<uint2>(data[i]) << ' ';
      while(i++ < sizeof(data))
         out << "   ";
      out << " ";
      
      // output the printable representation of each character
      for(i = 0; i < length; i++)
         if(isprint(data[i]))
            out << static_cast<char>(data[i]);
         else
            out << '.';
   } // format
};

