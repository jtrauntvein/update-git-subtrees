/* Cora.Sec2.Sec2Base.h

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 23 December 2002
   Last Change: Monday 04 January 2016
   Last Commit: $Date: 2016-01-04 13:10:08 -0600 (Mon, 04 Jan 2016) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Sec2_Sec2Base_h
#define Cora_Sec2_Sec2Base_h

#include "Cora.ClientBase.h"
#include "Cora.Sec2.Defs.h"


namespace Cora
{
   namespace Sec2
   {
      ////////////////////////////////////////////////////////////
      // class Sec2Base
      //
      // Defines a base class on which LoggerNet client components can be built that access the
      // server's security interface.
      ////////////////////////////////////////////////////////////
      class Sec2Base: public ClientBase
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Sec2Base();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Sec2Base();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         virtual void start(router_handle &router);
         virtual void start(ClientBase *other_client);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // get_sec2_access_level
         ////////////////////////////////////////////////////////////
         uint4 get_sec2_access_level() const
         { return sec2_access_level; }

      protected:
         //@group pure virtual methods
         ////////////////////////////////////////////////////////////
         // on_sec2base_ready
         //
         // Called when the session with the server security interface has been established.
         ////////////////////////////////////////////////////////////
         virtual void on_sec2base_ready() = 0;

         ////////////////////////////////////////////////////////////
         // on_sec2base_failure
         ////////////////////////////////////////////////////////////
         enum sec2base_failure_type
         {
            sec2base_failure_unknown,
            sec2base_failure_logon,
            sec2base_failure_session,
            sec2base_failure_unsupported,
            sec2base_failure_security,
         };
         virtual void on_sec2base_failure(sec2base_failure_type failure) = 0;

         /**
          * Formats the specified failure code.
          */
         static void format_failure(std::ostream &out, sec2base_failure_type failure);
         
         //@endgroup

         //@group methods overloaded from ClientBase
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();

         ////////////////////////////////////////////////////////////
         // onNetSesBroken
         ////////////////////////////////////////////////////////////
         virtual void onNetSesBroken(
            Csi::Messaging::Router *rtr,
            uint4 session_no,
            uint4 reason,
            char const *why);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);
         //@endgroup

      protected:
         ////////////////////////////////////////////////////////////
         // sec2_session
         ////////////////////////////////////////////////////////////
         uint4 sec2_session;

      private:
         ////////////////////////////////////////////////////////////
         // open_transaction
         ////////////////////////////////////////////////////////////
         uint4 open_transaction;

         ////////////////////////////////////////////////////////////
         // sec2_access_level
         ////////////////////////////////////////////////////////////
         uint4 sec2_access_level;
      };
   };
};


#endif
