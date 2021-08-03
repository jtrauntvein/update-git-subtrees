/* Cora.Posix.DllNetConnManager.cpp

   Copyright (C) 2020 Campbell Scientific, Inc.

   Written by: Andrew Mortenson
   Date Begun: Tuesday 20 May 2020
   Last Change: Tuesday 20 May 2020
   Last Commit: $Date: $
   Committed by: $Author: $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Posix.DllNetConnManager.h"
#include "Cora.Posix.DllNetConn.h"
#include "MsgExcept.h"
#include "Csi.Protector.h"
#include "Csi.Messaging.Message.h"
#include <map>
#include <dlfcn.h>


namespace Cora
{
   namespace Posix
   {
      namespace DllNetConnManager
      {
         ////////////////////////////////////////////////////////////
         // connections
         //
         // The set of all currently registered connections mapped through their connection identifier
         // handle. This map is wrapped in a protector class to protect it from multiple thread access. 
         ////////////////////////////////////////////////////////////
         typedef std::map<uint8, DllNetConn *> connections_map_type;
         typedef Csi::Protector<connections_map_type> connections_type;
         connections_type connections;

         ////////////////////////////////////////////////////////////
         // messages
         //
         // The set of all messages that have been sent by the server that haven't been deleted by the
         // client yet.  
         ////////////////////////////////////////////////////////////
         typedef std::map<void const *, Csi::Messaging::Message> messages_map_type;
         typedef Csi::Protector<messages_map_type> messages_type;
         messages_type messages;


         ////////////////////////////////////////////////////////////
         // message_delete_function
         //
         // Reference to the function that deletes messages
         ////////////////////////////////////////////////////////////
         typedef void(__stdcall message_delete_function_type)(void const *);
         message_delete_function_type *message_delete_function = 0;


         void add_connection(
            DllNetConn *connection,
            void* coralib_module)
         {
            if(message_delete_function == 0)
            {

                message_delete_function = (message_delete_function_type*)::dlsym(
                    coralib_module,
                    "cora_messaging_buffer_delete"
                );

               if(message_delete_function == 0)
                  throw Csi::OsException("Function call lookup failed");
            }
            connections_type::key_type key(connections);
            if(key->find(connection->get_identifier()) == key->end())
               (*key)[connection->get_identifier()] = connection;
            else
               throw Csi::MsgExcept("DllNetConnManager::add_connection -- redundant connection identifier");
         } // add_connection


         void remove_connection(DllNetConn *connection)
         {
            connections_type::key_type key(connections);
            connections_map_type::iterator ci = key->find(connection->get_identifier());
            if(ci != key->end())
               key->erase(ci);
            if(key->empty())
               message_delete_function = 0;
         } // remove_connection


         int __stdcall receive_event(
            uint8 connection_id,
            uint4 event_id,
            uint4 message_body_len,
            void const *message_body)
         {
            int rtn = cora_messaging_rtn_accepted;
            connections_type::key_type key(connections);
            connections_map_type::iterator ci = key->find(connection_id);
            if(ci != key->end())
            {
               switch(event_id)
               {
                  case cora_messaging_message:
                     DllNetConn::event_message_received::create_and_post(
                        ci->second,
                        message_body,
                        message_body_len);
                     if(!ci->second->get_is_server())
                        message_delete_function(message_body);
                     break;

                  case cora_messaging_cancelled:
                     DllNetConn::event_connection_cancelled::create_and_post(
                        ci->second);
                     break;

                  default:
                     rtn = cora_messaging_rtn_invalid_event_id;
                     break;
               }
            }
            else
               rtn = cora_messaging_rtn_invalid_connection_id;
            return rtn;
         } // receive_event


         void add_message(Csi::Messaging::Message &message)
         {
            // the message keeps its content in a shared pointer to a buffer.  Copies of message
            // objects share this internal buffer.  Because of this, we can get the pointer to the
            // beginning of the buffer and use it as a unique key for the message.  
            messages_type::key_type key(messages);
            Csi::Messaging::Message msg(message);
            (*key)[message.getMsg()] = msg;
         } // add_message


         void delete_message(void const *message_body)
         {
            messages_type::key_type key(messages);
            messages_map_type::iterator mi = key->find(message_body);
            if(mi != key->end())
               key->erase(mi);
         } // delete_message
      };
   };
};
