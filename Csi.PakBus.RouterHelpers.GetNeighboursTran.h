/* Csi.PakBus.RouterHelpers.GetNeighboursTran.h

   Copyright (C) 2002, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 01 April 2002
   Last Change: Monday 28 November 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_PakBus_RouterHelpers_GetNeighboursTran_h
#define Csi_PakBus_RouterHelpers_GetNeighboursTran_h

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
         // class GetNeighboursTran
         ////////////////////////////////////////////////////////////
         class GetNeighboursTran: public PakBusTran
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GetNeighboursTran(
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

         private:
            ////////////////////////////////////////////////////////////
            // router_entry
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<router_type> router_entry;
         };
      };
   };
};

#endif

