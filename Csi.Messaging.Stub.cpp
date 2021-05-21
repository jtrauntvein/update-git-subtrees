/* Csi.Messaging.Stub.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 June 2000
   Last Change: Thursday 21 January 2016
   Last Commit: $Date: 2016-01-21 15:58:49 -0600 (Thu, 21 Jan 2016) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Messaging.Stub.h"
#include "Csi.Messaging.Server.h"
#include "Csi.Messaging.Message.h"
#include "Csi.Messaging.Connection.h"


namespace Csi
{
   namespace Messaging
   {
      ////////////////////////////////////////////////////////////
      // class Stub definitions
      ////////////////////////////////////////////////////////////
      StrUni Stub::serverName; 


      StrUni Stub::releaseVer;


      void Stub::set_serverName(char const *serverName_)
      { serverName = serverName_; }


      void Stub::set_releaseVer(char const *releaseVer_)
      { releaseVer = releaseVer_; }


      Stub::Stub(Server *server, Connection *conn):
         Router(conn),
         newSvr(server)
      { server->addStub(this); }


      void Stub::rcvMessage(Message *msg)
      {
         // extract the client session number
         uint4 clntSesNo = msg->getClntSesNo();

         // check to see if this is the first message through
         if(newSvr != NULL)
         {
            addRoute(newSvr,clntSesNo);
            newSvr->onSessionOpen(this,clntSesNo);
            newSvr = NULL;
         }

         // Is this message a query command
         if(msg->getMsgType() == Messages::type_query_server_cmd)
         {
            Message ack(clntSesNo,Messages::type_query_server_ack);

            ack.addWStr(serverName.c_str());
            ack.addWStr(releaseVer.c_str());
            sendMessage(&ack);
         }
         else
            Router::rcvMessage(msg);
      } // rcvMessage


      void Stub::finishSession(uint4 sesNo, Server *server)
      {
         addRoute(server,sesNo);
         server->onSessionOpen(this,sesNo);
      } // finishSession


      void Stub::closeSession(uint4 session_no,
                              Messages::session_closed_reason_type reason)
      {
         // find the specified route
         routes_type::iterator ri = routes.find(session_no);
         if(ri != routes.end())
         {
            // send a closed notification
            if(!ri->second->will_close)
            {
               Message msg(session_no,Messages::type_session_closed_not);
         
               msg.addUInt4(reason);
               conn->sendMessage(&msg);
            }

            // remove the route and close the connection (if needed)
            routes.erase(ri);
            if(routes.empty())
            {
               conn->detach();
               delete this;
            }
         }
      } // closeSession


      void Stub::rejectMessage(Message *rejected_message,
                               Messages::message_rejected_reason_type reason)
      {
         Message reply(rejected_message->getClntSesNo(),Messages::type_message_rejected_not);
         reply.addUInt4(reason);
         reply.addBytes(rejected_message->getMsg(),rejected_message->getLen());
         sendMessage(&reply); 
      } // rejectMessage


      void Stub::onConnClosed(closed_reason_type code)
      {
         handle_conn_closed(code);
         if(newSvr)
            newSvr->removeStub(this);
         delete this;
      } // onConnClosed 
   };
};

