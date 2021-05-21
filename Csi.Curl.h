/* Csi.Curl.h

   Copyright (C) 2014, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Saturday 06 December 2014
   Last Change: Saturday 06 December 2014
   Last Commit: $Date:$
   Last Changed by: $Author:$

*/

#ifndef Csi_Curl_h
#define Csi_Curl_h

namespace Csi
{
   namespace Curl
   {
      /**
       * Lists the possible error codes as an enumeration
       */
      enum
      {
         curle_ok = 0,
         curle_unsupported_protocol = 1,
         curle_failed_init = 2,
         curle_url_malformat = 3,
         curle_not_built_in = 4,
         curle_couldnt_resolve_proxy = 5,
         curle_couldnt_resolve_host = 6,
         curle_couldnt_connect = 7,
         curle_ftp_weird_server_reply = 8,
         curle_remote_access_denied = 9,
         curle_ftp_accept_failed = 10,
         curle_ftp_weird_pass_reply = 11,
         curle_ftp_accept_timeout = 12,
         curle_ftp_weird_pasv_reply = 13,
         curle_ftp_weird_227_format = 14,
         curle_ftp_cant_get_host = 15,
         curle_http2 = 16,
         curle_ftp_couldnt_set_type = 17,
         curle_partial_file = 18,
         curle_ftp_couldnt_retr_file = 19,
         curle_quote_error = 21,
         curle_http_returned_error = 22,
         curle_write_error = 23,
         curle_upload_failed = 25,
         curle_read_error = 26,
         curle_out_of_memory = 27,
         curle_operation_timed_out = 28,
         curle_ftp_port_failed = 30,
         curle_ftp_couldnt_use_rest = 31,
         curle_ftp_range_error = 33,
         curle_http_post_error = 34,
         curle_ssl_connect_error = 35,
         curle_bad_download_resume = 36,
         curle_couldnt_read_file = 37,
         curle_ldap_cannot_bind = 38,
         curle_ldap_search_failed = 39,
         curle_functin_not_found = 41,
         curle_aborted_by_callback = 42,
         curle_bad_function_argument = 43,
         curle_interface_failed = 45,
         curle_too_many_redirects = 47,
         curle_unknown_option = 48,
         curle_telnet_option_syntax = 49,
         curle_peer_failed_verification = 51,
         curle_got_nothing = 52,
         curle_ssl_engine_not_found = 53,
         curle_ssl_engine_set_failed = 54,
         curle_send_error = 55,
         curle_recv_error = 56,
         curle_ssl_cert_problem = 58,
         curle_ssl_cipher = 59,
         curle_ssl_cacert = 60,
         curle_bad_content_encoding = 61,
         curle_ldap_invalid_url = 62,
         curle_filesize_exceeded = 63,
         curle_use_ssl_failed = 64,
         curle_send_fail_rewind = 65,
         curle_ssl_engine_init_failed = 66,
         curle_logon_denied = 67,
         curle_tfpt_not_found = 68,
         curle_tfpt_perm = 69,
         curle_remote_disk_full = 70,
         curle_tfpt_illegal = 71,
         curle_tfpt_unknown_id = 72,
         curle_remote_file_exists = 73,
         curle_tfpt_no_such_user = 74,
         curle_conv_failed = 75,
         curle_conv_required = 76,
         curle_ssl_cacert_bad_file = 77,
         curle_remote_file_not_found = 78,
         curle_ssh = 79,
         curle_ssl_shutdown_failed = 80,
         curle_again = 81,
         curle_ssl_crl_bad_file = 82,
         curle_ssl_issuer_error = 83,
         curle_ftp_pret_failed = 84,
         curle_rtsp_cseq_error = 85,
         curle_rtsp_session_error = 86,
         curle_ftp_bad_file_list = 87,
         curle_chunk_failed = 88,
         curle_no_connection_available = 89,
      };

      
      /**
       * @return Returns the error message associated with the specified curl return code.
       *
       * @param curl_resp Specifies the exit code returned from the curl program.
       */
      char const *desc_for_response(int curl_resp);
   };
};


#endif
