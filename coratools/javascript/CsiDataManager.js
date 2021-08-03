/* CsiDataManager.js

   Copyright (C) 2010, 2016 Campbell Scientific, Inc.

   Written by: Kevin Westwood
   Date Begun: Tuesday 03 August 2010
   Last Change: Thursday 11 February 2016
   Last Commit: $Date: 2016-04-22 12:18:01 -0600 (Fri, 22 Apr 2016) $
   Last Changed by: $Author: jon $

*/

var dataManager = null; //GLOBAL DECLARATION
var currentWebQuery = null; //GLOBAL DECLARATION

/**
 * Defines the object that will manager the retrieval of data from the web server.
 *
 * @param webQueries  Specifies the set of web queries that this manager should manage.
 *
 * @param web_sockets_enabled_ Set to true if this manager is to attempt
 * to use web sockets for server data requests.  If set to false, this
 * manager will use ajax requests to poll for all of the queries.
 */
function CsiDataManager(webQueries, web_sockets_enabled_)
{
   this.webQueries = webQueries; //List of WebQueries
   this.web_socket = null;
   this.web_sockets_enabled = web_sockets_enabled_;
   this.last_transaction = 0;
   this.web_socket_was_opened = false;
   this.web_socket_opened_count = 0;
   
   // Specifies the time difference between the local time and the last server time.
   this.last_server_diff = null;

   // Specifies the local time when the server time was last checked.
   this.last_server_checked = null;

   // Set to true to indicate that the server time is being queried.
   this.checking_server_time = false;
}


/* global WebSocket: true */


/**
 * Called to initiate data collection from the web server.
 */
CsiDataManager.prototype.start = function()
{
   var query = null;
   var len = this.webQueries.length;
   if(this.web_sockets_enabled)
   {
      this.start_web_sockets();
   }
   else
   {
      for (var i = 0; i < len; i++)
      {
         query = this.webQueries[i];
         this.loadData(query);
      }
   }
};


/**
 * Performs the ajax query for a data request.
 *
 * @param webQuery Specifies the query for which data should be requested.
 */
CsiDataManager.prototype.loadData = function(webQuery)
{
   if(!webQuery.loadingData)
   {
      currentWebQuery = webQuery;
      webQuery.loadingData = true;
      $.ajax({
         url: webQuery.getCommand(),
         dataType: 'json',
         cache: false,
         timeout: 300000,       // five minute timeout
         beforeSend: function(xhr){
            xhr.webQuery = currentWebQuery;
         },
         success: function(json, status, xhr){
            dataManager.dataReceived(json, status, xhr);
         },
         error: function(xhr, status, error){
            dataManager.xhrError(xhr, status, error);
         }
      });
   }
};


/**
 * Called when new data has been received for the an ajax request.
 *
 * @param json Specifies the CSIJson data object that has been
 * received.
 *
 * @param status Speifies the status of the ajax request.
 *
 * @param xhr  Specifies the object that keeps track of the ajax
 * request.
 */
CsiDataManager.prototype.dataReceived = function (json, status, xhr)
{
   if(json)
   {
      xhr.webQuery.loadingData = false;
      if(!xhr.webQuery.newData(json))
      {
         if(!xhr.webQuery.query_satisfied)
            oneShotTimer.setTimeout(dataManager, xhr.webQuery, xhr.webQuery.requestInterval);
      }
      else
      {
         oneShotTimer.setTimeout(dataManager, xhr.webQuery, 100);
      }
   }
   else //request failed
   {
      oneShotTimer.setTimeout(dataManager, xhr.webQuery, 10000); //retry after 10 seconds
   }
};


/**
 * Called when an ajax request has failed.
 *
 * @param xhr Specifies the ajax request.
 *
 * @param status Specifies the state of the request.
 *
 * @param error Specifies the error that has been encountered.
 */
CsiDataManager.prototype.xhrError = function(xhr, status, error)
{
   if(xhr.webQuery)
   {
      xhr.webQuery.loadingData = false;
      oneShotTimer.setTimeout(dataManager, xhr.webQuery, 10000); //retry after 10 seconds
      xhr.webQuery.on_query_fail(error, false);
   }
};


/**
 * Called by the application timer to renew any pending requests.
 */
CsiDataManager.prototype.onOneShotTimer = function(query)
{
   if(this.web_sockets_enabled && this.web_socket === null)
      this.start_web_sockets();
   else if(this.web_sockets_enabled && this.web_socket !== null)
      this.send_websock_requests();
   else if(query)
   {
      if(!query.query_satisfied)
      {
         this.loadData(query);
      }
   }
};


/**
 * Called to start a web socket connection and start pending data
 * requests.
 */
CsiDataManager.prototype.start_web_sockets = function()
{
   if(this.web_sockets_enabled && this.web_socket === null)
   {
      // we need to create the web socket first.  The socket URI will
      // depend upon the document URL.
      var protocol = document.location.protocol;
      var host = document.location.hostname;
      var path = document.location.pathname;
      var websock_uri;

      if(document.location.port !== "")
      {
         host = host + ":" + document.location.port;
      }
      websock_uri = (protocol === "https:" ? "wss://" : "ws://") + host + path;
      try
      {
         this.web_socket_was_opened = false;
         this.web_socket = new WebSocket(websock_uri, [ "com.campbellsci.webdata" ]);
         this.web_socket.onopen = function(e) {
            dataManager.web_socket_was_opened = true;
            dataManager.web_socket_opened_count += 1;
            dataManager.send_websock_requests();
         };
         this.web_socket.onerror = function(e) {
            dataManager.on_websock_error(e);
         };
         this.web_socket.onclose = function(e) {
            dataManager.on_websock_error(e);
         };
         this.web_socket.onmessage = function(e) {
            dataManager.on_websock_message(e);
         };
      }
      catch(e)
      {
         this.web_sockets_enabled = false;
         this.start();
      }
   }
};



