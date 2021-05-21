/* Csi.Curl.cpp

   Copyright (C) 2014, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Saturday 06 December 2014
   Last Change: Saturday 06 December 2014
   Last Commit: $Date:$
   Last Changed by: $Author:$

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Curl.h"


namespace Csi
{
   namespace Curl
   {
      struct resp_code
      {
         int const code;
         char const *description;
      };
      struct resp_code const response_codes[] = 
      {
         { curle_ok, "ok" },
         { curle_unsupported_protocol, "unsupported_protocol" },
         { curle_failed_init, "failed_init" },
         { curle_url_malformat, "url_malformat" },
         { curle_not_built_in, "not_built_in" },
         { curle_couldnt_resolve_proxy, "couldnt_resolve_proxy" },
         { curle_couldnt_resolve_host, "couldnt_resolve_host" },
         { curle_couldnt_connect, "couldnt_connect" },
         { curle_ftp_weird_server_reply, "ftp_weird_server_reply" },
         { curle_remote_access_denied, "remote_access_denied" },
         { curle_ftp_accept_failed, "ftp_accept_failed" },
         { curle_ftp_weird_pass_reply, "ftp_weird_pass_reply" },
         { curle_ftp_accept_timeout, "ftp_accept_timeout" },
         { curle_ftp_weird_pasv_reply, "ftp_weird_pasv_reply" },
         { curle_ftp_weird_227_format, "ftp_weird_227_format" },
         { curle_ftp_cant_get_host, "ftp_cant_get_host" },
         { curle_http2, "http2" },
         { curle_ftp_couldnt_set_type, "ftp_couldnt_set_type" },
         { curle_partial_file, "partial_file" },
         { curle_ftp_couldnt_retr_file, "ftp_couldnt_retr_file" },
         { curle_quote_error, "quote_error" },
         { curle_http_returned_error, "http_returned_error" },
         { curle_write_error, "write_error" },
         { curle_upload_failed, "upload_failed" },
         { curle_read_error, "read_error" },
         { curle_out_of_memory, "out_of_memory" },
         { curle_operation_timed_out, "operation_timed_out" },
         { curle_ftp_port_failed, "ftp_port_failed" },
         { curle_ftp_couldnt_use_rest, "ftp_couldnt_use_rest" },
         { curle_ftp_range_error, "ftp_range_error" },
         { curle_http_post_error, "http_post_error" },
         { curle_ssl_connect_error, "ssl_connect_error" },
         { curle_bad_download_resume, "bad_download_resume" },
         { curle_couldnt_read_file, "couldnt_read_file" },
         { curle_ldap_cannot_bind, "ldap_cannot_bind" },
         { curle_ldap_search_failed, "ldap_search_failed" },
         { curle_functin_not_found, "functin_not_found" },
         { curle_aborted_by_callback, "aborted_by_callback" },
         { curle_bad_function_argument, "bad_function_argument" },
         { curle_interface_failed, "interface_failed" },
         { curle_too_many_redirects, "too_many_redirects" },
         { curle_unknown_option, "unknown_option" },
         { curle_telnet_option_syntax, "telnet_option_syntax" },
         { curle_peer_failed_verification, "peer_failed_verification" },
         { curle_got_nothing, "got_nothing" },
         { curle_ssl_engine_not_found, "ssl_engine_not_found" },
         { curle_ssl_engine_set_failed, "ssl_engine_set_failed" },
         { curle_send_error, "send_error" },
         { curle_recv_error, "recv_error" },
         { curle_ssl_cert_problem, "ssl_cert_problem" },
         { curle_ssl_cipher, "ssl_cipher" },
         { curle_ssl_cacert, "ssl_cacert" },
         { curle_bad_content_encoding, "bad_content_encoding" },
         { curle_ldap_invalid_url, "ldap_invalid_url" },
         { curle_filesize_exceeded, "filesize_exceeded" },
         { curle_use_ssl_failed, "use_ssl_failed" },
         { curle_send_fail_rewind, "send_fail_rewind" },
         { curle_ssl_engine_init_failed, "ssl_engine_init_failed" },
         { curle_logon_denied, "logon_denied" },
         { curle_tfpt_not_found, "tfpt_not_found" },
         { curle_tfpt_perm, "tfpt_perm" },
         { curle_remote_disk_full, "remote_disk_full" },
         { curle_tfpt_illegal, "tfpt_illegal" },
         { curle_tfpt_unknown_id, "tfpt_unknown_id" },
         { curle_remote_file_exists, "remote_file_exists" },
         { curle_tfpt_no_such_user, "tfpt_no_such_user" },
         { curle_conv_failed, "conv_failed" },
         { curle_conv_required, "conv_required" },
         { curle_ssl_cacert_bad_file, "ssl_cacert_bad_file" },
         { curle_remote_file_not_found, "remote_file_not_found" },
         { curle_ssh, "ssh" },
         { curle_ssl_shutdown_failed, "ssl_shutdown_failed" },
         { curle_again, "again" },
         { curle_ssl_crl_bad_file, "ssl_crl_bad_file" },
         { curle_ssl_issuer_error, "ssl_issuer_error" },
         { curle_ftp_pret_failed, "ftp_pret_failed" },
         { curle_rtsp_cseq_error, "rtsp_cseq_error" },
         { curle_rtsp_session_error, "rtsp_session_error" },
         { curle_ftp_bad_file_list, "ftp_bad_file_list" },
         { curle_chunk_failed, "chunk_failed" },
         { curle_no_connection_available, "no_connection_available" },
         { -1, 0 },
      };
      

      
      char const *desc_for_response(int curl_resp)
      {
         char const *rtn("");
         for(int i = 0; response_codes[i].description != 0; ++i)
         {
            if(curl_resp == response_codes[i].code)
            {
               rtn = response_codes[i].description;
               break;
            }
         }
         return rtn;
      } // desc_for_response
   };
};

