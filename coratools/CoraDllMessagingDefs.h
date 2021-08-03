/* CoraDllMessagingDefs.h

   Copyright (C) 2000, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 02 June 2000
   Last Change: Saturday 24 September 2005
   Last Commit: $Date: 2020-05-20 11:22:13 -0600 (Wed, 20 May 2020) $ (UTC)
   Committed by: $Author: amortenson $
   
*/

#ifndef CoraDllMessagingDefs_h
#define CoraDllMessagingDefs_h

#include "CsiTypeDefs.h"
#ifndef _WIN32
#ifndef __stdcall
#define __stdcall
#endif
#endif


extern "C"
{
   //@group CSI Messaging support functions
   // Declares functions and call-back declarations that can be used to establish and work with
   // messaging sessions that use forward calls and call-backs to support CSI messaging


   ////////////////////////////////////////////////////////////
   // enum cora_messaging_event_id_type
   //
   // Specifies the types of events that can be specified in cora_messaging_call_forward() as well
   // as in the callback method.
   ////////////////////////////////////////////////////////////
   enum cora_messaging_event_id_type
   {
      // Identifies the event when a message is being sent
      cora_messaging_message = 1,

      // identifies the event when a connection is being closed
      cora_messaging_cancelled = 2, 
      // the server
   };

   
   ////////////////////////////////////////////////////////////
   // enum cora_messaging_rtn_type
   //
   // Describes valid return values that can be returned by a messaging callback function or by
   // cora_messaging_call_forward().
   ////////////////////////////////////////////////////////////
   enum cora_messaging_rtn_type
   {
      // The event was processed successfully
      cora_messaging_rtn_accepted = 1,

      // an invalid connection identifier was specified
      cora_messaging_rtn_invalid_connection_id = 2,

      // an invalid event identifier was specified
      cora_messaging_rtn_invalid_event_id = 3,
   };


   ////////////////////////////////////////////////////////////
   // typedef cora_messaging_callback_type
   //
   // Defines the type for the function that should be passed to cora_start_connection() to begin a
   // new connection. This function will respond to message events and notification of shutdown. The
   // parameters have the following significance:
   //
   //  connection_id -- Identifies the connection. Corresponds with the return value of
   //                   cora_start_connection() 
   //
   //  event_id      -- Identifies the type of event that is being reported. Must be one of the
   //                   values described in the cora_messaging_event_id_type enumeration
   //
   //  message_body_len -- Identifies the length of the message (if any) that is to follow. This
   //                      value will be zero if the event_id parameter is anything other than
   //                      cora_messaging_message.
   //
   //  message_body -- Contains the message bytes if the value of event_id is equal to
   //                  cora_messaging_message. The message will begin with the session number since
   //                  the message length is already being supplied as a parameter. This value will
   //                  be NULL if event_id is not equal to cora_messaging_message. The callee is
   //                  responsible for calling cora_messaging_delete_buffer() with this pointer
   //                  specified as its parameter when it is finished with the pointer.  The callee
   //                  can do what it needs with the pointer but should treat it as read-only
   //                  memory.  
   //           
   //
   // The return value will be one of the values described in the cora_messaging_rtn_type
   // enumeration. 
   ////////////////////////////////////////////////////////////
   typedef int (__stdcall *cora_messaging_callback_type)(
#ifdef _WIN32
      uint4 connection_id,
#else
      uint8 connection_id,
#endif
      uint4 event_id,
      uint4 message_body_len,
      void const *message_body);

   
   ////////////////////////////////////////////////////////////
   // cora_start_connection
   //
   // Creates a new CSI messaging connection with the server by registering a call-back function to
   // receive messages and notifications. This function will only succeed when the server is in a
   // started state. The return value will be a unique, positive, non-zero integer that will be used
   // to identify the connection until the connection is released by the client or by the server. If
   // the server is in a non-started state, the return value will be zero.
   ////////////////////////////////////////////////////////////
#ifdef _WIN32
   uint4 __stdcall cora_start_connection(cora_messaging_callback_type call_back);
#else
   uint8 __stdcall cora_start_connection(cora_messaging_callback_type call_back, void* coralib_module);
#endif


   ////////////////////////////////////////////////////////////
   // cora_messaging_buffer_delete
   //
   // Called by the client when it is finished with the message_body pointer and it can now be
   // deleted. 
   ////////////////////////////////////////////////////////////
   void __stdcall cora_messaging_buffer_delete(void const *buffer);

   
   ////////////////////////////////////////////////////////////
   // cora_messaging_call_forward
   //
   // Used by the client to send events associated with the connection to the server. The parameters
   // and return value are the same as those described in  cora_messaging_callback_type().
   ////////////////////////////////////////////////////////////
   int __stdcall cora_messaging_call_forward(
#ifdef _WIN32
      uint4 connection_id,
#else
      uint8 connection_id,
#endif
      uint4 event_id,
      uint4 message_body_len,
      void const *message_body);
   //@endgroup 
}

#endif
