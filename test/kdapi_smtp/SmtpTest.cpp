// SmtpTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Csi.CommandLine.h"
#include "Csi.HttpClient.h"
#include "MyRequestClient.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include "Csi.Win32Dispatch.h"
#include "Csi.Json.h"
#include "Csi.Hmac.h"
#include "Csi.Base64.h"
#include "Csi.LgrDate.h"
#include "Csi.Utils.h"
#include "Csi.Digest.h"
#include <fstream>
#include <sstream>
#include <array>
#include "Csi.SignedJson.h"
#include "Csi.StrAscStream.h"

/* Curl example to blacklist an email

curl -d {\"email\":\"blacklisted@email.com\"} -H "Content-Type: application/json" -X POST https://localhost:44300/blacklist --insecure

*/

//Localhost test
static StrAsc smtp_base_uri = "https://localhost:44300/smtp/";
static StrAsc blacklist_base_uri = "https://localhost:44300/blacklist/";

//Staging Server
//static StrAsc base_uri = "https://52.174.150.136/smtp";
//static StrAsc base_uri = "https://32e21803aeb74a03a42e4a2cca4a7335.cloudapp.net/smtp";
//Dev Server
//static StrAsc base_uri = "https://kdapidevmessaging.cloudapp.net/smtp";

//Live Production Server
//static StrAsc base_uri = "https://kdapiemail.konectgds.com/smtp";


MyRequestClient *smtp_request_client = nullptr;
MyRequestClient *blacklist_request_client = nullptr;
Csi::SharedPtr<Csi::HttpClient::Request> smtp_request;
Csi::SharedPtr<Csi::HttpClient::Request> blacklist_request;
Csi::SharedPtr<Csi::HttpClient::Connection> http_connection;

struct mime_extension_type
{
   StrAsc const extension;
   StrAsc const content_type;
} mime_extensions[] =
{
   { "htm", "text/html; charset=utf-8" },
   { "html", "text/html; charset=utf-8" },
   { "jpg", "image/jpeg" },
   { "jpeg", "image/jpeg" },
   { "jpe",  "image/jpeg" },
   { "png", "image/png" },
   { "gif", "image/gif" },
   { "bmp", "image/bmp" },
   { "tif", "image/tiff" },
   { "tiff", "image/tiff" },
   { "pdf",  "application/pdf" },
   { "swf", "application/x-shockwave-flash" },
   { "mp3", "audio/mpeg" },
   { "mpga", "audio/mpeg" },
   { "mp2", "audio/mpeg" },
   { "kar", "audio/mpeg" },
   { "mid", "audio/midi" },
   { "midi", "audio/midi" },
   { "wav", "audio/x-wav" },
   { "ogg", "audio/ogg" },
   { "au", "audio/basic" },
   { "snd", "audio/basic" },
   { "aif", "audio/x-aiff" },
   { "aifc", "audio/x-aiff" },
   { "aiff", "audio/x-aiff" },
   { "xml", "application/xml" },
   { "css", "text/css" },
   { "js", "application/javascript" },
   { "json", "application/json" },
   { "ico", "image/vnd.microsoft.icon" },
   { "jar", "application/x-java-archive" },
   { "jnlp", "application/java-jnlp-file" },
   { "class", "application/octet-stream" },
   { "mpeg", "video/mp4v-es" },
   { "mpg",  "video/mp4v-es" },
   { "mpe",  "video/mp4v-es" },
   { "qt",   "video/quicktime" },
   { "mov",  "video/quicktime" },
   { "mxu",  "video/vnd.mpegurl" },
   { "flv",  "video/x-flv" },
   { "mp4", "video/mp4" },
   { "m4v", "video/mp4" },
   { "ogv", "video/ogg" },
   { "webm", "video/webm" },
   { "avi", "video/avi" },
   { "txt",  "text/plain" },
   { "dat",  "text/plain" },
   { "svg",  "image/svg+xml" },
   { "svge", "image/svg+xml" },
   { "", "" }
};
StrAsc get_mime_type(StrAsc const &file_name)
{
   StrAsc rtn;
   size_t extension_pos = file_name.rfind(".");
   if(extension_pos < file_name.length())
   {
      // we need to determine whether the extension indicates that the file is compressed
      StrAsc extension;
      file_name.sub(extension, extension_pos + 1, file_name.length());
      for(int i = 0; mime_extensions[i].extension.length() > 0; ++i)
      {
         if(mime_extensions[i].extension == extension)
         {
            rtn = mime_extensions[i].content_type;
            break;
         }
      }
   }
   if(rtn.length() == 0)
      rtn = "application/octet-stream";

   return rtn;
} // get_mime_type


