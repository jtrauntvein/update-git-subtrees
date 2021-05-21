/* Csi.Rfc2217.Defs.h

   Copyright (C) 2011, 2011 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Tuesday 22 February 2011
   Last Change: Tuesday 26 April 2011
   Last Commit: $Date: 2011-04-26 13:30:38 -0600 (Tue, 26 Apr 2011) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Rfc2217_Defs_h
#define Csi_Rfc2217_Defs_h


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // namespace Rfc2217
   //
   // This namespace encapsulates definitions and classes that are used to
   // support the Serial Control Protocol as defined in RFC2217 which, in turn,
   // extends RFC854 (TelNet Protocol). 
   //////////////////////////////////////////////////////////// 
   namespace Rfc2217
   {
      ////////////////////////////////////////////////////////////
      // enum code_type
      ////////////////////////////////////////////////////////////
      enum code_type
      {
         code_telnet_iac = 255,
         code_telnet_will = 251,
         code_telnet_wont = 252,
         code_telnet_do = 253,
         code_telnet_dont = 254,
         code_telnet_sb = 250,
         code_telnet_se = 240,
         code_serial_option = 44,
         code_serial_signature = 0,
         code_serial_set_baud = 1,
         code_serial_set_datasize = 2,
         code_serial_set_parity = 3,
         code_serial_set_stopsize = 4,
         code_serial_set_control = 5,
         code_serial_notify_linestate = 6,
         code_serial_notify_modemstate = 7,
         code_serial_flowcontrol_suspend = 8,
         code_serial_flowcontrol_resume = 9,
         code_serial_set_linestate_mask = 10,
         code_serial_set_modemstate_mask = 11,
         code_serial_purge_data = 12,
         code_telnet_cr = 13
      };

      
      ////////////////////////////////////////////////////////////
      // enum datasize_option_type
      ////////////////////////////////////////////////////////////
      enum datasize_option_type
      {
         datasize_5 = 5,
         datasize_6 = 6,
         datasize_7 = 7,
         datasize_8 = 8
      };


      ////////////////////////////////////////////////////////////
      // enum parity_option_type
      ////////////////////////////////////////////////////////////
      enum parity_option_type
      {
         parity_request_current = 0,
         parity_none = 1,
         parity_odd = 2,
         parity_even = 3,
         parity_mark = 4,
         parity_space = 5
      };


      ////////////////////////////////////////////////////////////
      // enum stopsize_option_type
      ////////////////////////////////////////////////////////////
      enum stopsize_option_type
      {
         stopsize_request_current = 0,
         stopsize_1 = 1,
         stopsize_2 = 2,
         stopsize_1_5 = 3
      };


      ////////////////////////////////////////////////////////////
      // enum control_option_type
      ////////////////////////////////////////////////////////////
      enum control_option_type
      {
         control_request_flow_control = 0,
         control_no_flow_control_both = 1,
         control_xon_flow_control_both = 2,
         control_hardware_flow_control_both = 3,
         control_request_break_state = 4,
         control_set_break_on = 5,
         control_set_break_off = 6,
         control_request_dtr_state = 7,
         control_set_dtr_on = 8,
         control_set_dtr_off = 9,
         control_request_rts_state = 10,
         control_set_rts_on = 11,
         control_set_rts_off = 12,
         control_request_flow_control_inbound = 13,
         control_no_flow_control_inbound = 14,
         control_xon_flow_control_inbound = 15,
         control_hardware_flow_control_inbound = 16,
         control_dcd_flow_control_both = 17,
         control_dtr_flow_control_inbound = 18,
         control_dsr_flow_control_both = 19
      };


      ////////////////////////////////////////////////////////////
      // enum linestate_mask_type
      ////////////////////////////////////////////////////////////
      enum linestate_mask_type
      {
         linestate_timeout_error = 128,
         linestate_transfer_shift_empty = 64,
         linestate_transfer_holding_empty = 32,
         linestate_break_detect_error = 16,
         linestate_framing_error = 8,
         linestate_parity_error = 4,
         linestate_overrun_error = 2,
         linestate_data_ready = 1
      };


      ////////////////////////////////////////////////////////////
      // enum modemstate_mask_type
      ////////////////////////////////////////////////////////////
      enum modemstate_mask_type
      {
         modemstate_cd = 128,
         modemstate_ring = 64,
         modemstate_dsr = 32,
         modemstate_cts = 16,
         modemstate_delta_cd = 8,
         modemstate_trailing_edge_ring = 4,
         modemstate_delta_dsr = 2,
         modemstate_delta_cts = 1
      };


      ////////////////////////////////////////////////////////////
      // enum purge_option_type
      ////////////////////////////////////////////////////////////
      enum purge_option_type
      {
         purge_rx_buff = 1,
         purge_tx_buff = 2,
         purge_both = 3
      };
   };
};

#endif
