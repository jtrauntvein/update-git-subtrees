/* CsiWebQuery.js

   Copyright (C) 2010, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 02 August 2010
   Last Change: Friday 22 April 2016
   Last Commit: $Date: 2020-07-14 15:47:08 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/


function ExcSynchValues()
{ }


////////////////////////////////////////////////////////////
// class CsiWebQuery
//
// Defines an object that represents a data request to the web server
////////////////////////////////////////////////////////////
function CsiWebQuery(
   uri,
   mode,
   p1,
   p2,
   order,
   requestInterval,
   override_interval,
   variables,
   js_name,
   report_offset)
{
   this.uri = uri;
   this.mode = mode;
   this.p1 = p1;
   this.p2 = p2;
   this.order = order;
   this.tableDefSignature = 0;
   this.requestInterval = requestInterval; //how often should this query occur
   this.override_interval = override_interval;
   this.variables = variables; //Variables that should be updated when data comes in
   this.last_record_no = -1;
   this.last_stamp = new CsiLgrDate();
   this.js_name = js_name;
   this.report_offset = report_offset;
   var i;

   //keep list of expressions that are affected by this webQuery
   this.expressions = [];
   var len = variables.length;
   for (i = 0; i < len; i++)
   {
      //todo: This check for ownerExpression can be removed once the RTMC project is generated automatically
      if (variables[i].ownerExpression)
      {
         if($.inArray(variables[i].ownerExpression, this.expressions) === -1)
         {
            this.expressions.push(variables[i].ownerExpression);
         }
      }
   }
   this.loadingData = false;  //actively loading data?
   this.nextQueryTime = 0; //next time to request data

   this.query_satisfied = false; //date-range queries should only poll once
   this.supervisor = null;
   this.websock_transaction = null; // this query has no associated websocket request.
}


//returns the command for the HTTPRequest
CsiWebQuery.prototype.getCommand = function ()
{
   if(this.supervisor && this.supervisor.on_query_begin)
   {
      this.supervisor.on_query_begin(this);
   }

   var rtn = ".?command=DataQuery" +
         "&uri=" + encodeURIComponent(this.uri) +
         "&format=json" +
         "&mode=" + this.mode +
         "&p1=" + this.p1 +
         "&p2=" + this.p2 +
         "&headsig=" + this.tableDefSignature +
         "&nextpoll=" + this.requestInterval;
   if(this.order.length > 0)
   {
      rtn = rtn + "&order=" + this.order;
   }

   if(this.override_interval >= 0)
   {
      rtn = rtn + "&refresh=" + this.override_interval;
   }
   return rtn;
};


/**
 * @return Returns an object that represents a request for the data for this query.
 */
CsiWebQuery.prototype.get_websock_request = function(transaction)
{
   var rtn = {
      uri: this.uri,
      mode: this.mode,
      p1: this.p1,
      p2: this.p2,
      transaction: transaction
   };
   if(this.override_interval >= 0)
      rtn.refresh = this.override_interval;
   if(this.order.length > 0)
      rtn.order = this.order;
   this.websock_transaction = transaction;
   return rtn;
};


/**
 * Called when a complete header has been received from the server.
 *
 * @param header  Specifies the CSIJson header structure.
 *
 * @param do_reset Set to true if the state of expressions and components should be reset.
 */
CsiWebQuery.prototype.process_header = function(head, do_reset)
{
   //cache tableDefSignature for future Queries
   this.tableDefSignature = head.signature;
   
   //cache field index for variable
   if(head.fields)
   {
      var variables_count = this.variables.length;
      var variable;
      var i;
      var component;
      
      for(i = 0; i < variables_count; i++)
      {
         variable = this.variables[i];
         variable.fieldIndex = -1;
         component = variable.ownerExpression.ownerComponent;
         if(variable.is_table)
         {
            //Send the comp the table defs if it needs them
            if(component.tableDefs)
               component.tableDefs(head);
         }
         else
         {
            if(do_reset)
            {
               variable.ownerExpression.reset();
               if(component && component.reset_data)
                  component.reset_data(false);
            }
            variable.fieldIndex = CsiWebQuery.getFieldIndex(head.fields, variable.simpleUri);
            if(variable.fieldIndex > -1)
               variable.type = head.fields[variable.fieldIndex].type;
         }
      }
   }
};


/**
 * Called to process new records that have been received.
 *
 * @param json Specifies the CSIJson structure for the new data.
 */
