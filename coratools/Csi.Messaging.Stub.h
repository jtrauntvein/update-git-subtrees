/* Csi.Messaging.Stub.h

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 June 2000
   Last Change: Thursday 21 January 2016
   Last Commit: $Date: 2016-01-21 15:58:49 -0600 (Thu, 21 Jan 2016) $ 
   Commited by: $Author: jon $
   
*/

#ifndef Csi_Messaging_Stub_h
#define Csi_Messaging_Stub_h

#include "Csi.Messaging.Defs.h"
#include "Csi.Messaging.Router.h"
#include "StrUni.h"


namespace Csi
{
   namespace Messaging
   {
      //@group class forward declarations
      class Server;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class Stub
      //
      //  Defines a server-side message router. When a listener type object detects a connect
      //  condition, an object of this class will be created and the default Server known by the
      //  listener will be passed to it.
      //////////////////////////////////////////////////////////// 
      class Stub: public Router
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // The server is the default service that is known by the listener
         // object. When the first message arrives from the client, a session will be
         // established with the server which will complete the connection.
         //////////////////////////////////////////////////////////// 
         Stub(Server *default_server, Connection *conn);

         ////////////////////////////////////////////////////////////
         // rcvMessage
         //
         // overloads the Router RcvMessage method.  This version will add the
         // additional functionality of checking for a recently established
         // connection on the server side.
         //////////////////////////////////////////////////////////// 
         virtual void rcvMessage(Message *msg);

         ////////////////////////////////////////////////////////////
         // finishSession
         //
         // Allows the server to finish a session that was started on the client
         // side.  This will usually happen as a result of a message sent to an
         // object on the server requesting a connection and providing an allocated
         // client session number
         //////////////////////////////////////////////////////////// 
         virtual void finishSession(uint4 sesNo, Server *server);

         ////////////////////////////////////////////////////////////
         // closeSession
         //
         // Performs the same work as the closeSession method associated with
         // Router. This version will send a session closed message to the client
         // along with the provided explanation. In general, a server should invoke
         // this method rather than the simple closeSession
         //////////////////////////////////////////////////////////// 
         virtual void closeSession(
            uint4 session_no,
            Messages::session_closed_reason_type reason);

         ////////////////////////////////////////////////////////////
         // rejectMessage
         //
         // allows an upper level process to reject the message. If a valid pointer
         // supplied for the text, then that will be used in the rejected message,
         // otherwise, a suitable string will be substituted using the reason code
         //////////////////////////////////////////////////////////// 
         virtual void rejectMessage(
            Message *rejected_message,
            Messages::message_rejected_reason_type reason);

         ////////////////////////////////////////////////////////////
         // onConnClosed
         //
         // Overloads the onConnClosed method to cause self-destruction. This is
         // necessary because the stub will otherwise persist until the server object
         // is shut down
         //////////////////////////////////////////////////////////// 
         virtual void onConnClosed(closed_reason_type code);

         ////////////////////////////////////////////////////////////
         // set_serverName
         //
         // Sets ths value of the serverName static variable
         //////////////////////////////////////////////////////////// 
         static void set_serverName(char const *serverName_);

         ////////////////////////////////////////////////////////////
         // set_releaseVer
         //
         // Sets the value if the releaseVer static variable
         //////////////////////////////////////////////////////////// 
         static void set_releaseVer(char const *releaseVer_);

         ////////////////////////////////////////////////////////////
         // get_client_app_name
         ////////////////////////////////////////////////////////////
         StrUni const get_client_app_name() const
         { return client_app_name; }

         ////////////////////////////////////////////////////////////
         // set_client_app_name
         ////////////////////////////////////////////////////////////
         void set_client_app_name(StrUni const &name)
         { client_app_name = name; }

         ////////////////////////////////////////////////////////////
         // get_client_user_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_client_user_name() const
         { return client_user_name; }

         ////////////////////////////////////////////////////////////
         // set_client_user_name
         ////////////////////////////////////////////////////////////
         void set_client_user_name(StrUni const &name)
         { client_user_name = name; }
         
      protected:
         ////////////////////////////////////////////////////////////
         // newSvr
         ////////////////////////////////////////////////////////////
         Server *newSvr;

         ////////////////////////////////////////////////////////////
         // client_app_name
         //
         // Can be used to store the name of the client application (received
         // through the last LoggerNet logon transaction, for instance).
         ////////////////////////////////////////////////////////////
         StrUni client_app_name;

         ////////////////////////////////////////////////////////////
         // client_user_name
         //
         // Can be used to store the user name for the client connection. 
         ////////////////////////////////////////////////////////////
         StrUni client_user_name;
         
      private:
         ////////////////////////////////////////////////////////////
         // serverName
         //
         // Describes the server name for the query server command. This value can be
         // set by calling the static set_serverName method.
         //////////////////////////////////////////////////////////// 
         static StrUni serverName;

         ////////////////////////////////////////////////////////////
         // releaseVer
         //
         // Describes the release version for the query server command. This value
         // can be set by calling the static set_releaseVer method.
         //////////////////////////////////////////////////////////// 
         static StrUni releaseVer;
      };

   };
};

#endif
