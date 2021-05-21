/* Csi.ChallengeResponder.cpp

   Copyright (C) 2008, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 16 April 2008
   Last Change: Wednesday 19 April 2017
   Last Commit: $Date: 2017-04-19 13:09:43 -0600 (Wed, 19 Apr 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.ChallengeResponder.h"
#include "Csi.Utils.h"
#include "coratools.strings.h"
#include "Csi.Digest.h"
#include <cstdlib>


namespace Csi
{
   namespace
   {
      uint4 const max_challenge_time = 120000; // allow two minutes for the server to send a
                                               // challenge
      uint4 const max_response_time = 120000;  // allow two minutes for the client to send a
                                               // response.
      uint4 const md5_digest_size = Csi::Md5Digest::digest_size;
      uint4 const authenticate_size = sizeof(uint4) + md5_digest_size;
   };
   
   
   ////////////////////////////////////////////////////////////
   // class ChallengeResponder definitions
   ////////////////////////////////////////////////////////////
   ChallengeResponder::ChallengeResponder(
      client_type *client_,
      StrAsc const &password_,
      bool is_server_,
      Csi::SharedPtr<OneShot> timer_):
      timer(timer_),
      is_server(is_server_),
      client(client_),
      challenge_timer(0),
      response_timer(0),
      password(password_),
      start_challenge_timer(0)
   {
      // if we are a server, we will need to send the challenge immediately.  Otherwise, we will
      // need to set the timer to wait for the challenge
      if(timer == 0)
         timer.bind(new OneShot);
      if(is_server)
         start_challenge_timer = timer->arm(this, 10);
      else
         challenge_timer = timer->arm(this, max_challenge_time);
   } // constructor


   ChallengeResponder::~ChallengeResponder()
   {
      if(challenge_timer)
         timer->disarm(challenge_timer);
      if(response_timer)
         timer->disarm(response_timer);
      if(start_challenge_timer)
         timer->disarm(start_challenge_timer);
      timer.clear();
   } // destructor


   void ChallengeResponder::on_data(ByteQueue &data)
   {
      if(start_challenge_timer == 0)
      {
         char buff[authenticate_size];
         uint4 count(data.pop(buff, authenticate_size));
         on_data(buff, count);
      }
   } // on_data

   
   void ChallengeResponder::on_data(void const *data, uint4 data_len)
   {
      if(start_challenge_timer == 0)
      {
         rx_buffer.append(data, data_len);
         if(rx_buffer.length() >= authenticate_size)
         {
            // we must verify the digest attached to the message.  How this is done depends upon
            // whether this responder is configured as a client or server.
            Csi::Md5Digest md5;
            byte my_digest[md5_digest_size];
            
            md5.add(rx_buffer.getContents(), sizeof(uint4));
            if(is_server)
               md5.add(server_rand, sizeof(server_rand));
            md5.add(password.c_str(), password.length());
            memcpy(my_digest, md5.final(), sizeof(my_digest));
            
            // we can now compare the calculated digest with the transmitted digest.  if they match, we
            // can go on, otherwise, we must quit
            int rcd = memcmp(my_digest, rx_buffer.getContents() + sizeof(server_rand), md5_digest_size);
            if(rcd == 0)
            {
               if(!is_server)
               {
                  // we need to generate our own random number
                  uint4 rand_value = std::rand() + (std::rand() << 16);
                  byte client_rand[sizeof(server_rand)];
                  memcpy(client_rand, &rand_value, sizeof(client_rand));
                  
                  // if we are a client, we must still send the response and a digest for that response.
                  byte response[authenticate_size];
                  
                  memcpy(response, client_rand, sizeof(client_rand));
                  md5.reset();
                  md5.add(client_rand, sizeof(client_rand));
                  md5.add(rx_buffer.getContents(), sizeof(server_rand));
                  md5.add(password.c_str(), password.length());
                  memcpy(response + sizeof(client_rand), md5.final(), md5_digest_size);
                  if(client_type::is_valid_instance(client))
                  {
                     client->send_data(this, response, sizeof(response));
                     do_on_complete(client_type::outcome_success);
                  }
                  else
                     do_on_complete(client_type::outcome_unknown);
               }
               else
                  do_on_complete(client_type::outcome_success);
            }
            else
               do_on_complete(is_server ? client_type::outcome_invalid_response : client_type::outcome_invalid_challenge);
         }
      }
      else
         do_on_complete(client_type::outcome_invalid_response);
   } // on_data


   void ChallengeResponder::onOneShotFired(uint4 id)
   {
      if(id == response_timer)
      {
         // we are the server waiting for a response
         response_timer = 0;
         do_on_complete(client_type::outcome_timed_out_on_response);
      }
      else if(id == challenge_timer)
      {
         challenge_timer = 0;
         do_on_complete(client_type::outcome_timed_out_on_challenge);
      }
      else if(id == start_challenge_timer)
      {
         // we need to generate the server random.  We could use the boost or TR1 random but I'm too
         // lazy right now to figure those out.  For the time being, we'll suffice with calling
         // rand() twice.  This will compensate for some implementations that peg RAND_MAX at
         // 32767.
         uint4 rand_val = std::rand() + (std::rand() << 16);
         start_challenge_timer = 0;
         memcpy(server_rand, &rand_val, sizeof(server_rand));

         // we can now generate the challenge
         byte challenge[authenticate_size];
         Csi::Md5Digest md5;

         memcpy(challenge, server_rand, sizeof(server_rand));
         md5.add(server_rand, sizeof(server_rand));
         md5.add(password.c_str(), password.length());
         memcpy(challenge + sizeof(server_rand), md5.final(), md5.digest_size);
         response_timer = timer->arm(this, max_response_time);
         client->send_data(this, challenge, sizeof(challenge));
      }
   } // onOneShotFired


   void ChallengeResponder::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
   {
      using namespace ChallengeResponderStrings;
      switch(outcome)
      {
      case client_type::outcome_success:
         out << my_strings[strid_outcome_success];
         break;
         
      case client_type::outcome_timed_out_on_challenge:
         out << my_strings[strid_outcome_timed_out_on_challenge];
         break;
         
      case client_type::outcome_timed_out_on_response:
         out << my_strings[strid_outcome_timed_out_on_response];
         break;
         
      case client_type::outcome_invalid_challenge:
         out << my_strings[strid_outcome_invalid_challenge];
         break;
         
      case client_type::outcome_invalid_response:
         out << my_strings[strid_outcome_invalid_response];
         break;
         
      default:
         out << my_strings[strid_outcome_unknown];
         break;
      }
   } // describe_outcome
   

   void ChallengeResponder::do_on_complete(client_type::outcome_type outcome)
   {
      client_type *client = this->client;
      this->client = 0;
      if(response_timer)
         timer->disarm(response_timer);
      if(challenge_timer)
         timer->disarm(challenge_timer);
      if(start_challenge_timer)
         timer->disarm(start_challenge_timer);;
      if(client_type::is_valid_instance(client))
         client->on_complete(this, outcome);
   } // do_on_complete
};

