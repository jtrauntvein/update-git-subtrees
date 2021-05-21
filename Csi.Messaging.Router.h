/* Csi.Messaging.Router.h

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 June 2000
   Last Change: Monday 04 January 2016
   Last Commit: $Date: 2016-01-21 15:58:49 -0600 (Thu, 21 Jan 2016) $ 
   Commited by: $Author: jon $
   
*/

#ifndef Csi_Messaging_Router_h
#define Csi_Messaging_Router_h

#include <map> 
#include "Csi.Events.h"
#include "StrAsc.h"


namespace Csi
{
   namespace Messaging
   {
      //@group class forward declarations
      class Connection;
      class Node;
      class Message;
      //@endgroup


      /**
       * Defines a component that can be used to route messages to and from
       * multiple sessions using shared connection.
       */
      class Router: public Csi::EventReceiver
      {
      public:
         /**
          * Constructor.
          *
          * @param conn Specifies the connection object to be used with this
          * router.  This value must be set to a valid pointer.  If set to
          * null, the bind_connection() method must be called before this
          * router is used.
          */
         Router(Connection *conn = 0);

         /**
          * Destructor
          */
         virtual ~Router();

         /**
          * Prepares the specified message to be sent to the other side.
          *
          * @param msg Specifies the message to be sent.
          */
         virtual void sendMessage(Message *msg);

         /**
          * Handles an incoming message from the connection and uses the
          * message's session to determine the session to which the message
          * will be sent.
          *
          * @param msg Specifies the incoming message to process.
          */
         virtual void rcvMessage(Message *msg);

         /**
          * Handles the notification that the connection has lost its link.
          *
          * @param code Specifies the reason why this method was called.
          */
         enum closed_reason_type
         {
            closed_remote_disconnect,
            closed_heart_beat,
            closed_unknown_failure
         };
         virtual void onConnClosed(closed_reason_type code = closed_remote_disconnect);

         /**
          * Called by a node to open a network  session.  This session will not
          * be completed until the first message is sent for the session.  
          *
          * @return Returns the identifier for the created session.  If this
          * value is zero, the connection will be considered to be invalid.
          */
         virtual uint4 openSession(Node *node);

         /**
          * Closes a session opened previously by a call to openSession().  If
          * this is the last session hosted by this router, the connection will
          * be detached.
          *
          * @param session_no Specifies the identifier for the session to be
          * closed.
          */
         virtual void closeSession(uint4 session_no);

         /**
          * @return Returns the reference to the connection used by this
          * router.
          */
         Connection *get_conn() { return conn; }

         /**
          * @return Returns true if the connection is being closed.
          */
         bool get_isClosing() const { return isClosing; }

         /**
          * @return Returns the number of active sessions hosted by this
          * router.
          */
         uint4 numRoutes() const
         { return static_cast<uint4>(routes.size()); }

         /**
          * @return Returns true if the specified session number is valid.
          *
          * @param session_no Specifies the identifier for the session to test.
          */
         bool isValidSesNo(uint4 session_no);

         /**
          * Overloads the base class event handler.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * @return Returns the total number of routers in existence in the
          * application.
          */
         static uint4 get_instance_count() { return instance_count; }

         /**
          * Binds the specified connection object to this router.
          *
          * @param conn_ Speciifes the connection object that will be used by
          * this router.  This should have been allocated on the heap and, once
          * assigned to the router, will be managed by the router.
          *
          * @return Returns true if the connection was successfully bound.
          * Will return false if the router already has a bound connection.
          */
         bool bind_connection(Connection *conn_);

         /**
          * @return Returns a string that represents the address of the link
          * peer.  For non-IP links, this will be an empty string.
          */
         StrAsc get_remote_address();

         /**
          * @return Returns true if the peer connection is on a remote machine.
          */
         bool peer_is_remote();
         
      protected:
         /**
          * Handles an incoming message.
          */
         void handle_message_received(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Handles a notification of a connection closed event.
          *
          * @param code Specifies the reason for the closure.
          */
         void handle_conn_closed(closed_reason_type code);

      protected:
         /**
          * Defines an object that represents the state of a session.
          */
         struct route_type
         {
            ////////////////////////////////////////////////////////////
            // constructor
            //////////////////////////////////////////////////////////// 
            route_type(Node *node_ = 0, uint4 session_no_ = 0):
               node(node_),
               session_no(session_no_),
               will_close(false)
            { }

            Node *node;
            uint4 session_no;
            bool will_close;
         };

         /**
          * Specifies the sessions currently supported by this router.
          */
         typedef Csi::SharedPtr<route_type> route_handle;
         typedef std::map<uint4, route_handle> routes_type;
         routes_type routes;
   
         /**
          * Reference to the connection object bound to this router.
          */
         Connection *conn;

         /**
          * Specifies the total number of router instances that have been
          * created in the application.
          */
         static uint4 instance_count;

         /**
          * Set to true if the router has begun the process of shutting down.
          */
         bool isClosing;

         /**
          * Adds the specified route to the set managed by this router.
          *
          * @param node Specifies the node object associated with the session.
          *
          * @param session_no Specifies the session identifier.
          */
         void addRoute(Node *node, uint4 session_no);

      private:
         /**
          * Specifies the last session number that was used.  This will be used
          * to generate a guess for the next session to be allocated.
          */
         uint4 last_session_no;
      };
   };
};

#endif
