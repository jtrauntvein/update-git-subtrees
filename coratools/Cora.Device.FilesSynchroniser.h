/* Cora.Device.FilesSynchroniser.h

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 04 January 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_FilesSynchroniser_h
#define Cora_Device_FilesSynchroniser_h

#include "Cora.Device.DeviceBase.h"
#include "Cora.Device.DeviceSettingTypes.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      ////////////////////////////////////////////////////////////
      // class FilesSynchroniserClient
      ////////////////////////////////////////////////////////////
      class FilesSynchroniser;
      class FilesSynchroniserClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_status
         //
         // called when the server has sent a status notification.  The status
         // code indicates the nature of the event and the file name
         // (optionally empty depending on the status code) will indicate the
         // datalogger file name.
         ////////////////////////////////////////////////////////////
         enum status_type
         {
            status_getting_dir = 1,
            status_file_already_retrieved = 2,
            status_starting_retrieve = 3,
            status_retrieve_failed = 4,
            status_file_skipped = 5,
            status_file_retrieved = 6
         };
         virtual void on_status(
            FilesSynchroniser *synchroniser,
            status_type status,
            StrAsc const &file_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_comm_failed = 2,
            outcome_comm_disabled = 3,
            outcome_invalid_logger_security = 4,
            outcome_invalid_device_name = 5,
            outcome_invalid_server_security = 6,
            outcome_session_failed = 7,
            outcome_invalid_logon = 8,
            outcome_unsupported = 9
         };
         virtual void on_complete(
            FilesSynchroniser *synchroniser,
            outcome_type outcome) = 0;
      };

      
      ////////////////////////////////////////////////////////////
      // class FilesSynchroniser
      //
      // Defines a component that drives the Device Synchronise Files
      // transaction.  in order to use this component, an application must
      // provide an object that is derived from class FilesSynchroniserClient
      // (typedef given as client_type).  It must then create an instance of
      // this class, specify the device name and, optionally, the rules (via
      // one of the overloads of set_rules()), and then invoke one of the two
      // overloads of start().
      //
      // As the transaction is carried out and the server sends status
      // notifications, these will be sent to the client via its on_status()
      // method.  When  the transaction is complete, the client will be
      // notified by its on_complete() method.  
      ////////////////////////////////////////////////////////////
      class FilesSynchroniser:
         public DeviceBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         FilesSynchroniser();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~FilesSynchroniser()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_rules
         ////////////////////////////////////////////////////////////
         typedef FileSynchControlEx rules_type;
         typedef Csi::SharedPtr<rules_type> rules_handle;
         void set_rules(char const *rules_);
         void set_rules(rules_handle rules_);

         ////////////////////////////////////////////////////////////
         // get_rules
         ////////////////////////////////////////////////////////////
         rules_handle &get_rules()
         { return rules; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef FilesSynchroniserClient client_type;
         void start(client_type *client_, router_handle router);
         void start(client_type *client_, ClientBase *other_client);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // format_status
         ////////////////////////////////////////////////////////////
         static void format_status(
            std::ostream &out, client_type::status_type status, StrAsc const &file_name = "");

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(
            std::ostream &out, client_type::outcome_type outcome);

      protected:
         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // rules
         ////////////////////////////////////////////////////////////
         rules_handle rules;
      };
   };
};

#endif
