/* Cora.Device.MemorySender.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 16 April 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $
   CVS $Header: /home/group/cvs2/cora/coratools/Cora.Device.MemorySender.h,v 1.3 2005/09/16 17:31:12 jon Exp $

*/

#ifndef Cora_Device_MemorySender_h
#define Cora_Device_MemorySender_h

#include "Cora.Device.DeviceBase.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class MemorySender;
      //@end group


      ////////////////////////////////////////////////////////////
      // class MemorySenderClient
      ////////////////////////////////////////////////////////////
      class MemorySenderClient: public Csi::InstanceValidator
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
            outcome_server_security_blocked = 4,
            outcome_logger_security_blocked = 5,
            outcome_communication_failed = 6,
            outcome_communication_disabled = 7,
            outcome_invalid_device_name = 8,
            outcome_unsupported = 9,
            outcome_image_too_large = 10,
         };
         virtual void on_complete(
            MemorySender *receiver,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class MemorySender
      //
      // Defines a component that can be used to load memory contents directly to the datalogger
      // at a specified address.  In order to use this component, an application must provide an
      // object derived from class MemorySenderClient (also known as MemorySender::client_type)
      // in order to receive event notifications affecting the transaction.  Once the component is
      // created by the application, the application must set the appropriate properties through
      // calls such as set_device_name(), set_address(), set_image(), and other properties as
      // needed.  Once the properties are set up as needed, the application must invoke one of the
      // two versions of start().
      //
      // When the server transaction is complete, the component will invoke the client object's
      // on_complete() method.  At this point, the component will return to a standby state.
      ////////////////////////////////////////////////////////////
      class MemorySender:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // address
         //
         // Specifies the beginning address for the datalogger memory dump
         //////////////////////////////////////////////////////////// 
         uint4 address;

         ////////////////////////////////////////////////////////////
         // image
         //
         // Specifies the memory image that should be sent to the datalogger.
         ////////////////////////////////////////////////////////////
         StrBin image;
         //@endgroup

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         MemorySender();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~MemorySender();

         ////////////////////////////////////////////////////////////
         // set_address
         ////////////////////////////////////////////////////////////
         void set_address(uint4 address_)
         {
            if(state == state_standby)
               address = address_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_address
         ////////////////////////////////////////////////////////////
         uint4 get_address() const
         { return address; }


         ////////////////////////////////////////////////////////////
         // set_image
         ////////////////////////////////////////////////////////////
         void set_image(StrBin const &image_)
         {
            if(state == state_standby)
               image = image_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // set_image (void buffer)
         ////////////////////////////////////////////////////////////
         void set_image(void const *buff, uint4 buff_len)
         {
            if(state == state_standby)
               image.setContents(buff,buff_len);
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_image
         ////////////////////////////////////////////////////////////
         StrBin const &get_image() const
         { return image; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef MemorySenderClient client_type;
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

      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;
      };
   };
};


#endif