namespace
{
   class MyWatcher: public Csi::HttpClient::ConnectionWatcher
   {
   public:
      virtual void on_log_comment(Csi::HttpClient::Connection *sender, StrAsc const &comment)
      {
         std::cout << "comment:" << comment << "\n";
      }

      virtual void on_data(Csi::HttpClient::Connection *sender, void const *buff, size_t buff_len, bool received)
      {
         StrBin content(buff, buff_len);
         content.append(0);
         if(received)
            std::cout << "received: " << content.getContents() << "\n";
         else
            std::cout << "sent: " << content.getContents() << "\n";
      }
   };
};

void send_email()
{
   /* Test Creds */
   StrAsc device_id = "SN55555-CPPTEST";
   StrAsc konect_id;// = "KID.7D8B1B88-83BB-CCCC-BD0A-7DE8A20A845C";
   StrAsc konect_secret_key;// = "KSK.58397-8167-Ewz9ydYVAUB0Ron9QeF8PZHhRRC6IM8UWud7ri6PQlY";

   /*Example:

   "attachments":[{
      "filename": "mime_dat.dat",
      "type": "text/plain",
      "content": "IlRPQTUiLCJDUjMwMFNlcmllcyIsIkNSMzAwIiwiNCIsIkNSMzAwLlN0ZC4wMS4wMiIsIkNQVTpERU1PX1dFQVRIRVJfU1RBVElPTi5ETEQiLCIzNjQ3IiwiTUlOVVRFIg0KIlRJTUVTVEFNUCIsIlJFQ09SRCIsIlRlbXBfRl9NaW4iLCJUZW1wX0ZfVE1uIiwiVGVtcF9GX01heCIsIlRlbXBfRl9UTXgiLCJUZW1wX0ZfQXZnIiwiV2luZF9NUEgiLCJXaW5kX0RlZyIsIlNvbGFyX1JhZF9BdmciLCJSYWluX01NX1RvdCIsIkJhdHRfVl9NaW4iLCJSSF9NaW4iLCJSSF9NYXgiLCJSSF9BdmciLCJCUCIsInNldHRhYmxlX3N0ciIsInNldHRhYmxlX2Zsb2F0Iiwic2V0dGFibGVfbG9uZyIsInNldHRhYmxlX2Jvb2xlYW4iLCJPZmZpY2VUZW1wIiwiY291bnQiDQoiVFMiLCJSTiIsIsKwRiIsIsKwRiIsIsKwRiIsIsKwRiIsIsKwRiIsIiIsIiIsIiIsIiIsIiIsIiIsIiIsIiIsIiIsIiIsIiIsIiIsIiIsIiIsIiINCiIiLCIiLCJNaW4iLCJUTW4iLCJNYXgiLCJUTXgiLCJBdmciLCJTbXAiLCJTbXAiLCJBdmciLCJUb3QiLCJNaW4iLCJNaW4iLCJNYXgiLCJBdmciLCJTbXAiLCJTbXAiLCJTbXAiLCJTbXAiLCJTbXAiLCJTbXAiLCJTbXAiDQoiMjAxNi0wNC0xMyAwODoyMDowMCIsMTQyMywwLCIyMDE2LTA0LTEzIDA4OjE5OjAxIiwxMDAsIjIwMTYtMDQtMTMgMDg6MTk6MzkiLDUwLjM3LDI1LDEwNiw0OTAuNCwyOS41MiwxMy4xNiwwLDk5LDQ3LjI4LDI5LCIiLDAsMCwwLDI0LjIzLDM0DQoiMjAxNi0wNC0xMyAwODoyMTowMCIsMTQyNCwwLCIyMDE2LTA0LTEzIDA4OjIwOjE2Iiw5OCwiMjAxNi0wNC0xMyAwODoyMDowMiIsNTMuMTUsMTcsMjgyLDUwNy41LDI4Ljk4LDEzLjE2LDAsMTAwLDUxLjQ4LDMwLCIiLDAsMCwwLDI0LjIzLDk0DQoiMjAxNi0wNC0xMyAwODoyMjowMCIsMTQyNSwwLCIyMDE2LTA0LTEzIDA4OjIxOjAyIiwxMDAsIjIwMTYtMDQtMTMgMDg6MjE6MDEiLDU3LjA1LDQ5LDI3OSw0OTIuOSwyNi4wMSwxMy4xNSwyLDEwMCw1MC44MywzMSwiIiwwLDAsMCwyNC4yNSwxNTQNCiIyMDE2LTA0LTEzIDA4OjIzOjAwIiwxNDI2LDgsIjIwMTYtMDQtMTMgMDg6MjI6NTAiLDEwMCwiMjAxNi0wNC0xMyAwODoyMjoyMSIsNjAuODcsNDksNzEsNTA4LjYsMjguODYsMTMuMTYsMCwxMDAsNDcuNzcsMzEsIiIsMCwwLDAsMjQuMjIsMjE0DQoiMjAxNi0wNC0xMyAwODoyNDowMCIsMTQyNyw1LCIyMDE2LTA0LTEzIDA4OjIzOjM3IiwxMDAsIjIwMTYtMDQtMTMgMDg6MjM6NDciLDUzLjA1LDU5LDE3OCw1NDYuMSwzMS42NSwxMy4xNiwwLDk0LDQ1LjU3LDI5LCIiLDAsMCwwLDI0LjIyLDI3NA0KIjIwMTYtMDQtMTMgMDg6MjU6MDAiLDE0MjgsMCwiMjAxNi0wNC0xMyAwODoyNDoyOSIsMTAwLCIyMDE2LTA0LTEzIDA4OjI0OjA3Iiw1My4zNyw5LDMzNiw0NzQuNCwyOC4yOSwxMy4xNiwxLDk4LDQ4LjY4LDI4LCIiLDAsMCwwLDI0LjIzLDMzNA0KIjIwMTYtMDQtMTMgMDg6MjY6MDAiLDE0MjksNCwiMjAxNi0wNC0xMyAwODoyNToxNSIsMTAwLCIyMDE2LTA0LTEzIDA4OjI1OjAzIiw1My45MiwyNiwyNTIsNTgzLjIsMzIuMzQsMTMuMTYsMyw5OCw0Ny42NywzMCwiIiwwLDAsMCwyNC4xOSwzOTQNCiIyMDE2LTA0LTEzIDA4OjI3OjAwIiwxNDMwLDUsIjIwMTYtMDQtMTMgMDg6MjY6MzIiLDk3LCIyMDE2LTA0LTEzIDA4OjI2OjAzIiw0My45NSwzNSwzMzYsNTEyLjksMzAuNTMsMTMuMTYsMCwxMDAsNDYuMzgsMjgsIiIsMCwwLDAsMjQuMiw0NTQNCiIyMDE2LTA0LTEzIDA4OjI4OjAwIiwxNDMxLDIsIjIwMTYtMDQtMTMgMDg6Mjc6NTciLDEwMCwiMjAxNi0wNC0xMyAwODoyNzoyOCIsNTEuOTcsNDIsMTA4LDUwNS42LDI5Ljk2LDEzLjE2LDEsMTAwLDUyLjY4LDI5LCIiLDAsMCwwLDI0LjIxLDUxNA0KIjIwMTYtMDQtMTMgMDg6Mjk6MDAiLDE0MzIsMSwiMjAxNi0wNC0xMyAwODoyODowNCIsOTgsIjIwMTYtMDQtMTMgMDg6Mjg6MDkiLDQ4Ljc1LDYyLDg3LDQ2OSwzMi44NiwxMy4xNiwwLDEwMCw1MC4zLDMwLCIiLDAsMCwwLDI0LjE5LDU3NA0KIjIwMTYtMDQtMTMgMDg6MzA6MDAiLDE0MzMsMiwiMjAxNi0wNC0xMyAwODoyOTowMSIsMTAwLCIyMDE2LTA0LTEzIDA4OjI5OjA5Iiw0OS4xMyw1MSwyOSw0MTUuOCwyNy40OSwxMy4xNiwwLDEwMCw1Mi4zNywzMCwiIiwwLDAsMCwyNC4yLDYzNA0KIjIwMTYtMDQtMTMgMDg6MzE6MDAiLDE0MzQsMCwiMjAxNi0wNC0xMyAwODozMDozMyIsMTAwLCIyMDE2LTA0LTEzIDA4OjMwOjU4Iiw1MC40NSwzLDY0LDU4MC44LDI2Ljk5LDEzLjE2LDMsOTcsNDkuNTgsMzEsIiIsMCwwLDAsMjQuMjIsNjk0DQoiMjAxNi0wNC0xMyAwODozMjowMCIsMTQzNSwwLCIyMDE2LTA0LTEzIDA4OjMxOjAyIiwxMDAsIjIwMTYtMDQtMTMgMDg6MzE6MDgiLDQzLjUsMTYsMTY3LDQ4OCwzMC41NSwxMy4xNiwyLDk2LDQ3LjA1LDI5LCIiLDAsMCwwLDI0LjIsNzU0DQoiMjAxNi0wNC0xMyAwODozMzowMCIsMTQzNiwxLCIyMDE2LTA0LTEzIDA4OjMyOjEyIiwxMDAsIjIwMTYtMDQtMTMgMDg6MzI6MjkiLDQ3LjMsMywxMTEsNDU3LjUsMzQuMTUsMTMuMTYsMCw5Niw0Ny40NywyOCwiIiwwLDAsMCwyNC4yMSw4MTQNCiIyMDE2LTA0LTEzIDA4OjM0OjAwIiwxNDM3LDEsIjIwMTYtMDQtMTMgMDg6MzM6MDYiLDk4LCIyMDE2LTA0LTEzIDA4OjMzOjIyIiw0NC4xNSw2NywxMDcsNTQyLjMsMjcuNjQsMTMuMTYsMCwxMDAsNDUuNTIsMjgsIiIsMCwwLDAsMjQuMjMsODc0DQoiMjAxNi0wNC0xMyAwODozNTowMCIsMTQzOCwwLCIyMDE2LTA0LTEzIDA4OjM0OjE4Iiw5NywiMjAxNi0wNC0xMyAwODozNDozOSIsNTMuNDgsNTUsMjcyLDQxNS44LDMyLjcyLDEzLjE1LDQsMTAwLDUwLjk1LDMxLCIiLDAsMCwwLDI0LjIsOTM0DQoiMjAxNi0wNC0xMyAwODozNjowMCIsMTQzOSwwLCIyMDE2LTA0LTEzIDA4OjM1OjU5IiwxMDAsIjIwMTYtMDQtMTMgMDg6MzU6NTUiLDUwLjE3LDE3LDE0NCw1MTcuMiwyNy4zNSwxMy4xNiwyLDkzLDU1LjE4LDI4LCIiLDAsMCwwLDI0LjIxLDk5NA0KIjIwMTYtMDQtMTMgMDg6Mzc6MDAiLDE0NDAsMiwiMjAxNi0wNC0xMyAwODozNjozMSIsMTAwLCIyMDE2LTA0LTEzIDA4OjM2OjU1Iiw1MS4zNyw2OSwxNTksNDQ0LjIsMjguNTYsMTMuMTYsMiwxMDAsNDYuMTgsMzEsIiIsMCwwLDAsMjQuMjQsMTA1NA0KIjIwMTYtMDQtMTMgMDg6Mzg6MDAiLDE0NDEsMiwiMjAxNi0wNC0xMyAwODozNzoyOSIsOTgsIjIwMTYtMDQtMTMgMDg6Mzc6MzgiLDUzLjA1LDU3LDIzMiw0NTcuMiwyOS43NywxMy4xNiwwLDk5LDU2LjUyLDMxLCIiLDAsMCwwLDI0LjIsMTExNA0KIjIwMTYtMDQtMTMgMDg6Mzk6MDAiLDE0NDIsMiwiMjAxNi0wNC0xMyAwODozODo0OCIsOTksIjIwMTYtMDQtMTMgMDg6Mzg6MDQiLDUwLjg3LDM0LDUzLDU0MS4yLDI2LjM0LDEzLjE2LDEsMTAwLDUxLjczLDMxLCIiLDAsMCwwLDI0LjIsMTE3NA0KIjIwMTYtMDQtMTMgMDg6NDA6MDAiLDE0NDMsNCwiMjAxNi0wNC0xMyAwODozOToyMiIsMTAwLCIyMDE2LTA0LTEzIDA4OjM5OjMwIiw0OS42Myw0MSw4Miw0NTEuMywzMC4zNiwxMy4xNiwxLDEwMCw0OS4zMiwyOSwiIiwwLDAsMCwyNC4yMSwxMjM0DQoiMjAxNi0wNC0xMyAwODo0MTowMCIsMTQ0NCwwLCIyMDE2LTA0LTEzIDA4OjQwOjA2Iiw5NywiMjAxNi0wNC0xMyAwODo0MDozNyIsNTAuOCw1MSwxMSw0NzUuNSwyOS42NywxMy4xNiwzLDk4LDQ3LjQzLDI4LCIiLDAsMCwwLDI0LjE4LDEyOTQNCiIyMDE2LTA0LTEzIDA4OjQyOjAwIiwxNDQ1LDAsIjIwMTYtMDQtMTMgMDg6NDE6NDciLDEwMCwiMjAxNi0wNC0xMyAwODo0MTo0MSIsNDIuOTcsNTksMTkxLDQ2NS44LDI5LjkzLDEzLjE2LDAsMTAwLDQ4LjIsMjgsIiIsMCwwLDAsMjQuMiwxMzU0DQoiMjAxNi0wNC0xMyAwODo0MzowMCIsMTQ0NiwwLCIyMDE2LTA0LTEzIDA4OjQyOjE3Iiw5NSwiMjAxNi0wNC0xMyAwODo0MjoyMiIsNTAuMzcsMjgsMzMxLDQ0OS4yLDMxLjkyLDEzLjE2LDAsMTAwLDQ1LjE4LDI4LCIiLDAsMCwwLDI0LjIsMTQxNA0KIjIwMTYtMDQtMTMgMDg6NDQ6MDAiLDE0NDcsMSwiMjAxNi0wNC0xMyAwODo0Mzo0MyIsOTgsIjIwMTYtMDQtMTMgMDg6NDM6MzEiLDQ5LjAyLDYsMzA5LDQ4My44LDI5LjIyLDEzLjE2LDEsMTAwLDU2Ljc3LDI5LCIiLDAsMCwwLDI0LjE2LDE0NzQNCiIyMDE2LTA0LTEzIDA4OjQ1OjAwIiwxNDQ4LDIsIjIwMTYtMDQtMTMgMDg6NDQ6MjMiLDEwMCwiMjAxNi0wNC0xMyAwODo0NDowNSIsNTEuNzUsNTAsODcsNDUzLjMsMjYuODUsMTMuMTYsMiw5OSw1Mi4yLDI4LCIiLDAsMCwwLDI0LjE5LDE1MzQNCiIyMDE2LTA0LTEzIDA4OjQ2OjAwIiwxNDQ5LDIsIjIwMTYtMDQtMTMgMDg6NDU6NTciLDEwMCwiMjAxNi0wNC0xMyAwODo0NTowNCIsNTIuNDgsNjgsMTE2LDU0OC41LDI2Ljk4LDEzLjE2LDAsOTksNDkuNDMsMzAsIiIsMCwwLDAsMjQuMjEsMTU5NA0K"
   },
   {
      "filename": "mime_jpg.jpg",
      "type": "image/jpeg",
      "content": "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"
   }]

   */

   Csi::Json::ObjectHandle json_doc(new Csi::Json::Object);
   json_doc->set_property_str("messagetype", "smtp");
   json_doc->set_property_str("message", "send-email");
   Csi::Json::ObjectHandle data_obj(new Csi::Json::Object);

   Csi::Json::ArrayHandle to_array(new Csi::Json::Array);

   to_array->push_back("blacklist@mechaminc.com");
   to_array->push_back("tsmecham@gmail.com");
   to_array->push_back("tmecham@campbellsci.com");
   data_obj->set_property("to", to_array.get_handle());

   Csi::Json::ArrayHandle cc_array(new Csi::Json::Array);
   cc_array->push_back("blacklist@mechaminc.com");
   data_obj->set_property("cc", cc_array.get_handle());

   //Csi::Json::ArrayHandle bcc_array(new Csi::Json::Array);
   //bcc_array->push_back("tsmecham@gmail.com");
   //data_obj->set_property("bcc", bcc_array.get_handle());

   data_obj->set_property_str("subject", "Test");
   StrUni message_uni(L"English: How is your day?\r\n");
   message_uni.append(L"Korean: 너 오늘 어때?\r\n");
   message_uni.append(L"Japanese: 今日はどうでしたか？\r\n");
   message_uni.append(L"Bulgarian: Как е денят ти?\r\n");
   message_uni.append(L"Chinese: 你過得好嗎？\r\n");
   message_uni.append(L"Arabic: كيف يومك؟\r\n");
   data_obj->set_property_str("message", message_uni.to_utf8()); //Korean

   bool attach = false;
   if(attach)
   {
      Csi::Json::ArrayHandle attachment_array(new Csi::Json::Array);
      std::array<StrAsc, 1> file_attachments =
      {
         "mime_jpg.jpg"//, "mime_dld.dld", "mime_docx.docx", "mime_pdf.pdf"
      };
      for each(StrAsc file_name in file_attachments)
      {
         std::ifstream is(file_name.c_str(), std::ifstream::binary);
         if(is)
         {
            // get length of file:
            is.seekg(0, is.end);
            int length = is.tellg();
            is.seekg(0, is.beg);
            char * buffer = new char[length];
            is.read(buffer, length);
            if(is)
            {
               StrAsc base64_content;
               Csi::Base64::encode(base64_content, buffer, length);
               Csi::Json::ObjectHandle attachment1(new Csi::Json::Object);
               attachment1->set_property_str("filename", file_name);
               attachment1->set_property_str("type", get_mime_type(file_name));
               attachment1->set_property_str("content", base64_content);
               attachment_array->push_back(attachment1.get_handle());
            }
            is.close();
            delete[] buffer;
         }
      }

      Csi::Json::Object::value_type attachments_pair;
      attachments_pair.first = "attachments";
      attachments_pair.second = attachment_array.get_handle();
      data_obj->push_back(attachments_pair);
   }
   Csi::Json::Object::value_type data_pair;
   data_pair.first = "data";
   data_pair.second = data_obj.get_handle();
   json_doc->push_back(data_pair);

   Csi::OStrAscStream json_out;
   json_doc->format(json_out);
   //std::cout << json_out.str() << std::endl;
   OutputDebugString(json_out.str().c_str());

   std::ofstream fout("c:\\temp\\gateway.txt");
   fout << json_out.str().c_str();
   fout.close();


   /*
   The following values should be concatenated into a single base-string, each value being separated by
   an ampersand character (&):
   1. deviceid
   2. konectid
   3. messagetype
   4. message
   5. timestamp
   6. nonce
   it is important that the values are concatenated in the above order and is case sensitive.
   If a particular value is not present it should still be included as an empty string.
   */
   Csi::Uri smtp_test_uri(smtp_base_uri);
   smtp_request_client = new MyRequestClient(json_doc);
   smtp_request.bind(
      new Csi::HttpClient::Request(
         smtp_request_client, smtp_test_uri,
         Csi::HttpClient::Request::method_post, false));
   smtp_request->set_authorisation(
      new Csi::HttpClient::AuthorisationKdapi(
         device_id, "smtp", "send-email", konect_id, konect_secret_key));
   smtp_request->set_content_type("application/json");
   http_connection->add_request(smtp_request);
}

