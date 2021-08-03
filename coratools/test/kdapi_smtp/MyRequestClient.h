#pragma once
#include "Csi.HttpClient.h"
#include "Csi.Json.h"

class MyRequestClient:
   public Csi::HttpClient::RequestClient,
   public OneShotClient
{
public:
   MyRequestClient(Csi::Json::ObjectHandle json_doc);
   virtual ~MyRequestClient();

   ////////////////////////////////////////////////////////////
   // on_failure
   //
   // Called when the HTTP request has failed for some reason.  The
   // return value will indicate whether the connection should proceed to
   // any other requests.
   ////////////////////////////////////////////////////////////
   virtual bool on_failure(Csi::HttpClient::Request *request);

   ////////////////////////////////////////////////////////////
   // on_connected
   //
   // Called by the request when the supporting connection has been
   // made. 
   ////////////////////////////////////////////////////////////
   virtual void on_connected(Csi::HttpClient::Request *request);

   ////////////////////////////////////////////////////////////
   // on_response_header
   //
   // Called when the header portion of the web server response has been
   // received.  
   ////////////////////////////////////////////////////////////
   virtual void on_response_header(Csi::HttpClient::Request *request);

   ////////////////////////////////////////////////////////////
   // on_response_data
   //
   // Called when data has been received for the HTTP request body.  This
   // data can be accessed using the request's get_rx_buff() method. 
   ////////////////////////////////////////////////////////////
   virtual void on_response_data(Csi::HttpClient::Request *request);

   ////////////////////////////////////////////////////////////
   // on_response_complete
   //
   // Called when the entire response has been received.   If the return
   // value is true, the connection will go on to service subsequent
   // commands.  
   ////////////////////////////////////////////////////////////
   virtual bool on_response_complete(Csi::HttpClient::Request *request);

   virtual void onOneShotFired(uint4 id);

protected:
   /* 
   * json_doc
   */
   Csi::Json::ObjectHandle json_doc;

   /*
   * oneshot
   */
   Csi::SharedPtr<OneShot> oneshot;

   /*
   * total_sent_bytes
   */
   long total_sent_bytes;

   Csi::HttpClient::Request *the_request;
   Csi::ByteQueue send_buff;
};

