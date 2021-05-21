/* Cora.Device.Bmp3Identifier.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 July 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_Bmp3Identifier_h
#define Cora_Device_Bmp3Identifier_h

#include "Cora.Device.DeviceBase.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class Bmp3Identifier;
      //@endgroup

      
      ////////////////////////////////////////////////////////////
      // class Bmp3IdentifierClient
      ////////////////////////////////////////////////////////////
      class Bmp3IdentifierClient: public Csi::InstanceValidator
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
            outcome_logger_security_blocked = 4,
            outcome_invalid_device_name = 5,
            outcome_communication_disabled = 6,
            outcome_communication_failed = 7,
            outcome_unsupported = 8,
            outcome_security_blocked = 9,
         };
         virtual void on_complete(
            Bmp3Identifier *identifier,
            outcome_type outcome,
            byte bmp_version,
            uint4 model_no,
            uint4 serial_no,
            StrAsc const &station_name) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class Bmp3Identifier
      //
      // Defines a component that exercises the LoggerNet transaction BMP3 logger identification.
      // In order to use this component, an application must provide a client object derived from
      // type Bmp3IdentifierClient (also known as Bmp3Identifier::client_type).  The application
      // should then create an instance of this class, set the appropriate properties such as
      // set_device_name(), and invoke one of the two version of start().  When the server
      // transaction is completed, the client's on_complete() method will be invoked.
      //
      // This component also has the ability to change the station name stored in the datalogger.
      // In order to do so, the application should call set_station_name() in order to set up the
      // property that will be sent to the logger. 
      ////////////////////////////////////////////////////////////
      class Bmp3Identifier:
         public Csi::EventReceiver,
         public DeviceBase
      {
      private:
         ////////////////////////////////////////////////////////////
         // station_name
         //
         // Holds station_name property that will get written to the datalogger when the server
         // transaction is executed.  By default this property is an empty string indicating that
         // the server will not attempt to change the station name.
         ////////////////////////////////////////////////////////////
         StrAsc station_name;

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
         // client
         ////////////////////////////////////////////////////////////
         Bmp3IdentifierClient *client;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Bmp3Identifier();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Bmp3Identifier();

         ////////////////////////////////////////////////////////////
         // get_station_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_station_name() const
         { return station_name; }

         ////////////////////////////////////////////////////////////
         // set_station_name
         ////////////////////////////////////////////////////////////
         void set_station_name(StrAsc const &station_name_)
         {
            if(state == state_standby)
               station_name = station_name_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef Bmp3IdentifierClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

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

      };
   };
};


#endif