void add_blacklist()
{
   Csi::Json::Object email_blacklist_json;
   email_blacklist_json.set_property_str("email", "blacklist@youremail.com");
   Csi::OStrAscStream email_json;
   email_blacklist_json.format(email_json);

   Csi::Json::ObjectHandle signed_envelope(new Csi::Json::Object());
   Csi::SignedJson::generate_envelope(*signed_envelope, email_json.c_str(), email_json.length());

   Csi::Uri blacklist_test_uri(blacklist_base_uri);
   blacklist_request_client = new MyRequestClient(signed_envelope);
   blacklist_request.bind(
      new Csi::HttpClient::Request(
         blacklist_request_client, blacklist_test_uri,
         Csi::HttpClient::Request::method_post, false));
   blacklist_request->set_content_type("application/json");
   http_connection->add_request(blacklist_request);
}

int main()
{
   Csi::MessageWindow::initialise(::GetModuleHandle(0));
   Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
   Csi::Win32::WinsockInitialisor sockets_init;

   http_connection.bind(new Csi::HttpClient::Connection());
   http_connection->set_watcher(new MyWatcher);

   MSG message;
   send_email();
   while(GetMessage(&message, 0, 0, 0))
   {
      TranslateMessage(&message);
      DispatchMessage(&message);
   }

   add_blacklist();
   while(GetMessage(&message, 0, 0, 0))
   {
      TranslateMessage(&message);
      DispatchMessage(&message);
   }


   if(smtp_request_client)
   {
      delete smtp_request_client;
      smtp_request_client = nullptr;
      smtp_request.clear();
   }

   if(blacklist_request_client)
   {
      delete blacklist_request_client;
      blacklist_request_client = nullptr;
      blacklist_request.clear();
   }

   http_connection.clear();

   return 0;
}

