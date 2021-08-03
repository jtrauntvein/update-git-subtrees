/* Cora.Device.Rf95Tester.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 April 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_Rf95Tester_h
#define Cora_Device_Rf95Tester_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class Rf95tTester;

      
      ////////////////////////////////////////////////////////////
      // class Rf95TesterClient
      ////////////////////////////////////////////////////////////
      class Rf95tTesterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_session_failed = 2,
            outcome_invalid_logon = 3,
            outcome_invalid_device_name = 4,
            outcome_server_security_blocked = 5,
            outcome_logger_security_blocked = 6,
            outcome_rf_link_failed = 7,
            outcome_in_progress = 8,
            outcome_comm_disabled = 9,
         };
         struct record_type
         {
            byte test_packet_size;
            byte front2t;
            byte back2t;
            byte front1t;
            byte back1t;
         };
         typedef std::vector<record_type> records_type;
         virtual void on_complete(
            Rf95tTester *tester,
            outcome_type outcome,
            uint2 prom_sig,
            records_type const &records) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class Rf95tTester
      //
      // This class defines a component that exercises the server's RF Quality
      // Test transaction.  In order to use this component, an application must
      // provide an object derived from Rf95tTesterClient.  It should create
      // and instance of this component and set its properties including the
      // device name and repeater list.  It should then call start().  When
      // the transaction is complete, the component will invoke the client
      // object's on_complete() method to report the outcome and other response
      // parameters. 
      ////////////////////////////////////////////////////////////
      class Rf95tTester:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         Rf95tTesterClient *client;

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
         // repeaters
         ////////////////////////////////////////////////////////////
         std::vector<byte> repeaters;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Rf95tTester():
            state(state_standby),
            client(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Rf95tTester()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_repeaters
         ////////////////////////////////////////////////////////////
         typedef std::vector<byte> repeaters_type;
         void set_repeaters(repeaters_type const &repeaters_)
         {
            if(state == state_standby)
               repeaters = repeaters_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_repeaters
         ////////////////////////////////////////////////////////////
         repeaters_type const &get_repeaters() const
         { return repeaters; }

         ////////////////////////////////////////////////////////////
         // start (new router)
         ////////////////////////////////////////////////////////////
         typedef Rf95tTesterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (copy other component)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

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
         virtual void on_devicebase_session_failure();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
      };
   };
};


#endif
