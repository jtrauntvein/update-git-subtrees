/* Csi.LogByte.h

   Copyright (C) 2000, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 28 January 1998
   Last Change: Wednesday 15 April 2020
   Last Commit: $Date: 2020-04-16 09:32:31 -0600 (Thu, 16 Apr 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_LogByte_h
#define Csi_LogByte_h

#include "Csi.LogBaler.h"
#include "Csi.Events.h"
#include "Csi.LgrDate.h"
#include <list>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class LogByte
   //
   // Extends class LogBaler to provide the capability of streaming binary I/O data. Also adds a
   // mechanism where a client object (derived from class Csi::EventReceiver) can receive
   // notifications regarding the arrival of new stream data.
   //////////////////////////////////////////////////////////// 
   class LogByte: public LogBaler, public EventReceiver
   {
   public:
      ////////////////////////////////////////////////////////////
      // class Record
      //
      // Describes a signle line of I/O in the log including a time stamp, an I/O indicator, and up
      // to sixteen bytes of bianry data.
      //////////////////////////////////////////////////////////// 
      class Record: public LogRecord
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructors
         //////////////////////////////////////////////////////////// 
         Record();
         Record(Record const &other);

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~Record();

         ////////////////////////////////////////////////////////////
         // copy operator
         //////////////////////////////////////////////////////////// 
         Record &operator =(Record const &other);

         ////////////////////////////////////////////////////////////
         // initialise
         //
         // Clears any existing data in the record
         //////////////////////////////////////////////////////////// 
         void initialise();

         ////////////////////////////////////////////////////////////
         // add_byte
         //
         // Adds a byte to end of the data. Returns true if there was room for the byte or false if
         // the byte would not fit. Will also return false if the record is not empty and the
         // is_input_ parameter conflicts with the is_input member.
         //////////////////////////////////////////////////////////// 
         bool add_byte(byte val, bool is_input_, LgrDate const &stamp_);

         ////////////////////////////////////////////////////////////
         // format
         //////////////////////////////////////////////////////////// 
         virtual void format(std::ostream &out) const;

         ////////////////////////////////////////////////////////////
         // formatReq
         //////////////////////////////////////////////////////////// 
         virtual uint4 formatReq() const { return 80; }

      public:
         ////////////////////////////////////////////////////////////
         // stamp
         //
         // The time stamp when this record was last initialised
         //////////////////////////////////////////////////////////// 
         LgrDate stamp;

         ////////////////////////////////////////////////////////////
         // is_input
         //
         // Set to true if the data in the record was read, Set to false if the data was written
         //////////////////////////////////////////////////////////// 
         bool is_input;

         ////////////////////////////////////////////////////////////
         // length
         //
         // Relates the number of valid bytes that are in the record
         //////////////////////////////////////////////////////////// 
         byte length;

         ////////////////////////////////////////////////////////////
         // data
         //////////////////////////////////////////////////////////// 
         byte data[16];
      };

      ////////////////////////////////////////////////////////////
      // ev_log_recs_available
      //
      // Identifies the message type that will be posted to the client to indicate that log records
      // have become available
      //////////////////////////////////////////////////////////// 
      static uint4 const ev_log_recs_available;
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      //////////////////////////////////////////////////////////// 
      LogByte(char const *path, char const *file_name);

      ////////////////////////////////////////////////////////////
      // destructor
      //////////////////////////////////////////////////////////// 
      virtual ~LogByte();

      ////////////////////////////////////////////////////////////
      // wr
      //
      // Writes a sequence of characters to the log. This method is safe to invoke from any thread
      //////////////////////////////////////////////////////////// 
      void wr(void const *buff, uint4 buff_len, bool is_input);

      ////////////////////////////////////////////////////////////
      // setEnable
      //
      // Overloads LogBaler::setEnable() to ensure that any remaining output available in the
      // accumulator is flushed before the file is closed.
      //////////////////////////////////////////////////////////// 
      virtual void setEnable(bool is_enabled);

      ////////////////////////////////////////////////////////////
      // force_break
      //
      // Forces a break in the log and flushes any bytes that have not yet been logged
      //////////////////////////////////////////////////////////// 
      void force_break(char const *annotation = "");

      ////////////////////////////////////////////////////////////
      // pop_history
      //
      // Removes up to the specified number of records from the history queue. This method is
      // intended to be used by the client that has requested notifications. Returns the total
      // number of records that were popped.
      //////////////////////////////////////////////////////////// 
      uint4 pop_history(Record recs[], uint4 max_records);

      ////////////////////////////////////////////////////////////
      // set_client
      //
      // Sets up the client that will receive notifications regarding new log records
      //////////////////////////////////////////////////////////// 
      void set_client(EventReceiver *client_);

      ////////////////////////////////////////////////////////////
      // check_remaining
      //
      // Checks to see if there are any bytes remaining in the accumulator that should be flushed.
      //////////////////////////////////////////////////////////// 
      void check_remaining();

      /**
       * Overloads the base class version to ensure that any remainder record is written and the
       * output flushed.
       */
      virtual void flush() override;
      
      ////////////////////////////////////////////////////////////
      // receive
      //
      // Overloads EvReceiver::receive() to handle low level log receive events and break
      // events. These events will be posted when wr() and force_break() are called.
      //////////////////////////////////////////////////////////// 
      virtual void receive(SharedPtr<Event> &ev) override;

   protected:
      ////////////////////////////////////////////////////////////
      // on_log_baled
      ////////////////////////////////////////////////////////////
      virtual void on_log_baled(char const *new_path) override;
      
   private:
      ////////////////////////////////////////////////////////////
      // history
      //////////////////////////////////////////////////////////// 
      std::list<Record> history;

      ////////////////////////////////////////////////////////////
      // accumulator
      //
      // Used to hold bytes that are being buffered before being written to the log. This object
      // will be flushed when the log is disabled, a break is forced, when a certain time period has
      // passed between records, the current contents are emptied, or when the direction of
      // transmission changes.
      //////////////////////////////////////////////////////////// 
      Record accumulator;

      ////////////////////////////////////////////////////////////
      // client
      //
      // Reference to the object that will receive notifications when the accumulator is flushed.
      //////////////////////////////////////////////////////////// 
      EventReceiver *client;

      ////////////////////////////////////////////////////////////
      // disable_send
      //
      // Set to true to prevent further notifications from being sent to the client. This value is
      // set to true when a notification is sent and reset to false when pop_history() is invoked.
      //////////////////////////////////////////////////////////// 
      bool disable_send;
   };
};

#endif
