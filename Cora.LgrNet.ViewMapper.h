/* Cora.LgrNet.ViewMapper.h

   Copyright (C) 2012, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 23 August 2012
   Last Change: Wednesday 06 November 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_ViewMapper_h
#define Cora_LgrNet_ViewMapper_h

#include "Cora.ClientBase.h"
#include "Cora.LgrNet.Defs.h"
#include "Csi.Events.h"

namespace Cora
{
   namespace LgrNet
   {
      namespace ViewMapperHelpers
      {
         class map_entry_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // device_type
            ////////////////////////////////////////////////////////////
            DeviceTypes::device_type_code device_type;

            ////////////////////////////////////////////////////////////
            // name
            ////////////////////////////////////////////////////////////
            StrUni name;

            ////////////////////////////////////////////////////////////
            // object_id
            ////////////////////////////////////////////////////////////
            uint4 object_id;

            ////////////////////////////////////////////////////////////
            // level
            ////////////////////////////////////////////////////////////
            uint4 level;

            ////////////////////////////////////////////////////////////
            // parent
            ////////////////////////////////////////////////////////////
            map_entry_type *parent;

            ////////////////////////////////////////////////////////////
            // children
            ////////////////////////////////////////////////////////////
            typedef std::list<map_entry_type *> children_type;
            children_type children;

            ////////////////////////////////////////////////////////////
            // construct from message
            ////////////////////////////////////////////////////////////
            map_entry_type(Csi::Messaging::Message &in):
               parent(0)
            {
               uint4 temp;
               in.readUInt4(temp);
               in.readUInt4(object_id);
               in.readWStr(name);
               in.readUInt4(level);
               if((temp > DeviceTypes::unknown && temp < DeviceTypes::max_device_type) ||
                   temp == DeviceTypes::view_group)
                  device_type = static_cast<DeviceTypes::device_type>(temp);
               else
                  device_type = DeviceTypes::unknown;
            }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class ViewMapperClient
      ////////////////////////////////////////////////////////////
      class ViewMapper;
      class ViewMapperClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_logon = 1,
            failure_session = 2,
            failure_unsupported = 3,
            failure_security = 4,
            failure_invalid_view_id = 5,
            failure_view_deleted = 6
         };
         virtual void on_failure(ViewMapper *mapper, failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(ViewMapper *mapper)
         { }

         ////////////////////////////////////////////////////////////
         // on_view_changed
         ////////////////////////////////////////////////////////////
         typedef ViewMapperHelpers::map_entry_type entry_type;
         typedef Csi::SharedPtr<entry_type> value_type;
         typedef std::list<value_type> map_network_type;
         virtual void on_view_changed(
            ViewMapper *mapper, map_network_type const &network)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class ViewMapper
      //
      // Defines a component that wraps the LgrNet Enumerate View Map
      // transaction.  In order to use this component, the application must
      // provide an object that extends class ViewMapperClient.  It should then
      // create an instance of class ViewMapper, call its set_view_id() method
      // to set the view ID property, call its set_view_option() method to set
      // the view option property, and call one of its two start methods.
      // Once the server transaction has been started, the component will
      // call the client's on_view_changed() method to report the current state
      // of the view and will then call the client's on_started() method.  If
      // the transaction cannot be started or cannot continue, the client's
      // on_failure() method will get called. 
      ////////////////////////////////////////////////////////////
      class ViewMapper: public ClientBase, public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // enum view_option_type
         ////////////////////////////////////////////////////////////
         enum view_option_type
         {
            view_network = 1,
            view_groups = 2,
            view_stations_only = 3
         };
         
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ViewMapperClient *client;

         ////////////////////////////////////////////////////////////
         // view_id
         ////////////////////////////////////////////////////////////
         uint4 view_id;

         ////////////////////////////////////////////////////////////
         // view_option
         ////////////////////////////////////////////////////////////
         view_option_type view_option;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_starting,
            state_started
         } state;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ViewMapper():
            client(0),
            state(state_standby),
            view_option(view_network)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ViewMapper()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // set_view_id
         ////////////////////////////////////////////////////////////
         void set_view_id(uint4 id)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            view_id = id;
         }

         ////////////////////////////////////////////////////////////
         // set_view_option
         ////////////////////////////////////////////////////////////
         void set_view_option(view_option_type option)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            view_option = option;
         }

         ////////////////////////////////////////////////////////////
         // get_view_id
         ////////////////////////////////////////////////////////////
         uint4 get_view_id() const
         { return view_id; }

         ////////////////////////////////////////////////////////////
         // get_view_option
         ////////////////////////////////////////////////////////////
         view_option_type get_view_option() const
         { return view_option; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef ViewMapperClient client_type;
         void start(client_type *client_, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(other_component);
         }
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            ClientBase::finish();
         }

         ////////////////////////////////////////////////////////////
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(std::ostream &out, client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure_);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};

#endif
