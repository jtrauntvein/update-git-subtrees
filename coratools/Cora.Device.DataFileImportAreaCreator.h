/* Cora.Device.DataFileImportAreaCreator.h

   Copyright (C) 2019, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 03 July 2019
   Last Change: Monday 15 July 2019
   Last Commit: $Date: 2019-07-15 15:13:11 -0600 (Mon, 15 Jul 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_DataFileImportAreaCreator_h
#define Cora_Device_DataFileImportAreaCreator_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class DataFileImportAreaCreator;


      /**
       * Defines the interface that the application must extend in order to use the
       * DataFileImportAreaCreator component type.
       */
      class DataFileImportAreaCreatorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction  has completed.
          *
          * @param sender Specifies the component sending this event.
          *
          * @param outcome Specifies the outcome of the transaction.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_invalid_device_name = 2,
            outcome_failure_server_blocked = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_session = 5,
            outcome_failure_invalid_area_name = 6,
            outcome_failure_logon = 7
         };
         virtual void on_complete(DataFileImportAreaCreator *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines the component that can be used to create a collect area that will be responsible
       * for importing data from CSI data files into the server cache tables.
       * In order to use this component, the application must provide a client object that derives
       * from class DataFileImportAreaCreatorClient.  It should then create an instance of this
       * component, set its properties including device name and the name for the new collect area,
       * and then calls one of the two versions of start().  When the server transaction has
       * completed, the client object's on_complete() method will be called.
       */
      class DataFileImportAreaCreator: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the name of the new collect area.
          */
         StrUni area_name;

         /**
          * Specifies the state of this transaction.
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
         DataFileImportAreaCreatorClient *client;
         
      public:
         /**
          * Constructor
          */
         DataFileImportAreaCreator():
            state(state_standby)
         { }

         /**
          * Destructor
          */
         virtual ~DataFileImportAreaCreator()
         { finish(); }

         /**
          * @return Returns the area name.
          */
         StrUni const &get_area_name() const
         { return area_name; }

         /**
          * @param value Specifies the area name.
          */
         void set_area_name(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            area_name = value;
         }

         /**
          * Starts the session with the server to execute the transaction.
          *
          * @param client_ Specifies the client for this transaction.
          *
          * @param router Specifies a messaging router that has been newly created.
          *
          * @param other_client Specifies another component that has an active session with the
          * server that can be cloned.
          */
         typedef DataFileImportAreaCreatorClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(area_name.length() == 0)
               throw std::invalid_argument("invalid area name");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_client)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(area_name.length() == 0)
               throw std::invalid_argument("invalid area name");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_client);
         }

         /**
          * Overloads the base class to release any resources.
          */
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            DeviceBase::finish();
         }

         /**
          * Overloads the base class version to handle asynchronous messages.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Writes a description of the failure code to the specified stream.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outcome Specifies the outcome code to format.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);
         
      protected:
         /**
          * Overloads the base class version to handle the event where session with the device is
          * ready.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle the report of a failure with the device
          * session.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Overloads the base class version to handle a report of a failure of the device session.
          */
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         /**
          * Overloads the base class version to handle incoming messages from the server.
          */
         virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};


#endif