CsiDataManager.prototype.send_websock_requests = function()
{
   if(this.web_sockets_enabled && this.web_socket_was_opened && this.web_socket !== null)
   {
      // we need to send requests for any queries that are not already started.  We will send each request
      // in its own message in order to work around the datalogger messing up all requests if one request
      // is bad.
      var queries_count = this.webQueries.length;
      var some_delayed = false;
      for(var i = 0; i < queries_count; ++i)
      {
         var query = this.webQueries[i];
         if(query.websock_transaction === null && !query.query_satisfied)
         {
            if(query.needs_data_now())
            {
               var request_tran = ++this.last_transaction;
               var message = { message:"AddRequests", requests:[ query.get_websock_request(request_tran) ]};
               var message_str = JSON.stringify(message);
               this.web_socket.send(message_str);
            }
            else
               some_delayed = true;
         }
      }
      if(some_delayed)
         oneShotTimer.setTimeout(this, null, 1000);
   }
};


/**
 * Cancels a request associated with the specified query.
 *
 * @param query Specifies the query to cancel.
 */
CsiDataManager.prototype.cancel_request = function(query)
{
   if(query.websock_transaction !== null && this.web_socket !== null)
   {
      var message = { message: "RemoveRequests", transactions: [ query.websock_transaction ] };
      var message_str = JSON.stringify(message);
      this.web_socket.send(message_str);
   }
   query.websock_transaction = null;
   query.query_satisfied = false;
};


CsiDataManager.prototype.on_websock_error = function(e)
{
   // since the socket has failed, we will need to mark all requests
   // so that they will get restarted on retry
   var queries_count = this.webQueries.length;
   for(var i = 0; i < queries_count; ++i)
   {
      var query = this.webQueries[i];
      query.on_query_fail(e, false);
   }
   this.web_socket = null;
   if(this.web_socket_was_opened || this.web_socket_opened_count > 0)
      oneShotTimer.setTimeout(this, null, 10000);
   else
   {
      this.web_sockets_enabled = false;
      this.start();
      if(theAlarmsManager)
         theAlarmsManager.start();
   }
};


CsiDataManager.prototype.on_websock_message = function(e)
{
   // parsing the message can result in a syntax error
   try
   {
      var message = JSON.parse(e.data);
      if(message.message === "RequestStarted")
         this.on_websock_request_started(message);
      else if(message.message === "RequestFailed")
         this.on_websock_request_failed(message);
      else if(message.message === "RequestRecords")
         this.on_websock_request_records(message);
      else if(message.message === "AlarmChanged")
      {
         if(theAlarmsManager)
            theAlarmsManager.on_alarm_data(message, null);
      }
   }
   catch(syntax_error)
   {
      csi_log("record handler exception: " + syntax_error.toString());
   }
};


CsiDataManager.prototype.on_websock_request_started = function(message)
{
   // we need to locate the request that is associated with this message
   var queries_count = this.webQueries.length;
   for(var i = 0; i < queries_count; ++i)
   {
      var query = this.webQueries[i];
      if(query.websock_transaction === message.transaction)
      {
         query.process_header(message.head, true);
         break;
      }
   }
};


CsiDataManager.prototype.on_websock_request_failed = function(message)
{
   // we need to locate the request that is associated with this message
   var queries_count = this.webQueries.length;
   for(var i = 0; i < queries_count; ++i)
   {
      var query = this.webQueries[i];
      if(query.websock_transaction === message.transaction)
      {
         query.on_query_fail(message, false);
         oneShotTimer.setTimeout(this, query, query.requestInterval);
         break;
      }
   }
};


CsiDataManager.prototype.on_websock_request_records = function(message)
{
   // we need to locate the request that is associated with this message
   var queries_count = this.webQueries.length;
   for(var i = 0; i < queries_count; ++i)
   {
      var query = this.webQueries[i];
      if(query.websock_transaction === message.transaction)
      {
         query.newData(message.records);
         break;
      }
   }
};


/**
 * @return Returns the estimate of the current server time
 * based upon the last time that the server time was checked.
 */
CsiDataManager.prototype.get_server_time = function()
{
   var rtn = CsiLgrDate.local();
   if(this.last_server_checked === null || rtn.milliSecs - this.last_server_checked.milliSecs > 300000)
   {
      if(!this.checking_server_time)
      {
         this.checking_server_time = true;
         $.ajax({
            url: ".?command=ClockCheck&format=json",
            dataType: 'json',
            cache: false,
            timeout: 300000,
            success: function(json, status, xhr) {
               var server_time = new CsiLgrDate(json.time);
               dataManager.last_server_checked = CsiLgrDate.local();
               dataManager.last_server_diff = dataManager.last_server_checked.milliSecs - server_time.milliSecs;
               dataManager.checking_server_time = false;
            },
            error: function(xhr, status, error) {
               dataManager.last_server_diff = null;
               dataManager.last_server_checked = null;
               dataManager.checking_server_time = false;
            }
         });
      }
   }
   if(this.last_server_diff !== null)
      rtn.milliSecs -= this.last_server_diff;
   return rtn;
};
