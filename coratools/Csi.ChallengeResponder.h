/* Csi.ChallengeResponder.h

   Copyright (C) 2008, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 16 April 2008
   Last Change: Wednesday 13 April 2011
   Last Commit: $Date: 2011-04-13 14:41:11 -0600 (Wed, 13 Apr 2011) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_ChallengeResponder_h
#define Csi_ChallengeResponder_h

#include "OneShot.h"
#include "StrBin.h"
#include "Csi.ByteQueue.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class ChallengeResponderClient
   //
   // Defines the application's interface to the challenge/response protocol.
   // This interface serves two purposes:
   //
   //   - Notififies the application when the authentication is complete or has
   //     failed
   //
   //   - Defines the mechanism where the module can request the application to
   //     write low level output.
   ////////////////////////////////////////////////////////////
   class ChallengeResponder;
   class ChallengeResponderClient: public Csi::InstanceValidator
   {
   public:
      ////////////////////////////////////////////////////////////
      // on_complete
      //
      // Called by the responder when the protocol has successfully concluded
      // or encountered an unrecoverable error. 
      ////////////////////////////////////////////////////////////
      enum outcome_type
      {
         outcome_unknown = 0,
         outcome_success = 1,
         outcome_timed_out_on_challenge = 2,
         outcome_timed_out_on_response = 3,
         outcome_invalid_challenge = 4,
         outcome_invalid_response = 5
      };
      virtual void on_complete(
         ChallengeResponder *responder, outcome_type outcome) = 0;

      ////////////////////////////////////////////////////////////
      // send_data
      //
      // Called by the responder when data needs to be sent on the link.  
      ////////////////////////////////////////////////////////////
      virtual void send_data(
         ChallengeResponder *responder, void const *data, uint4 data_len) = 0;
   };


   ////////////////////////////////////////////////////////////
   // class ChallengeResponder
   //
   // Defines an object that implements the CSI challenge/response protocol as
   // implemented in the CR1000.  The application that uses this object is
   // reponsible for providing the low level I/O (calling on_data() when data
   // has been received and handling the client object's send_data() method
   // invocations).   In order to use this object, the application should
   // create an instance and provide a password, role, and client object.
   //
   // The role will dictate the initial state of the responder.  If the
   // assigned role is as a server, the challenge will be sent immediately.
   // Otherwise, a timer will be set to wait for the challenge.  When the
   // protocol has concluded, the application will be notified through its
   // client object's on_complete() method.
   ////////////////////////////////////////////////////////////
   class ChallengeResponder: public OneShotClient
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      typedef ChallengeResponderClient client_type;
      ChallengeResponder(
         client_type *client_,
         StrAsc const &password_,
         bool is_server_,
         Csi::SharedPtr<OneShot> timer_ = 0);

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~ChallengeResponder();

      ////////////////////////////////////////////////////////////
      // on_data
      //
      // Should be called by the application when data from the link has been
      // recceived.
      ////////////////////////////////////////////////////////////
      void on_data(void const *data, uint4 data_len);
      void on_data(ByteQueue &data);

      ////////////////////////////////////////////////////////////
      // onOneShotFired
      ////////////////////////////////////////////////////////////
      virtual void onOneShotFired(uint4 id);

      ////////////////////////////////////////////////////////////
      // get_is_server
      ////////////////////////////////////////////////////////////
      bool get_is_server() const
      { return is_server; }
      
      ////////////////////////////////////////////////////////////
      // describe_outcome
      ////////////////////////////////////////////////////////////
      static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

   private:
      ////////////////////////////////////////////////////////////
      // do_on_complete
      ////////////////////////////////////////////////////////////
      void do_on_complete(client_type::outcome_type outcome);
      
   private: 
      ////////////////////////////////////////////////////////////
      // client
      ////////////////////////////////////////////////////////////
      client_type *client;

      ////////////////////////////////////////////////////////////
      // password
      ////////////////////////////////////////////////////////////
      StrAsc password;

      ////////////////////////////////////////////////////////////
      // timer
      ////////////////////////////////////////////////////////////
      Csi::SharedPtr<OneShot> timer;

      ////////////////////////////////////////////////////////////
      // is_server
      //
      // Identifies whether this responder is acting in the role of a server
      // (true) or as a client (false). 
      ////////////////////////////////////////////////////////////
      bool is_server;

      ////////////////////////////////////////////////////////////
      // challenge_timer
      //
      // Identifies the timer used to wait for a challenge (used in a client
      // role). 
      ////////////////////////////////////////////////////////////
      uint4 challenge_timer;

      ////////////////////////////////////////////////////////////
      // response_timer
      //
      // Identifies the timer used to wait for a response.
      ////////////////////////////////////////////////////////////
      uint4 response_timer;

      ////////////////////////////////////////////////////////////
      // start_challenge_timer
      //
      // Identifies the timer that is used to start sending the challenge.
      // This timer provides a delay between the construction of this object
      // and the communication attempts it may make. 
      ////////////////////////////////////////////////////////////
      uint4 start_challenge_timer;

      ////////////////////////////////////////////////////////////
      // server_rand
      ////////////////////////////////////////////////////////////
      byte server_rand[4];

      ////////////////////////////////////////////////////////////
      // rx_buffer
      ////////////////////////////////////////////////////////////
      StrBin rx_buffer;
   };
};


#endif
