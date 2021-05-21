/* Cora.Device.ProgramStatsGetter.h

   Copyright (C) 2005, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 29 December 2005
   Last Change: Friday 08 February 2019
   Last Commit: $Date: 2019-02-08 18:32:07 -0600 (Fri, 08 Feb 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_ProgramStatsGetter_h
#define Cora_Device_ProgramStatsGetter_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"
#include "Csi.LgrDate.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ProgramStatsGetter;
      //@endgroup


      namespace ProgramStatsGetterHelpers
      {
         /**
          * Defines the structure that will return the program stats information to the
          * application.
          */
         struct program_stats_type
         {
            StrAsc os_version;
            uint2 os_sig;
            StrAsc serial_no;
            StrAsc power_up_prog;
            uint4 compile_state;
            StrAsc program_name;
            uint2 program_sig;
            Csi::LgrDate compile_time;
            StrAsc compile_result;
            StrAsc station_name;

            /**
             * Default constructor
             */
            program_stats_type():
               os_sig(0),
               program_sig(0)
            { }

            /**
             * copy constructor
             */
            program_stats_type(program_stats_type const &other):
               os_version(other.os_version),
               os_sig(other.os_sig),
               serial_no(other.serial_no),
               power_up_prog(other.power_up_prog),
               compile_state(other.compile_state),
               program_name(other.program_name),
               program_sig(other.program_sig),
               compile_time(other.compile_time),
               station_name(other.station_name),
               compile_result(other.compile_result)
            { }

            /**
             * copy operator
             */
            program_stats_type &operator =(program_stats_type const &other)
            {
               os_version = other.os_version;
               os_sig = other.os_sig;
               serial_no = other.serial_no;
               power_up_prog = other.power_up_prog;
               compile_state = other.compile_state;
               program_name = other.program_name;
               program_sig = other.program_sig;
               compile_time = other.compile_time;
               station_name = other.station_name;
               compile_result = other.compile_result;
               return *this;
            }

            /**
             * Formats the current program state to the specified stream.
             */
            void format_compile_state(std::ostream &out) const;
         };
      };
      

      /**
       * Defines the interface that the application must implement in order use the program stats
       * getter component.
       */
      class ProgramStatsGetterClient:
         public Csi::InstanceValidator
      {
      public:
         /**
          * Must be overloaded by the application object in order to handle the notification that
          * the operation is complete.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_session_failed = 2,
            outcome_invalid_logon = 3,
            outcome_server_security_blocked = 4,
            outcome_communication_failed = 5,
            outcome_communication_disabled = 6,
            outcome_logger_security_blocked = 7,
            outcome_invalid_device_name = 8,
            outcome_unsupported = 9
         };
         typedef ProgramStatsGetterHelpers::program_stats_type program_stats_type;
         virtual void on_complete(
            ProgramStatsGetter *getter,
            outcome_type outcome,
            program_stats_type const &program_stats) = 0; // ignore last if outcome is not success
      };


      /**
       * Defines a component that allows the application to get the current program state in the
       * datalogger.  If the component succeeds, the application will be able to get the currently
       * running program, the program set to run on power up, compile results, and other metadata
       * that describes the current program.
       *
       * In order to use this component, the application must provide an object that implements the
       * interface defined by class ProgramStatsGetterClient.  It should then create an instance of
       * this class, set its properties including device name, and call one of the two versions of
       * start().  When the component is finished, it will call the client's on_complete() method. 
       */
      class ProgramStatsGetter:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the current state of the component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the client for this component.
          */
         ProgramStatsGetterClient *client;

         /**
          * Set to true if the server should return the information that it has cached.
          */
         bool use_cached;

      public:
         /**
          * Constructor
          */
         ProgramStatsGetter():
            state(state_standby),
            client(0),
            use_cached(false)
         { }

         /**
          * Destructor
          */
         virtual ~ProgramStatsGetter()
         { finish(); }

         /**
          * @return Returns true if the server should return information cached in the server.
          */
         bool get_use_cached() const
         { return use_cached; }

         /**
          * @param use_cached_ Set to true if the server should respond with information it has
          * cached.
          */
         void set_use_cached(bool use_cached_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            use_cached = use_cached_; 
         }
         
         /**
          * Called to start the server transaction.
          *
          * @param client_ Specifies the application object that will receive notification of
          * completion.
          *
          * @param router Specifies a router that has not been previously connected.
          *
          * @param other_client Specifies a component that already has a connection to the server.
          */
         typedef ProgramStatsGetterClient client_type;
         void start(
            client_type *client_,
            router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(
            client_type *client_,
            ClientBase *other_client)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_client);
         }

         /**
          * Overloads the base class version to cancel the connection to the server.
          */
         virtual void finish()
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
         }

         /**
          * Overloads the base class version to handle events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Formats the specified outcome to the specified stream.
          *
          * @param out Specifies the stream to which the outcome will be described.
          *
          * @param outcome Specifies the outcome to format.
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Formats the program stats structure to the specified stream as text.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param stats Specifies the statistics to be formatted.
          */
         static void format_program_stats(std::ostream &out, ProgramStatsGetterHelpers::program_stats_type const &stats);

      protected:
         /**
          * Overloads the base class version to handle start operations when the connection is
          * ready.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the case class version to handle notification of failure.
          */
         virtual void on_devicebase_failure(
            devicebase_failure_type failure);

         /**
          * Overloads the base class version to handle notification of a session failure.
          */
         virtual void on_devicebase_session_failure();

         /**
          * Overloads the base class version to handle incoming messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);
      }; 
   };
};


#endif
