/* Csi.PakBus.RouterHelpers.HelloTran.h

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 23 March 2002
   Last Change: Thursday 29 November 2012
   Last Commit: $Date: 2012-11-30 10:08:16 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_RouterHelpers_HelloTran_h
#define Csi_PakBus_RouterHelpers_HelloTran_h

#include "Csi.PakBus.RouterHelpers.h"
#include "Csi.PakBus.PakBusTran.h"
#include "Csi.SharedPtr.h"


namespace Csi
{
   namespace PakBus
   {
      namespace RouterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class HelloTran
         ////////////////////////////////////////////////////////////
         class HelloTran: public PakBusTran
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            HelloTran(
               Router *router,
               Csi::SharedPtr<OneShot> &timer,
               Csi::SharedPtr<neighbour_type> &neighbour_,
               bool send_if_dialed_ = false,
               PakBusTran *replaced_other_ = 0);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~HelloTran();

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
            // is_special_hello
            ////////////////////////////////////////////////////////////
            virtual bool is_special_hello(PakBusTran *other) const
            { return other == replaced_other; }

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
            // neighbour
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<neighbour_type> neighbour;

            ////////////////////////////////////////////////////////////
            // send_if_dialed
            //
            // Controls whether the hello message will be sent even if the port
            // is dialed and inactive.  
            ////////////////////////////////////////////////////////////
            bool send_if_dialed;

            ////////////////////////////////////////////////////////////
            // replaced_other
            //
            // Pointer to another transaction that this hello might have "replaced".
            ////////////////////////////////////////////////////////////
            PakBusTran *replaced_other;
         };
      };
   };
};


#endif

