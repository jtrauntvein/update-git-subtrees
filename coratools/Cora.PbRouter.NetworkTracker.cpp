/* Cora.PbRouter.NetworkTracker.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 27 February 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.NetworkTracker.h"
#include "Cora.Device.DeviceSettingFactory.h"
#include <algorithm>
#include <set>


namespace Cora
{
   namespace PbRouter
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // tracker
            ////////////////////////////////////////////////////////////
            NetworkTracker *tracker;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef NetworkTracker::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            void do_notify()
            {
               if(client_type::is_valid_instance(client))
                  notify();
            }

         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               NetworkTracker *tracker_,
               client_type *client_):
               Event(event_id,tracker_),
               tracker(tracker_),
               client(client_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               NetworkTracker *tracker,
               client_type *client)
            {
               try{(new event_started(tracker,client))->post();}
               catch(Csi::Event::BadPost &) { }
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(tracker); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               NetworkTracker *tracker,
               client_type *client):
               event_base(event_id,tracker,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::PbRouter::NetworkTracker:event_started");


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               NetworkTracker *tracker,
               client_type *client,
               failure_type failure)
            {
               try{(new event_failure(tracker,client,failure))->post();}
               catch(Csi::Event::BadPost &) { }
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(tracker,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               NetworkTracker *tracker,
               client_type *client,
               failure_type failure_):
               event_base(event_id,tracker,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::PbRouter::NetworkTracker:event_failure");


         ////////////////////////////////////////////////////////////
         // class event_node_added
         ////////////////////////////////////////////////////////////
         class event_node_added: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // node_info
            ////////////////////////////////////////////////////////////
            typedef client_type::node_info node_info_type;
            node_info_type node_info;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &node_info)
            {
               try{(new event_node_added(tracker,client,node_info))->post();}
               catch(Csi::Event::BadPost &) { }
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_node_added(tracker,node_info); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_node_added(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &node_info_):
               event_base(event_id,tracker,client),
               node_info(node_info_)
            { }
         };


         uint4 const event_node_added::event_id =
         Csi::Event::registerType("Cora::PbRouter::NetworkTracker::event_node_added");


         ////////////////////////////////////////////////////////////
         // class event_node_deleted
         ////////////////////////////////////////////////////////////
         class event_node_deleted: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // node_info
            ////////////////////////////////////////////////////////////
            typedef client_type::node_info node_info_type;
            node_info_type node_info;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &node_info)
            {
               try{(new event_node_deleted(tracker,client,node_info))->post();}
               catch(Csi::Event::BadPost &) { }
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_node_deleted(tracker,node_info); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_node_deleted(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &node_info_):
               event_base(event_id,tracker,client),
               node_info(node_info_)
            { }
         };


         uint4 const event_node_deleted::event_id =
         Csi::Event::registerType("Cora::PbRouter::NetworkTracker:event_node_deleted");


         ////////////////////////////////////////////////////////////
         // class event_node_changed
         ////////////////////////////////////////////////////////////
         class event_node_changed: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // node_info
            ////////////////////////////////////////////////////////////
            typedef client_type::node_info node_info_type;
            node_info_type node_info;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &node_info)
            {
               try{(new event_node_changed(tracker,client,node_info))->post();}
               catch(Csi::Event::BadPost &) { }
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_node_changed(tracker,node_info); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_node_changed(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &node_info_):
               event_base(event_id,tracker,client),
               node_info(node_info_)
            { }
         };


         uint4 const event_node_changed::event_id =
         Csi::Event::registerType("Cora::PbRouter::NetworkTracker:event_node_changed");


         ////////////////////////////////////////////////////////////
         // class event_link_added
         ////////////////////////////////////////////////////////////
         class event_link_added: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // info1
            ////////////////////////////////////////////////////////////
            typedef client_type::node_info node_info_type;
            node_info_type info1;

            ////////////////////////////////////////////////////////////
            // info2
            ////////////////////////////////////////////////////////////
            node_info_type info2;

            ////////////////////////////////////////////////////////////
            // response_time
            ////////////////////////////////////////////////////////////
            uint4 response_time;

            ////////////////////////////////////////////////////////////
            // is_static
            ////////////////////////////////////////////////////////////
            bool is_static;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &info1,
               node_info_type &info2,
               uint4 response_time,
               bool is_static)
            {
               try
               {
                  event_link_added *ev = new event_link_added(
                     tracker,
                     client,
                     info1,
                     info2,
                     response_time,
                     is_static);
                  ev->post();
               }
               catch(Csi::Event::BadPost &) { }
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_link_added(tracker,info1,info2,response_time,is_static); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_link_added(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &info1_,
               node_info_type &info2_,
               uint4 response_time_,
               bool is_static_):
               event_base(event_id,tracker,client),
               info1(info1_),
               info2(info2_),
               response_time(response_time_),
               is_static(is_static_)
            { }
         };


         uint4 const event_link_added::event_id =
         Csi::Event::registerType("Cora::PbRouter::NetworkTracker:event_link_added");


         ////////////////////////////////////////////////////////////
         // class event_link_changed
         ////////////////////////////////////////////////////////////
         class event_link_changed: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // info1
            ////////////////////////////////////////////////////////////
            typedef client_type::node_info node_info_type;
            node_info_type info1;

            ////////////////////////////////////////////////////////////
            // info2
            ////////////////////////////////////////////////////////////
            node_info_type info2;

            ////////////////////////////////////////////////////////////
            // response_time
            ////////////////////////////////////////////////////////////
            uint4 response_time;

            ////////////////////////////////////////////////////////////
            // is_static
            ////////////////////////////////////////////////////////////
            bool is_static;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &info1,
               node_info_type &info2,
               uint4 response_time,
               bool is_static)
            {
               try
               {
                  event_link_changed *ev = new event_link_changed(
                     tracker,
                     client,
                     info1,
                     info2,
                     response_time,
                     is_static);
                  ev->post();
               }
               catch(Csi::Event::BadPost &) { }
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_link_changed(tracker,info1,info2,response_time,is_static); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_link_changed(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &info1_,
               node_info_type &info2_,
               uint4 response_time_,
               bool is_static_):
               event_base(event_id,tracker,client),
               info1(info1_),
               info2(info2_),
               response_time(response_time_),
               is_static(is_static_)
            { }
         };


         uint4 const event_link_changed::event_id =
         Csi::Event::registerType("Cora::PbRouter::NetworkTracker:event_link_changed");


         ////////////////////////////////////////////////////////////
         // class event_link_deleted
         ////////////////////////////////////////////////////////////
         class event_link_deleted: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // info1
            ////////////////////////////////////////////////////////////
            typedef client_type::node_info node_info_type;
            node_info_type info1;

            ////////////////////////////////////////////////////////////
            // info2
            ////////////////////////////////////////////////////////////
            node_info_type info2;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &info1,
               node_info_type &info2)
            {
               try
               {
                  event_link_deleted *ev = new event_link_deleted(
                     tracker,
                     client,
                     info1,
                     info2);
                  ev->post();
               }
               catch(Csi::Event::BadPost &) { }
            }
            
         protected:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_link_deleted(tracker,info1,info2); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_link_deleted(
               NetworkTracker *tracker,
               client_type *client,
               node_info_type &info1_,
               node_info_type &info2_):
               event_base(event_id,tracker,client),
               info1(info1_),
               info2(info2_)
            { }
         };


         uint4 const event_link_deleted::event_id =
         Csi::Event::registerType("Cora::PbRouter::NetworkTracker:event_link_deleted");


         ////////////////////////////////////////////////////////////
         // class link_has_pair
         //
         // Defines a predicate that evaluates whether a given link object has the address pair
         // given in the predicate constructor
         ////////////////////////////////////////////////////////////
         class link_has_pair
         {
         private:
            uint2 address1, address2;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            link_has_pair(uint2 address1_, uint2 address2_):
               address1(address1_),
               address2(address2_)
            { }
            
            ////////////////////////////////////////////////////////////
            // predicate
            ////////////////////////////////////////////////////////////
            bool operator ()(NetworkTracker::link_type const &link)
            {
               return ((address1 == link.address1 && address2 == link.address2) ||
                       (address2 == link.address1 && address1 == link.address2));
            }
         };


         ////////////////////////////////////////////////////////////
         // class setting_factory_type
         ////////////////////////////////////////////////////////////
         class setting_factory_type: public Device::DeviceSettingFactory
         {
         public:
            ////////////////////////////////////////////////////////////
            // make_setting
            ////////////////////////////////////////////////////////////
            virtual Setting *make_setting(uint4 setting_id)
            {
               Setting *rtn = 0;
               if(setting_id == Device::Settings::pakbus_node_identifier)
                  rtn = DeviceSettingFactory::make_setting(setting_id);
               return rtn;
            }
         };
      };


      ////////////////////////////////////////////////////////////
      // class NetworkTracker definitions
      ////////////////////////////////////////////////////////////
      NetworkTracker::NetworkTracker():
         client(0),
         state(state_standby),
         refresh_timer(0)
      { }

      
      NetworkTracker::~NetworkTracker()
      { finish(); }


      void NetworkTracker::set_one_shot(Csi::SharedPtr<OneShot> one_shot_)
      {
         if(state == state_standby)
            one_shot = one_shot_;
         else
            throw exc_invalid_state();
      } // set_one_shot

      
      void NetworkTracker::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               PbRouterBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void NetworkTracker::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               PbRouterBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void NetworkTracker::finish()
      {
         state = state_standby;
         client = 0;
         links.clear(); 
         nodes.clear();
         links_lister.clear();
         nodes_lister.clear();
         device_mapper.clear();
         settings.clear();
         if(one_shot != 0 && refresh_timer != 0)
            one_shot->disarm(refresh_timer);
         one_shot.clear();
      } // finish

      
      bool NetworkTracker::find_node(node_type &node, uint2 address)
      {
         bool rtn = false;
         nodes_type::iterator ni = nodes.find(address);
         if(ni != nodes.end())
         {
            node = ni->second;
            rtn = true;
         }
         return rtn; 
      } // find_node

      
      void NetworkTracker::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;
            if(event->getType() == event_failure::event_id)
               finish();
            if(event->client == client)
               event->do_notify();
         }
      } // receive

      
      void NetworkTracker::on_pbrouterbase_ready()
      {
         // we want to have the device information available when we get the first nodes and link
         // info.  For this reason, we will first start the network map enumeration.
         state = state_before_active;
         device_mapper.bind(new LgrNet::DeviceMapper);
         device_mapper->start(this,this);
         if(one_shot == 0)
            one_shot.bind(new OneShot);
      } // on_pbrouterbase_ready

      
      void NetworkTracker::on_pbrouterbase_failure(pbrouterbase_failure_type failure_)
      {
         client_type::failure_type failure;
         switch(failure_)
         {
         case pbrouterbase_failure_logon:
            failure = client_type::failure_invalid_logon;
            break;
            
         case pbrouterbase_failure_session:
            failure = client_type::failure_server_session_failed;
            break;
            
         case pbrouterbase_failure_invalid_router_id:
            failure = client_type::failure_invalid_router_id;
            break;
            
         case pbrouterbase_failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         case pbrouterbase_failure_security:
            failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            failure = client_type::failure_unknown;
            break;   
         }
         event_failure::cpost(this,client,failure);
      } // on_pbrouterbase_failure

      
      void NetworkTracker::on_started(LinksEnumerator *tran)
      {
         if(state == state_before_active)
         {
            // we should now have a complete picture of the network.  Since we have blocked updates,
            // we should send them now
            state = state_active;
            for(nodes_type::iterator ni = nodes.begin();
                ni != nodes.end();
                ++ni)
               event_node_added::cpost(this,client,ni->second);
            for(links_type::iterator li = links.begin();
                li != links.end();
                ++li)
            {
               nodes_type::iterator node1 = nodes.find(li->address1);
               nodes_type::iterator node2 = nodes.find(li->address2);
               if(node1 != nodes.end() && node2 != nodes.end())
                  event_link_added::cpost(
                     this,client,
                     node1->second,
                     node2->second,
                     li->response_time_msec,
                     li->is_static);
               else
                  assert(false);
            }
            event_started::cpost(this,client);
            refresh_timer = one_shot->arm(this,3000);
         }
      } // on_started

      
      void NetworkTracker::on_failure(
         LinksEnumerator *tran,
         LinksEnumeratorClient::failure_type failure_)
      {
         client_type::failure_type failure;
         switch(failure_)
         {
         case LinksEnumeratorClient::failure_server_session_failed:
         case LinksEnumeratorClient::failure_connection_failed:
            failure = client_type::failure_connection_failed;
            break;
            
         case LinksEnumeratorClient::failure_invalid_logon:
            failure = client_type::failure_invalid_logon;
            break;
            
         case LinksEnumeratorClient::failure_server_security_blocked:
            failure = client_type::failure_server_security_blocked;
            break;
            
         case LinksEnumeratorClient::failure_invalid_router_id:
            failure = client_type::failure_invalid_router_id;
            break;
            
         case LinksEnumeratorClient::failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         default:
            failure = client_type::failure_unknown;
            break;
         }
         event_failure::cpost(this,client,failure);
      } // on_failure

      
      void NetworkTracker::on_link_added(
         LinksEnumerator *tran,
         uint2 node1_address,
         uint4 node1_net_map_id,
         bool node1_is_router,
         uint2 node2_address,
         uint4 node2_net_map_id,
         bool node2_is_router,
         uint4 worst_case_resp_time)
      {
         // the processing that we need to do here is exactly the same as that needed if the link
         // got changed.  In order to avoid unnecessary duplication, we will invoke the
         // on_link_changed() handler. 
         on_link_changed(
            tran,
            node1_address,
            node1_net_map_id,
            node1_is_router,
            node2_address,
            node2_net_map_id,
            node2_is_router,
            worst_case_resp_time);

         // a dynamic link will replace any static links involving these nodes.  Because of this, we
         // will delete any static links after making the appropriate adjustments in
         // on_link_changed().
         //
         // This operation used to take place first but this could result in nodes disappearing when
         // the transistion took place.
         links_type temp(links);
         while(!temp.empty())
         {
            link_type &link = temp.front();
            if(link.is_static &&
               (link.address1 == node1_address ||
                link.address2 == node1_address ||
                link.address1 == node2_address ||
                link.address2 == node2_address))
            {
               // rather than deleting the link directly, we need to check to see if this static
               // route has been made redundant by the new dynamic route
               uint2 other_address = link.address1;
               bool other_has_dynamic = false;
               
               if(other_address == node1_address || other_address == node2_address)
                  other_address = link.address2;
               for(links_type::iterator li = links.begin();
                   !other_has_dynamic && li != links.end();
                   ++li)
               {
                  if((li->address1 == other_address || li->address2 == other_address) &&
                     !li->is_static)
                     other_has_dynamic = true;
               }
               if(other_has_dynamic)
                  on_link_deleted(
                     tran,
                     link.address1,
                     node1_net_map_id,
                     link.address2,
                     node2_net_map_id); 
            }
            temp.pop_front();
         } 
      } // on_link_added

      
      void NetworkTracker::on_link_deleted(
         LinksEnumerator *tran,
         uint2 node1_address,
         uint4 node1_net_map_id,
         uint2 node2_address,
         uint4 node2_net_map_id)
      {
         links_type::iterator li = std::find_if(
            links.begin(),
            links.end(),
            link_has_pair(node1_address,node2_address));
         if(li != links.end())
         {
            nodes_type::iterator node1 = nodes.find(node1_address);
            nodes_type::iterator node2 = nodes.find(node2_address);
            links.erase(li);
            
            if(state != state_before_active &&
               node1 != nodes.end() && node2 != nodes.end())
               event_link_deleted::cpost(
                  this,client,node1->second,node2->second);
            nodes_lister.bind(new NodesLister);
            nodes_lister->set_pakbus_router_name(get_pakbus_router_name());
            nodes_lister->start(this,this);
         }
      } // on_link_deleted

      
      void NetworkTracker::on_link_changed(
         LinksEnumerator *tran,
         uint2 node1_address,
         uint4 node1_net_map_id,
         bool node1_is_router,
         uint2 node2_address,
         uint4 node2_net_map_id,
         bool node2_is_router,
         uint4 worst_case_resp_time)
      {
         // make sure that the specifies nodes exist
         nodes_type::iterator node1 = update_node(
            node1_address,node1_is_router,node1_net_map_id);
         nodes_type::iterator node2 = update_node(
            node2_address,node2_is_router,node2_net_map_id);
         
         // now we need to update/create the link objects
         links_type::iterator li = std::find_if(
            links.begin(),
            links.end(),
            link_has_pair(node1_address,node2_address));
         if(li == links.end())
         {
            link_type link = {
               node1_address, node2_address,
               worst_case_resp_time,
               false
            };
            links.push_back(link);
            if(state != state_before_active)
               event_link_added::cpost(
                  this,client,node1->second,node2->second,worst_case_resp_time,false);
         }
         else
         {
            if(li->is_static == true || li->response_time_msec != worst_case_resp_time)
            {
               li->is_static = false;
               li->response_time_msec = worst_case_resp_time;
               if(state != state_before_active)
                  event_link_changed::cpost(
                     this,client,node1->second,node2->second,worst_case_resp_time,false);
            }
         }  
      } // on_link_changed


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class link_has_address
         ////////////////////////////////////////////////////////////
         class link_has_address
         {
            ////////////////////////////////////////////////////////////
            // address
            ////////////////////////////////////////////////////////////
            uint2 address;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            link_has_address(uint2 address_):
               address(address_)
            { }

            ////////////////////////////////////////////////////////////
            // function call operator
            ////////////////////////////////////////////////////////////
            bool operator ()(NetworkTracker::link_type const &link)
            {
               return (link.address1 == address || link.address2 == address);
            }
         };
      };
      
      
      void NetworkTracker::on_complete(
         NodesLister *lister,
         outcome_type outcome,
         NodesListerClient::nodes_list_type &nodes_list)
      {
         if(outcome == NodesListerClient::outcome_success)
         {
            // if we are still in a pre-active state, the initialisation must be finished.
            if(state == state_before_active && links_lister == 0)
            {
               links_lister.bind(new LinksEnumerator);
               links_lister->set_pakbus_router_name(get_pakbus_router_name());
               links_lister->start(this,this);
            }

            // we need to make sure that the server is represented as a node
            nodes_type::iterator ni = nodes.find(pbrouter_address);
            if(ni == nodes.end())
               add_node(pbrouter_address,true);
            
            // we will process the new list of nodes in several passes.  First, we need to make sure
            // that all of the nodes given in the supplied list are represented.  We will keep track
            // of each address used.
            NodesListerClient::nodes_list_type::iterator cl;
            typedef std::set<uint2> used_addresses_type;
            used_addresses_type used_addresses;
            for(cl = nodes_list.begin(); cl != nodes_list.end(); ++cl)
            {
               nodes_type::iterator ni = nodes.find(cl->pakbus_address);
               if(ni == nodes.end())
                  add_node(
                     cl->pakbus_address,
                     false,
                     0,
                     cl->network_map_name.c_str());
               else
               {
                  device_handle device;
                  if(device_mapper->find_device(device,cl->network_map_name))
                     update_node(
                        cl->pakbus_address,
                        ni->second.is_router,
                        device->get_object_id()); 
               }
               used_addresses.insert(cl->pakbus_address);
            }

            // in the second pass, we need to remove any nodes whose address does not show up in the
            // used address set.  If a node is removed, any links associated with node will be
            // removed as well. 
            ni = nodes.begin();
            while(ni != nodes.end())
            {
               if(ni->first != pbrouter_address &&
                  used_addresses.find(ni->first) == used_addresses.end())
               {
                  nodes_type::iterator dni = ni++;
                  remove_node(dni);
               }
               else
                  ++ni;
            }

            // finally, we need to make sure that the appropriate static links are created for each
            // node.  we will make a pass accumulatinmg these "static" links to a container and will
            // then add them to the list
            links_type static_links;
            for(cl = nodes_list.begin(); cl != nodes_list.end(); ++cl) 
            {
               // we will ignore any node that does not appear in the network map
               if(cl->network_map_name.length() > 0)
               {
                  // if there is no link in the network that as currently known that allows the node
                  // to be reached, then a static link will be instantiated to reach it.
                  links_type::iterator li = std::find_if(
                     links.begin(),
                     links.end(),
                     link_has_address(cl->pakbus_address));
                  if(li == links.end())
                  {
                     // one end of the link will either be the server or the router
                     uint2 address1;
                     if(cl->router_address == cl->pakbus_address)
                        address1 = pbrouter_address;
                     else
                        address1 = cl->router_address;

                     // insert a new link as a static
                     link_type link = {
                        address1,
                        cl->pakbus_address,
                        cl->worst_case_response_time,
                        true 
                     };
                     static_links.push_back(link);
                  }
               }
            } 
            while(!static_links.empty())
            {
               link_type &link = static_links.front();
               nodes_type::iterator node1 = nodes.find(link.address1);
               nodes_type::iterator node2 = nodes.find(link.address2);
               
               if(state != state_before_active &&
                  node1 != nodes.end() && node2 != nodes.end())
               {
                  links.push_back(static_links.front());
                  event_link_added::cpost(
                     this,
                     client,
                     node1->second,
                     node2->second,
                     link.response_time_msec,
                     true);
               }
               static_links.pop_front();
            }
         }
         else
         {
            client_type::failure_type failure;
            switch(outcome)
            {
            case NodesListerClient::outcome_invalid_logon:
               failure = client_type::failure_invalid_logon;
               break;
               
            case NodesListerClient::outcome_invalid_router_id:
               failure = client_type::failure_invalid_router_id;
               break;
               
            case NodesListerClient::outcome_server_permission_denied:
               failure = client_type::failure_server_security_blocked;
               break;
               
            case NodesListerClient::outcome_unsupported:
               failure = client_type::failure_unsupported;
               break;
               
            default:
               failure = client_type::failure_unknown;
               break;
            }
            event_failure::cpost(this,client,failure);
         }
         nodes_lister.clear();
      } // on_complete

      
      void NetworkTracker::on_change_complete(
         LgrNet::DeviceMapper *mapper)
      {
         if(state == state_before_active)
         {
            // the next stage of initialisation is to get the first nodes list from the router.
            nodes_lister.bind(new NodesLister);
            nodes_lister->set_pakbus_router_name(get_pakbus_router_name());
            nodes_lister->start(this,this);
         }
      } // on_change_complete

      
      void NetworkTracker::on_device_added(
         LgrNet::DeviceMapper *mapper,
         device_handle &device)
      {
         if(state == state_active)
         {
            switch(device->get_type_code())
            {
            case DevCr10XPbId:
            case DevCr510PbId:
            case DevCr23XPbId:
            case DevCr20xId:
            case DevCr1000Id:
            case DevOtherPbRouterId:
               // restart the nodes lister to take into account the new device.
               nodes_lister.bind(new NodesLister);
               nodes_lister->set_pakbus_router_name(get_pakbus_router_name());
               nodes_lister->start(this,this);
               break;
            }
         }
      } // on_device_added

      
      void NetworkTracker::on_device_deleted(
         LgrNet::DeviceMapper *mapper,
         device_handle &device)
      {
         if(state == state_active)
         {
            // if the deleted device matches one of our nodes, we will delete it and then restart
            // the node lister if needed
            for(nodes_type::iterator ni = nodes.begin();
                ni != nodes.end();
                ++ni)
            {
               if(ni->second.device_info == device)
               {
                  remove_node(ni);
                  nodes_lister.bind(new NodesLister);
                  nodes_lister->set_pakbus_router_name(get_pakbus_router_name());
                  nodes_lister->start(this,this);
                  break;
               }
            }
         }
      } // on_device_deleted

      
      void NetworkTracker::on_device_renamed(
         LgrNet::DeviceMapper *mapper,
         device_handle &device,
         StrUni const &old_name)
      {
         if(state == state_active)
         {
            settings.erase(device->get_object_id());
            for(nodes_type::iterator ni = nodes.begin();
                ni != nodes.end();
                ++ni)
            {
               if(ni->second.device_info == device)
               {
                  settings_type::iterator si = settings.insert(
                     settings_type::value_type(
                        device->get_object_id(),
                        new Device::SettingsEnumerator)).first;
                  si->second->set_device_name(device->get_name());
                  si->second->start(this,this);
                  event_node_changed::cpost(this,client,ni->second); 
                  break;
               }
            }
         }
      } // on_device_renamed

      
      void NetworkTracker::on_device_parent_change(
         LgrNet::DeviceMapper *mapper,
         device_handle &device,
         uint4 old_parent_id)
      {
         if(state == state_active)
         {
            // if the change affects one of our nodes, we will delete it and then let the new nodes
            // list re-create it with the appropriate links.
            for(nodes_type::iterator ni = nodes.begin();
                ni != nodes.end();
                ++ni)
            {
               if(ni->second.device_info == device)
               {
                  // we won't remove the node if there is a dynamic link given to it.
                  links_type::iterator li = std::find_if(
                     links.begin(),
                     links.end(),
                     link_has_address(ni->first));
                  if(li->is_static)
                  {
                     remove_node(ni);
                     nodes_lister.bind(new NodesLister);
                     nodes_lister->set_pakbus_router_name(get_pakbus_router_name());
                     nodes_lister->start(this,this);
                     break;
                  }
               }
            }
         }
      } // on_device_parent_change

      
      void NetworkTracker::on_failure(
         LgrNet::DeviceMapper *mapper,
         DeviceMapperClient::failure_type failure_)
      {
         client_type::failure_type failure;
         switch(failure_)
         {
         case DeviceMapperClient::failure_invalid_logon:
            failure = client_type::failure_invalid_logon;
            break;
            
         case DeviceMapperClient::failure_session_broken:
            failure = client_type::failure_connection_failed;
            break;
            
         case DeviceMapperClient::failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         case DeviceMapperClient::failure_server_security:
            failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            failure = client_type::failure_unknown;
            break;
         }
         event_failure::cpost(this,client,failure);
      } // on_failure


      void NetworkTracker::on_failure(
         Device::SettingsEnumerator *enumerator,
         SettingsEnumeratorClient::failure_type failure_)
      {
         // we need to remove the setting enumerator from the list
         settings_type::iterator si = settings.begin();
         while(si != settings.end())
         {
            if(si->second == enumerator)
            {
               settings.erase(si);
               break;
            }
            else
               ++si;
         }

         // now we need to determine whether this represents a catastrophic failure
         if(failure_ != SettingsEnumeratorClient::failure_connection_failed &&
            failure_ != SettingsEnumeratorClient::failure_device_name_invalid)
         {
            client_type::failure_type failure;
            switch(failure_)
            {
            case SettingsEnumeratorClient::failure_invalid_logon:
               failure = client_type::failure_invalid_logon;
               break;
               
            case SettingsEnumeratorClient::failure_server_security_blocked:
               failure = client_type::failure_server_security_blocked;
               break;
               
            default:
               failure = client_type::failure_unknown;
               break; 
            }
            event_failure::cpost(this,client,failure);
         }
      } // on_failure


      void NetworkTracker::on_setting_changed(
         Device::SettingsEnumerator *enumerator,
         Csi::SharedPtr<Setting> &setting)
      {
         if(setting->get_identifier() == Device::Settings::pakbus_node_identifier)
         {
            nodes_lister.bind(new NodesLister);
            nodes_lister->set_pakbus_router_name(get_pakbus_router_name());
            nodes_lister->start(this,this);
         }
      } // on_setting_changed


      void NetworkTracker::onOneShotFired(uint4 event_id)
      {
         if(event_id == refresh_timer)
         {
            refresh_timer = one_shot->arm(this,3000);
            nodes_lister.bind(new NodesLister);
            nodes_lister->set_pakbus_router_name(get_pakbus_router_name());
            nodes_lister->start(this,this);
         }
      } // onOneShotFired
      
      
      NetworkTracker::nodes_type::iterator NetworkTracker::add_node(
         uint2 pakbus_address,
         bool is_router,
         uint4 object_id,
         wchar_t const *device_name)
      {
         nodes_type::iterator rtn = nodes.insert(
            nodes_type::value_type(
               pakbus_address,
               node_type(
                  pakbus_address,
                  is_router,
                  pakbus_address == pbrouter_address))).first;
         if(object_id != 0)
            device_mapper->find_device(
               rtn->second.device_info,object_id);
         else if(device_name != 0)
            device_mapper->find_device(
               rtn->second.device_info,device_name);

         // if the device information is known, we need to monitor that device's settings so that we
         // can track if the identity of the device changes.
         if(rtn->second.device_info != 0)
         {
            if(settings.find(rtn->second.device_info->get_object_id()) == settings.end())
            {
               settings_type::iterator si = settings.insert(
                  settings_type::value_type(
                     rtn->second.device_info->get_object_id(),
                     new Device::SettingsEnumerator)).first;
               si->second->set_setting_factory(new setting_factory_type);
               si->second->set_device_name(rtn->second.device_info->get_name());
               si->second->start(this,this);
            }
         }

         // finally, we need to notify the client that a node has been added.
         if(state != state_before_active)
            event_node_added::cpost(this,client,rtn->second);
         return rtn; 
      } // add_node


      void NetworkTracker::remove_node(nodes_type::iterator dni)
      {
         // we will ignore the removal if the node address refers to the router address
         if(dni->first == pbrouter_address)
            return;
         
         // we need to remove any links that involve the specified node iterator
         links_type::iterator li = links.begin();
         while(li != links.end())
         {
            if(li->address1 == dni->first || li->address2 == dni->first)
            {
               links_type::iterator dli = li++;
               if(dli->address1 == dni->first)
               {
                  nodes_type::iterator other = nodes.find(dli->address2);
                  if(other != nodes.end())
                     event_link_deleted::cpost(
                        this,client,dni->second,other->second);
               }
               else
               {
                  nodes_type::iterator other = nodes.find(dli->address1);
                  if(other != nodes.end())
                     event_link_deleted::cpost(
                        this,client,dni->second,other->second);
               }
               links.erase(dli);
            }
            else
               ++li;
         }

         // we need to cancel the settings enumerators associated with the node
         if(dni->second.device_info != 0)
         {
            bool references_object_id = false;
            for(nodes_type::iterator ni = nodes.begin();
                !references_object_id && ni != nodes.end();
                ++ni)
            {
               if(ni != dni && ni->second.device_info == dni->second.device_info)
                  references_object_id = true;
            }
            if(!references_object_id)
               settings.erase(dni->second.device_info->get_object_id()); 
         }
         
         // finally, we need to remove the specified node
         event_node_deleted::cpost(this,client,dni->second);
         nodes.erase(dni);
      } // remove_node


      NetworkTracker::nodes_type::iterator NetworkTracker::update_node(
         uint2 pakbus_address,
         bool is_router,
         uint4 object_id)
      {
         nodes_type::iterator ni = nodes.find(pakbus_address);
         if(ni == nodes.end())
            ni = add_node(pakbus_address,is_router,object_id);
         else
         {
            if(ni->second.is_router != is_router ||
               ni->second.device_info == 0 ||
               ni->second.device_info->get_object_id() != object_id)
            {
               device_handle device;
               device_mapper->find_device(device,object_id);
               if(device != ni->second.device_info ||
                  ni->second.is_router != is_router)
               {
                  ni->second.is_router = is_router;
                  ni->second.device_info = device;
                  if(state != state_before_active)
                     event_node_changed::cpost(this,client,ni->second);
               }
            }
         }
         return ni;
      } // update_node
   };
};
