/* Csi.PakBus.Router.cpp

   Copyright (C) 2001, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 01 March 2001
   Last Change: Friday 22 June 2018
   Last Commit: $Date: 2018-06-22 17:05:06 -0600 (Fri, 22 Jun 2018) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.Router.h"
#include "Csi.PakBus.PortBase.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.PakBus.PakBusTran.h"
#include "Csi.PakBus.RouterHelpers.HelloTran.h"
#include "Csi.PakBus.RouterHelpers.GetNeighboursTran.h"
#include "Csi.PakBus.RouterHelpers.SendNeighboursTran.h"
#include "Csi.PakBus.RouterHelpers.TerminateTran.h"
#include "Csi.PakBus.TranEcho.h"
#include "Csi.ByteOrder.h"
#include "Csi.MaxMin.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include "truediv.h"
#include <algorithm>
#include <stdexcept>
#include <set>
#include <deque>
#include <assert.h>


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class Router definitions
      ////////////////////////////////////////////////////////////
      byte const Router::max_hop_count = 15;
      uint2 const Router::broadcast_address = 0x0fff;
      uint4 const Router::maintenance_interval = 1000;
      
      
      Router::Router(timer_handle timer_):
         this_node_address(0),
         last_transaction_id(0),
         timer(timer_),
         neighbour_list_version(0xff),
         is_shutting_down(false),
         is_leaf_node(false),
         prevent_next_focus(false),
         time_since_last_report(0)
      {
         last_transaction_id = static_cast<byte>(rand()%0xff);
         maintenance_id = timer->arm(this,maintenance_interval);
      } // constructor


      Router::~Router()
      { shut_down(); }


      void Router::shut_down()
      {
         // if there are transactions pending, these need to be alerted that the router is now
         // invalid.
         if(!is_shutting_down)
         {
            is_shutting_down = true;
            for(transactions_type::iterator ti = transactions.begin();
                ti != transactions.end();
                ++ti)
            {
               if(ti->second->get_report_id() >= 0)
                  remove_report(ti->second->get_report_id());
               ti->second->on_router_close(); 
            }
            transactions.clear();
            if(maintenance_id)
               timer->disarm(maintenance_id);
         }
      } // shut_down


      void Router::register_port(PortBase *port)
      {
         ports_type::iterator pi = std::find(ports.begin(),ports.end(),port);
         if(pi == ports.end())
            ports.push_back(port); 
      } // register_port


      void Router::unregister_port(PortBase *port)
      {
         // make sure that the port is registered
         ports_type::iterator pi = std::find(ports.begin(),ports.end(),port);
         if(pi != ports.end())
         {
            // remove any static routes associated with the port
            routes_type::iterator ri = static_routes.begin();
            while(ri != static_routes.end())
            {
               if(ri->second.port == port)
               {
                  routes_type::iterator dri = ri++;
                  static_routes.erase(dri);
               }
               else
                  ++ri;
            }

            // remove any neighbours associated with the port
            neighbours_type::iterator ni = neighbours.begin();
            while(ni != neighbours.end())
            {
               if(ni->second->port == port)
               {
                  // the link (if any) must be removed for this neighbour
                  neighbours_type::iterator rni = ni++;
                  uint2 neighbour_address = rni->first;
                  on_neighbour_change(rni->first, link_change_deleted);
                  rni = neighbours.find(neighbour_address);
                  if(rni != neighbours.end())
                     neighbours.erase(rni);
               }
               else
                  ++ni;
            }

            // finally remove the port reference
            ports.erase(pi);

            // we now need to regenerate the routes list to trim off links and routers that are no
            // longer reachable
            generate_routes_from_links();
         }
      } // unregister_port


      uint2 Router::get_this_node_address() const
      { return this_node_address; }


      void Router::set_this_node_address(uint2 this_node_address_)
      {
         // ignore the change if the new address is the same
         if(this_node_address_ != this_node_address)
         {
            if(this_node_address != 0)
            {
               // we need to broadcast a goodbye command on all ports that are in an on-line state
               // using the old address.
               pakctrl_message_handle goodbye_cmd(new PakCtrlMessage);
               goodbye_cmd->set_message_type(PakCtrl::Messages::goodbye_cmd);
               goodbye_cmd->set_priority(Priorities::extra_high);
               goodbye_cmd->set_source(this_node_address);
               goodbye_cmd->set_destination(broadcast_address);
               for(ports_type::iterator pi = ports.begin();
                   pi != ports.end();
                   ++pi)
               {
                  PortBase *port = *pi;
                  if(port->link_is_active())
                     port->broadcast_message(goodbye_cmd.get_handle());
               }
            }
            this_node_address = this_node_address_;

            // we will treat this event the same as a destructive reset.
            std::list<neighbour_handle> temp;
            for(neighbours_type::iterator ni = neighbours.begin(); ni != neighbours.end(); ++ni)
               temp.push_back(ni->second);
            while(!temp.empty())
            {
               neighbour_handle neighbour(temp.front());
               on_neighbour_lost(neighbour->port, neighbour->physical_address);
               temp.pop_front();
            }
            if(current_transaction != 0)
            {
               current_transaction->on_failure(
                  PakCtrl::DeliveryFailure::unreachable_destination);
               current_transaction.clear();
            }
            if(current_transaction_focus != 0)
            {
               current_transaction_focus->on_failure(
                  PakCtrl::DeliveryFailure::unreachable_destination);
               if(current_transaction_focus->get_report_id() != -1)
                  remove_report(current_transaction_focus->get_report_id());
               current_transaction_focus.clear();
            }
            waiting_transactions.clear();
            while(!transactions.empty())
            {
               transaction_handle tran(transactions.begin()->second);
               if(tran->get_report_id() != -1)
                  remove_report(tran->get_report_id());
               transactions.erase(transactions.begin());
               tran->on_failure(PakCtrl::DeliveryFailure::unreachable_destination);
            }
            ++neighbour_list_version;
            if(this_node_address != 0)
               update_routers(this_node_address,link_change_added);
         }
      } // set_this_node_address


      void Router::on_port_message(
         PortBase *port,
         message_handle &message)
      {
         // any incoming message represents potentially new routing information
         if(is_shutting_down)
            return;
         message->set_port(port);

         // is this message destined for this node?
         if(message->get_destination() == this_node_address ||
            message->get_destination() == broadcast_address)
         {
            if(!on_message(message))
               on_unhandled_message(message);
         }
         else
            forward_message(message); 
      } // on_port_message


      bool Router::send_message_from_app(message_handle &message)
      {
         if(is_shutting_down)
            return false;
         
         // the expect more flag on this message will depend upon the state of all of the
         // transactions for the message destination.  The main goal in changing it here is to not
         // stomp on any local transactions when we are sending a response.
         transactions_type::iterator ti = transactions.begin();
         message->set_expect_more(ExpectMoreCodes::last);
         while(message->get_destination() != broadcast_address &&
               ti != transactions.end() &&
               message->get_expect_more() == ExpectMoreCodes::last)
         {
            if(ti->second->get_destination_address() == message->get_destination() &&
               !ti->second->will_terminate())
               message->set_expect_more(ExpectMoreCodes::expect_more);
            else
               ++ti;
         }

         // set the source for the message and route it.
         message->set_source(this_node_address); 
         return route_message(message);
      } // send_message_from_app


      bool Router::cancel_message_from_app(message_handle &message)
      {
         // search through the queue of unrouted messages to find the specified message.
         bool rtn = false;
         if(is_shutting_down)
            return false;
         for(unrouted_messages_type::iterator umi = unrouted_messages.begin();
             !rtn && umi != unrouted_messages.end();
             ++umi)
         {
            if(*umi == message)
            {
               // remove the message from the queue
               rtn = true;
               unrouted_messages.erase(umi);
               
               // we now need to search out the port that would have routed this message so that it
               // can be told that it may need to abort dialing.
               RouterHelpers::route_type *route = find_route(message->get_destination());
               if(route)
                  route->port->on_message_aborted(route->neighbour_id);
               break;
            }
         } 
         return rtn;
      } // cancel_message_from_app


      void Router::send_delivery_fault_message(
         message_handle &message,
         PakCtrl::DeliveryFailure::failure_type error_code)
      {
         if(is_shutting_down)
            return;
         
         // We should not send a fault message if the hop count is exceeded because an avalanche
         // could occur
         bool should_send_fault = true;

         if(message->get_hop_count() >= max_hop_count)
            should_send_fault = false;

         // we also cannot send a fault message if the message type is fault
         byte message_type = 0;
         if(message->whatsLeft() > 0)
            message_type = message->readByte();
         message->reset();
         if(message->get_high_protocol() == ProtocolTypes::control &&
            message_type == PakCtrl::Messages::delivery_failure)
            should_send_fault = false;

         // it makes no sense to send a fault for a message that is marked as a response.
         if((message->get_high_protocol() == ProtocolTypes::control ||
             message->get_high_protocol() == ProtocolTypes::bmp) &&
            (message_type & 0x80) != 0)
            should_send_fault = false;
         
         // now continue if the previous tests succeeded
         if(should_send_fault)
         {
            // form the rejection message
            pakctrl_message_handle reject(new PakCtrlMessage);
            uint4 body_len = Csi::csimin<uint4>(message->length(),16);
            uint2 temp;
            
            reject->set_destination(message->get_source());
            reject->set_message_type(PakCtrl::Messages::delivery_failure);
            reject->set_priority(Priorities::extra_high);
            reject->addByte(error_code);
            temp = static_cast<uint2>(message->get_high_protocol()) << 12;
            temp |= message->get_destination() & 0xfff;
            reject->addUInt2(temp,!is_big_endian());
            temp = static_cast<uint2>(message->get_hop_count()) << 12;
            temp |= message->get_source() & 0xfff;
            reject->addUInt2(temp,!is_big_endian()); 
            reject->addBytes(message->getMsg(),body_len,false);
            send_message_from_app(reject.get_handle());
         }
      } // send_delivery_fault_message


      bool Router::get_next_port_message(
         PortBase *port,
         uint2 physical_destination,
         message_handle &message)
      {
         bool rtn = false;
         if(is_shutting_down)
            return rtn;
         for(unrouted_messages_type::iterator mi = unrouted_messages.begin();
             !rtn && mi != unrouted_messages.end();
             ++mi)
         {
            // Will this message route to the specified port?
            message_handle &this_message = *mi;
            routes_type::iterator ri = routes.find(this_message->get_destination());
            PortBase *route_port = 0;
            uint2 route_physical_destination = 0;

            if(this_message->get_use_own_route() && this_message->get_port() == port)
            {
               route_port = this_message->get_port();
               route_physical_destination = this_message->get_physical_destination();
            }
            else if(ri != routes.end() && ri->second.port == port)
            {
               route_port = ri->second.port;
               route_physical_destination = ri->second.neighbour_id;
            }
            else if(ri == routes.end())
            {
               ri = static_routes.find(this_message->get_destination());
               if(ri != static_routes.end() && ri->second.port == port)
               {
                  route_port = ri->second.port;
                  route_physical_destination = ri->second.neighbour_id;
               }
            } 

            if(route_physical_destination == physical_destination && route_port == port)
            {
               // increment the message's hop count (unless it is zero (originating here))
               byte hop_count = this_message->get_hop_count();
               if(hop_count != 0 && hop_count + 1 < max_hop_count)
                  this_message->set_hop_count(hop_count + 1);

               // get ready to send the message
               message = this_message;
               message->set_physical_source(this_node_address);
               message->set_physical_destination(route_physical_destination);
               message->set_port(route_port);
               rtn = true;
               unrouted_messages.erase(mi);
               break;
            }
         }
         if(rtn)
         {
            on_sending_message(message);
            if(should_encrypt_message(message))
            {
               ciphers_type::iterator ci(ciphers.find(message->get_destination()));
               if(ci == ciphers.end())
                  ci = ciphers.find(broadcast_address);
               if(ci != ciphers.end())
               {
                  message_handle encrypted(new Message(*message, 0, true));
                  byte flags(message->get_high_protocol());
                  StrBin content;

                  flags = (flags << 4) + ci->second->get_cipher_code();
                  encrypted->set_headerLen(0);
                  encrypted->clear();
                  encrypted->set_high_protocol(ProtocolTypes::encrypted);
                  encrypted->addByte(flags);
                  encrypted->addUInt2(
                     static_cast<uint2>(message->length()), !is_big_endian());
                  encrypted->addUInt2(
                     calcSigFor(message->getMsg(), message->length()),
                     !is_big_endian());
                  ci->second->set_initialisation_vector(encrypted->getMsg(), 5);
                  ci->second->encrypt(content, message->getMsg(), message->length());
                  encrypted->addBytes(content.getContents(), (uint4)content.length());
                  message = encrypted;
               }
            }
         }
         else
            message.clear();
         return rtn;
      } // get_next_port_message


      bool Router::is_route_reachable(uint2 node_address) const
      {
         if(is_shutting_down)
            return false; 
         bool rtn = node_address == this_node_address ||
            node_address == broadcast_address;
         if(!rtn)
         {
            rtn = routes.find(node_address) != routes.end();
            if(!rtn)
               rtn = static_routes.find(node_address) != static_routes.end();
         }
         return rtn;
      } // is_route_reachable


      void Router::on_port_delivery_failure(
         PortBase *port,
         uint2 physical_destination)
      {
         if(is_shutting_down)
            return;
         log_debug("Csi::PakBus::Router::on_port_delivery_failure","entering method");
         
         // we need to reject any messages that must be delivered through this port. In order to do
         // so, we will first iterate through the list of unrouted messages and copy any that would
         // be routed through the specified port. In the process of copying, we will remove them
         // from the unrouted messages list.
         typedef std::list<message_handle> delete_list_type;
         delete_list_type delete_list;
         unrouted_messages_type::iterator umi = unrouted_messages.begin();

         while(umi != unrouted_messages.end())
         {
            // search for the route associated with this message
            //
            // if the route can be found, we need to see if its port is the same as the failing
            // one. If it is, then the message should be deleted.
            message_handle message(*umi);
            bool reject_this_message = false;

            if(message->get_use_own_route() &&
               message->get_port() == port && 
               message->get_physical_destination() == physical_destination)
               reject_this_message = true;
            else
            {
               routes_type::iterator ri = routes.find(
                  message->get_destination());
               if(ri == routes.end())
               {
                  ri = static_routes.find(message->get_destination());
                  if(ri == static_routes.end())
                     reject_this_message = true;
               }
               if(!reject_this_message &&
                  ri->second.port == port && 
                  (physical_destination == 0 || physical_destination == ri->second.neighbour_id))
                  reject_this_message = true;
            }

            if(reject_this_message)
            {
               unrouted_messages_type::iterator dmi = umi++;
               delete_list.push_back(message);
               unrouted_messages.erase(dmi);
            }
            else
               ++umi;
         }

         // if a specific address has failed, we will deal with it by purging that neighbour record
         if(physical_destination != 0)
            on_neighbour_lost(port, physical_destination);
         // otherwise, all neighbours associated with the specified port will have to be purged
         else
         {
            std::list<uint2> neighbours_lost;
            for(neighbours_type::iterator ni = neighbours.begin();
                ni != neighbours.end();
                ++ni)
            {
               if(ni->second->port == port)
                  neighbours_lost.push_back(ni->first);
            }
            while(!neighbours_lost.empty())
            {
               on_neighbour_lost(port,neighbours_lost.front());
               neighbours_lost.pop_front();
            }
         }

         // all that remains now is to send delivery fault notifications for any messages that are
         // in the delete list.
         while(!delete_list.empty())
         {
            send_delivery_fault_message(
               delete_list.front(),
               PakCtrl::DeliveryFailure::timed_out_or_resource_error);
            delete_list.pop_front();
         }

         // any pending transactions that would use the physical address should be reported as a
         // failure as well.
         std::list<transaction_handle> transactions(
            waiting_transactions.begin(),
            waiting_transactions.end());
         if(current_transaction_focus != 0)
            transactions.push_front(current_transaction_focus);
         prevent_next_focus = true;
         while(!transactions.empty())
         {
            RouterHelpers::route_type *route;
            bool tran_failed = false;
            
            transaction_handle tran(transactions.front());
            transactions.pop_front();
            route = find_route(tran->get_destination_address());
            if(route)
            {
               if(route->port == port &&
                  (physical_destination == 0 || route->neighbour_id == physical_destination))
                  tran_failed = true;
            }
            else
               tran_failed = true;
            if(tran_failed)
            {
               if(PakBusTran::is_valid_instance(tran.get_rep()))
                  tran->on_failure(PakCtrl::DeliveryFailure::unreachable_destination);
               else
                  assert(false);
               if(current_transaction_focus == tran)
                  current_transaction_focus.clear();
            }
         }
         prevent_next_focus = false;
         set_next_transaction_focus();
         log_debug("Csi::PakBus::Router::on_port_delivery_failure","leaving method");
      } // on_port_delivery_failure


      void Router::add_static_route(
         uint2 destination,
         PortBase *port,
         uint2 physical_destination,
         byte hop_count)
      {
         static_routes.insert(
            routes_type::value_type(
               destination,
               RouterHelpers::route_type(
                  destination,
                  physical_destination,
                  port,
                  port->get_worst_case_response())));
      } // add_static_route


      void Router::remove_static_route(uint2 destination)
      {
         // delete any static route
         routes_type::iterator ri = static_routes.find(destination);
         if(ri != static_routes.end())
            static_routes.erase(ri);
      } // remove_static_route


      uint4 Router::get_route_response_time(uint2 destination)
      {
         if(is_shutting_down)
            return 0;
         
         // search for the specified route
         uint4 rtn = 0;
         routes_type::iterator ri = routes.find(destination);
         if(ri != routes.end())
            rtn = ri->second.resp_time_msec;
         else
         {
            ri = static_routes.find(destination);
            if(ri != static_routes.end())
               rtn = ri->second.resp_time_msec;
            else
            {
               neighbours_type::iterator ni = neighbours.find(destination);
               if(ni != neighbours.end() && PortBase::is_valid_instance(ni->second->port))
                  rtn = ni->second->port->get_worst_case_response();
            }
         }
         return rtn;
      } // get_route_response_time


      byte Router::open_transaction(transaction_handle transaction)
      {
         if(is_shutting_down)
            return 0;
         using namespace RouterHelpers;
         byte rtn = new_transaction_id(transaction->get_destination_address());
         transactions[transaction_id(transaction->get_destination_address(),rtn)] = transaction;
         transaction->on_new_transaction_id(rtn);
         transaction->start();
         return rtn;
      } // open_transaction


      byte Router::reassign_transaction_id(
         uint2 destination,
         byte old_transaction_id)
      {
         if(is_shutting_down)
            return 0;
         
         // search for the transaction object
         using namespace RouterHelpers;
         transactions_type::iterator ti = transactions.find(
            transaction_id(destination,old_transaction_id));
         if(ti == transactions.end())
            throw MsgExcept("Csi::PakBus::Router::reassign_transaction_id -- invalid id");

         // now get the object and remove it from the list
         transaction_handle transaction(ti->second);
         transactions.erase(ti);

         // allocate a new transaction id and assign it to the transaction
         byte rtn = new_transaction_id(destination);
         transaction->on_new_transaction_id(rtn);
         transactions[transaction_id(destination,rtn)] = transaction;
         return rtn;
      } // reassign_transaction_id


      void Router::close_transaction(
         uint2 destination,
         byte transaction_no)
      {
         // We will make sure that the focus for the specified transaction is released
         log_debug("Csi::PakBus::Router","entering close_transaction");
         if(is_shutting_down)
            return;
         release_transaction_focus(destination,transaction_no);
         
         // search for the specified transaction
         using namespace RouterHelpers;
         transactions_type::iterator ti = transactions.find(
            transaction_id(destination,transaction_no));
         if(ti != transactions.end())
         {
            // save off the transaction object into a local handle and then remove the transaction
            // from the list
            transaction_handle transaction(ti->second);
            transactions.erase(ti);
            transaction->on_close();
            if(transaction->get_report_id() != -1)
               remove_report(transaction->get_report_id());
            on_transaction_closed(transaction);

            // this transaction might be the current transaction used by this router
            if(transaction == current_transaction)
            {
               current_transaction.clear();
               do_next_router_transaction();
            }
            else if(current_transaction != 0 &&
                    current_transaction->is_special_hello(transaction.get_rep()))
            {
               close_transaction(
                  current_transaction->get_destination_address(),
                  current_transaction->get_transaction_id());
            }
            
            // if the port for the router associated with this transaction has an open session for
            // the pair represented by the this node id and the transaction destination id, then we
            // need to check to see if the session should be closed.
            RouterHelpers::route_type *route = find_route(
               transaction->get_destination_address());
            if(route && !transaction->will_terminate())
            {
               bool should_send_end = route->port->has_session(
                  this_node_address,
                  transaction->get_destination_address());
               for(ti = transactions.begin();
                   ti != transactions.end() && should_send_end;
                   ++ti)
               {
                  // if another transaction has the same destination, then we shouldn't send the end
                  // of link message.
                  if(ti->second->get_destination_address() == transaction->get_destination_address())
                     should_send_end = false;
               }
               
               // send the end session message if needed
               if(should_send_end)
               {
                  open_transaction(
                     new RouterHelpers::TerminateTran(
                        this,
                        timer,
                        transaction->get_destination_address()));
               } 
            }
         }
         log_debug("Csi::PakBus::Router","leaving close_transaction");
      } // close_transaction


      void Router::close_all_transactions_for_node(uint2 destination)
      {
         // the first step is to release the current transaction focus if it applies to the
         // specified address
         OStrAscStream msg;
         if(current_transaction_focus != 0 &&
            current_transaction_focus->get_destination_address() == destination)
         {
            current_transaction_focus->get_transaction_description(msg);
            msg << "\",\"" << static_cast<uint2>(current_transaction_focus->get_transaction_id());
            log_debug("Release Transaction Focus", msg.str().c_str());
            current_transaction_focus.clear();
         }

         // we also need to clear the current "router sponsored" transaction if it applies to the
         // node.
         if(current_transaction != 0 &&
            current_transaction->get_destination_address() == destination)
         {
            msg.str("");
            current_transaction->get_transaction_description(msg);
            msg << "\",\"" << static_cast<uint2>(current_transaction->get_transaction_id());
            log_debug("Release Transaction Focus", msg.str().c_str());
            current_transaction.clear();
         }

         // we need to remove any waiting transactions that might match
         waiting_transactions_type::iterator wti = waiting_transactions.begin();
         while(wti != waiting_transactions.end())
         {
            transaction_handle &tran = *wti;
            if(tran->get_destination_address() == destination)
            {
               waiting_transactions_type::iterator dwti = wti++;
               msg.str("");
               tran->get_transaction_description(msg);
               msg << "\",\"" << static_cast<uint2>(tran->get_transaction_id());
               log_debug("Release Transaction Focus Request", msg.str().c_str());
               waiting_transactions.erase(dwti);
            }
            else
               ++wti;
         }

         // finally, we need to remove any matching transactions
         std::list<RouterHelpers::transaction_id> remove_keys;
         for(transactions_type::iterator ti = transactions.begin(); ti != transactions.end(); ++ti)
         {
            transaction_handle &tran = ti->second;
            if(tran->get_destination_address() == destination)
               remove_keys.push_back(ti->first);
         }
         while(!remove_keys.empty())
         {
            RouterHelpers::transaction_id key(remove_keys.front());
            remove_keys.pop_front();
            transaction_handle tran(transactions[key]);
            msg.str("");
            tran->get_transaction_description(msg);
            msg << "\",\"" << static_cast<uint2>(tran->get_transaction_id());
            log_debug("Close Transaction", msg.str().c_str());
            if(tran->get_report_id() != -1)
               remove_report(tran->get_report_id());
            transactions.erase(key);
         }
         do_next_router_transaction();
         set_next_transaction_focus();
      } // close_all_transactions_for_node
      

      void Router::request_transaction_focus(
         uint2 destination,
         byte transaction_id)
      {
         // we need to find the transaction that matches the specified identification criteria.
         if(is_shutting_down)
            return;
         transactions_type::iterator ti = transactions.find(
            RouterHelpers::transaction_id(destination,transaction_id));
         if(ti != transactions.end())
         {
            // put the transaction in the queue
            OStrAscStream msg;
            ti->second->get_transaction_description(msg);
            msg << "\",\"" << (uint2)ti->second->get_transaction_id() << "\",\""
                << ti->second->get_destination_address();
            log_debug("Request Transaction Focus",msg.str().c_str());
            waiting_transactions.push(ti->second);

            // if the port is dialed, we may need to issue a hello message before the transaction
            // requesting focus.
            RouterHelpers::route_type *route = find_route(ti->second->get_destination_address());

            if(route &&
               route->port->should_cap_timeout() && 
               !route->port->link_is_active())
            {
               // all neighbours accessed through this port need to be marked as needing
               // verification.  We will set the delay timer to a liberal amount so that the
               // verification won't start immediately.
               for(neighbours_type::iterator ni = neighbours.begin();
                   ni != neighbours.end();
                   ++ni)
               {
                  neighbour_handle &neighbour = ni->second;
                  if(neighbour->port == route->port && neighbour->hello_tries == 0)
                  {
                     neighbour->hello_tries = 1;
                     neighbour->send_hello_delay_base = counter(0);
                     neighbour->send_hello_delay = 60000;
                  }
               }
            }
            
            // make a note of the current transaction, if any
            if(current_transaction_focus != 0)
            {
               msg.str("");
               current_transaction_focus->get_transaction_description(msg);
               msg << "\",\"" << (uint2)current_transaction_focus->get_transaction_id()
                   << "\",\"" << current_transaction_focus->get_destination_address();
               log_debug("Current Focus", msg.str().c_str());
            }
         }
         set_next_transaction_focus();
      } // request_transaction_focus


      void Router::release_transaction_focus(
         uint2 destination,
         byte transaction_id)
      {
         if(is_shutting_down)
            return;
         if(current_transaction_focus != 0 &&
            current_transaction_focus->is_same(destination,transaction_id))
         {
            // release the focus
            OStrAscStream msg;
            current_transaction_focus->get_transaction_description(msg);
            msg << "\",\""
                << static_cast<uint2>(current_transaction_focus->get_transaction_id());
            log_debug("Release Transaction Focus",msg.str().c_str());
            current_transaction_focus.clear();

            // clear the current router transaction and optionally start the next one
            if(current_transaction != 0 &&
               current_transaction->is_same(destination,transaction_id))
            {
               current_transaction.clear();
               do_next_router_transaction();
            }
         }
         else
         {
            // we need to search through the queue of waiting transactions to find the specified
            // one.
            waiting_transactions_type::iterator wti = waiting_transactions.begin();
            while(wti != waiting_transactions.end())
            {
               transaction_handle &tran = *wti;
               if(tran->is_same(destination,transaction_id))
               {
                  waiting_transactions.erase(wti);
                  break;
               }
               else
                  ++wti;
            }
         }
         set_next_transaction_focus();
      } // release_transaction_focus


      void Router::on_application_transaction_finished(
         uint2 destination,
         byte transaction_no)
      {
         using namespace RouterHelpers;
         if(is_shutting_down)
            return;
         transactions_type::iterator ti = transactions.find(
            transaction_id(destination,transaction_no));
         if(ti != transactions.end())
            ti->second->on_application_finished();
      } // on_application_transaction_finished
         

      bool Router::find_transaction(
         transaction_handle &dest,
         uint2 address,
         byte transaction_no)
      {
         using namespace RouterHelpers;
         bool rtn = false;
         transactions_type::iterator ti = transactions.find(
            transaction_id(address,transaction_no));
         if(ti != transactions.end() && !ti->second->is_closing())
         {
            dest = ti->second;
            rtn = true;
         }
         if(!rtn)
            dest.clear();
         return rtn;
      } // find_transaction

      
      void Router::on_beacon(
         PortBase *port,
         uint2 physical_source,
         bool is_broadcast)
      {
         // search for the neighbour that matches the the source address
         if(is_shutting_down || !port->can_accept_neighbour(physical_source))
            return;
         neighbours_type::iterator ni = neighbours.find(physical_source);
         neighbour_handle neighbour;
         
         if(ni != neighbours.end())
         {
            // the neighbour should be on the same port as known previously.  If the port object is
            // different, this represents a new kind of link.  We will remove the old neighbour
            // record and allow the new obe to be added
            neighbour = ni->second;
            if(port != neighbour->port)
            {
               on_neighbour_lost(port,physical_source);
               neighbour.clear();
            }
         }

         // if the neighbour was found, we need to update its beacon timer and other fields
         if(neighbour != 0)
         {
            if((!is_broadcast || neighbour->broadcast_is_beacon()) && !neighbour->needs_hello_info)
            {
               neighbour->hello_tries = 0;
               neighbour->time_since_last_beacon = Csi::counter(0);
            }
            neighbour->send_hello_delay = 0;
         }
         else
         {
            // we will enter this neighbour into the table but will not update the links table until
            // the response to the hello command has been received.  The hello transaction will know
            // how to route the message by way of the information in the neighbour record.  
            neighbour.bind(
               new RouterHelpers::neighbour_type(
                  port,
                  port->get_hop_metric(),
                  physical_source));
            neighbour->hello_tries = 1;
            neighbours[physical_source] = neighbour;
            neighbour->needs_hello_info = true;

            // if this beacon was sent to a broadcast address, we need to set a random delay in the
            // port that should not be greater than the port's worst case response time.
            if(is_broadcast)
               neighbour->send_hello_delay = rand() % port->get_worst_case_response();
         }
      } // on_beacon


      void Router::on_neighbour_lost(
         PortBase *port,
         uint2 physical_address)
      {
         if(is_shutting_down)
            return;
         neighbours_type::iterator ni = neighbours.find(physical_address);
         if(ni != neighbours.end())
         {
            update_routers(physical_address,link_change_deleted);
            if(!ni->second->needs_hello_info)
               on_neighbour_change(physical_address,link_change_deleted); 
            neighbours.erase(physical_address);
         }
      } // on_neighbour_lost


      void Router::on_neighbour_info(
         PortBase *port,
         uint2 physical_address,
         bool is_router,
         HopMetric hop_metric,
         uint2 beacon_interval)
      {
         // this information may or may not represent any change to the neighbour list
         neighbours_type::iterator ni = neighbours.find(physical_address);
         link_change_type link_change = link_change_none;
         if(ni != neighbours.end())
         {
            // we need to determine if anything about the neighbour has changed
            neighbour_handle &neighbour = ni->second;
            HopMetric new_hop_metric = csimax(hop_metric,port->get_hop_metric());
            
            if(neighbour->is_router != is_router)
            {
               if(is_router)
                  update_routers(physical_address,link_change_added);
               else
               {
                  update_routers(physical_address,link_change_deleted);
                  generate_routes_from_links();
               }
               neighbour->is_router = is_router;
               link_change = link_change_changed; 
            }
            if(neighbour->hop_metric != new_hop_metric)
            {
               link_change = link_change_changed;
               neighbour->hop_metric = new_hop_metric;
            }
            neighbour->verification_interval = beacon_interval;
            if(neighbour->needs_hello_info)
            {
               link_change = link_change_added;
               neighbour->hello_tries = 0;
               neighbour->needs_hello_info = false;
            }
         }
         else
         {
            neighbour_handle neighbour(
               new RouterHelpers::neighbour_type(
                  port,
                  csimax(hop_metric,port->get_hop_metric()),
                  physical_address));
            neighbours[physical_address] = neighbour;
            neighbour->verification_interval = beacon_interval;
            link_change = link_change_added;
            neighbour->needs_hello_info = false;
         }

         // make sure that the router that uses this address is marked as validated
         all_routers_type::iterator ri = all_routers.find(physical_address);
         if(ri != all_routers.end())
            ri->second->validated = true;
         
         // if the neighbour list changed, we need to update the links table and make sure that the
         // neighbour list gets distributed.
         if(link_change != link_change_none)
         {
            on_neighbour_change(
               physical_address,
               link_change);
         }
      } // on_neighbour_info


      void Router::on_router_neighbour_list(
         uint2 router_id,
         pakctrl_message_handle &message,
         neighbour_list_update_type change_code)
      {
         // leaf nodes will not process neighbour lists from routers.
         if(is_leaf_node)
            return;
         
         // read the neighbour list version from the message
         byte router_neighbour_list_version = message->readByte();
         
         // check for a record to match this router id.  If the router record exists, we should
         // check its neighbour list version number. Otherwise, we need to create the router
         // record and add it to the list.
         all_routers_type::iterator ri = all_routers.find(router_id);
         router_handle router;
         
         if(ri != all_routers.end())
         {
            router = ri->second;
            if(router->neighbour_list_version != router_neighbour_list_version &&
               router->neighbour_list_version + 1 != router_neighbour_list_version)
               router->get_all = true;
            router->neighbour_list_version = router_neighbour_list_version;
            router->validated = true;
         }
         else
         {
            router.bind(
               new RouterHelpers::router_type(
                  router_id,
                  router_neighbour_list_version));
            all_routers[router_id] = router; 
         }
         
         // if the list is marked as complete, we should make note of any link currently
         // associated with the router's ID.  After processing, we can trim out any links that
         // did not previously appear
         typedef std::set<uint2> old_neighbours_type;
         old_neighbours_type old_neighbours;
         if(change_code == neighbour_list_update_complete)
         {
            // remove all of the links that are associated with this router
            for(links_type::iterator li = links.begin();
                li != links.end();
                ++li)
            {
               uint2 other_id = li->has_id(router_id);
               if(other_id)
                  old_neighbours.insert(other_id);
            }
            
            // since this is supposed to be a complete neighbour list, we can clear the router
            // record's get_all flag as well
            router->get_all = false;
         }
         
         // we now process the neighbour list in the message.
         while(message->whatsLeft() >= 2)
         {
            // read the node info and decode it.
            uint2 coded_neighbour = message->readUInt2(!is_big_endian());
            bool is_router = (coded_neighbour & 0x8000) ? true : false;
            HopMetric hop_metric =
               static_cast<byte>(
                  (coded_neighbour & 0x7000) >> 12);
            uint2 node_id = (coded_neighbour & 0x0fff);
            
            // if this router is claiming us as a neighbour but we have no substantiating neighbour
            // record, we need to ignore that link,  otherwise, the router will be discarded during
            // the regeneration process.
            if(node_id == this_node_address &&
               neighbours.find(router_id) == neighbours.end())
               continue;
            
            // we will remove this id from the old neighbours set
            old_neighbours.erase(node_id);
            
            // the processing done will be different depending on the change code
            if(change_code == neighbour_list_update_complete ||
               change_code == neighbour_list_update_add)
            {
               // check to see if the specified router is on the list
               all_routers_type::iterator ri = all_routers.find(node_id);
               if(is_router && ri == all_routers.end())
               {
                  all_routers[node_id] = router_handle(
                     new RouterHelpers::router_type(node_id,0));
               }

               // if the node was previously known as a router and is now being reported as a leaf
               // node, we need to remove it from the routers list.  When the routes are
               // regenerated, any links that are unreachable because of this change will be
               // trimmed. 
               if(!is_router && ri != all_routers.end())
                  all_routers.erase(ri);

               // if this is a complete update, we can add the link without searching
               if(change_code == neighbour_list_update_complete)
               {
                  // Some routers (the CR23x-PB) don't appear to report the correct hop metric for
                  // the link.  If this link is a neighbour link, it would be an error if the
                  // reported hop metric differed from our negotiated one.  Because of this, we will
                  // not allow a neighbour record from a router to change an already negotiated link
                  // hop metric.
                  if(router_id != this_node_address && node_id != this_node_address)
                     update_links(router_id, node_id, hop_metric, link_change_added, false);
               }
               else
               {
                  // an add can also signify a change so we need to search for an existing link
                  // record before adding
                  links_type::iterator li = links.begin();
                  while(li != links.end())
                  {
                     if(li->has_id(node_id) == router_id)
                     {
                        // Some routers (CR23X-PB) misbehave by not reporting the correct link
                        // metric for a neighbour.  We don't want that to effect the hop metric that
                        // was already determined by the hello sequence.
                        if(node_id != this_node_address && router_id != this_node_address)
                           update_links(router_id, node_id, hop_metric, link_change_changed, false);
                        break;
                     }
                     else
                        ++li;
                  }

                  // if the link was not found, we need to add it.
                  if(li == links.end())
                     update_links(router_id,node_id,hop_metric,link_change_added,false);
               }
            }
            else if(change_code == neighbour_list_update_remove)
            {
               // remove the link associated with this router and node
               update_links(node_id,router_id,hop_metric,link_change_deleted,false);
            }
         }

         // if there are any old neighbours left after we have processed the response, we need to
         // remove these links from the set
         bool router_is_neighbour = neighbours.find(router_id) != neighbours.end();
         while(!old_neighbours.empty())
         {
            // we need to remove the orphaned links
            uint2 node_id = *(old_neighbours.begin());
            old_neighbours.erase(old_neighbours.begin());
            if(node_id != this_node_address || !router_is_neighbour)
               update_links(router_id,node_id,0,link_change_deleted,false);
            else
            {
               OStrAscStream msg;
               msg << "Neighbour list from neighbour router, " << router_id << ", is incomplete";
               log_comms(comms_fault,msg.str().c_str());
            }
         }
         
         // we can now generate the new routes and check to see if there is more router processing
         // that needs to be done.
         generate_routes_from_links();
      } // on_router_neighbour_list


      void Router::send_reset_command(
         uint2 destination,
         bool destructive)
      {
         pakctrl_message_handle command(new pakctrl_message_type);
         command->set_message_type(PakCtrl::Messages::reset_router_cmd);
         command->set_destination(destination);
         command->set_expect_more(ExpectMoreCodes::last);
         command->set_source(this_node_address);
         command->addByte(destructive ? 0 : 1);
         route_message(command.get_handle());
      } // send_reset_command


      Router::priority_type Router::get_current_port_priority(PortBase *port)
      {
         // iterate through the transactions to see if there is one pending that might be associated
         // with this port.
         priority_type rtn = Priorities::low; 
         for(transactions_type::const_iterator ti = transactions.begin();
             ti != transactions.end();
             ++ti)
         {
            transaction_handle const &tran = ti->second;
            if(!tran->is_closing())
            {
               RouterHelpers::route_type *route = find_route(
                  tran->get_destination_address());
               if(route && route->port == port && tran->get_priority() > rtn)
                  rtn = tran->get_priority();
            }
         }

         // we might want to iterate through the set of pending messages as well
         for(unrouted_messages_type::const_iterator mi = unrouted_messages.begin();
             mi != unrouted_messages.end();
             ++mi)
         {
            message_handle const &msg = *mi;
            RouterHelpers::route_type *route = find_route(msg->get_destination());
            if(((route && route->port == port) ||
                (msg->get_use_own_route() && msg->get_port() == port)) &&
               msg->get_priority() > rtn)
            { rtn = msg->get_priority(); } 
         }
         return rtn;
      } // get_current_port_priority


      void Router::on_port_allowed_neighbours_changed(PortBase *port)
      {
         // we need to gather a list of all neighbour objects that would be effected by this
         // change.
         std::list<neighbour_handle> effected_neighbours;
         for(neighbours_type::iterator ni = neighbours.begin();
             ni != neighbours.end();
             ++ni)
         {
            neighbour_handle &neighbour = ni->second;
            if(neighbour->port == port && !port->can_accept_neighbour(ni->first))
               effected_neighbours.push_back(neighbour);
         }

         // we also need to remove any static routes that are effected by this change
         routes_type::iterator sri = static_routes.begin();
         while(sri != static_routes.end())
         {
            if(sri->second.port == port &&
               !port->can_accept_neighbour(sri->second.neighbour_id))
            {
               routes_type::iterator dsri = sri++;
               static_routes.erase(dsri);
            }
            else
               ++sri;
         }

         // we now need to report all of the effected neighbours as lost
         while(!effected_neighbours.empty())
         {
            neighbour_handle neighbour = effected_neighbours.front();
            on_neighbour_lost(
               neighbour->port,
               neighbour->physical_address);
            effected_neighbours.pop_front();
         } 
      } // on_port_allowed_neighbours_changed


      bool Router::route_port_is_active(uint2 address)
      {
         bool rtn = false;
         RouterHelpers::route_type *route = find_route(address);
         if(route)
            rtn = route->port->link_is_active();
         return rtn;
      } // route_port_is_active


      bool Router::route_port_should_cap_timeout(uint2 address)
      {
         bool rtn = true;
         RouterHelpers::route_type *route = find_route(address);
         if(route)
            rtn = route->port->should_cap_timeout();
         return rtn;
      } // route_port_should_cap_timeout


      void Router::route_port_reset_session_timer(uint2 address)
      {
         RouterHelpers::route_type *route = find_route(address);
         if(route)
            route->port->reset_session_timer(
               this_node_address,
               address);
      } // route_port_reset_session_timer

      
      void Router::set_is_leaf_node(bool is_leaf_node_)
      {
         if(is_leaf_node != is_leaf_node_)
         {
            // we need to clear all of the neighbours, routers, and links.  We can do this by
            // reporting each of the neighbours as lost.
            while(!neighbours.empty())
            {
               neighbours_type::iterator ni = neighbours.begin();
               neighbour_handle neighbour(ni->second);
               on_neighbour_lost(neighbour->port,neighbour->physical_address);
               if(ni == neighbours.begin())
                  neighbours.erase(ni);
            }
            ++neighbour_list_version;
            
            // we will now set the flag and start re-learning in our new role
            is_leaf_node = is_leaf_node_;
         }
      } // set_is_leaf_node


      void Router::on_port_ready(PortBase *port)
      {
         // all of the neighbours that we have that are associated with this port should be marked
         // as needing hello transactions immediately.
         for(neighbours_type::iterator ni = neighbours.begin();
             ni != neighbours.end();
             ++ni)
         {
            neighbour_handle &neighbour = ni->second;
            if(neighbour->port == port)
            {
               neighbour->hello_tries = 1;
               neighbour->send_hello_delay_base = 0;
            }
         }
      } // on_port_ready


      void Router::reassign_port_messages(PortBase *new_port, PortBase *old_port)
      {
         for(unrouted_messages_type::iterator mi = unrouted_messages.begin();
             mi != unrouted_messages.end();
             ++mi)
         {
            message_handle &message = *mi;
            if(message->get_use_own_route() && message->get_port() == old_port)
               message->set_port(new_port);
         }
      } // reassign_port_messages

      
      bool Router::on_message(message_handle &message)
      {
         bool rtn = false;

         try
         {
            if(message->get_high_protocol() == ProtocolTypes::control)
            {
               // ignore any empty messages
               if(message->length() >= PakCtrlMessage::header_len_bytes)
               {
                  pakctrl_message_handle pmessage(new PakCtrlMessage(*message));
                  rtn = on_pakctrl_message(pmessage);
               }
               else
                  rtn = true;
            }
            else if(message->get_high_protocol() == ProtocolTypes::bmp)
            {
               // ignore any empty messages
               if(message->length() >= Bmp5Message::header_len_bytes)
               {
                  bmp5_message_handle bmessage(new Bmp5Message(*message));
                  rtn = on_bmp5_message(bmessage);
                  if(!rtn && bmessage->get_message_type() == Bmp5Messages::one_way_data_not)
                     rtn = true;
               }
               else
                  rtn = true;
            }
            else if(message->get_high_protocol() == ProtocolTypes::encrypted)
               rtn = on_encrypted_message(message);
         }
         catch(std::exception &e)
         {
            log_debug(
               "Csi::PakBus::Router::on_message",
               e.what());
            rtn = false; 
         }
         do_next_router_transaction();
         return rtn;
      } // on_message


      bool Router::on_pakctrl_message(pakctrl_message_handle &message)
      {
         bool rtn = false;
         try
         {
            switch(message->get_message_type())
            {
            case PakCtrl::Messages::delivery_failure:
               on_delivery_failure(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::leaf_information:
               rtn = true;
               break;
               
            case PakCtrl::Messages::hello_cmd:
               on_hello_cmd(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::get_neighbour_list_cmd:
               on_get_neighbour_list_cmd(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::send_neighbour_list_cmd:
               on_send_neighbour_list_cmd(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::echo_cmd:
               on_echo_cmd(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::reset_router_cmd:
               on_reset_routes_cmd(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::goodbye_cmd:
               on_goodbye_cmd(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::hello_request_cmd:
               on_hello_request_cmd(message);
               rtn = true;
               break;

            case PakCtrl::Messages::remote_echo_cmd:
               on_remote_echo_cmd(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::list_port_names_cmd:
               on_list_port_names_cmd(message);
               rtn = true;
               break;
               
            case PakCtrl::Messages::remote_hello_request_cmd:
               on_remote_hello_request_cmd(message);
               rtn = true;
               break;
            }
            
            // if the message wasn't dispatched by message type, it might be dispatched by
            // transaction number
            if(!rtn)
            {
               using namespace RouterHelpers;
               transactions_type::iterator ti = transactions.find(
                  transaction_id(
                     message->get_source(),
                     message->get_transaction_no()));
               if(ti != transactions.end())
               {
                  rtn = true;
                  set_report_receive_time(ti->second->get_report_id(), LgrDate::system());
                  ti->second->on_pakctrl_message(message);
               }
            }
         }
         catch(std::exception &e)
         {
            log_debug(
               "Csi::PakBus::Router::on_pakctrl_message",
               e.what());
            rtn = false;
         }
         return rtn;
      } // on_pakctrl_message


      bool Router::on_bmp5_message(bmp5_message_handle &message)
      {
         using namespace RouterHelpers;
         bool rtn = false;
         try
         {
            transactions_type::iterator ti = transactions.find(
               transaction_id(
                  message->get_source(),
                  message->get_transaction_no()));
            if(ti != transactions.end())
            {
               rtn = true;
               set_report_receive_time(ti->second->get_report_id(), LgrDate::system());
               ti->second->on_bmp5_message(message);
            }
         }
         catch(std::exception &e)
         {
            log_debug(
               "Csi::PakBus::Router::on_bmp5_message",
               e.what());
            rtn = false;
         }
         return rtn;
      } // on_bmp5_message


      bool Router::on_encrypted_message(message_handle &message)
      {
         bool rtn(true);
         try
         {
            // decode and check the content
            byte flags(message->readByte());
            byte content_protocol_type((flags & 0xF0) >> 4);
            byte cipher_code(flags & 0x0F);
            uint2 content_len(message->readUInt2(!is_big_endian()));
            uint2 content_sig(message->readUInt2(!is_big_endian()));
            ciphers_type::iterator ci = ciphers.find(message->get_source());
            StrBin content;
            uint2 calc_content_sig;
            
            if(ci == ciphers.end())
               ci = ciphers.find(broadcast_address);
            if(ci == ciphers.end() || ci->second->get_cipher_code() != cipher_code)
            {
               
               throw std::invalid_argument("unsupported cipher");
            }
            if(content_len > message->whatsLeft())
               throw std::invalid_argument("invalid content length");
            ci->second->set_initialisation_vector(message->getMsg(), 5);
            ci->second->decrypt(content, message->objAtReadIdx(), message->whatsLeft());
            calc_content_sig = calcSigFor(content.getContents(), content_len);
            if(content_sig != calc_content_sig)
               throw std::invalid_argument("invalid content signature");

            // we can now create a new message using the content.  We will copy the original message
            // so far as its header is concerned.
            message_handle decrypted(new Message(*message, 0, true));
            decrypted->set_headerLen(0);
            decrypted->clear();
            decrypted->set_high_protocol(
               static_cast<ProtocolTypes::protocol_type>(content_protocol_type));
            decrypted->addBytes(content.getContents(), content_len);
            decrypted->set_encrypted(true);
            rtn = on_message(decrypted);
         }
         catch(std::exception &e)
         {
            OStrAscStream temp;
            temp << "encrypted message decode failed\",\"" << e.what() << "\"";
            log_comms(comms_fault, temp.c_str());
         }
         return rtn;
      } // on_encrypted_message


      void Router::on_delivery_failure(
         failure_type failure,
         message_handle &fragment)
      {
         // we need to log a fault message for this delivery failure
         OStrAscStream temp;
         static char const *failure_reasons[] = {
            "unrecognised failure",
            "unreachable destination",
            "unreachable high level protocol",
            "timed out or resource error",
            "unsupported message type",
            "malformed message",
            "failed static route",
            "message length larger than 64 bytes",
            "message length larger than 90 bytes",
            "message length larger than 128 bytes",
            "message length larger than 256 bytes",
            "message length larger than 512 bytes",
            "an unsupported cipher or an invalid encryption key was used",
            "encryption is required"
         };
         if(failure >= PakCtrl::DeliveryFailure::max_delivery_failure)
            failure = PakCtrl::DeliveryFailure::unknown_reason;
         temp << "delivery failure received\",\"";
         if(failure == PakCtrl::DeliveryFailure::unreachable_high_level_protocol &&
            fragment->get_high_protocol() == ProtocolTypes::encrypted)
            temp << "encryption is not supported";
         else
            temp << failure_reasons[failure];
         log_comms(comms_fault, temp.c_str());
         
         // is there a transaction associated with the fragment?
         if(fragment->get_high_protocol() == ProtocolTypes::control &&
            fragment->length() >= pakctrl_message_type::header_len_bytes)
         {
            pakctrl_message_handle pakctrl_message(
               new pakctrl_message_type(*fragment));
            transactions_type::iterator ti = transactions.find(
               RouterHelpers::transaction_id(
                  pakctrl_message->get_destination(),
                  pakctrl_message->get_transaction_no()));
            if(ti != transactions.end())
               ti->second->on_failure(failure); 
         }
         else if(fragment->get_high_protocol() == ProtocolTypes::bmp &&
            fragment->length() >= Bmp5Message::header_len_bytes)
         {
            typedef Csi::PolySharedPtr<Message, Bmp5Message> bmp5_message_handle;
            bmp5_message_handle bmp5_message(
               new Bmp5Message(*fragment));
            transactions_type::iterator ti = transactions.find(
               RouterHelpers::transaction_id(
                  bmp5_message->get_destination(),
                  bmp5_message->get_transaction_no()));
            
            if(ti != transactions.end())
               ti->second->on_failure(failure); 
         }
      } // on_delivery_failure


      void Router::on_hello_cmd(pakctrl_message_handle &message)
      {
         try
         {
            // we must not acknowledge any attempts to say hello if we are not allowed to use this
            // neighbour
            if(!message->get_port()->can_accept_neighbour(message->get_source()))
               return;
            
            // read the command parameters.  We can use these to update the neighbours list
            byte is_router = message->readByte();
            HopMetric hop_metric = message->readByte();
            uint2 beacon_interval = message->readUInt2(!is_big_endian());
            
            on_neighbour_info(
               message->get_port(),
               message->get_physical_source(),
               is_router ? true : false,
               hop_metric,
               beacon_interval);
            
            // form the response to send back to the requester
            pakctrl_message_handle ack(new pakctrl_message_type);
            ack->set_message_type(PakCtrl::Messages::hello_ack);
            ack->set_transaction_no(message->get_transaction_no());
            ack->set_destination(message->get_source());
            ack->set_priority(message->get_priority());
            ack->set_expect_more(ExpectMoreCodes::last);
            ack->set_port(message->get_port());
            ack->set_physical_destination(message->get_physical_source());
            ack->set_use_own_route(true);
            ack->addByte(is_leaf_node ? 0 : 1);
            ack->addByte(message->get_port()->get_hop_metric());
            ack->addUInt2(
               message->get_port()->get_verify_interval(),
               !is_big_endian());
            send_message_from_app(ack.get_handle());
         }
         catch(std::exception &e)
         {
            log_debug(
               "Csi::PakBus::Router::on_hello_cmd",
               e.what());
         }
      } // on_hello_cmd


      void Router::on_send_neighbour_list_cmd(pakctrl_message_handle &message)
      {
         // process the router's list
         byte change_code = message->readByte();
         on_router_neighbour_list(
            message->get_source(),
            message,
            static_cast<neighbour_list_update_type>(change_code));

         // send the acknowledgement
         pakctrl_message_handle ack(new pakctrl_message_type);
         ack->set_message_type(PakCtrl::Messages::send_neighbour_list_ack);
         ack->set_transaction_no(message->get_transaction_no());
         ack->set_priority(message->get_priority());
         ack->set_destination(message->get_source());
         ack->set_source(this_node_address);
         ack->set_expect_more(ExpectMoreCodes::last);
         send_message_from_app(ack.get_handle());
      } // on_send_neighbour_list_cmd


      void Router::on_get_neighbour_list_cmd(pakctrl_message_handle &message)
      {
         // prepare the acknowledgement
         pakctrl_message_handle ack(new pakctrl_message_type);
         ack->set_message_type(PakCtrl::Messages::get_neighbour_list_ack);
         ack->set_priority(message->get_priority());
         ack->set_transaction_no(message->get_transaction_no());
         ack->set_destination(message->get_source());
         ack->set_source(this_node_address);
         ack->set_priority(message->get_priority());

         // now add the acknowledgement contents
         ack->addByte(neighbour_list_version);
         for(neighbours_type::iterator ni = neighbours.begin();
             ni != neighbours.end();
             ++ni)
         {
            if(!ni->second->needs_hello_info)
            {
               ack->addUInt2(
                  ni->second->pack_list_entry(),
                  !is_big_endian());
            }
         }
         send_message_from_app(ack.get_handle());
      } // on_get_neighbour_list_cmd


      void Router::on_echo_cmd(pakctrl_message_handle &message)
      {
         // form the acknowledgement message
         pakctrl_message_handle ack(new pakctrl_message_type);
         int8 command_stamp;
         StrBin content;
         int8 q,r;
         
         ack->set_message_type(PakCtrl::Messages::echo_ack);
         ack->set_transaction_no(message->get_transaction_no());
         ack->set_destination(message->get_source());
         ack->set_source(this_node_address);
         ack->set_priority(message->get_priority());
         if(message->whatsLeft() >= sizeof(command_stamp))
            message->readBytes(&command_stamp,sizeof(command_stamp),false);
         if(message->whatsLeft() > 0)
            message->readBytes(content,message->whatsLeft(),false);
         truediv(q,r,LgrDate::system().get_nanoSec(),LgrDate::nsecPerSec);
         ack->addUInt4(static_cast<uint4>(q),!is_big_endian());
         ack->addUInt4(static_cast<uint4>(r),!is_big_endian());
         if(content.length())
            ack->addBytes(content.getContents(), (uint4)content.length());

         // if this is a leaf node, we will send the message back the way it was received
         if(is_leaf_node)
         {
            ack->set_port(message->get_port());
            ack->set_physical_destination(message->get_physical_source());
            ack->set_use_own_route(true);
         }
         send_message_from_app(ack.get_handle());
      } // on_echo_cmd


      void Router::on_reset_routes_cmd(pakctrl_message_handle &message)
      {
         bool destructive_reset = true;
         if(message->whatsLeft())
            destructive_reset = (message->readByte() == 0);

         if(destructive_reset)
         {
            // we need to clear all of the neighbours, routers, and links.  We can do this by
            // reporting each of the neighbours as lost.
            std::list<neighbour_handle> temp;
            for(neighbours_type::iterator ni = neighbours.begin(); ni != neighbours.end(); ++ni)
               temp.push_back(ni->second);
            while(!temp.empty())
            {
               neighbour_handle neighbour(temp.front());
               on_neighbour_lost(neighbour->port, neighbour->physical_address);
               temp.pop_front();
            }
            ++neighbour_list_version;
            assert(links.empty());
            assert(neighbours.empty());
            
            // we need to send notifications to all pending transactions that routes are no longer
            // valid
            log_debug(
               "Csi::PakBus::Router::on_reset_routes_cmd",
               "Cancelling all transactions");
            if(current_transaction != 0)
            {
               current_transaction->on_failure(
                  PakCtrl::DeliveryFailure::unreachable_destination);
               current_transaction.clear();
            }
            if(current_transaction_focus != 0)
            {
               current_transaction_focus->on_failure(
                  PakCtrl::DeliveryFailure::unreachable_destination);
               current_transaction_focus.clear();
            }
            waiting_transactions.clear();
            while(!transactions.empty())
            {
               transaction_handle tran(transactions.begin()->second);
               transactions.erase(transactions.begin());
               if(tran->get_report_id() != -1)
                  remove_report(tran->get_report_id());
               tran->on_failure(PakCtrl::DeliveryFailure::unreachable_destination);
            }
            unrouted_messages.clear();
         }
         else
         {
            // we need to verify all of the neighbour records
            for(neighbours_type::iterator ni = neighbours.begin();
                ni != neighbours.end();
                ++ni)
            {
               neighbour_handle &neighbour = ni->second;
               if(neighbour->hello_tries == 0)
                  neighbour->hello_tries = 1;
            }

            // we also need to send and receive neighbour lists from all known routers.
            for(all_routers_type::iterator ri = all_routers.begin();
                ri != all_routers.end();
                ++ri)
            {
               router_handle &router = ri->second;
               router->send_all = router->get_all = true;
            }
            do_next_router_transaction();
         }
      } // on_reset_routes_cmd


      void Router::on_goodbye_cmd(pakctrl_message_handle &message)
      {
         // because we just got this message from the node, we can assume that we will no longer
         // have it as a neighbour.
         neighbours_type::iterator ni = neighbours.find(message->get_physical_source());
         if(ni != neighbours.end())
         {
            if(PortBase::is_valid_instance(ni->second->port))
               ni->second->port->on_neighbour_lost(
                  message->get_physical_source());
            on_neighbour_lost(
               message->get_port(),
               message->get_physical_source());
         }
      } // on_goodbye_cmd


      void Router::on_hello_request_cmd(pakctrl_message_handle &message)
      {
         // we will not respond to this message if the node is not allowed
         if(!message->get_port()->can_accept_neighbour(message->get_source()))
            return;
         
         // make sure that the neighbour object exists
         neighbours_type::iterator ni = neighbours.find(message->get_physical_source());
         neighbour_handle neighbour;

         if(ni != neighbours.end())
         {
            // make sure that the neighbour is on the same port
            neighbour = ni->second;
            if(message->get_port() != neighbour->port)
            {
               on_neighbour_lost(neighbour->port,neighbour->physical_address);
               neighbour.clear();
            }
         }

         // make sure that the neighbour is allocated
         if(neighbour == 0)
         {
            neighbour.bind(
               new RouterHelpers::neighbour_type(
                  message->get_port(),
                  message->get_port()->get_hop_metric(),
                  message->get_physical_source()));
            neighbour->hello_tries = 1;
            neighbours[message->get_physical_source()] = neighbour;
            neighbour->needs_hello_info = true;
         }
         else
         {
            if(neighbour->hello_tries == 0)
               neighbour->hello_tries = 1;
         }
         do_next_router_transaction();
      } // on_hello_request_cmd 


      void Router::on_remote_echo_cmd(pakctrl_message_handle &message)
      {
         try
         {
            uint2 remote_address = message->readUInt2(!is_big_endian());
            uint2 packet_size = message->readUInt2(!is_big_endian()); 
            Csi::PolySharedPtr<PakBusTran, TranEcho> tran(
               new TranEcho(
                  this,
                  timer,
                  message->get_priority(),
                  remote_address,
                  0,            // no client object
                  packet_size,
                  message->get_source(),
                  message->get_transaction_no(),
                  is_leaf_node ? message->get_physical_source() : 0,
                  is_leaf_node ? message->get_port() : 0));
            open_transaction(tran.get_handle());
         }
         catch(std::exception &)
         { }
      } // on_remote_echo_cmd

      
      void Router::on_list_port_names_cmd(pakctrl_message_handle &message)
      {
         pakctrl_message_handle ack(new pakctrl_message_type);

         ack->set_message_type(PakCtrl::Messages::list_port_names_ack);
         ack->set_transaction_no(message->get_transaction_no());
         ack->set_destination(message->get_source());
         ack->set_source(this_node_address);
         ack->set_priority(message->get_priority());
         for(ports_type::iterator pi = ports.begin();
             pi != ports.end();
             ++pi)
         {
            PortBase *port = *pi;
            ack->addAsciiZ(port->get_port_name().c_str()); 
         }
         if(is_leaf_node)
         {
            ack->set_physical_destination(message->get_physical_destination());
            ack->set_port(message->get_port());
            ack->set_use_own_route(true);
         }
         send_message_from_app(ack.get_handle());
      } // on_list_port_names_cmd
      

      void Router::on_remote_hello_request_cmd(pakctrl_message_handle &message)
      {
         try
         {
            // read the port name from the message and search for that port
            StrAsc port_name;
            PortBase *port = 0;

            message->readAsciiZ(port_name);
            for(ports_type::iterator pi = ports.begin();
                port == 0 && pi != ports.end();
                ++pi)
            {
               PortBase *temp = *pi;
               if(temp->get_port_name() == port_name)
                  port = temp;
            }

            // get ready to post the message
            byte outcome = 0;
            if(port != 0)
            {
               pakctrl_message_handle req(new pakctrl_message_type);
               req->set_message_type(PakCtrl::Messages::hello_request_cmd);
               req->set_destination(broadcast_address);
               req->set_source(this_node_address);
               port->broadcast_message(req.get_handle());
            }
            else
               outcome = 1;

            // send the acknowledgement
            pakctrl_message_handle ack(new pakctrl_message_type);
            ack->set_message_type(PakCtrl::Messages::remote_hello_request_ack);
            ack->set_priority(message->get_priority());
            ack->set_destination(message->get_source());
            ack->set_source(this_node_address);
            ack->set_transaction_no(message->get_transaction_no());
            ack->addByte(outcome);
            if(is_leaf_node)
            {
               ack->set_physical_destination(message->get_physical_source());
               ack->set_port(message->get_port());
               ack->set_use_own_route(true);
            }
            send_message_from_app(ack.get_handle());
         }
         catch(std::exception &)
         { }
      } // on_remote_hello_request_cmd

      
      bool Router::route_message(message_handle &message)
      {
         bool rtn = false;
         if(message->get_destination() == broadcast_address)
         {
            // The message should be prepared to be broadcast.  The port needs to take
            // responsibility for sending the message.  If the port is not in a state where it can,
            // it should try to get on-line so that it can send the message.
            rtn = true;
            message->set_physical_source(this_node_address);
            message->reset_age();
            for(ports_type::iterator pi = ports.begin();
                pi != ports.end();
                ++pi)
               (*pi)->broadcast_message(message);

            // we also need to handle this message ourselves
            on_sending_message(message);
            on_message(message);
         }
         else if(message->get_destination() != this_node_address)
         {
            // determine whether the message can be routed
            RouterHelpers::route_type *route = find_route(message->get_destination());
            PortBase *route_port = 0;
            uint2 route_dest = 0;
            
            if(message->get_use_own_route())
            {
               route_port = message->get_port();
               route_dest = message->get_physical_destination();
            }
            else if(route)
            {
               route_port = route->port;
               route_dest = route->neighbour_id;
            }

            // now we can notify the port and push the message onto the queue
            if(PortBase::is_valid_instance(route_port))
            {
               // prepare this message to be queued
               rtn = true;
               message->set_physical_source(this_node_address);
               message->reset_age();
               unrouted_messages.push(message);
               route_port->on_message_ready(
                  route_dest,
                  message->get_priority());

               // if this message will not close and there is another message in the queue with the
               // same destination that will, we will need to remove that other message.
               if(!message->get_will_close())
               {
                  unrouted_messages_type::iterator ui = unrouted_messages.begin();
                  while(ui != unrouted_messages.end())
                  {
                     message_handle &other(*ui);
                     if(other != message && other->get_will_close() && other->get_destination() == message->get_destination())
                     {
                        unrouted_messages_type::iterator dui = ui++;
                        unrouted_messages.erase(dui);
                     }
                     else
                        ++ui;
                  }
               }
            }
         }
         else
         {
            rtn = true;
            on_sending_message(message);
            on_message(message);
         }
         return rtn;
      } // route_message


      void Router::on_sending_message(message_handle &message)
      {
         if((message->get_high_protocol() == ProtocolTypes::control ||
            message->get_high_protocol() == ProtocolTypes::bmp) &&
            message->length() >= PakCtrlMessage::header_len_bytes)
         {
            using namespace RouterHelpers;
            PakCtrlMessage ctrl_message(*message);
            transactions_type::iterator ti = transactions.find(
               transaction_id(
                  ctrl_message.get_destination(),
                  ctrl_message.get_transaction_no()));
            if(ti != transactions.end())
            {
               ti->second->on_sending_message(message);
               if(ti->second->get_report_id() != -1)
                  set_report_transmit_time(ti->second->get_report_id(), LgrDate::system());
            }
         }
      } // on_sending_message


      RouterHelpers::route_type *Router::find_route(uint2 node_id)
      {
         RouterHelpers::route_type *rtn = 0;
         routes_type::iterator ri = routes.find(node_id);
         if(ri != routes.end())
            rtn = &(ri->second);
         else
         {
            ri = static_routes.find(node_id);
            if(ri != static_routes.end())
               rtn = &(ri->second);
         }
         return rtn;
      } // find_route


      void Router::on_link_change(
         uint2 node1,
         uint2 node2,
         HopMetric hop_metric,
         link_change_type change)
      {
         // it will be useful to make note of this change in the comms log
         if(change != link_change_none && change != link_change_changed)
         {
            OStrAscStream msg;
            switch(change)
            {
            case link_change_added:
               msg << "link added";
               break;
               
            case link_change_deleted:
               msg << "link lost";
               break;
            }
            msg << "\",\"" << node1 << "\",\"" << node2;
            log_comms(comms_status_neutral,msg.str().c_str());
         }
         
         if(change == link_change_deleted)
         {
            // if any link is deleted, there is a chance that any associated router also needs to be
            // deleted. 
            uint2 nodes[2] = { node1, node2 };
            for(int i = 0; i < 2; ++i)
            {
               all_routers_type::iterator ri = all_routers.find(nodes[i]);
               if(ri != all_routers.end())
               {
                  // we need to check now to see if the router ID can be found in any link
                  bool found_link = false;
                  for(links_type::iterator li = links.begin();
                      !found_link && li != links.end();
                      ++li)
                  {
                     if(li->has_id(nodes[i]))
                        found_link = true; 
                  }
                  if(!found_link)
                     all_routers.erase(ri);
               }
            }

            // if one of the addresses is our own, this link loss may affect any transactions that
            // would rely on this link.
            if(node1 == this_node_address || node2 == this_node_address)
            {
               uint2 other_address(node1 == this_node_address ? node2 : node1);
               std::deque<transaction_handle> affected;
               for(transactions_type::iterator ti = transactions.begin(); ti != transactions.end(); ++ti)
               {
                  transaction_handle &candidate(ti->second);
                  if(candidate->get_destination_address() == other_address)
                     affected.push_back(candidate);
               }
               while(!affected.empty())
               {
                  transaction_handle transaction(affected.front());
                  affected.pop_front();
                  transaction->on_failure(PakCtrl::DeliveryFailure::unreachable_destination);
               }
            }
         }
      } // on_link_change

      
      void Router::onOneShotFired(uint4 id)
      {
         if(id == maintenance_id)
         {
            // check on the current focus
            if(current_transaction_focus != 0)
            {
               // the transaction must still be present in the list and be internally consistent
               bool is_valid = current_transaction_focus->is_still_valid();
               PakCtrl::DeliveryFailure::failure_type failure_code(PakCtrl::DeliveryFailure::timed_out_or_resource_error);
               if(is_valid)
               {
                  transactions_type::iterator ti = transactions.find(
                     RouterHelpers::transaction_id(
                        current_transaction_focus->get_destination_address(),
                        current_transaction_focus->get_transaction_id()));
                  if(ti == transactions.end())
                     is_valid = false;
               }

               // if the transaction is still valid, we will need some form of route to support the
               // transaction
               if(is_valid && !current_transaction_focus->router_sponsored())
               {
                  RouterHelpers::route_type *route(find_route(current_transaction_focus->get_destination_address()));
                  if(route == 0)
                  {
                     is_valid = false;
                     failure_code = PakCtrl::DeliveryFailure::unreachable_destination;
                  }
               }
               
               if(!is_valid)
               {
                  log_debug("Csi::PakBus::Router::onOneShotFired", "current focus invalid");
                  current_transaction_focus->on_failure(failure_code);
                  current_transaction_focus.clear();
               }
            }
            
            // we will iterate through the list of unrouted messages and reject those that have aged
            // too much or that have become unroutable.
            unrouted_messages_type::iterator mi = unrouted_messages.begin();
            std::list<message_handle> failed_messages;
            
            while(mi != unrouted_messages.end())
            {
               // look for the route for this message
               message_handle message = *mi;
               RouterHelpers::route_type *route = find_route(message->get_destination());
               bool failure = false;
               
               if(route == 0 || message->get_use_own_route())
               {
                  failure = true;
                  if(message->get_use_own_route() &&
                     PortBase::is_valid_instance(message->get_port()))
                  {
                     if(!message->get_port()->link_is_active())
                     {
                        message->get_port()->on_message_ready(
                           message->get_physical_destination(),
                           message->get_priority());
                     }
                     failure = false;
                  }
               }
               else
               {
                  // check to see how long the message has been queued.  If it has been waiting a
                  // long time and the port is active, we will have to time it out.
                  uint4 const max_queue_life = 1200000;
                  if(message->get_age_msec() > max_queue_life)
                  {
                     // we could be waiting for access to the link resources for the port.  In the
                     // server, this wait is not deterministic.  Because of this, will not time
                     // messages out of the queue where the port is waiting for resources 
                     if(route->port->link_is_active() && !route->port->should_cap_timeout())
                        failure = true;
                     else
                        message->reset_age();
                  }
                  if(!route->port->link_is_active())
                     message->reset_age();

                  // we might need to kick the port
                  if(!route->port->link_is_active())
                  {
                     if(!message->get_will_close())
                        route->port->on_message_ready(
                           route->neighbour_id,
                           message->get_priority());
                     else
                        failure = true;
                  }
               }

               if(failure)
               {
                  failed_messages.push_back(message);
                  unrouted_messages_type::iterator dmi = mi++;
                  unrouted_messages.erase(dmi);
               }
               else
                  ++mi;
            }

            // if there were any failed messages, we need to dispatch them now
            while(!failed_messages.empty())
            {
               log_debug(
                  "Csi::PakBus::Router::onOneShotFired",
                  "sending delivery fault");
               message_handle message(failed_messages.front());
               failed_messages.pop_front();
               send_delivery_fault_message(
                  message,
                  PakCtrl::DeliveryFailure::timed_out_or_resource_error); 
            }
            
            // start the next router transaction
            maintenance_id = 0;
            do_next_router_transaction();

            if(time_since_last_report == 0 || counter(time_since_last_report) > 10 * LgrDate::msecPerMin)
            {
               time_since_last_report = counter(0);
               do_debug_report();
            }
            
            // set up the timer for the next maintenance event
            maintenance_id = timer->arm(this,maintenance_interval);
         }
      } // onOneShotFired


      void Router::on_unhandled_message(message_handle &message)
      {
         send_delivery_fault_message(
            message, PakCtrl::DeliveryFailure::unsupported_message_type);
      } // on_unhandled_message



      void Router::set_cipher(uint2 address, cipher_handle cipher)
      {
         if(cipher != 0)
            ciphers[address] = cipher;
         else
            ciphers.erase(address);
      } // set_cipher


      bool Router::should_encrypt_message(message_handle &message)
      {
         bool rtn(message->should_encrypt());
         if(rtn)
         {
            ciphers_type::const_iterator ci(ciphers.find(message->get_destination()));
            if(ci == ciphers.end())
               ci = ciphers.find(broadcast_address);
            rtn = (ci != ciphers.end());
            message->set_encrypted(rtn);
         }
         return rtn;
      } // should_encrypt_message


      uint4 Router::get_max_body_len(uint2 destination) const
      {
         uint4 rtn = PakCtrlMessage::max_body_len;
         ciphers_type::const_iterator ci(ciphers.find(destination));
         if(ci == ciphers.end())
            ci = ciphers.find(broadcast_address);
         if(ci != ciphers.end())
            rtn = ci->second->max_payload_size();
         return rtn;
      } // get_max_body_len
      

      namespace
      {
         ////////////////////////////////////////////////////////////
         // functor debug_report
         ////////////////////////////////////////////////////////////
         struct debug_report
         {
            Router *router;
            OStrAscStream &msg;
            debug_report(Router *router_, OStrAscStream &msg_):
               router(router_),
               msg(msg_)
            { }

            void operator ()(PortBase *port)
            {
               msg.str("");
               msg << port->get_port_name() << "\",\"";
               if(port->link_is_active())
                  msg << "active";
               else
                  msg << "offline";
               msg << "\",\"beacon: " << port->get_beacon_interval()
                   << "\",\"verify: " << port->get_verify_interval()
                   << "\",\"response: " << port->get_worst_case_response()
                   << "\",\"waiting: " << router->count_messages_for_port(port, 0);
               router->log_debug("Csi::PakBus::Router Port", msg.str().c_str());
            }

            void operator ()(Router::message_handle &message)
            {
               msg.str("");
               message->describe_message(msg);
               router->log_debug("Csi::PakBus::Router Unrouted Message", msg.str().c_str());
            }

            void operator ()(Router::transactions_type::value_type &tran_) 
            {
               Router::transaction_handle &tran = tran_.second;
               msg.str("");
               tran->get_transaction_description(msg);
               msg << "\",\"dest: " << tran->get_destination_address()
                   << "\",\"tran: " << static_cast<int>(tran->get_transaction_id())
                   << "\",\"timeout: " << tran->get_time_out()
                   << "\",\"sent: " << tran->get_messages_sent();
               router->log_debug("Csi::PakBus::Router Transaction", msg.str().c_str());
            }
         };


         ////////////////////////////////////////////////////////////
         // functor debug_report_waiting
         ////////////////////////////////////////////////////////////
         struct debug_report_waiting
         {
            Router *router;
            OStrAscStream &msg;
            debug_report_waiting(Router *router_, OStrAscStream &msg_):
               router(router_),
               msg(msg_)
            { }

            void operator ()(Router::transaction_handle tran) 
            {
               msg.str("");
               tran->get_transaction_description(msg);
               msg << "\",\"dest: " << tran->get_destination_address()
                   << "\",\"tran: " << static_cast<int>(tran->get_transaction_id());
               router->log_debug("Csi::PakBus::Router waiting", msg.str().c_str());
            }
         };
      };


      void Router::do_debug_report()
      {
         OStrAscStream temp;
         log_debug("Csi::PakBus::Router", "start debug report");
         std::for_each(ports.begin(), ports.end(), debug_report(this, temp));
         std::for_each(unrouted_messages.begin(), unrouted_messages.end(), debug_report(this, temp));
         std::for_each(transactions.begin(), transactions.end(), debug_report(this, temp));
         if(current_transaction_focus != 0)
         {
            temp.str("");
            current_transaction_focus->get_transaction_description(temp);
            temp << "\",\"dest: " << current_transaction_focus->get_destination_address()
                 << "\",\"tran: " << static_cast<int>(current_transaction_focus->get_transaction_id());
            log_debug("Csi::PakBus::Router current focus", temp.str().c_str());
         }
         std::for_each(waiting_transactions.begin(), waiting_transactions.end(), debug_report_waiting(this, temp));
         log_debug("Csi::PakBus::Router", "stop debug report");
      } // do_debug_report


      uint4 Router::count_messages_for_port(
         PortBase *port,
         uint2 physical_destination)
      {
         uint4 rtn = 0;
         for(unrouted_messages_type::iterator umi = unrouted_messages.begin();
             umi != unrouted_messages.end();
             ++umi)
         {
            // find the route associated with this message
            message_handle const &message = *umi;
            RouterHelpers::route_type *route = find_route(message->get_destination());
            if(message->get_use_own_route())
            {
               if(message->get_port() == port &&
                  (message->get_physical_destination() == physical_destination ||
                   physical_destination == 0))
                  ++rtn;
            }   
            else if(route)
            {
               if(route->port == port &&
                  (route->neighbour_id == physical_destination || physical_destination == 0))
               ++rtn;
            } 
         }
         return rtn;
      } // count_messages_for_port


      bool Router::port_is_needed(PortBase *port)
      {
         bool rtn = count_messages_for_port(port,0) > 0;
         if(!rtn)
         {
            if(current_transaction != 0)
            {
               RouterHelpers::route_type *route = find_route(
                  current_transaction->get_destination_address());
               if(route && route->port == port)
                  rtn = true; 
            } 
            for(transactions_type::iterator ti = transactions.begin();
                ti != transactions.end() && !rtn;
                ++ti)
            {
               RouterHelpers::route_type *route = find_route(
                  ti->second->get_destination_address());
               if(route && route->port == port)
                  rtn = true;
            }
         }
         return rtn;
      } // port_is_needed
      

      void Router::forward_message(message_handle &message)
      {
         // route the message. If the routing fails, we need to generate a failure message
         if(message->get_hop_count() + 1 <= max_hop_count)
         {
            message_handle new_message(message);
            if(!route_message(new_message))
               send_delivery_fault_message(
                  new_message,
                  PakCtrl::DeliveryFailure::unreachable_destination);
         }
      } // forward_message


      void Router::on_delivery_failure(pakctrl_message_handle &message)
      {
         message->reset();
         byte error_code = message->readByte();
         uint2 t1 = message->readUInt2(!is_big_endian());
         uint2 t2 = message->readUInt2(!is_big_endian());
         byte high_protocol_code = t1 >> 12;
         byte hop_count = t2 >> 12; 
         message_handle fragment(
            new Message(
               message->objAtReadIdx(),
               message->whatsLeft()));
         
         if(error_code > PakCtrl::DeliveryFailure::max_delivery_failure)
            error_code = PakCtrl::DeliveryFailure::unknown_reason;
         fragment->set_high_protocol(
            static_cast<ProtocolTypes::protocol_type>(high_protocol_code));
         fragment->set_hop_count(hop_count);
         fragment->set_destination(t1 & 0xfff);
         fragment->set_source(t2 & 0xfff);
         on_delivery_failure(
            static_cast<failure_type>(error_code),
            fragment);

         // if the failure indicates no route available, then there is a possiblity that our links
         // table and/or routers list is out of synch with the rest of the network,
         if(error_code == PakCtrl::DeliveryFailure::unreachable_destination)
         {
            // there's a chance that verifying the link with the neighbour will result in other
            // changes.
            neighbours_type::iterator ni = neighbours.find(
               message->get_physical_source());
            if(ni != neighbours.end() && ni->second->hello_tries == 0)
               ni->second->hello_tries = 1;

            // we will also want to retrieve a neighbour list from the router that sent the
            // message.
            all_routers_type::iterator ri = all_routers.find(message->get_source());
            if(ri != all_routers.end())
               ri->second->get_all = true;
         } 
      } // on_delivery_failure


      byte Router::new_transaction_id(uint2 destination)
      {
         // search for a new unique transaction number
         using namespace RouterHelpers;
         byte rtn = last_transaction_id + 1;
         bool is_unique = false;

         while(!is_unique && rtn != last_transaction_id)
         {
            if(rtn == 0)
               rtn = 1;
            if(transactions.find(transaction_id(destination,rtn)) == transactions.end())
               is_unique = true;
            else
               ++rtn;
         }
         if(!is_unique)
         {
            do_debug_report();
            throw MsgExcept("no available transaction numbers");
         }
         last_transaction_id = rtn;
         return rtn;
      } // new_transaction_id


      void Router::do_next_router_transaction()
      {
         // we will do nothing if there is already a pending command
         if(current_transaction == 0 && !is_shutting_down)
         {
            // we need to check to see if a hello command needs to be started with a neighbour.
            neighbour_handle next_hello;
            for(neighbours_type::iterator ni = neighbours.begin();
                ni != neighbours.end();
                ++ni)
            {
               // we should evaluate whether we have recieved a beacon from the neighbour recently. 
               neighbour_handle neighbour(ni->second);
               uint4 neighbour_timeout = neighbour->calculate_verification_interval();

               if(neighbour->hello_tries == 0 &&
                  neighbour_timeout != 0 && 
                  Csi::counter(neighbour->time_since_last_beacon) > neighbour_timeout &&
                  !neighbour->port->must_close_link())
                  neighbour->hello_tries = 1;

               // if the neighbour needs a hello command, is there a delay that still has not
               // expired?
               if(neighbour->hello_tries &&
                  Csi::counter(neighbour->send_hello_delay_base) >= neighbour->send_hello_delay &&
                  !neighbour->port->must_close_link())
               {
                  // we will use the neighbour that has the fewest tries first.  That way, one
                  // neighbour won't monopolise all of the router's attention
                  if(next_hello == 0)
                     next_hello = neighbour;
                  else if(next_hello->hello_tries > neighbour->hello_tries)
                     next_hello = neighbour;
               }
            }

            // if there is a neighbour that we need to say hello to, we should start the
            // transaction here. 
            if(next_hello != 0)
            {
               // we need to send a hello command to this neighbour.
               current_transaction.bind(
                  new RouterHelpers::HelloTran(
                     this,timer,next_hello));
               open_transaction(current_transaction);
            }
            else if(!is_leaf_node)
            {
               // if there is no hello that needs to take place, we might also need to carry on a
               // transaction with one of the routers. we will choose the router that has the fewest
               // send_get retries
               router_handle next_router;
               
               for(all_routers_type::iterator ri = all_routers.begin();
                   current_transaction == 0 && ri != all_routers.end();
                   ++ri)
               {
                  router_handle &router = ri->second;
                  if(ri->first != this_node_address)
                  {
                     if(router->validated &&
                        (router->send_change || router->send_all || router->get_all))
                     {
                        // we don't want to consider the current router if its send delay hasn't
                        // expired
                        if(router->send_delay_base != 0 &&
                           counter(router->send_delay_base) < 60000)
                           continue;

                        // we want to choose the router that has been waiting the longest or the one
                        // that doesn't have a delay.
                        if(next_router == 0 || router->send_delay_base == 0)
                           next_router = router;
                        else if(next_router->send_delay_base != 0)
                        {
                           uint4 time_current = counter(router->send_delay_base);
                           uint4 time_next = counter(next_router->send_delay_base);
                           if(time_current > time_next)
                              next_router = router;
                        }
                     }
                  }
                  else
                     router->get_all = router->send_change = router->send_all = false;
               }

               // we can now start the transaction with the router.
               if(next_router != 0 && (next_router->send_all || next_router->send_change))
               {
                  current_transaction.bind(
                     new RouterHelpers::SendNeighboursTran(
                        this,timer,next_router));
                  open_transaction(current_transaction);
               }
               else if(next_router != 0 && next_router->get_all)
               {
                  current_transaction.bind(
                     new RouterHelpers::GetNeighboursTran(
                        this,timer,next_router));
                  open_transaction(current_transaction);
               }
            }
         }
      } // do_next_router_transaction


      void Router::set_next_transaction_focus()
      {
         if(current_transaction_focus == 0 && !prevent_next_focus)
         {
            // we will iterate through the list of waiting transactions until we find a transaction
            // associated with an open session that has a long delay time or we come to the end of
            // the list.  If we make it to the end of the list, the first element of the list will
            // be used.
            waiting_transactions_type::iterator wti = waiting_transactions.begin();
            waiting_transactions_type::iterator candidate_it = waiting_transactions.end();
            
            while(candidate_it == waiting_transactions.end() &&
                  wti != waiting_transactions.end())
            {
               // look up the route for this transactions destination
               transaction_handle &candidate = *wti;
               RouterHelpers::route_type *route = find_route(
                  candidate->get_destination_address());

               // we are looking for a transaction that may use an already opened session on a
               // dialed link (dialing is considered an expensive operation).  If such a transaction
               // exists, it should be given precedence regardless of priority. 
               if(!candidate->is_closing() &&
                  route &&
                  PortBase::is_valid_instance(route->port) && 
                  route->port->has_session(
                     this_node_address,
                     candidate->get_destination_address()) &&
                  route->port->link_is_dialed())
               {
                  candidate_it = wti;
               }
               else
                  ++wti;
            }

            // if we made it to the end of the list, we will use the waiting transaction at the
            // head of the queue
            while(candidate_it == waiting_transactions.end() &&
                  !waiting_transactions.empty())
            {
               transaction_handle &candidate = waiting_transactions.front();
               if(!candidate->is_closing())
                  candidate_it = waiting_transactions.begin();
               else
                  waiting_transactions.pop_front();
            }

            // we should now have a candidate for the final focus or nothing at all
            if(candidate_it != waiting_transactions.end())
            {
               transaction_handle &candidate = *candidate_it;
               RouterHelpers::route_type *route = find_route(
                  candidate->get_destination_address());

               if(route && current_transaction == 0 &&
                  !candidate->router_sponsored() &&
                  !candidate->will_terminate())
               {
                  // we need to find the neighbour.  If it can't be found, we will create him.
                  neighbours_type::iterator ni = neighbours.find(
                     route->neighbour_id);
                  neighbour_handle neighbour;

                  if(ni != neighbours.end())
                     neighbour = ni->second;
                  else
                  {
                     neighbour.bind(
                        new RouterHelpers::neighbour_type(
                           route->port,
                           route->port->get_hop_metric(),
                           route->neighbour_id));
                     neighbours[route->neighbour_id] = neighbour;
                     neighbour->needs_hello_info = true;
                     neighbour->hello_tries = 1;
                  }

                  // we need to decide if a hello is in order first
                  if(neighbour->hello_tries > 0 && !candidate->get_was_preempted())
                  {
                     // we will push a hello out first
                     current_transaction.bind(
                        new RouterHelpers::HelloTran(
                           this,
                           timer,
                           neighbour,
                           true,
                           candidate.get_rep()));
                     candidate->set_was_preempted();
                     open_transaction(current_transaction);
                     current_transaction_focus = current_transaction;

                     // we will also make sure that all routers accessed through this neighbour will
                     // be marked for immediate validation as well
                     for(all_routers_type::iterator ari = all_routers.begin();
                         ari != all_routers.end();
                         ++ari)
                     {
                        router_handle &router = ari->second;
                        RouterHelpers::route_type *route = find_route(
                           router->router_id);
                        if(route && route->neighbour_id == neighbour->physical_address)
                        {
                           router->send_all = router->get_all = true;
                           router->validated = false;
                           router->send_delay_base = 0;
                        }
                     }
                  }
                  else
                  {
                     current_transaction_focus = *candidate_it;
                     waiting_transactions.erase(candidate_it);
                  }
               }
               else
               {
                  current_transaction_focus = *candidate_it;
                  waiting_transactions.erase(candidate_it);
               }
            }

            // if there is a current focus now, it should be notified that it just gained the focus
            if(current_transaction_focus != 0)
            {
               OStrAscStream msg;
               current_transaction_focus->get_transaction_description(msg);
               msg << "\",\""
                   << static_cast<uint2>(current_transaction_focus->get_transaction_id())
                   << "\",\"" << current_transaction_focus->get_destination_address();
               log_debug("Transaction focus start",msg.str().c_str());
               current_transaction_focus->on_focus_start();
            }
         }
      } // set_next_transaction_focus


      void Router::update_links(
         uint2 node1,
         uint2 node2,
         HopMetric hop_metric,
         link_change_type change,
         bool regenerate_routes)
      {
         if(change == link_change_added || change == link_change_changed)
         {
            // we want to make sure that the link does not already exist
            links_type::iterator li;
            for(li = links.begin(); li != links.end(); ++li)
            {
               if(li->has_id(node1) == node2)
               {
                  if(li->get_hop_metric() != hop_metric)
                  {
                     li->set_hop_metric(hop_metric);
                     change = link_change_changed;
                  }
                  else
                     change = link_change_none;
                  break;
               }
            }
            if(li == links.end())
            {
               change = link_change_added;
               links.push_back(RouterHelpers::link_entry_type(node1,node2,hop_metric));
            }
         }
         else if(change == link_change_deleted)
         {
            links_type::iterator li = links.begin();
            change = link_change_none;
            while(li != links.end())
            {
               // search for and delete the specified link if it exists
               if(li->has_id(node1) == node2)
               {
                  // delete the link
                  links.erase(li);
                  li = links.end();
                  change = link_change_deleted;

                  // if either of the specified addresses are our address, we need to make sure
                  // that the neighbour record is eliminated as well.
                  if(node1 == this_node_address || node2 == this_node_address)
                  {
                     uint2 neighbour_id = node1;
                     neighbours_type::iterator ni;
                     
                     if(neighbour_id == this_node_address)
                        neighbour_id = node2;
                     ni = neighbours.find(neighbour_id);
                     if(ni != neighbours.end())
                        neighbours.erase(ni);
                  }
               }
               else
                  ++li;
            } 
         }

         // we now need to use the new link information to update the routing list.
         if(regenerate_routes && change != link_change_none)
            generate_routes_from_links();
         if(change != link_change_none)
            on_link_change(node1,node2,hop_metric,change);
      } // update_links


      void Router::update_routers(
         uint2 router_id,
         link_change_type change)
      {
         all_routers_type::iterator ri = all_routers.find(router_id);
         
         if(!is_leaf_node && change == link_change_added && ri == all_routers.end())
            all_routers[router_id] = new RouterHelpers::router_type(router_id,0);
         else if(change == link_change_deleted && ri != all_routers.end()) 
            all_routers.erase(ri);
      } // update_routers


      void Router::generate_routes_from_links()
      {
         // we will start by removing all dynmaic routes.  These will be re-generated by the
         // algorithm as needed.
         routes.clear();

         // we also need to make sure that our own node is represented in the all_routers set.
         if(all_routers.find(this_node_address) == all_routers.end())
            all_routers[this_node_address] = router_handle(
               new RouterHelpers::router_type(this_node_address,0));
               
         // all of the nodes in the network are identified in the links table.  We will iterate the
         // links table and consider the path to each node.
         links_type::iterator li = links.begin();
         while(li != links.end())
         {
            bool should_keep_link = true;

            if(!evaluate_route_for_node(li->get_id1()))
               should_keep_link = false;
            else if(!evaluate_route_for_node(li->get_id2()))
               should_keep_link = false;
            
            // if the link should be kept, all we have to do is go on to the next link.  Otherwise,
            // we need to delete the current link before moving on.
            if(should_keep_link)
               ++li;
            else
            {
               links_type::iterator rli = li++;
               uint2 id1 = rli->get_id1(), id2 = rli->get_id2();
               links.erase(rli);
               on_link_change(id1,id2,HopMetric(),link_change_deleted);
            }
         }
      } // generate_routes_from_links


      bool Router::evaluate_route_for_node(uint2 node_id)
      {
         bool rtn = true;

         // there is nothing to do if the route to this node is already known.
         if(routes.find(node_id) == routes.end() && node_id != this_node_address)
         {
            // we need to initialise all of the temporary members in the all_routers list
            all_routers_type::iterator ari;
            for(ari = all_routers.begin(); ari != all_routers.end(); ++ari)
            {
               ari->second->temp_is_visited = false;
               ari->second->temp_neighbour_id = 0;
               ari->second->temp_resp_msec = UInt4_Max;
            }

            // we will start at the remote end and seek a path to our own address
            uint2 current_node = node_id;
            uint4 current_resp_msec = 0;
            router_handle current_router;

            ari = all_routers.find(node_id);
            if(ari != all_routers.end())
               current_router = ari->second;
            do
            {
               // if the current node is a router, we will mark it as visited to start
               if(current_router != 0)
                  current_router->temp_is_visited = true;

               // for all links associated with the current node, we need to adjust the neighbour id
               // and response time
               for(links_type::iterator li = links.begin();
                   li != links.end();
                   ++li)
               {
                  uint2 router_id = li->has_id(current_node);
                  if(router_id != 0)
                  {
                     ari = all_routers.find(router_id);
                     if(ari != all_routers.end())
                     {
                        uint4 msec =
                           current_resp_msec +
                           li->get_hop_metric().get_response_time_msec();
                        if(msec < ari->second->temp_resp_msec)
                        {
                           ari->second->temp_resp_msec = msec;
                           ari->second->temp_neighbour_id = current_node;
                        }
                     }
                  }
               }

               // we will pick the next current node by finding the unvisited neighbour router with
               // the shortest response time.
               current_resp_msec = UInt4_Max;
               current_node = 0;
               current_router.clear();

               for(ari = all_routers.begin(); ari != all_routers.end(); ++ari)
               {
                  router_handle &router = ari->second;
                  if(!router->temp_is_visited &&
                     router->temp_resp_msec < current_resp_msec)
                  {
                     current_resp_msec = router->temp_resp_msec;
                     current_node = ari->first;
                     current_router = router;
                  }
               }
            }
            while(current_node != this_node_address && current_node != 0);

            // if we made it back to our own address, then we now have a good route
            if(current_node == this_node_address)
            {
               // we need to look up the neighbour associated with the neighbour address so that we
               // can get its associated port object.
               assert(current_router != 0);
               neighbours_type::iterator ni = neighbours.find(current_router->temp_neighbour_id);
               if(ni != neighbours.end())
               {
                  routes[node_id] = RouterHelpers::route_type(
                     node_id,
                     current_router->temp_neighbour_id,
                     ni->second->port,
                     current_resp_msec);
               }
               else
                  rtn = false;
            }
            else
               rtn = false;
         }
         return rtn;
      } // evaluate_route_for_node


      void Router::on_neighbour_change(
         uint2 neighbour_id,
         link_change_type change)
      {
         // look up the neighbour record that has changed
         neighbours_type::iterator ni = neighbours.find(neighbour_id);
         if(ni != neighbours.end() && change != link_change_none)
         {
            // set the last change record
            last_neighbour_change.change = change;
            last_neighbour_change.neighbour = ni->second;

            // update all of the router records
            for(all_routers_type::iterator ri = all_routers.begin();
                ri != all_routers.end();
                ++ri)
            {
               router_handle &router = ri->second;
               if(router->send_change)
                  router->send_all = true;
               else
                  router->send_change = true;
            }

            // this change will probably result in the links state changing
            update_links(
               this_node_address,
               ni->second->physical_address,
               ni->second->hop_metric,
               change,
               true);
            
            // make sure that something happens as a result of the change
            ++neighbour_list_version;
         }
      } // on_neighbour_change


      bool Router::waiting_transaction_gtr::operator ()(
         transaction_handle const &t1,
         transaction_handle const &t2)
      { return t1->get_priority() > t2->get_priority(); } 
   };
};