CsiWebQuery.prototype.newData = function (json)
{
   var more_data = json.more;
   var component = null;
   var i;
   var j;
   var variable = null;
   var len;
   var len2;

   if(more_data === null)
      more_data = false;
   if(this.mode === "date-range" && !more_data) //Date range is always satisfied
      this.query_satisfied = true;

   //does the head exist?
   if(json.head)
      this.process_header(json.head, false);
   
   // for each record
   len = json.data.length;
   var lastRecordIndex = len - 1;
   var report_data = len > 0;
   var was_bad = false;
   var was_nan = false;
   var value;

   for(i = 0; report_data && i < len; i++)
   {
      var record = json.data[i];
      var timestamp = CsiLgrDate.fromStr(record.time);
      var moreToCome = i < lastRecordIndex;

      if(this.supervisor && this.supervisor.on_new_data)
      {
         report_data = this.supervisor.on_new_data(this, record, timestamp);
      }

      // the record number and time stamp for this record should be
      // different from the last record number and time stamp
      // reported.  If they are the same, we expect that this record
      // is the same as previously reported.  
      if((this.last_record_no !== record.no ||
          this.last_stamp.milliSecs !== timestamp.milliSecs) &&
         report_data)
      {
         //update variable data
         if(report_data)
         {
            this.last_record_no = record.no;
            this.last_stamp = timestamp;
            len2 = this.variables.length;
            for(j = 0; j < len2; j++)
            {
               variable = this.variables[j];
               variable.recnum = record.no;
               variable.timestamp = new CsiLgrDate(timestamp);
               value = record.vals[variable.fieldIndex];
               if(!variable.is_table && variable.fieldIndex > -1)
                  variable.set_value(value, timestamp);

               variable.has_been_set = true;
               if(variable.is_table)
               {
                  component = variable.ownerExpression.ownerComponent;
                  if(component)
                  {
                     component.latest_timestamp = this.last_stamp;
                     was_bad = component.bad_data;
                     if(component.newRecord)
                     {
                        component.bad_data = false;
                        component.newRecord(record, timestamp, moreToCome);
                        if(was_bad)
                           component.invalidate();
                     }
                     else
                     {
                        component.bad_data = true;
                        if(!was_bad)
                           component.invalidate();
                     }
                  }
               }
            }
         }

         // update components that want newValue(value, timestamp)
         len2 = this.expressions.length;
         value = null;
         var expression;
         for(j = 0; report_data && j < len2; j++)
         {
            expression = this.expressions[j];
            component = expression.ownerComponent;
            if(component)
            {
               component.latest_timestamp = this.last_stamp;
               was_bad = component.bad_data; //If we switch from bad to good or good to bad, we need to invalidate
               if(!expression.has_table_ref && component.newValue)
               {
                  try
                  {
                     var result = this.expressions[j].evaluate();
                     if(result)
                     {
                        component.bad_data = false;
                        was_nan = component.nan_data;
                        //Check for string types
                        if((typeof result.value === "string" || result.value instanceof String) && component.newStringValue)
                        {
                           component.nan_data = false;
                           component.newStringValue(result.value, result.timestamp, moreToCome);
                           if (was_nan !== component.nan_data)
                           {
                              component.invalidate();
                           }
                        }
                        else //Check numbers to see if they are finite or not
                        {
                           if(result.value === -Infinity || result.value === Infinity || isNaN(result.value))
                           {
                              component.nan_data = true;
                              if(component.newNanValue)
                              {
                                 component.newNanValue(result.value, result.timestamp, moreToCome);
                              }
                           }
                           else
                           {
                              component.nan_data = false;
                              component.newValue(result.value, result.timestamp, moreToCome);
                           }
                        }

                        if(was_bad || (was_nan !== component.nan_data))
                        {
                           component.invalidate();
                        }
                     }
                     else
                     {
                        component.bad_data = true;
                        if(!was_bad)
                        {
                           component.invalidate();
                        }
                     }
                  }
                  catch(e)
                  {
                     if(!(e instanceof ExcSynchValues))
                     {
                        console.log('CsiWebQuery.prototype.newData: ' + e.toString());
                        component.bad_data = true;
                        if(!was_bad)
                        {
                           component.invalidate();
                        }
                     }
                  }
               }
            }
         }
      }
   }

   // we need to determine what mode will be used for the next query
   if(report_data && this.websock_transaction === null)
   {
      if(this.order !== "real-time" && this.mode !== "date-range")
      {
         this.mode = "since-record";
         this.p1 = this.last_record_no;
         this.p2 = this.last_stamp.format("%Y-%m-%dT%H:%M:%S%x");
      }
      else if(this.mode === "date-range" && more_data)
      {
         var next_stamp = new CsiLgrDate(this.last_stamp.milliSecs + 1);
         var end_stamp = CsiLgrDate.fromStr(this.p2);
         if(next_stamp < end_stamp)
            this.p1 = next_stamp.format("%Y-%m-%dT%H:%M:%S%x");
         else
            this.query_satisfied = true;
      }
   }
   return more_data && !this.query_satisfied;
};


/**
 * Called to indicate that the last attempt to query data has failed.
 *
 * @param error Specifies the type of error.
 *
 * @param do_reset Set to true if the expression and components should be reset because of this failure.
 */
CsiWebQuery.prototype.on_query_fail = function (error, do_reset)
{
   var expressions_count = this.expressions.length;
   var i;
   for(i = 0; i < expressions_count; ++i)
   {
      var expression = this.expressions[i];
      if(do_reset)
         expression.reset();
      if(expression.ownerComponent)
      {
         var component = expression.ownerComponent;
         component.bad_data = true;
         component.invalidate();
         if(do_reset && component)
            expression.ownerComponent.reset_data(false);
      }
   }
   this.websock_transaction = null;
};


/**
 * @return Returns true if the query and the components associated with it are in a state where they need
 * data now.
 */
CsiWebQuery.prototype.needs_data_now = function()
{
   var rtn = this.expressions.some(function(expression)
   {
      if(expression.ownerComponent)
      {
         if(expression.ownerComponent.needs_data_now)
         {
            return expression.ownerComponent.needs_data_now();
         }
         else
         {
            console.debug("Undefined Function: needs_data_now() for component - " + expression.ownerComponent.constructor.name);
            return true;
         }
      }
      else
         return true;
   });
   return rtn;
};


//get the FieldIndex from the json.head.fields object
CsiWebQuery.getFieldIndex = function (jsonfields, fieldName)
{
   var len = jsonfields.length;
   var lower_name = fieldName.toLowerCase();
   var lower_field;
   var i;
   for(i = 0; i < len; i++)
   {
      lower_field = jsonfields[i].name.toLowerCase();
      if(lower_name === lower_field)
      {
         return i;
      }
   }
   return -1;
};


