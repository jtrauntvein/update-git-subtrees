/* Csi.Messaging.Router.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 6 August 1996
   Last Change: Monday 25 January 2016
   Last Commit: $Date: 2016-01-25 17:13:01 -0600 (Mon, 25 Jan 2016) $ 
   Commited by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Messaging.Router.h"
#include "Csi.Messaging.Defs.h"
#include "Csi.Messaging.Connection.h"
#include "Csi.Messaging.Node.h"
#include "Csi.Messaging.Message.h"
#include "Csi.PolySharedPtr.h"
#include <list>
#include <set>
#include <assert.h>


namespace Csi
{
   namespace Messaging
   {
      namespace RouterHelpers
      {
         ////////// class ev_message_received_type
         class ev_message_received_type: public Csi::Event
         {
         public:
            ////////// event_id
            static uint4 const event_id;

            ////////// create_and_post
            static void create_and_post(Router *router, Message *message);

            ////////// message
            Csi::SharedPtr<Message> message;
      
         private:
            ////////// constructor
            ev_message_received_type(Router *router, Message *message_);
         };

         ////////// class ev_conn_closed
         class ev_conn_closed: public Csi::Event
         {
         private:
            ////////// constructor
            ev_conn_closed(Router *router, Router::closed_reason_type code_);

         public:
            ////////// event_id
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // by_heart_beat
            ////////////////////////////////////////////////////////////
            Router::closed_reason_type code;

            ////////// create_and_post
            static void create_and_post(Router *router, Router::closed_reason_type code);
         };
      };

      ////////////////////////////////////////////////////////////
      // class Router definitions
      ////////////////////////////////////////////////////////////

      uint4 Router::instance_count = 0;


      Router::Router(Connection *conn_):
         isClosing(false),
         conn(0),
         last_session_no(0)
      {
         bind_connection(conn_);
         ++instance_count;
      } // constructor


      Router::~Router()
      {
         if(instance_count > 0)
            --instance_count;
         handle_conn_closed(closed_remote_disconnect);
         if(Connection::is_valid_instance(conn))
         {
            delete conn;
            conn = 0;
         }
      } // destructor


      void Router::sendMessage(Message *msg)
      {
         if(Connection::is_valid_instance(conn))
            conn->sendMessage(msg);
      } // sendMessage


      void Router::rcvMessage(Message *msg)
      { RouterHelpers::ev_message_received_type::create_and_post(this,msg); }


      void Router::onConnClosed(closed_reason_type code)
      { RouterHelpers::ev_conn_closed::create_and_post(this, code); }


      uint4 Router::openSession(Node *node)
      {
         // allocate a new session number by incrementing the last one that was used.
         uint4 rtn = last_session_no + 1;
         bool is_unique = false;
         
         if(rtn == 0)
            rtn = 1;
         while(!is_unique)
         {
            if(routes.find(rtn) == routes.end())
               is_unique = true;
            else
            {
               ++rtn;
               if(rtn == 0)
                  rtn = 1;
            }
         }

         // we also need to make sure that the connection is valid and that the client specified is
         // a valid instance.
         if(Connection::is_valid_instance(conn))
         {
            if(Node::is_valid_instance(node))
            {
               last_session_no = rtn;
               addRoute(node,rtn);
            }
            else
               rtn = 0;
         }
         else
            rtn = 0;
         return rtn;
      } // openSession


      void Router::closeSession(uint4 sesNo)
      {
         // locate the specified route
         routes_type::iterator ri = routes.find(sesNo);
         if(ri != routes.end())
         {
            // send a close command
            if(!ri->second->will_close)
            {
               Message msg(sesNo,Messages::type_session_close_cmd);
               conn->sendMessage(&msg);
            }

            // remove the route and close the connection if needed
            routes.erase(ri);
            if(routes.empty())
               conn->detach();
         }
      } // closeSession


      bool Router::isValidSesNo(uint4 sesNo)
      { return routes.find(sesNo) != routes.end(); }


      void Router::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace RouterHelpers;
         if(ev->getType() == ev_message_received_type::event_id)
            handle_message_received(ev);
         else if(ev->getType() == ev_conn_closed::event_id)
         {
            ev_conn_closed *event = static_cast<ev_conn_closed *>(ev.get_rep());
            handle_conn_closed(event->code);
         }
      } // receive


      void Router::addRoute(Node *node, uint4 clntSesNo)
      {
         assert(Node::is_valid_instance(node));
         route_handle route(new route_type(node,clntSesNo));
         if(routes.empty())
            conn->attach();
         routes[clntSesNo] = route;
      } // addRoute


      void Router::handle_message_received(Csi::SharedPtr<Csi::Event> &ev)
      {
         try
         {
            using namespace RouterHelpers;
            Csi::PolySharedPtr<Csi::Event, ev_message_received_type> event(ev);
            Csi::SharedPtr<Message> msg(event->message);
            
            // extract the client session number
            uint4 clntSesNo = msg->getClntSesNo();
            uint4 msgType = msg->getMsgType();
            
            // route the message
            routes_type::iterator ri = routes.find(msg->getClntSesNo());
            if(ri != routes.end())
            {
               // This message might be a notification that the server is shutting down or a command
               // from the client to shut down the session.
               if(msgType == Messages::type_session_close_cmd ||
                  msgType == Messages::type_session_closed_not)
               {
                  // fill in information if this is a session closed notification
                  uint4 respCode = 0;
                  static const char *msgs[] =
                     {
                        "Specified object does not exist",
                        "Insufficient resources",
                        "The server object was deleted or shut down"
                     };
                  char const *error = "";
                  
                  if(msgType == Messages::type_session_closed_not)
                  {
                     msg->readUInt4(respCode);
                     if(respCode <= 3)
                        error = msgs[respCode - 1];
                  }
                  
                  // send the notification
                  Node *node = ri->second->node;
                  routes.erase(ri);
                  if(Node::is_valid_instance(node))
                     node->onNetSesBroken(this,clntSesNo,respCode,error);
               }
               // It might also be a rejected message informing the node that the session is orphaned
               else if(msgType == Messages::type_message_rejected_not)
               {
                  uint4 respCode;
                  
                  msg->readUInt4(respCode);
                  if(respCode == Messages::message_rejected_orphaned_session)
                  {
                     Node *node = ri->second->node;
                     routes.erase(ri);
                     if(Node::is_valid_instance(node))
                        node->onNetSesBroken(
                           this,
                           clntSesNo,
                           Messages::session_closed_no_object,
                           "Orphaned session");
                  }
                  else
                  {
                     msg->reset();
                     if(Node::is_valid_instance(ri->second->node))
                        ri->second->node->onNetMessage(this,msg.get_rep());
                     else
                        routes.erase(ri);
                  }
               } 
               else
               {
                  msg->reset();
                  if(Node::is_valid_instance(ri->second->node))
                     ri->second->node->onNetMessage(this,msg.get_rep());
                  else
                     routes.erase(ri);
               }
            } // route the message
            else
            {
               if(msgType != Messages::type_session_close_cmd &&
                  msgType != Messages::type_message_rejected_not)
               {
                  Message ack(clntSesNo,Messages::type_message_rejected_not);
                  
                  ack.addUInt4(Messages::message_rejected_orphaned_session);
                  ack.addBytes(msg->getMsg(),msg->getLen());
                  conn->sendMessage(&ack);
               } 
            } // unroutable message
         }
         catch(std::exception &e)
         {
            trace("Exception thrown while handling a message: %s", e.what());
            handle_conn_closed(closed_unknown_failure);
         }
      } // handle_message_received


      void Router::handle_conn_closed(closed_reason_type code)
      {
         // flush the routing table
         std::list<route_handle> closing_sessions;

         for(routes_type::iterator ri = routes.begin(); ri != routes.end(); ++ri)
            closing_sessions.push_back(ri->second);
         isClosing = true;
         routes.clear();

         // inform each node that its session is broken
         char const *message = "";
         SessionBrokenReasons::session_broken_reason_type reason =
            SessionBrokenReasons::connection_failed;
         switch(code)
         {
         case closed_remote_disconnect:
            message = "remote disconnect";
            break;

         case closed_heart_beat:
            message = "heart beat triggered";
            reason = SessionBrokenReasons::heart_beat_failed;
            break;

         default:
            message = "unrecognised failure";
            break;
         }
         while(!closing_sessions.empty())
         {
            route_handle route(closing_sessions.front());
            closing_sessions.pop_front();
            if(Node::is_valid_instance(route->node))
               route->node->onNetSesBroken(
                  this,
                  route->session_no,
                  reason,
                  message);
         }
      } // handle_conn_closed


      bool Router::bind_connection(Connection *conn_)
      {
         bool rtn;
         if(!Connection::is_valid_instance(conn))
         {
            if(Connection::is_valid_instance(conn_))
            {
               rtn = true;
               conn = conn_;
               conn->setRouter(this);
            }
            else
               rtn = false;
         }
         else
            rtn = false;
         return rtn;
      } // bind_connection


      StrAsc Router::get_remote_address()
      {
         StrAsc rtn;
         if(Connection::is_valid_instance(conn))
            rtn = conn->get_remote_address();
         return rtn;
      } // get_remote_address


      bool Router::peer_is_remote()
      {
         bool rtn(false);
         if(Connection::is_valid_instance(conn))
            rtn = conn->peer_is_remote();
         return rtn;
      }

      
      namespace RouterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class ev_message_received_type definitions
         ////////////////////////////////////////////////////////////

         uint4 const ev_message_received_type::event_id =
         Csi::Event::registerType("ev_message_received_type");
   
   
         void ev_message_received_type::create_and_post(Router *router,
                                                        Message *message)
         {
            try
            {
               ev_message_received_type *ev = new ev_message_received_type(router,message);
               ev->post();
            }
            catch(Csi::Event::BadPost &)
            { }
         } // ev_message_received_type
   

         ev_message_received_type::ev_message_received_type(Router *router,
                                                            Message *message_):
            Event(event_id,router)
         { message.bind(new Message(*message_)); }


         ////////////////////////////////////////////////////////////
         // class ev_conn_closed definitions
         ////////////////////////////////////////////////////////////

         ev_conn_closed::ev_conn_closed(Router *router, Router::closed_reason_type code_):
            Event(event_id, router),
            code(code_)
         { }


         uint4 const ev_conn_closed::event_id =
         Csi::Event::registerType("RouterHelpers::ev_conn_closed");


         void ev_conn_closed::create_and_post(Router *router, Router::closed_reason_type code)
         {
            try
            {
               ev_conn_closed *ev = new ev_conn_closed(router, code);
               ev->post();
            }
            catch(Csi::Event::BadPost &)
            { }
         } // create_and_post
      };
   };
};
