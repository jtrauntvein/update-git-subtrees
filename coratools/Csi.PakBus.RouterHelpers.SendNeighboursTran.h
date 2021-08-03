/* Csi.PakBus.RouterHelpers.SendNeighboursTran.h

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 29 March 2002
   Last Change: Thursday 29 November 2012
   Last Commit: $Date: 2012-11-30 10:08:16 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_RouterHelpers_SendNeighboursTran_h
#define Csi_PakBus_RouterHelpers_SendNeighboursTran_h

#include "Csi.PakBus.RouterHelpers.h"
#include "Csi.PakBus.PakBusTran.h"
#include "Csi.SharedPtr.h"


namespace Csi
{
   namespace PakBus
   {
      namespace RouterHelpers
      {
         //@group class forward declarations
         class router_type;
         //@endgroup

         
         ////////////////////////////////////////////////////////////
         // class SendNeighboursTran
         ////////////////////////////////////////////////////////////
         class SendNeighboursTran: public PakBusTran
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            //////////////////////////////////////////////////////////// 
            SendNeighboursTran(
               Router *router,
               Csi::SharedPtr<OneShot> &timer,
               Csi::SharedPtr<router_type> &router_entry_);

            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            virtual void start();

            ////////////////////////////////////////////////////////////
            // on_focus_start
            ////////////////////////////////////////////////////////////
            virtual void on_focus_start();

            ////////////////////////////////////////////////////////////
            // get_transaction_description
            ////////////////////////////////////////////////////////////
            virtual void get_transaction_description(std::ostream &out);

            ////////////////////////////////////////////////////////////
            // router_sponsored
            ////////////////////////////////////////////////////////////
            virtual bool router_sponsored() const
            { return true; }

         protected:
            ////////////////////////////////////////////////////////////
            // on_failure
            ////////////////////////////////////////////////////////////
            virtual void on_failure(failure_type failure);

            ////////////////////////////////////////////////////////////
            // on_pakctrl_message
            ////////////////////////////////////////////////////////////
            virtual void on_pakctrl_message(pakctrl_message_handle &message);

            ////////////////////////////////////////////////////////////
            // on_sending_message
            //
            // Overloaded here to fill in the message with the most recent information.
            ////////////////////////////////////////////////////////////
            virtual void on_sending_message(message_handle &message);

         private:
            ////////////////////////////////////////////////////////////
            // router_entry
            //
            // The router that we should be performing the transaction with
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<router_type> router_entry;
         };
      };
   };
};


#endif

