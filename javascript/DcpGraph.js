/* DcpGraph.js

   Copyright (C) 2010, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 July 2010
   Last Change: Friday 15 February 2019
   Last Commit: $Date: 2019-02-19 18:19:16 -0600 (Tue, 19 Feb 2019) $
   Last Changed by: $Author: jon $

*/


function CsiGraphSeries(expression, owner, axis)
{
   //Do not add properties to prototype
   if(arguments.length === 0)
      return;
   this.ownerGraph = owner;
   if(expression)
   {
      this.expression = expression;
      this.expression.ownerComponent = this;
   }
   else
      this.expression = null;
   this.label = "Series";
   this.series_type = Enum.SERIES_TYPE.LINE;
   this.axis = axis;
   this.line_type = 0;
   this.line_color = "#FF0000";
   this.line_width = 1;
   this.use_stairs = false;
   this.point_type = 1;
   this.point_color = "#FF0000";
   this.point_size = 2;
   this.timeOffset = 0;
   this.marks_enabled = false;
   this.marks_show_on_click = true;
   this.marks_transparent = 0;
   this.marks_color = "#FFFFFF";
   this.marks_rounded = false;
   this.marks_draw_every = 1;
   this.marks_format = 2;
   this.bar_border_line_type = 0;
   this.bar_border_color = "#000000";
   this.bar_border_width = 1;
   this.bar_type = 0;
   this.bar_color = "#FFFF00";
   this.bar_width_percentage = 0.70;
   this.bar_position = 0;
   this.data = [];
   this.mark_cache = [];
   this.current_mark = null;
   this.mark_font = "10pt Arial";

   this.bad_data = true;
   this.nan_data = false;

   this.domain_axis = this.ownerGraph.bottomAxis;
   this.domain_axis.add_series(this);
   if(this.axis === 0)
      this.range_axis = this.ownerGraph.leftAxis;
   else
      this.range_axis = this.ownerGraph.rightAxis;
   this.range_axis.add_series(this);
}


CsiGraphSeries.prototype.invalidate = function ()
{
   this.ownerGraph.valid = false;
};


CsiGraphSeries.prototype.newNanValue = function (value, timestamp, expect_more)
{
   //NAN points are assumed to be NOPLOT
   this.newValue(Number.NaN, timestamp, expect_more);
};


function binarySearch(values, target, start, end)
{
   if(start > end) 
   { return -1; } //does not exist

   var middle = Math.floor((start + end) / 2);
   var value = values[middle][0].milliSecs;

   if(value > target)
      return binarySearch(values, target, start, middle - 1); 
   else if(value < target)
      return binarySearch(values, target, middle + 1, end); 
   return middle; //found!
}


function findIndex(values, target)
{
   return binarySearch(values, target.milliSecs, 0, values.length - 1);
}


CsiGraphSeries.prototype.newValue = function (value, timestamp, expect_more)
{
   //We need to make sure values are in the right order.
   //Splice into the array if the timestamp is older than the last value in our data array.
   //Use a binary search to find the index at which to slice into the array.
   this.bad_data = false;
   var newTS = new CsiLgrDate(timestamp.milliSecs + this.timeOffset);
   var value_pair = this.data[this.data.length - 1];
   if(value_pair && newTS.milliSecs < value_pair[0].milliSecs) //Timestamps are out of order
   {
      //Find where to insert the new record using a binary search
      var index = findIndex(this.data, newTS);
      if(index >= 0)
         this.data.splice(index, 0, [newTS, value]); //Insert the new value at the proper index
   }
   else //Timestamps in order, so just put it on the end
      this.data.push([newTS, value]);

   if(!expect_more)
   {
      //update the newesttimestamp
      var newest_value_pair = this.data[this.data.length - 1];
      if(newest_value_pair && newest_value_pair[0].milliSecs > this.ownerGraph.newestTimeStamp)
         this.ownerGraph.newestTimeStamp = newest_value_pair[0].milliSecs;

      if(!this.ownerGraph.show_restore_btn) //Don't remove data if we are zoomed or panned
      {
         //find old data
         var i = 0;
         var len = this.data.length;
         var cutoff = this.ownerGraph.newestTimeStamp - this.ownerGraph.graphWidth;
         while((i < len) && (this.data[i][0].milliSecs < cutoff))
            i++;

         //remove old data
         if(i > 0)
            this.data.splice(0, i);
      }

      if(!this.ownerGraph.zooming)
      {
         if(this.current_mark)
         {
            if(CsiLgrDate.local().milliSecs - this.current_mark.start_time.milliSecs > 2000)
            {
               this.current_mark.start_time = null;
               this.current_mark = null;
            }
         }
         this.ownerGraph.positionsInvalid = true;
         this.ownerGraph.invalidate();
      }
   }
};


CsiGraphSeries.prototype.get_bounds = function (axis_type)
{
   // we will extract the max and min values from the data
   var rtn = { max: -Number.MAX_VALUE, min: Number.MAX_VALUE };
   var cnt = this.data.length;
   var i;
   for(i = 0; i < cnt; ++i)
   {
      var datum = this.data[i];
      if(axis_type === CsiGraphAxis.axis_bottom)
      {
         rtn.min = Math.min(rtn.min, datum[0]);
         rtn.max = Math.max(rtn.max, datum[0]);
      }
      else
      {
         if(isFinite(datum[1])) //Ignore NOPLOT since they are NANs
         {
            rtn.min = Math.min(rtn.min, datum[1]);
            rtn.max = Math.max(rtn.max, datum[1]);
         }
      }
   }

   if(this.series_type === Enum.SERIES_TYPE.BAR && axis_type !== CsiGraphAxis.axis_bottom)
   {
      //Bars must be oriented around 0, so make sure zero is always present
      if(rtn.max < 0)
         rtn.max = 0;
      if(rtn.min > 0)
         rtn.min = 0;
   }
   return rtn;
};


CsiGraphSeries.prototype.reset_data = function (reset_settings)
{
   this.bad_data = true;
   if(reset_settings)
   {
      this.ownerGraph.displayWidth = Number.MAX_VALUE;
      this.ownerGraph.graphWidth = Number.MAX_VALUE;
      this.ownerGraph.bottomAxis.auto_max = true;
      this.ownerGraph.bottomAxis.auto_min = true;
      this.ownerGraph.newestTimeStamp = 0;
      this.ownerGraph.positionsInvalid = true;
   }
   this.data = [];
   this.current_mark = null;
   if(this.ownerGraph.show_restore_btn)
   {
      this.ownerGraph.RestoreState();
   }
   this.invalidate();
};


CsiGraphSeries.prototype.draw = function (context)
{
   this.mark_cache = []; //clear out the old mark positions

   switch(this.series_type)
   {
      case Enum.SERIES_TYPE.BAR:
         this.draw_bars(context);
         break;
      case Enum.SERIES_TYPE.POINT:
         if(this.point_type !== Enum.POINT_TYPE.NOTHING)
         {
            this.draw_points(context);
         }
         break;
      //case Enum.SERIES_TYPE.LINE: 
      default:
         if(this.line_type !== Enum.LINE_TYPE.CLEAR)
         {
            this.draw_lines(context);
         }
         if(this.point_type !== Enum.POINT_TYPE.NOTHING)
         {
            this.draw_points(context);
         }
         break;
   }
};


CsiGraphSeries.prototype.draw_lines = function (context)
{
   var prev_x = 0;
   var prev_y = 0;
   context.strokeStyle = this.line_color;
   context.lineWidth = this.line_width;

   context.lineJoin = "round";
   context.lineCap = "round";
   var len = this.data.length;
   context.beginPath();
   var i;
   for(i = 0; i < len; i++)
   {
      var ts = this.data[i][0].milliSecs;
      var val = this.data[i][1];

      var x = this.domain_axis.value_to_pos(ts);
      var y = this.range_axis.value_to_pos(val);

      if(x >= this.ownerGraph.plotRect.left && x <= this.ownerGraph.plotRect.right &&
         y >= this.ownerGraph.plotRect.top && y <= this.ownerGraph.plotRect.bottom)
      {
         if(this.marks_enabled)
         {
            if(this.marks_show_on_click || (i % this.marks_draw_every === 0))
            {
               //Store off the point so it can be drawn if needed
               this.mark_cache.push(new CsiGraphMark(ts, val, x, y));
            }
         }
      }

      if(i === 0) //Move to the first position
      {
         context.moveTo(x, y);
      }
      else //Draw the line
      {
         //Only draw visible lines
         if(x >= this.ownerGraph.plotRect.left || prev_x >= this.ownerGraph.plotRect.left /*Catch the duplicate data line if there is one*/)
         {
            if(x <= this.ownerGraph.plotRect.right || prev_x <= this.ownerGraph.plotRect.right) //Draw the last line outside plotRect
            {
               if(this.use_stairs)
               {
                  Csi.draw_line(context, prev_x, prev_y, x, prev_y, this.line_type);
                  Csi.draw_line(context, x, prev_y, x, y, this.line_type);
               }
               else
               {
                  var plot_pos = this.ownerGraph.plotRect.line_intersect(
                     new Point(prev_x, prev_y), new Point(x, y));
                  if(plot_pos)
                  {
                     Csi.draw_line(context, plot_pos.p1.x, plot_pos.p1.y, plot_pos.p2.x, plot_pos.p2.y, this.line_type);
                  }
               }
            }
         }
         else
         {
            //If the line starts outside the plotRect, just move to that point and don't draw the line
            //That way the first drawn line will be from the last point off the plotRect.
            context.moveTo(x, y);
         }
      }

      prev_x = x;
      prev_y = y;
   }
   context.stroke();
};


CsiGraphSeries.prototype.draw_points = function (context)
{
   context.lineWidth = 2;
   context.fillStyle = this.point_color;
   context.strokeStyle = this.point_color;
   var len = this.data.length;
   var i;
   for(i = 0; i < len; i++)
   {
      var x_val = this.data[i][0];
      if(x_val instanceof CsiLgrDate)
      {
         x_val = x_val.milliSecs;
      }

      var y_val = this.data[i][1];

      var x = this.domain_axis.value_to_pos(x_val);
      var y = this.range_axis.value_to_pos(y_val);

      if(x >= this.ownerGraph.plotRect.left && x <= this.ownerGraph.plotRect.right &&
         y >= this.ownerGraph.plotRect.top && y <= this.ownerGraph.plotRect.bottom)
      {
         if(this.marks_enabled && this.line_type === Enum.LINE_TYPE.CLEAR) //Only add them if the line didn't
         {
            if(this.marks_show_on_click || (i % this.marks_draw_every === 0))
               this.mark_cache.push(new CsiGraphMark(x_val, y_val, x, y));
         }
      }

      //Only draw visible points
      if(x >= this.ownerGraph.plotRect.left && x <= this.ownerGraph.plotRect.right)
         this.draw_point_type(context, this.point_type, x, y, this.point_size, false);
   }
};


CsiGraphSeries.prototype.draw_bars = function (context)
{
   var ts = 0;
   var val = 0;
   var x = 0;
   var y = 0;
   var i = 0;
   var len = this.data.length;
   if(len > 0)
   {
      var zero_pos = this.range_axis.value_to_pos(1E-38);
      context.fillStyle = this.bar_color;
      context.strokeStyle = this.bar_border_color;
      context.lineWidth = this.bar_border_width;

      //Calculate all of the point positions so we can figure out the max bar width allowed
      //The max bar width is the smallest distance between points.
      var points = [];
      var complete_bar_width = this.ownerGraph.plotRect.width; //Space available for all the bars
      var prev_x = 0;
      for(i = 0; i < len; i++)
      {
         ts = this.data[i][0].milliSecs;
         val = this.data[i][1];

         x = this.domain_axis.value_to_pos(ts);
         y = this.range_axis.value_to_pos(val);
         points[i] = [x, y];

         if(i > 0 && x > prev_x)
         {
            //See if the space between the 2 points is the smallest available.
            var space_between = x - prev_x;
            if(space_between < complete_bar_width)
            {
               complete_bar_width = space_between;
            }
         }

         prev_x = x;
      }

      //The bar width % affects the complete bar area
      complete_bar_width *= this.bar_width_percentage;

      //If we have more than one bar, we have divide up the complete space
      var bar_step_size = complete_bar_width / this.ownerGraph.bar_count;
      var bar_offset = bar_step_size * this.bar_position; //Offset into the side by side bars
      var single_bar_width = bar_step_size; //Max space for each bar

      //Draw each bar
      len = points.length;
      for(i = 0; i < len; i++)
      {
         y = points[i][1];
         x = points[i][0] + bar_offset;

         var bar_rect = new Rect(x - complete_bar_width / 2, y, single_bar_width, zero_pos - y);

         if(x >= this.ownerGraph.plotRect.left && x <= this.ownerGraph.plotRect.right &&
            y >= this.ownerGraph.plotRect.top && y <= this.ownerGraph.plotRect.bottom)
         {
            if(this.marks_enabled)
            {
               if(this.marks_show_on_click || (i % this.marks_draw_every === 0))
               {
                  ts = this.data[i][0].milliSecs;
                  val = this.data[i][1];
                  //Store off the point so it can be drawn if needed
                  this.mark_cache.push(new CsiGraphMark(ts, val, bar_rect.get_center().x, y));
               }
            }
         }

         //Only draw visible points
         if(bar_rect.right >= this.ownerGraph.plotRect.left && bar_rect.left <= this.ownerGraph.plotRect.right)
            this.draw_bar_type(context, this.bar_type, bar_rect);
      }
   }
};


CsiGraphSeries.prototype.draw_marks = function (context)
{
   var trace = this;
   if(this.current_mark)
      this.draw_mark(context, this.current_mark.x_val, this.current_mark.y_val, this.current_mark.pt_x, this.current_mark.pt_y);
   else if(this.marks_enabled && !this.marks_show_on_click)
   {
      this.mark_cache.forEach(function(mark) {
         trace.draw_mark(context, mark.x_val, mark.y_val, mark.point_x, mark.point_y);
      });
   }
};


CsiGraphSeries.prototype.draw_mark = function (context, x_value, y_value, x, y)
{
   context.save();
   context.font = this.mark_font;
   var mark_text = "";
   if(this.marks_format === Enum.MARKS_TYPE.TIMESTAMP)
      mark_text += (new CsiLgrDate(x_value).format(this.ownerGraph.bottomAxis.scale.time_format));
   else if(this.marks_format === Enum.MARKS_TYPE.BOTH)
      mark_text = String(sprintf("%.5g", y_value) + " @ " + (new CsiLgrDate(x_value).format(this.ownerGraph.bottomAxis.scale.time_format)));
   else //Enum.MARKS_TYPE.VALUE)
      mark_text += sprintf("%.5g", y_value);

   var size = measureText(context, mark_text);
   var tip_x = x;
   var tip_y = y - size.height;
   if(tip_x + size.width / 2 + 10> this.ownerGraph.width)
      tip_x -= size.width / 2;
   else if((tip_x - (size.width / 2 + 5)) < 0)
      tip_x += size.width / 2;
   if(tip_y - 25 - size.height < 0)
      tip_y = 25 + size.height;
   var mark_rect = new Rect(
      tip_x - 5 - size.width / 2,
      tip_y - 5 - size.height - 20,
      size.width + 10,
      size.height + 10);

   context.lineWidth = 1;
   context.strokeStyle = this.marks_color;
   context.beginPath();
   context.moveTo(tip_x, tip_y - 5 - size.height);
   context.lineTo(x, y);
   context.stroke();

   context.strokeStyle = "#000000";
   context.fillStyle = this.marks_color;
   if(!this.marks_transparent)
   {
      if(this.marks_rounded)
      {
         drawRoundedRect(context, mark_rect, 4);
         context.fill();
         context.stroke();
      }
      else
      {
         context.fillRect(mark_rect.left, mark_rect.top, mark_rect.width, mark_rect.height);
         context.strokeRect(mark_rect.left, mark_rect.top, mark_rect.width, mark_rect.height);
      }
   }

   context.textAlign = "center";
   context.textBaseline = "middle";
   context.fillStyle = CsiColour.parse(this.marks_color).background_complement().format();
   context.fillText(mark_text, mark_rect.left + mark_rect.width/2, mark_rect.top + mark_rect.height/2);
   context.restore();
};


CsiGraphSeries.prototype.make_legend_rect = function(context)
{
   var rtn = null;
   var legend_point_size = 5;
   var legend_line_length = 30;
   var legend_horizontal_margin = 2;
   var legend_vertical_margin = 3;
   var middle_rect;

   this.legend_title_rect = null;
   this.legend_symbol_rect = null;
   if(this.label && this.label.length > 0)
   {
      rtn = new CsiNestedRect();
      middle_rect = new CsiNestedRect();
      this.legend_title_rect = CsiScale.measure_text(this.label, context);
      this.legend_symbol_rect = new CsiNestedRect(0, 0, legend_line_length, legend_point_size);
      rtn.add(new CsiNestedRect(0, 0, legend_horizontal_margin, legend_vertical_margin));
      middle_rect.add(new CsiNestedRect(0, 0, legend_horizontal_margin, legend_vertical_margin));
      middle_rect.add(this.legend_symbol_rect);
      middle_rect.add(new CsiNestedRect(0, 0, legend_horizontal_margin, legend_vertical_margin));
      middle_rect.add(this.legend_title_rect);
      middle_rect.add(new CsiNestedRect(0, 0, legend_horizontal_margin, legend_vertical_margin));
      middle_rect.stack_horizontal();
      if(this.legend_title_rect.get_height() > this.legend_symbol_rect.get_height())
         this.legend_symbol_rect.centre_y(this.legend_title_rect.get_centre(false).y);
      else
         this.legend_title_rect.centre_y(this.legend_symbol_rect.get_centre(false).y);
      rtn.add(middle_rect);
      rtn.add(new CsiNestedRect(0, 0, legend_horizontal_margin, legend_vertical_margin));
      rtn.stack_vertical();
   }
   this.legend_rect = rtn;
   return rtn;
};


CsiGraphSeries.prototype.draw_legend = function(context, foreground_colour)
{
   var centre;
   var width;
   var left;
   var right;
   var point_left, point_right;
   var legend_point_size = this.point_size;
   
   if(this.legend_title_rect && this.legend_symbol_rect)
   {
      centre = this.legend_symbol_rect.get_centre(true);
      width = this.legend_symbol_rect.get_width(true) * 0.85;
      left = centre.x - width / 2;
      right = centre.x + width / 2;
      point_left = new Point(left, centre.y);
      point_right = new Point(right, centre.y);
      context.strokeStyle = this.line_color;
      context.lineWidth = 1;
      Csi.draw_line(context, point_left.x, point_left.y, point_right.x, point_right.y, this.line_type);
      context.fillStyle = this.point_color;
      this.draw_point_type(context, this.point_type, centre.x, centre.y, legend_point_size, true);
      context.fillStyle = foreground_colour;
      context.textAlign = "left";
      context.textBaseline = "top";
      context.fillText(this.label, this.legend_title_rect.get_left(true), this.legend_title_rect.get_top(true));
   }
};


CsiGraphSeries.prototype.draw_point_type = function (context, point_type, x, y, radius, do_stroke)
{
   var size;
   var height;
   context.beginPath();
   switch(point_type)
   {
   case Enum.POINT_TYPE.RECTANGLE:
      // pi * r^2 = (2s)^2  =>  s = r * sqrt(pi)/2
      size = radius * Math.sqrt(Math.PI) / 2;
      context.rect(x - size, y - size, size + size, size + size);
      context.fill();
      if(do_stroke)
         context.stroke();
      break;

   case Enum.POINT_TYPE.CIRCLE:
      context.arc(x, y, radius, 0, Math.PI * 2, false);
      context.fill();
      if(do_stroke)
         context.stroke();
      break;
      
   case Enum.POINT_TYPE.DIAMOND:
      // pi * r^2 = 2s^2  =>  s = r * sqrt(pi/2)
      size = radius * Math.sqrt(Math.PI / 2);
      context.moveTo(x - size, y);
      context.lineTo(x, y - size);
      context.lineTo(x + size, y);
      context.lineTo(x, y + size);
      context.lineTo(x - size, y);
      context.fill();
      if(do_stroke)
         context.stroke();
      break;

   case Enum.POINT_TYPE.TRIANGLE:
      // pi * r^2 = 1/2 * s^2 * sin (pi / 3)  =>  s = r * sqrt(2 * pi / sin(pi / 3))
      size = radius * Math.sqrt(2 * Math.PI / Math.sin(Math.PI / 3));
      height = size * Math.sin(Math.PI / 3);
      context.moveTo(x - size / 2, y + height / 2);
      context.lineTo(x + size / 2, y + height / 2);
      context.lineTo(x, y - height / 2);
      context.lineTo(x - size / 2, y + height / 2);
      context.fill();
      if(do_stroke)
         context.stroke();
      break;

   case Enum.POINT_TYPE.CROSS:
      size = radius * Math.sqrt(Math.PI) / 2;
      context.moveTo(x - size, y);
      context.lineTo(x + size, y);
         context.moveTo(x, y - size);
      context.lineTo(x, y + size);
      if(do_stroke)
      {
         context.strokeStyle = context.fillStyle;
         context.lineWidth = 2;
      }
      context.stroke();
      break;

   case Enum.POINT_TYPE.DOWNTRIANGLE:
      context.lineWidth = 0.1;
      size = radius * Math.sqrt(2 * Math.PI / Math.sin(Math.PI / 3));
      height = size * Math.sin(Math.PI / 3);
      context.moveTo(x - size / 2, y - height / 2);
      context.lineTo(x + size / 2, y - height / 2);
      context.lineTo(x, y + height / 2);
      context.lineTo(x - size / 2, y - height / 2);
      context.fill();
      if(do_stroke)
         context.stroke();
      break;

   case Enum.POINT_TYPE.DIAGCROSS:
      size = radius * Math.sqrt(Math.PI) / 2;
      context.moveTo(x - size, y - size);
      context.lineTo(x + size, y + size);
      context.moveTo(x - size, y + size);
      context.lineTo(x + size, y - size);
      if(do_stroke)
      {
         context.strokeStyle = context.fillStyle;
         context.lineWidth = 2;
      }
      context.stroke();
      break;

   case Enum.POINT_TYPE.STAR:
      size = radius * Math.sqrt(Math.PI) / 2;
      context.moveTo(x - size, y);
      context.lineTo(x + size, y);
      context.moveTo(x, y - size);
      context.lineTo(x, y + size);
      context.moveTo(x - size, y - size);
      context.lineTo(x + size, y + size);
      context.moveTo(x - size, y + size);
      context.lineTo(x + size, y - size);
      if(do_stroke)
      {
         context.strokeStyle = context.fillStyle;
         context.lineWidth = 2;
      }
      context.stroke();
      break;

   case Enum.POINT_TYPE.SMALLDOT:
      context.lineWidth = 0.1;
      context.arc(x, y, radius / 4, 0, Math.PI * 2, false);
      context.fill();
      if(do_stroke)
         context.stroke();
      break;

   case Enum.POINT_TYPE.LEFTTRIANGLE:
      context.lineWidth = 0.1;
      size = radius * Math.sqrt(2 * Math.PI / Math.sin(Math.PI / 3));
      height = size * Math.sin(Math.PI / 3);
      context.moveTo(x + size / 2, y - height / 2);
      context.lineTo(x + size / 2, y + height / 2);
      context.lineTo(x - size / 2, y);
      context.lineTo(x + size / 2, y - height / 2);
      context.fill();
      if(do_stroke)
         context.stroke();
      break;

   case Enum.POINT_TYPE.RIGHTTRIANGLE:
      context.lineWidth = 0.1;
      size = radius * Math.sqrt(2 * Math.PI / Math.sin(Math.PI / 3));
      height = size * Math.sin(Math.PI / 3);
      context.moveTo(x - size / 2, y - height / 2);
      context.lineTo(x - size / 2, y + height / 2);
      context.lineTo(x + size / 2, y);
      context.lineTo(x - size / 2, y - height / 2);
      context.fill();
      if(do_stroke)
         context.stroke();
      break;
   }
};


CsiGraphSeries.prototype.draw_bar_type = function (context, bar_type, bar_rect)
{
   var draw_border = bar_rect.height > 0 && this.bar_border_line_type !== Enum.LINE_TYPE.CLEAR;
   if(draw_border)
   {
      bar_rect.set_left(bar_rect.left + this.bar_border_width / 2);
      bar_rect.set_width(bar_rect.width - this.bar_border_width);
   }

   if(bar_rect.height === 0)
   {
      bar_rect.set_height(1);
      bar_rect.set_top(bar_rect.top - 0.5);
   }

   switch(bar_type)
   {
      case Enum.BAR_TYPE.PYRAMID:
         context.beginPath();
         context.moveTo(bar_rect.left + (bar_rect.width / 2), bar_rect.top);
         context.lineTo(bar_rect.right, bar_rect.bottom);
         context.lineTo(bar_rect.left, bar_rect.bottom);
         context.closePath();

         if(draw_border)
         {
            context.stroke();
         }
         context.fill();
         break;
      case Enum.BAR_TYPE.INVPYRAMID:
         context.beginPath();
         context.moveTo(bar_rect.left, bar_rect.top);
         context.lineTo(bar_rect.right, bar_rect.top);
         context.lineTo(bar_rect.left + bar_rect.width / 2, bar_rect.bottom);
         context.closePath();

         if(draw_border)
         {
            context.stroke();
         }
         context.fill();
         break;
      case Enum.BAR_TYPE.ELLIPSE:
         draw_ellipse(context, bar_rect);

         if(draw_border)
         {
            context.stroke();
         }
         context.fill();
         break;
      case Enum.BAR_TYPE.ARROW:
         var arrow_edge = bar_rect.width / 4;
         context.beginPath();
         context.moveTo(bar_rect.left + arrow_edge, bar_rect.bottom); //Bottom Left
         context.lineTo(bar_rect.right - arrow_edge, bar_rect.bottom); //Bottom Right
         context.lineTo(bar_rect.right - arrow_edge, bar_rect.top + 2 * arrow_edge); //Arrow base right
         context.lineTo(bar_rect.right, bar_rect.top + 2 * arrow_edge); //Arrow head right
         context.lineTo(bar_rect.right - bar_rect.width / 2, bar_rect.top); //Arrow tip
         context.lineTo(bar_rect.left, bar_rect.top + 2 * arrow_edge); //Arrow head left
         context.lineTo(bar_rect.left + arrow_edge, bar_rect.top + 2 * arrow_edge); //Arrow base left
         context.closePath();

         if(draw_border)
         {
            context.stroke();
         }
         context.fill();
         break;
      case Enum.BAR_TYPE.RECTGRADIENT:
         var bar_gradient = context.createLinearGradient(bar_rect.left, bar_rect.top, bar_rect.right, bar_rect.bottom);
         bar_gradient.addColorStop(0, this.bar_color);
         bar_gradient.addColorStop(1, "#FFFFFF");
         context.fillStyle = bar_gradient;

         if(draw_border)
         {
            context.strokeRect(bar_rect.left, bar_rect.top, bar_rect.width, bar_rect.height);
            context.fillRect(bar_rect.left, bar_rect.top, bar_rect.width, bar_rect.height);
         }
         else
         {
            //If we have no line border, we will frame the gradient bar with the bar color
            context.fillRect(bar_rect.left, bar_rect.top, bar_rect.width, bar_rect.height);

            context.lineWidth = 1;
            context.strokeStyle = this.bar_color;
            context.strokeRect(bar_rect.left, bar_rect.top, bar_rect.width, bar_rect.height);
         }
         break;
      //case Enum.BAR_TYPE.CILINDER: 
      //case Enum.BAR_TYPE.RECTANGLE: 
      default:
         //Build up the rectangle
         if(draw_border)
         {
            context.strokeRect(bar_rect.left, bar_rect.top, bar_rect.width, bar_rect.height);
         }
         context.fillRect(bar_rect.left, bar_rect.top, bar_rect.width, bar_rect.height);
         break;
   }
};


CsiGraphSeries.prototype.hit_test = function (pt, hit_space)
{
   var rtn = false;

   if (this.marks_show_on_click)
   {
      if (this.point_size > hit_space)
      {
         hit_space = this.point_size;
      }

      var len = this.mark_cache.length;
      var i;
      for (i = 0; i < len; i++)
      {
         var point_rect = new Rect(this.mark_cache[i].pt_x - hit_space, this.mark_cache[i].pt_y - hit_space, 2 * hit_space, 2 * hit_space);
         if (point_rect.contains(pt))
         {
            this.current_mark = this.mark_cache[i];
            this.current_mark.start_time = CsiLgrDate.local();
            rtn = true;
            break;
         }
      }

      if (!rtn)
      {
         this.current_mark = null;
      }
   }

   return rtn;
};


function CsiGraphMark(x_val, y_val, pt_x, pt_y)
{
   this.x_val = x_val;
   this.y_val = y_val;
   this.pt_x = pt_x;
   this.pt_y = pt_y;
   this.start_time = null;
}


Enum.SERIES_TYPE =
{
   LINE: 0,
   BAR: 1,
   POINT: 2
};


Enum.POINT_TYPE =
{
   RECTANGLE: 0,
   CIRCLE: 1,
   TRIANGLE: 2,
   DOWNTRIANGLE: 3,
   CROSS: 4,
   DIAGCROSS: 5,
   STAR: 6,
   DIAMOND: 7,
   SMALLDOT: 8,
   NOTHING: 9,
   LEFTTRIANGLE: 10,
   RIGHTTRIANGLE: 11
};


Enum.BAR_TYPE =
{
   RECTANGLE: 0,
   PYRAMID: 1,
   INVPYRAMID: 2,
   CILINDER: 3,
   ELLIPSE: 4,
   ARROW: 5,
   RECTGRADIENT: 6
};


Enum.MARKS_TYPE =
{
   VALUE: 0,
   TIMESTAMP: 1,
   BOTH: 2
};


function CsiScale()
{
   this.scale = 1.0;
   this.labels = [];
   this.size = 100;
   this.rotation = 0;
   this.inverted = false;
   this.max_value = 100;
   this.min_value = 0;
   this.vertical = false;
   this.time_domain = false;
   this.time_format = "%c";
   this.logarithmic = false;
   this.log_base = 10;
   this.fixed_decimals = false;
   this.decimal_places = 2;
   this.one_tick_only = false;
}


CsiScale.prototype.format_label = function (value)
{
   var rtn;
   if(this.custom_format)
      rtn = this.custom_format(this, value);
   else
   {
      if(!this.time_domain)
      {
         var format = "%.10g";
         if(this.fixed_decimals)
            format = "%." + this.decimal_places + "f";
         if(this.logarithmic)
            rtn = sprintf(format, Math.pow(this.log_base, value));
         else
            rtn = sprintf(format, value);
      }
      else
      {
         if(!(value instanceof CsiLgrDate))
            value = new CsiLgrDate(value);
         rtn = value.format(this.time_format);
      }
   }
   return rtn;
};


CsiScale.prototype.calculate_scale = function (min_value, max_value, context)
{
   // if the max and min values are the same (or very close), we will
   // need to fudge the range somewhat
   var range = Math.abs(max_value - min_value);
   this.one_tick_only = false;
   if(range <= 1e-38)
   {
      max_value = min_value + 0.5;
      min_value = max_value - 1;
      range = 1;
      this.one_tick_only = true;
   }

   // if this scale is logarithmic, we will need to evaluate the log
   // of the max and min.
   if(this.logarithmic)
   {
      if(min_value <= 0)
         min_value = 1;
      if(max_value < min_value)
         max_value = min_value + 1;
      max_value = Math.round(Math.log(max_value) / Math.log(this.log_base));
      min_value = Math.round(Math.log(min_value) / Math.log(this.log_base));
      range = Math.abs(max_value - min_value);
      if(range < 1)
      {
         min_value = max_value - 1;
         range = 1;
      }
   }

   // we can now calculate the scale and interval
   var format_min = this.format_label(min_value);
   var format_max = this.format_label(max_value);
   var min_rect = this.measure_label(format_min, context);
   var max_rect = this.measure_label(format_max, context);
   var label_req = Math.max(min_rect.get_width(), max_rect.get_width());

   if(this.vertical)
      label_req = Math.max(min_rect.get_height(), max_rect.get_height());
   this.scale = this.size / range;
   this.max_value = max_value;
   this.min_value = min_value;
   if(this.logarithmic)
   {
      // we will produce a label for each power of the log base within the range
      this.label_interval = 1;
   }
   else if(this.label_interval <= 0)
   {
      // we need to determine the optimal interval to use.  The
      // algorithm used will depend upon whether this is a time domain
      // axis.
      if(!this.time_domain)
      {
         this.label_interval = CsiScale.scalar_optimum_step_size(
            range, this.size, label_req);
      }
      else
      {
         this.label_interval = CsiScale.time_optimum_step_size(
            range, this.size, label_req);
      }
   }
   else
   {
      // the properties have set up a fixed interval for the labels.
      // Even if this is the case, we may need to revert to
      // auto-labels if the count specified results in labels that are
      // too close together.
      if(this.label_interval * this.scale < label_req)
      {
         if(!this.time_domain)
         {
            this.label_interval = CsiScale.scalar_optimum_step_size(
               range, this.size, label_req);
         }
         else
         {
            this.label_interval = CsiScale.time_optimum_step_size(
               range, this.size, label_req);
         }
      }
   }
   return { scale: this.scale, interval: this.label_interval };
};


CsiScale.prototype.generate_labels = function (min_value, max_value, context)
{
   // We can now calculate the value of the first tick.
   var first_tick = 0;
   this.calculate_scale(min_value, max_value, context);
   if(!this.one_tick_only)
   {
      if(this.min_value <= 0.0 && !this.logarithmic)
         first_tick = this.min_value + (Math.abs(this.min_value) % this.label_interval);
      else
         first_tick = this.min_value - (Math.abs(this.min_value) % this.label_interval);
   }
   else
      first_tick = (this.max_value - this.min_value) / 2 + this.min_value;

   // finally, we can generate the labels and their positions and values
   var tick_count = 0;
   var tick = first_tick;
   var max_places = 0;
   this.labels.splice(0);
   while(tick <= this.max_value)
   {
      if(tick >= this.min_value)
      {
         var tick_value = (this.logarithmic ? Math.pow(this.log_base, tick) : tick);
         var formatted_label = this.format_label(tick);
         var label = {
            "label": formatted_label,
            "value": tick_value,
            "position": this.value_to_pos(tick_value),
            "rect": this.measure_label(formatted_label, context)
         };
         if(!this.fixed_decimal && !this.time_domain)
            max_places = Math.max(CsiScale.count_decimals(formatted_label), max_places);
         if(this.vertical)
            label.rect.center_y(label.position);
         else
            label.rect.center_x(label.position);
         this.labels.push(label);
         if(this.one_tick_only)
            break;
      }
      tick = this.next_tick(first_tick, ++tick_count, this.label_interval);
   }

   // We need to adjust the formatting of automatically generated
   // labels so that the number of places following the decimal point
   // is consistent for all labels.  This should keep the decimal
   // points aligned on the left axis.
   var i = 0;
   var cnt = this.labels.length;
   if(max_places > 0 && !this.time_domain)
   {
      for(i = 0; i < cnt; ++i)
      {
         var adj_label = this.labels[i];
         adj_label.label = CsiScale.append_decimals(adj_label.label, max_places);
      }
   }

   // the final thing to do is to eliminate any labels that are duplicates.
   var last_valid = '';
   for(i = 1; i < cnt; ++i)
   {
      var cur_label = this.labels[i];
      var prev_label = this.labels[i - 1];
      if(cur_label.label === prev_label.label)
      {
         last_valid = cur_label.label;
         cur_label.label = '';
      }
      else if(cur_label.label === last_valid)
         cur_label.label = '';
      else
         last_valid = '';
   }
   return this.labels;
};


CsiScale.prototype.generate_minor_ticks = function (min_width, count)
{
   var rtn = [];
   if(this.labels.length > 0 && !this.one_tick_only)
   {
      // we may have to calculate the best minor tick interval
      var tick_interval = this.label_interval / (count + 1);
      var tick_width = tick_interval * this.scale;
      var space = this.label_interval * this.scale;
      if(tick_width < min_width || isNaN(tick_interval))
      {
         var intervals_count = space / min_width;
         if(intervals_count > 9)
            min_width = space / 9;
         if(this.time_domain)
         {
            tick_interval = CsiScale.time_optimum_step_size(
               this.label_interval, space, min_width);
         }
         else
         {
            tick_interval = CsiScale.scalar_optimum_step_size(
               this.label_interval, space, min_width);
            if(this.label_interval % tick_interval > 1e-38)
            {
               var formatted_tick = sprintf("%g", tick_interval);
               var least_digit_pos = formatted_tick.search(/[^0]([eE][+-]?\d)?$/);
               var formatted_tick_array = Csi.string_to_array(formatted_tick);
               if(least_digit_pos < 0)
               {
                  least_digit_pos = formatted_tick.length - 1;
               }

               if(formatted_tick_array[least_digit_pos] === '2')
               {
                  formatted_tick_array[least_digit_pos] = '5';
               }
               formatted_tick = formatted_tick_array.join('');
               tick_interval = Number(formatted_tick);
            }
         }
      }

      // we will generate minor ticks for each label interval
      var labels_count = this.labels.length;
      var scale_min = this.min_value;
      var scale_max = this.max_value;

      if(this.logarithmic)
      {
         scale_max = Math.pow(this.log_base, scale_max);
         scale_min = Math.pow(this.log_base, scale_min);
      }

      var i;
      for(i = 0; i <= labels_count; ++i)
      {
         var label_min;
         var label_max;
         var first_tick = 0;
         var tick = 0;
         var ticks_count = 0;

         var label;
         if(i < labels_count)
         {
            label = this.labels[i];
            label_max = label.value;
            if(!this.logarithmic)
               label_min = label.value - this.label_interval;
            else
            {
               label_min = Math.pow(
                  this.log_base,
                  (Math.log(label_max) / Math.log(this.log_base)) - 1);
            }
         }
         else
         {
            label = this.labels[i - 1];
            label_min = label.value;
            if(!this.logarithmic)
               label_max = label.value + this.label_interval;
            else
            {
               label_max = Math.pow(
                  this.log_base,
                  (Math.log(label_min) / Math.log(this.log_base)) + 1);
            }
         }
         if(this.logarithmic)
         {
            if(count > this.log_base)
               count = this.log_base;
            tick_interval = Math.abs(label_max - label_min) / count;
         }
         if(label_min <= 0)
            first_tick = label_min + Math.abs(label_min % tick_interval);
         else
            first_tick = label_min - Math.abs(label_min % tick_interval);
         tick = first_tick;
         while(tick < label_max && tick < scale_max)
         {
            if(tick > scale_min && tick > label_min)
               rtn.push(tick);
            tick = this.next_tick(first_tick, ++ticks_count, tick_interval);
         }
      }
   }
   return rtn;
};


CsiScale.prototype.value_to_pos = function (value)
{
   var rtn;
   if(this.logarithmic)
      value = Math.log(value) / Math.log(this.log_base);
   if(this.vertical)
   {
      if(this.inverted)
         rtn = this.scale * (value - this.min_value);
      else
         rtn = this.scale * (this.max_value - value);
   }
   else
   {
      if(this.inverted)
         rtn = this.scale * (this.max_value - value);
      else
         rtn = this.scale * (value - this.min_value);
   }
   return rtn;
};


CsiScale.prototype.value_offset_weight = function (value, offset)
{
   var value_pos = this.value_to_pos(value);
   var rtn;
   if(this.vertical)
   {
      if(this.inverted)
         rtn = Math.abs(this.pos_to_value(value_pos + offset) - value);
      else
         rtn = Math.abs(this.pos_to_value(value_pos - offset) - value);
   }
   else
   {
      if(this.inverted)
         rtn = Math.abs(this.pos_to_value(value_pos - offset) - value);
      else
         rtn = Math.abs(this.pos_to_value(value_pos + offset) - value);
   }
   return rtn;
};


CsiScale.prototype.pos_to_value = function (pos)
{
   var rtn;
   if(this.vertical)
   {
      if(this.inverted)
         rtn = pos / this.scale + this.min_value;
      else
         rtn = this.max_value - pos / this.scale;
   }
   else
   {
      if(this.inverted)
         rtn = this.max_value - pos / this.scale;
      else
         rtn = pos / this.scale + this.min_value;
   }
   if(this.logarithmic)
      rtn = Math.pow(this.log_base, rtn);
   return rtn;
};


CsiScale.prototype.measure_label = function (label, context)
{
   var rtn = CsiScale.measure_text(label, context);
   if(this.rotation !== 0)
      rtn.rotate(this.rotation);
   return rtn;
};


CsiScale.prototype.get_max_value = function ()
{
   var rtn = this.max_value;
   if(this.logarithmic)
      rtn = Math.pow(this.log_base, rtn);
   return rtn;
};


CsiScale.prototype.get_min_value = function ()
{
   var rtn = this.min_value;
   if(this.logarithmic)
      rtn = Math.pow(this.log_base, rtn);
   return rtn;
};


CsiScale.prototype.next_tick = function (first_tick, ticks_count, tick_interval)
{
   var rtn = first_tick + ticks_count * tick_interval;
   if(this.time_domain && tick_interval >= CsiScale.month_interval)
   {
      var temp = new CsiLgrDate(first_tick);
      var date = temp.toDate();
      date.day = 1;
      if(tick_interval >= CsiScale.month_interval &&
         tick_interval < CsiScale.quarter_interval)
      {
         date.month += ticks_count % 12;
         date.year += Math.floor(ticks_count / 12);
      }
      else if(tick_interval >= CsiScale.quarter_interval &&
              tick_interval < CsiScale.half_year_interval)
      {
         date.month += (ticks_count * 3) % 12;
         date.year += Math.floor((ticks_count * 3) / 12);
      }
      else if(tick_interval >= CsiScale.half_year_interval &&
              tick_interval < CsiScale.year_interval)
      {
         date.month += (ticks_count * 6) % 12;
         date.year += Math.floor((ticks_count * 6) / 12);
      }
      else
      {
         date.month = 1;
         date.year += ticks_count;
      }
      if(date.month > 12)
      {
         date.month = date.month - 12;
         date.year += 1;
      }
      temp.setDate(date.year, date.month, date.day);
      rtn = temp.milliSecs;
   }
   return rtn;
};


CsiScale.scalar_optimum_step_size = function (
   range, space, label_req)
{
   // we will make an initial estimate based upon the space available and the
   // range to fit.  We can then format that value as a fixed decimal string.
   var max_steps = space / label_req;
   var step_size = range / max_steps;
   var step_str = step_size.toFixed(7);

   // we now need to adjust this formatted step so that the scale is formatted
   // with convenient (to the user) labels.  
   var regex = /[^0\.](0|\.)*$/;
   var last_pos = step_str.search(regex);

   while(last_pos >= 0)
   {
      // we need to find the digit preceding this one.
      var temp = step_str.slice(0, last_pos);
      var previous_pos = temp.search(regex);
      if(previous_pos >= 0)
      {
         step_str = CsiScale.clear_with_carry(step_str, last_pos);
         last_pos = step_str.search(regex);
      }
      else
      {
         // we have reduced the step to its most significant digit.  This needs
         // to be rounded to the closest "convenient" digit.
         var array = Csi.string_to_array(step_str);
         switch(array[last_pos])
         {
            case '1':
            case '2':
            case '5':
               // these are digits that we want in the most significant place
               break;

            case '3':
            case '4':
               array[last_pos] = '5';
               break;

            case '6':
            case '7':
            case '8':
            case '9':
               temp = CsiScale.clear_with_carry(array.join(""), last_pos);
               array = Csi.string_to_array(temp);
               break;

            case '0':
               array[last_pos] = '1';
               break;
         }
         last_pos = -1;
         step_size = parseFloat(array.join(""));
      }
   }

   // as a final check, we need to make sure that there is at least one step.
   if(step_size <= 0)
      step_size = 1;
   return step_size;
};


CsiScale.clear_with_carry = function (buff, pos)
{
   // we need to find the digit before the specified position
   var array = Csi.string_to_array(buff);
   var previous_pos = pos - 1;
   if(previous_pos >= 0 && array[previous_pos] === '.')
      --previous_pos;
   array[pos] = '0';
   if(previous_pos >= 0)
   {
      if(array[previous_pos] !== '9')
         array[previous_pos] = String.fromCharCode(array[previous_pos].charCodeAt(0) + 1);
      else
         return CsiScale.clear_with_carry(array.join(""), previous_pos);
   }
   else
      array.unshift('1');
   return array.join("");
};


CsiScale.measure_text = function (text, context)
{
   var metrics = context.measureText(text);
   if(!("height" in metrics)) 
      metrics.height = context.measureText("W").width;
   return new CsiNestedRect(0, 0, metrics.width, metrics.height * 1.1);
};


CsiScale.month_interval = 4 * CsiLgrDate.msecPerWeek;
CsiScale.quarter_interval = 12 * CsiLgrDate.msecPerWeek;
CsiScale.half_year_interval = 26 * CsiLgrDate.msecPerWeek;
CsiScale.year_interval = 52 * CsiLgrDate.msecPerWeek;
CsiScale.time_intervals = [
   1,
   2,
   5,
   10,
   20,
   50,
   100,
   200,
   500,
   1 * CsiLgrDate.msecPerSec,
   5 * CsiLgrDate.msecPerSec,
   15 * CsiLgrDate.msecPerSec,
   30 * CsiLgrDate.msecPerSec,
   1 * CsiLgrDate.msecPerMin,
   5 * CsiLgrDate.msecPerMin,
   15 * CsiLgrDate.msecPerMin,
   30 * CsiLgrDate.msecPerMin,
   1 * CsiLgrDate.msecPerHour,
   2 * CsiLgrDate.msecPerHour,
   6 * CsiLgrDate.msecPerHour,
   12 * CsiLgrDate.msecPerHour,
   1 * CsiLgrDate.msecPerDay,
   2 * CsiLgrDate.msecPerDay,
   5 * CsiLgrDate.msecPerDay,
   1 * CsiLgrDate.msecPerWeek,
   2 * CsiLgrDate.msecPerWeek,
   4 * CsiScale.month_interval,
   12 * CsiScale.quarter_interval,
   26 * CsiScale.half_year_interval,
   52 * CsiScale.year_interval
];


CsiScale.time_optimum_step_size = function (
   range, space, label_req)
{
   var step_size;
   if(range !== Infinity)
   {
      // we will make an initial estimate based upon the parameters
      var max_steps = space / label_req;
      var len = CsiScale.time_intervals.length;
      var i = 0;
      step_size = range / max_steps;
      while(i < len && step_size > CsiScale.time_intervals[i])
         ++i;
      if(i < len)
         step_size = CsiScale.time_intervals[i];
      else
         step_size = CsiScale.scalar_optimum_step_size(range, space, label_req);
   }
   else
      step_size = 100;
   return step_size;
};


CsiScale.count_decimals = function (value)
{
   var decimal_pos = value.indexOf(".");
   var rtn = 0;
   if(decimal_pos >= 0)
   {
      var exp_pos = value.search(/[eE]/);
      if(exp_pos < 0)
         exp_pos = value.length;
      rtn = exp_pos - decimal_pos - 1;
   }
   return rtn;
};


CsiScale.append_decimals = function (value, decimals)
{
   var decimal_pos = value.indexOf(".");
   var needed = decimals - CsiScale.count_decimals(value);
   var array = Csi.string_to_array(value);
   var exp_pos = value.search(/[eE]/);

   if(exp_pos < 0)
      exp_pos = value.length;
   if(decimal_pos < 0)
   {
      array.splice(exp_pos, 0, '.');
      ++exp_pos;
   }
   while(needed > 0)
   {
      array.splice(exp_pos, 0, '0');
      --needed;
   }
   return array.join('');
};


function CsiNestedRect()
{
   this.children = [];
   this.is_stale = false;
   this.parent = null;
   if(arguments.length === 1)
   {
      var arg0 = arguments[0]; 
      if(arg0 instanceof Rect)
         Rect.call(this, arg0.left, arg0.top, arg0.width, arg0.height);
      else if(arg0 instanceof CsiNestedRect)
         Rect.call(this, arg0.left, arg0.top, arg0.width, arg0.height);
   }
   else if(arguments.length === 4)
      Rect.call(this, arguments[0], arguments[1], arguments[2], arguments[3]); 
   else
      Rect.call(this);
}
CsiNestedRect.prototype = new Rect();


CsiNestedRect.prototype.add = function ()
{
   if(arguments.length === 1)
   {
      var arg0 = arguments[0]; 
      if(arg0 instanceof CsiNestedRect)
      {
         this.children.push(arg0);
         arg0.parent = this;
         this.mark_stale();
      }
      else if(arg0 instanceof Rect)
      {
         this.add(new CsiNestedRect(arg0));
         this.mark_stale();
      }
   }
   if(arguments.length === 4)
   {
      this.add(
         new CsiNestedRect(
            arguments[0], arguments[1], arguments[2], arguments[3])); 
      this.mark_stale();
   }
};


CsiNestedRect.prototype.get_width = function ()
{ return this.get_rect(false).width; };


CsiNestedRect.prototype.get_height = function ()
{ return this.get_rect(false).height; };


CsiNestedRect.prototype.get_left = function (use_parent_coords)
{ return this.get_rect(use_parent_coords).left; };


CsiNestedRect.prototype.get_top = function (use_parent_coords)
{ return this.get_rect(use_parent_coords).top; };


CsiNestedRect.prototype.get_right = function (use_parent_coords)
{ return this.get_rect(use_parent_coords).right; };


CsiNestedRect.prototype.set_right = function(x)
{
   if(this.is_stale)
      this.layout();
   this.move(x - this.get_width(), this.y);
};


CsiNestedRect.prototype.get_bottom = function (use_parent_coords)
{ return this.get_rect(use_parent_coords).bottom; };


CsiNestedRect.prototype.set_bottom = function(y)
{
   if(this.is_stale)
      this.layout();
   this.move(this.get_left(), y - this.get_height());
};


CsiNestedRect.prototype.get_centre = function(use_parent_coords)
{
   return this.get_rect(use_parent_coords).get_center();
};


CsiNestedRect.prototype.centre_x = function(x)
{
   if(this.is_stale)
      this.layout();
   this.move(x - this.width / 2, this.top);
};


CsiNestedRect.prototype.centre_y = function(y)
{
   if(this.is_stale)
      this.layout();
   this.move(this.left, y - this.height / 2); 
};


CsiNestedRect.prototype.set_centre = function(x, y)
{
   if(this.is_stale)
      this.layout();
   this.move(x - this.width / 2, y - this.height / 2);
};


CsiNestedRect.prototype.get_centre = function(use_parent_coords)
{
   var temp;
   if(this.is_stale)
      this.layout();
   temp = this.get_rect(use_parent_coords);
   return temp.get_center();
};


CsiNestedRect.prototype.get_rect = function (use_parent_coords)
{
   // if children have been added, we will need to recalculate our own width
   // and height.
   if(this.is_stale)
      this.layout();

   // we must now determine whether to return our own coordinate system or to
   // translate ours into the parent's coordinate system
   var rtn = new Rect(this.left, this.top, this.width, this.height);
   var parent = this.parent;
   if(jQuery.type(use_parent_coords) === "undefined") 
      use_parent_coords = true;

   while(use_parent_coords && parent) 
   {
      rtn.offset(parent.left, parent.top);
      parent = parent.parent;
   }
   return rtn;
};


CsiNestedRect.prototype.layout = function ()
{
   // we will need to set the width and height of this rectangle
   // to match our own bounds as well as the bounds of our children
   if(this.children.length > 0)
   {
      var child_rect = new Rect();
      var i;
      this.children.forEach(function(child) {
         child_rect.union(child.get_rect(false));
      });
      if(this.width < child_rect.width)
         this.set_width(child_rect.width);
      if(this.height < child_rect.height)
         this.set_height(child_rect.height);
   }
   this.is_stale = false;
};


CsiNestedRect.prototype.mark_stale = function ()
{
   var pai = this;
   while(pai) 
   {
      pai.is_stale = true;
      pai = pai.parent;
   }
};


CsiNestedRect.prototype.move = function (top, left)
{
   Rect.prototype.move.call(this, top, left);
   this.mark_stale();
};


CsiNestedRect.prototype.rotate = function (degrees)
{
   Rect.prototype.rotate.call(this, degrees);
   this.mark_stale();
};


CsiNestedRect.prototype.offset = function (dx, dy)
{
   Rect.prototype.offset.call(this, dx, dy);
   this.mark_stale();
};


CsiNestedRect.prototype.center = function (cx, cy)
{
   Rect.prototype.center.call(this, cx, cy);
   this.mark_stale();
};


CsiNestedRect.prototype.get_center = function ()
{ return this.translate_point(this.width / 2, this.height / 2); };


CsiNestedRect.prototype.translate_point = function (x, y)
{
   var rtn = new Point(x, y);
   var pai = this;
   while(pai) 
   {
      rtn.x += pai.left;
      rtn.y += pai.top;
      pai = pai.parent;
   }
   return rtn;
};


CsiNestedRect.prototype.translate_from_point = function (x, y)
{
   var rtn = new Point(x, y);
   var pai = this;
   while(pai) 
   {
      rtn.x -= pai.left;
      rtn.y -= pai.top;
      pai = pai.parent;
   }
   return rtn;
};


CsiNestedRect.prototype.stack_vertical = function ()
{
   var offset = 0;
   this.children.forEach(function(child) {
      child.set_top(offset);
      offset += child.get_height();
   });
   this.layout();
};


CsiNestedRect.prototype.stack_horizontal = function ()
{
   var offset = 0;
   this.children.forEach(function(child) {
      child.set_left(offset);
      offset += child.get_width();
   });
   this.layout();
};


CsiNestedRect.prototype.stack_grid = function(fixed, is_vertical)
{
   // we need to calculate the maximum width and height of all of our children
   var max_width = 0;
   var max_height = 0;
   var fixed_count = 0;
   
   this.children.forEach(function(child) {
      var child_width = child.get_width();
      var child_height = child.get_height();
      if(child_width > max_width)
         max_width = child_width;
      if(child_height > max_height)
         max_height = child_height;
   });

   // we can now determine how many blocks will fit in the fixed dimension
   if(is_vertical)
      fixed_count = Math.floor(fixed / max_height);
   else
      fixed_count = Math.floor(fixed / max_width);
   if(fixed_count === 0)
      fixed_count = 1;

   // we can now iterate through the children in order and assign their positions within the grid.
   this.children.forEach(function(child, index) {
      if(is_vertical)
         child.move(Math.floor((index / fixed_count)) * max_width, (index % fixed_count) * max_height);
      else
         child.move((index % fixed_count) * max_width, Math.floor(index / fixed_count) * max_height);
   });
   this.layout();
};


CsiNestedRect.prototype.shift_children = function (dx, dy)
{
   this.children.forEach(function(child) {
      child.offset(dx, dy);
   });
};


function CsiGraphAxis(owner, axis_type)
{
   //Do not add properties to prototype
   if(arguments.length === 0)
   {
      return;
   }

   this.owner = owner;
   this.series = [];
   this.scale = null;
   this.axis_type = axis_type;
   this.time_format = "";
   this.title_caption = "";
   this.title_font = "12pt Arial";
   this.title_color = "#000000";
   this.title_angle = 0;
   this.auto_min = true;
   this.min = -1E38;
   this.min_offset = 5;
   this.auto_max = true;
   this.max = 1E38;
   this.max_offset = 5;
   this.auto_time = true;
   this.inverted = false;
   this.logarithmic = false;
   this.log_base = 10;
   this.labels_visible = true;
   this.labels_font = "10pt Arial";
   this.labels_color = "#000000";
   this.label_angle = 0;
   this.label_size = 0;
   this.auto_label = true;
   this.decimal_places = 0;
   this.fixed_decimals = false;
   this.increment = 0;
   this.minor_tick_count = 3;
   this.major_grid_pen_visible = 1;
   this.major_grid_pen_style = Enum.LINE_TYPE.SOLID;
   this.major_grid_pen_color = "rgba(126, 126, 126, .25)";
   this.major_grid_pen_width = 1;
   this.major_grid_pen_end_style = 0;
   this.major_grid_pen_small_space = 0;
   this.minor_grid_pen_visible = 0;
   this.minor_grid_pen_style = 0;
   this.minor_grid_pen_color = "#000000";
   this.minor_grid_pen_width = 1;
   this.minor_grid_pen_end_style = 0;
   this.minor_grid_pen_small_space = 0;
   this.major_ticks_pen_visible = 1;
   this.major_ticks_pen_style = 0;
   this.major_ticks_pen_color = "#000000";
   this.major_ticks_pen_width = 1;
   this.major_ticks_pen_end_style = 0;
   this.major_ticks_pen_small_space = 0;
   this.minor_ticks_pen_visible = 1;
   this.minor_ticks_pen_style = 0;
   this.minor_ticks_pen_color = "#000000";
   this.minor_ticks_pen_width = 1;
   this.minor_ticks_pen_end_style = 0;
   this.minor_ticks_pen_small_space = 0;
   this.axis_pen_visible = 1;
   this.axis_pen_style = 0;
   this.axis_pen_color = "#000000";
   this.axis_pen_width = 2;
   this.axis_pen_end_style = 0;
   this.axis_pen_small_space = 0;
}


CsiGraphAxis.axis_bottom = 0;
CsiGraphAxis.axis_left = 1;
CsiGraphAxis.axis_right = 2;


CsiGraphAxis.prototype.add_series = function (series)
{
   this.series.push(series);
   if(this.axis_type === CsiGraphAxis.axis_bottom)
      this.time_domain = true;
};


CsiGraphAxis.prototype.remove_series = function(series)
{
   var index = this.series.indexOf(series);
   if(index >= 0)
      this.series.splice(index, 1);
};


CsiGraphAxis.prototype.generate_axis_rect = function (context, space)
{
   // we need to initialise the scale if it has not already been created
   if(!this.scale)
   {
      this.scale = new CsiScale();
      if(this.custom_format)
         this.scale.custom_format = this.custom_format;
      if(this.axis_type !== CsiGraphAxis.axis_bottom)
         this.scale.vertical = true;
      this.scale.inverted = this.inverted;
      this.scale.time_domain = this.time_domain;
      this.scale.time_format = this.time_format;
      this.scale.rotation = this.label_angle;
      this.scale.logarithmic = this.logarithmic;
      this.scale.log_base = this.log_base;
      this.scale.fixed_decimals = this.fixed_decimals;
      this.scale.decimal_places = this.decimal_places;
   }
   
   // if there are no series associated with this axis, we will return an empty area.
   this.axis_rect = new CsiNestedRect();
   if(this.series.length > 0)
   {
      // set the spacing for the scale
      this.scale.size = space;
      this.scale.time_format = this.time_format;
      this.scale.time_domain = this.time_domain;

      // if this axis has a title, we will need to calculate the
      // rectangle associated with the title text.
      this.title_rect = null;
      if(this.title_caption && this.title_caption.length > 0)
      {
         context.font = this.title_font;
         this.title_rect = CsiScale.measure_text(this.title_caption, context);
         this.title_rect.rotate(this.title_angle);
      }

      // we now need to evaluate the min and max values for this scale.
      var max_value = -Number.MIN_VALUE, min_value = Number.MAX_VALUE;
      var cnt = this.series.length;
      var i;

      if(!this.auto_min)
      {
         if(this.time_domain && this.auto_time)
            min_value = this.owner.newestTimeStamp - this.owner.displayWidth;
         else
            min_value = this.min;
      }
      if(!this.auto_max)
      {
         if(this.time_domain && this.auto_time)
            max_value = this.owner.newestTimeStamp;
         else
            max_value = this.max;
      }
      for(i = 0; (this.auto_min || this.auto_max) && i < cnt; ++i)
      {
         var series = this.series[i];
         var series_maxima = series.get_bounds(this.axis_type);
         if(this.auto_max)
            max_value = Math.max(series_maxima.max, max_value);
         if(this.auto_min)
            min_value = Math.min(series_maxima.min, min_value);
      }
      if(max_value < min_value)
      {
         min_value = -0.5;
         max_value = 0.5;
      }
      if(this.time_domain && Math.abs(max_value - min_value) < 10)
         max_value = min_value + 10;
      if(!this.time_domain && Math.abs(max_value - min_value) < 0.01 && max_value !== min_value)
         max_value = min_value + 0.01;

      // generate the rectangle for the scale itself
      if(this.scale.vertical)
      {
         var scale_rect_width = 1;
         if(this.axis_pen_visible)
            scale_rect_width += this.axis_pen_width;
         if(this.major_ticks_pen_visible)
            scale_rect_width += 4;
         else if(this.minor_ticks_pen_visible)
            scale_rect_width += 2;
         this.scale_rect = new CsiNestedRect(0, 0, scale_rect_width, space);
      }
      else
      {
         var scale_rect_height = 1;
         if(this.axis_pen_visible)
            scale_rect_height += this.axis_pen_width;
         if(this.major_ticks_pen_visible)
            scale_rect_height += 4;
         else if(this.minor_ticks_pen_visible)
            scale_rect_height += 2;
         this.scale_rect = new CsiNestedRect(0, 0, space, scale_rect_height);
      }

      // for the iphone app, we will automatically adjust the labels based upon the min and max
      if(this.scale.time_domain && this.scale.time_format.length === 0)
      {
         var range = max_value - min_value;
         if(range > CsiLgrDate.msecPerDay)
            this.scale.time_format = "%m-%d %H:%M";
         else if(range > CsiLgrDate.msecPerHour)
            this.scale.time_format = "%H:%M";
         else
            this.scale.time_format = "%H:%M:%S%x";
      }
      
      // if the label interval is specified manually, we will need to
      // calculate the number of labels
      if(this.auto_label)
         this.scale.label_interval = 0;
      else
         this.scale.label_interval = this.increment;

      // we need to adjust the min and max values by the offset properties.
      this.scale.size = space;
      if(this.max_offset > 0 || this.min_offset > 0)
      {
         var offset;
         this.scale.calculate_scale(min_value, max_value, context);
         if(this.min_offset !== 0)
         {
            offset = this.scale.value_offset_weight(min_value, this.min_offset);
            min_value -= offset;
         }
         if(this.max_offset !== 0)
         {
            offset = this.scale.value_offset_weight(max_value, this.max_offset);
            max_value += offset;
         }
      }

      // we can now generate the labels
      var labels;
      this.labels_rect = new CsiNestedRect();
      context.font = this.labels_font;
      labels = this.scale.generate_labels(min_value, max_value, context);
      cnt = labels.length;
      for(i = 0; i < cnt; ++i)
         this.labels_rect.add(labels[i].rect);
      if(this.minor_tick_count > 0 &&
         (this.minor_grid_pen_visible || this.minor_ticks_pen_visible) &&
         this.scale.label_interval > 0.001)
      {
         var minor_tick_width = 0;
         if(this.minor_ticks_pen_visible)
            minor_tick_width = this.minor_ticks_pen_width;
         if(this.minor_grid_pen_visible && this.minor_grid_pen_width > minor_tick_width)
            minor_tick_width = this.minor_grid_pen_width;
         if(minor_tick_width > 0)
         {
            var tick_count = this.minor_tick_count;
            if(this.auto_label)
               tick_count = 1e38;
            this.minor_ticks = this.scale.generate_minor_ticks(minor_tick_width * 2, tick_count);
         }
      }
      else
         this.minor_ticks = [];

      // the final thing that we can do is to position the component rectangles within our axis rect
      switch(this.axis_type)
      {
         case CsiGraphAxis.axis_bottom:
            this.axis_rect.add(this.scale_rect);
            if(this.labels_visible)
            {
               this.axis_rect.add(this.labels_rect);
               if(this.labels_rect.get_height() < this.label_size)
               {
                  this.axis_rect.add(
                     new CsiNestedRect(
                        this.labels_rect.left,
                        this.labels_rect.top,
                        this.labels_rect.width,
                        this.label_size - this.labels_rect.height));
               }
            }
            if(this.title_rect)
            {
               this.title_rect.center(space / 2, 0);
               this.axis_rect.add(this.title_rect);
            }
            this.axis_rect.stack_vertical();
            break;

         case CsiGraphAxis.axis_left:
            if(this.title_rect)
            {
               this.title_rect.center(0, space / 2);
               this.axis_rect.add(this.title_rect);
            }

            if(this.labels_visible)
            {
               if(this.labels_rect.get_width() < this.label_size)
               {
                  this.axis_rect.add(
                     new CsiNestedRect(
                        this.labels_rect.left,
                        this.labels_rect.top,
                        this.label_size - this.labels_rect.width,
                        this.labels_rect.height));
               }
               this.axis_rect.add(this.labels_rect);
            }
            this.axis_rect.add(this.scale_rect);
            this.axis_rect.stack_horizontal();
            break;

         case CsiGraphAxis.axis_right:
            this.axis_rect.add(this.scale_rect);
            if(this.labels_visible)
            {
               this.axis_rect.add(this.labels_rect);
               if(this.labels_rect.get_width() < this.label_size)
               {
                  this.axis_rect.add(
                     new CsiNestedRect(
                        this.labels_rect.left,
                        this.labels_rect.top,
                        this.label_size - this.labels_rect.width,
                        this.labels_rect.height));
               }
            }
            if(this.title_rect)
            {
               this.title_rect.center(0, space / 2);
               this.axis_rect.add(this.title_rect);
            }
            this.axis_rect.stack_horizontal();
            break;
      }
   }
   return this.axis_rect;
};


CsiGraphAxis.prototype.get_labels = function ()
{ return this.scale.labels; };


CsiGraphAxis.prototype.draw_title = function (context)
{
   if(this.title_rect)
   {
      var centre = this.title_rect.get_center();
      context.save();
      context.fillStyle = this.title_color;
      context.font = this.title_font;
      context.textAlign = "center";
      context.textBaseline = "middle";
      context.translate(centre.x, centre.y);
      context.rotate(degreesToRadians(this.title_angle));
      context.fillText(this.title_caption, 0, 0);
      context.restore();
   }
};


CsiGraphAxis.prototype.draw_axis = function (context)
{
   if(this.axis_pen_visible)
   {
      // we will get the rectangle for this axis in parent coords
      var rect = this.owner.plotRect;
      var x1, y1;
      var x2, y2;

      switch(this.axis_type)
      {
         case CsiGraphAxis.axis_bottom:
            x1 = rect.left;
            y1 = rect.bottom;
            x2 = rect.right;
            y2 = rect.bottom;
            break;

         case CsiGraphAxis.axis_left:
            x1 = rect.left;
            y1 = rect.top;
            x2 = rect.left;
            y2 = rect.bottom;
            break;

         case CsiGraphAxis.axis_right:
            x1 = rect.right;
            y1 = rect.top;
            x2 = rect.right;
            y2 = rect.bottom;
            break;
      }
      context.lineWidth = this.axis_pen_width;
      context.strokeStyle = this.axis_pen_color;
      Csi.draw_line(context, x1, y1, x2, y2, this.axis_pen_style, [0, 0]);
   }
};


CsiGraphAxis.prototype.draw_labels = function (context)
{
   if(this.labels_visible)
   {
      var labels = this.scale.labels;
      var cnt = labels.length;
      context.fillStyle = this.labels_color;
      context.font = this.labels_font;
      if((this.label_angle % 360) === 0)
      {
         switch(this.axis_type)
         {
            case CsiGraphAxis.axis_bottom:
               context.textAlign = "center";
               context.textBaseline = "top";
               break;

            case CsiGraphAxis.axis_left:
               context.textAlign = "right";
               context.textBaseline = "middle";
               break;

            case CsiGraphAxis.axis_right:
               context.textAlign = "left";
               context.textBaseline = "middle";
               break;
         }
      }
      else
      {
         context.textAlign = "center";
         context.textBaseline = "middle";
      }

      var i;
      for(i = 0; i < cnt; ++i)
      {
         var label = labels[i];
         var rect = label.rect;
         var location = rect.get_center();

         if((this.label_angle % 360) === 0)
         {
            switch(this.axis_type)
            {
               case CsiGraphAxis.axis_bottom:
                  location.y = this.labels_rect.get_top();
                  break;

               case CsiGraphAxis.axis_left:
                  location.x = this.labels_rect.get_right();
                  break;

               case CsiGraphAxis.axis_right:
                  location.x = this.labels_rect.get_left();
                  break;
            }
            context.fillText(label.label, location.x, location.y);
         }
         else
         {
            context.save();
            context.translate(location.x, location.y);
            context.rotate(degreesToRadians(this.label_angle));
            context.fillText(label.label, 0, 0);
            context.restore();
         }
      }
   }
};


CsiGraphAxis.prototype.draw_major_ticks = function (context)
{
   if(this.major_ticks_pen_visible)
   {
      var labels = this.scale.labels;
      var cnt = labels.length;
      context.lineWidth = this.major_ticks_pen_width;
      context.lineCap = "butt";
      context.strokeStyle = this.major_ticks_pen_color;
      var i;
      for(i = 0; i < cnt; ++i)
      {
         var label = labels[i];
         var pos = this.value_to_pos(label.value);
         var x1, y1, x2, y2;
         switch(this.axis_type)
         {
            case CsiGraphAxis.axis_bottom:
               x1 = x2 = pos;
               y1 = this.scale_rect.get_top();
               y2 = this.scale_rect.get_bottom();
               break;

            case CsiGraphAxis.axis_left:
            case CsiGraphAxis.axis_right:
               y1 = y2 = pos;
               x1 = this.scale_rect.get_left();
               x2 = this.scale_rect.get_right();
               break;
         }
         context.beginPath();
         context.moveTo(Math.floor(x1) + 0.5, Math.floor(y1) + 0.5);
         context.lineTo(Math.floor(x2) + 0.5, Math.floor(y2) + 0.5);
         context.stroke();
      }
   }
};


CsiGraphAxis.prototype.draw_minor_ticks = function (context)
{
   if(!this.minor_ticks)
      this.minor_ticks = [];
   if(this.minor_ticks.length > 0 && this.minor_ticks_pen_visible)
   {
      context.lineWidth = this.minor_ticks_pen_width;
      context.lineCap = "butt";
      context.strokeStyle = this.minor_ticks_pen_color;
      var i;
      for(i = 0; i < this.minor_ticks.length; ++i)
      {
         var tick = this.minor_ticks[i];
         var pos = this.value_to_pos(tick);
         var x1, y1, x2, y2;
         switch(this.axis_type)
         {
            case CsiGraphAxis.axis_bottom:
               x1 = x2 = pos;
               y1 = this.scale_rect.get_top();
               y2 = this.scale_rect.get_top() + 3;
               break;

            case CsiGraphAxis.axis_left:
               y1 = y2 = pos;
               x1 = this.scale_rect.get_right();
               x2 = this.scale_rect.get_right() - 3;
               break;

            case CsiGraphAxis.axis_right:
               y1 = y2 = pos;
               x1 = this.scale_rect.get_left();
               x2 = this.scale_rect.get_left() + 3;
               break;
         }
         context.beginPath();
         context.moveTo(Math.floor(x1) + 0.5, Math.floor(y1) + 0.5);
         context.lineTo(Math.floor(x2) + 0.5, Math.floor(y2) + 0.5);
         context.stroke();
      }
   }
};


CsiGraphAxis.prototype.draw_minor_grid = function (context)
{
   var cnt = this.minor_ticks.length;
   if(cnt > 0 && this.minor_grid_pen_visible)
   {
      var rect = this.owner.plotRect;
      context.lineWidth = this.minor_grid_pen_width;
      context.lineCap = "butt";
      context.strokeStyle = this.minor_grid_pen_color;
      var i;
      for(i = 0; i < cnt; ++i)
      {
         var tick = this.minor_ticks[i];
         var pos = this.value_to_pos(tick);
         var x1, y1, x2, y2;
         switch(this.axis_type)
         {
            case CsiGraphAxis.axis_bottom:
               x1 = x2 = pos;
               y1 = rect.top;
               y2 = rect.bottom;
               break;

            case CsiGraphAxis.axis_left:
            case CsiGraphAxis.axis_right:
               y1 = y2 = pos;
               x1 = rect.left;
               x2 = rect.right;
               break;
         }
         Csi.draw_line(context, x1, y1, x2, y2, this.minor_grid_pen_style);
      }
   }
};


CsiGraphAxis.prototype.draw_grid = function (context)
{
   if(this.major_grid_pen_visible)
   {
      var labels = this.scale.labels;
      var cnt = labels.length;
      var plot_rect = this.owner.plotRect;

      context.lineWidth = this.major_grid_pen_width;
      context.lineCap = "butt";
      context.strokeStyle = this.major_grid_pen_color;
      var i;
      for(i = 0; i < cnt; ++i)
      {
         var label = labels[i];
         var pos = this.value_to_pos(label.value);
         var x1, y1, x2, y2;
         switch(this.axis_type)
         {
         case CsiGraphAxis.axis_bottom:
            x1 = pos;
            y1 = plot_rect.top;
            x2 = pos;
            y2 = plot_rect.bottom;
            break;
            
         case CsiGraphAxis.axis_left:
         case CsiGraphAxis.axis_right:
            x1 = plot_rect.left;
            y1 = pos;
            x2 = plot_rect.right;
            y2 = pos;
            break;
         }
         Csi.draw_line(context, x1, y1, x2, y2, this.major_grid_pen_style);
      }
   }
};


CsiGraphAxis.prototype.draw = function (context)
{
   if(this.scale)
   {
      this.draw_title(context);
      this.draw_axis(context);
      this.draw_labels(context);
      this.draw_minor_ticks(context);
      this.draw_major_ticks(context);
      this.draw_grid(context);
      this.draw_minor_grid(context);
   }
};


CsiGraphAxis.prototype.value_to_pos = function (value)
{
   var rtn = 0;
   if(this.scale && this.scale_rect)
   {
      var pos = this.scale.value_to_pos(value);
      switch(this.axis_type)
      {
         case CsiGraphAxis.axis_bottom:
            rtn = this.scale_rect.translate_point(pos, 0).x;
            break;

         case CsiGraphAxis.axis_left:
         case CsiGraphAxis.axis_right:
            rtn = this.scale_rect.translate_point(0, pos).y;
            break;
      }
   }
   return rtn;
};


CsiGraphAxis.prototype.pos_to_value = function (pos)
{
   var rtn = 0;
   if(this.scale && this.scale_rect)
   {
      var point = 0;
      switch(this.axis_type)
      {
         case CsiGraphAxis.axis_bottom:
            point = this.scale_rect.translate_from_point(pos, 0).x;
            break;

         case CsiGraphAxis.axis_left:
         case CsiGraphAxis.axis_right:
            point = this.scale_rect.translate_from_point(0, pos).y;
            break;
      }
      rtn = this.scale.pos_to_value(point);
   }
   return rtn;
};


CsiGraphAxis.prototype.save_state = function (clear_time_format)
{
   this.saved_min = this.min;
   this.saved_max = this.max;
   this.saved_auto_min = this.auto_min;
   this.saved_auto_max = this.auto_max;
   this.saved_auto_time = this.auto_time;
   this.saved_auto_label = this.auto_label;
   this.saved_increment = this.increment;
   if(this.time_domain)
   {
      this.saved_time_format = this.time_format;
      if(clear_time_format)
      {
         this.time_format = "";
      }
   }
};


CsiGraphAxis.prototype.restore_state = function ()
{
   this.min = this.saved_min;
   this.max = this.saved_max;
   this.auto_min = this.saved_auto_min;
   this.auto_max = this.saved_auto_max;
   this.auto_time = this.saved_auto_time;
   this.auto_label = this.saved_auto_label;
   this.increment = this.saved_increment;
   if(this.time_domain)
   {
      this.time_format = this.saved_time_format;
   }
};


function CsiGradient(rect)
{
   this.direction = Enum.GRADIENT_DIRECTION.TopBottom;
   this.startColor = "green";
   this.midColor = null;
   this.endColor = "black";
   this.radialOffset = new Point(0, 0);
   this.leftGradient = null;
   this.rightGradient = null;
   this.topGradient = null;
   this.bottomGradient = null;
   this.gradient = null;

   this.reset(rect);
}


CsiGradient.prototype.reset = function (rect)
{
   this.rect = rect;
   this.leftGradient = null;
   this.rightGradient = null;
   this.topGradient = null;
   this.bottomGradient = null;
   this.gradient = null;
};


CsiGradient.prototype.draw = function (context)
{
   var color1;
   var color2;
   var color3;
   var distance;
   var center;

   switch(this.direction)
   {
      case Enum.GRADIENT_DIRECTION.BottomTop:
      case Enum.GRADIENT_DIRECTION.RightLeft:
      case Enum.GRADIENT_DIRECTION.BottomRightTopLeft:
      case Enum.GRADIENT_DIRECTION.TopRightBottomLeft:
      case Enum.GRADIENT_DIRECTION.RectangleIn:
      case Enum.GRADIENT_DIRECTION.CircleIn:
         //swap color so drawing is the same
         color1 = this.endColor;
         color3 = this.startColor;
         break;
      default:
         color1 = this.startColor;
         color3 = this.endColor;
         break;
   }
   color2 = this.midColor;

   if((this.direction === Enum.GRADIENT_DIRECTION.RectangleOut) ||
       (this.direction === Enum.GRADIENT_DIRECTION.RectangleIn))
   {
      //draw from the center out (color1 is center, color3 is sides)
      center = new Point(this.rect.left + this.rect.width / 2, this.rect.top + this.rect.height / 2);

      //overlap gradients so there is not a blank line
      var overlap = 1;

      //left wall
      if(!this.leftGradient) 
      {
         this.leftGradient = context.createLinearGradient(center.x, this.rect.top, this.rect.left, this.rect.top);
         this.leftGradient.addColorStop(0, color1);
         if(color2) 
         {
            this.leftGradient.addColorStop(0.5, color2);
         }
         this.leftGradient.addColorStop(1, color3);
      }
      context.fillStyle = this.leftGradient;
      context.beginPath();
      context.moveTo(center.x + overlap, center.y);
      context.lineTo(this.rect.left, this.rect.top - overlap);
      context.lineTo(this.rect.left, this.rect.bottom + overlap);
      context.fill();

      //right wall
      if(!this.rightGradient) 
      {
         this.rightGradient = context.createLinearGradient(center.x, this.rect.top, this.rect.right, this.rect.top);
         this.rightGradient.addColorStop(0, color1);
         if(color2) 
         {
            this.rightGradient.addColorStop(0.5, color2);
         }
         this.rightGradient.addColorStop(1, color3);
      }
      context.fillStyle = this.rightGradient;
      context.beginPath();
      context.moveTo(center.x - overlap, center.y);
      context.lineTo(this.rect.right, this.rect.top - overlap);
      context.lineTo(this.rect.right, this.rect.bottom + overlap);
      context.fill();

      //top wall
      if(!this.topGradient) 
      {
         this.topGradient = context.createLinearGradient(this.rect.left, center.y, this.rect.left, this.rect.top);
         this.topGradient.addColorStop(0, color1);
         if(color2) 
         {
            this.topGradient.addColorStop(0.5, color2);
         }
         this.topGradient.addColorStop(1, color3);
      }
      context.fillStyle = this.topGradient;
      context.beginPath();
      context.moveTo(center.x, center.y + overlap);
      context.lineTo(this.rect.left - overlap, this.rect.top);
      context.lineTo(this.rect.right + overlap, this.rect.top);
      context.fill();

      //bottom wall
      if(!this.bottomGradient) 
      {
         this.bottomGradient = context.createLinearGradient(this.rect.left, center.y, this.rect.left, this.rect.bottom);
         this.bottomGradient.addColorStop(0, color1);
         if(color2) 
         {
            this.bottomGradient.addColorStop(0.5, color2);
         }
         this.bottomGradient.addColorStop(1, color3);
      }
      context.fillStyle = this.bottomGradient;
      context.beginPath();
      context.moveTo(center.x, center.y - overlap);
      context.lineTo(this.rect.left - overlap, this.rect.bottom);
      context.lineTo(this.rect.right + overlap, this.rect.bottom);
      context.fill();
   }
   else
   {
      if(!this.gradient) 
      {
         switch(this.direction)
         {
            case Enum.GRADIENT_DIRECTION.TopBottom:
            case Enum.GRADIENT_DIRECTION.BottomTop:
               this.gradient = context.createLinearGradient(this.rect.left, this.rect.top, this.rect.left, this.rect.bottom);
               break;
            case Enum.GRADIENT_DIRECTION.LeftRight:
            case Enum.GRADIENT_DIRECTION.RightLeft:
               this.gradient = context.createLinearGradient(this.rect.left, this.rect.top, this.rect.right, this.rect.top);
               break;
            case Enum.GRADIENT_DIRECTION.TopLeftBottomRight:
            case Enum.GRADIENT_DIRECTION.BottomRightTopLeft:
               //The diagonal gradient is not supported in current browsers so we use a radial gradient.
               distance = Math.sqrt(Math.pow(this.rect.right - this.rect.left, 2) + Math.pow(this.rect.bottom - this.rect.top, 2));
               this.gradient = context.createRadialGradient(this.rect.left, this.rect.top, 1, this.rect.left, this.rect.top, distance);
               break;
            case Enum.GRADIENT_DIRECTION.BottomLeftTopRight:
            case Enum.GRADIENT_DIRECTION.TopRightBottomLeft:
               //The diagonal gradient is not supported in current browsers so we use a radial gradient.
               distance = Math.sqrt(Math.pow(this.rect.right - this.rect.left, 2) + Math.pow(this.rect.bottom - this.rect.top, 2));
               this.gradient = context.createRadialGradient(this.rect.left, this.rect.bottom, 1, this.rect.left, this.rect.bottom, distance);
               break;
            case Enum.GRADIENT_DIRECTION.CircleOut:
            case Enum.GRADIENT_DIRECTION.CircleIn:
               center = new Point(this.rect.left + this.rect.width / 2 + this.radialOffset.x, this.rect.top + this.rect.height / 2 + this.radialOffset.y);
               distance = Math.sqrt(Math.pow(center.x - this.rect.left, 2) + Math.pow(center.y - this.rect.bottom, 2));
               this.gradient = context.createRadialGradient(center.x, center.y, 1, center.x, center.y, distance);
               break;
            default:
               break;
         }

         this.gradient.addColorStop(0, color1);
         if(color2) 
         {
            this.gradient.addColorStop(0.5, color2);
         }
         this.gradient.addColorStop(1, color3);
      }

      context.fillStyle = this.gradient;
      context.fillRect(this.rect.left, this.rect.top, this.rect.width, this.rect.height);
   }
};


Enum.GRADIENT_DIRECTION =
{
   TopBottom: 0,
   BottomTop: 1,
   LeftRight: 2,
   RightLeft: 3,
   TopLeftBottomRight: 4,
   BottomRightTopLeft: 5,
   BottomLeftTopRight: 6,
   TopRightBottomLeft: 7,
   RectangleOut: 8,
   RectangleIn: 9,
   CircleOut: 10,
   CircleIn: 11
};


function CsiComponent(left, top, width, height)
{
   //Do not add properties to prototype
   if(arguments.length === 0)
   {
      return;
   }

   this.ready = true;      //Is the control ready to draw?  Could be waiting for image or other resource
   this.animating = false; //Is the control animating? (if so, draw will continually be called)
   this.needs_mouse_events = false;
   this.valid = false;
   this.drawOnlyIfInvalid = false;
   this.active = false;  //is the control active (visible on the current tab)
   this.showSelectCursor = false; //should the cursor change to a select cursor when dragging
   this.bad_data = true;
   this.nan_data = false;

   //position
   this.left = left;
   this.top = top;
   this.width = width;
   this.height = height;
   this.right = left + width;
   this.bottom = top + height;

   this.border_style = Enum.BORDER_STYLE.NONE;

   this.expression = null;
   this.owner = null;
}

CsiComponent.prototype.invalidate = function ()
{
   this.valid = false;
   if(this.owner)
      this.owner.request_draw(this);
};


CsiComponent.prototype.refresh = function ()
{
   this.invalidate();
};


CsiComponent.prototype.getAnimating = function ()
{
   return this.animating;
};


function drawBorder(context, rect, borderStyle)
{
   if(borderStyle !== Enum.BORDER_STYLE.NONE)
   {
      var outsideRect = new Rect(rect.left, rect.top, rect.width-1.0, rect.height-1.0);
      outsideRect.adjustForLines();
      var insideRect = new Rect(outsideRect.left + 1.0, outsideRect.top + 1.0, outsideRect.width - 2.1, outsideRect.height - 2.1);
      insideRect.adjustForLines();

      if(borderStyle === Enum.BORDER_STYLE.RAISED)
      {
         //outside border
         context.beginPath();
         context.strokeStyle = "lightgray";
         context.moveTo(outsideRect.left, outsideRect.bottom);
         context.lineTo(outsideRect.left, outsideRect.top); //left line
         context.lineTo(outsideRect.right, outsideRect.top); //top line
         context.stroke();

         context.beginPath();
         context.strokeStyle = "darkgray";
         context.moveTo(outsideRect.right, outsideRect.top);
         context.lineTo(outsideRect.right, outsideRect.bottom); //right line
         context.lineTo(outsideRect.left, outsideRect.bottom); //bottom line
         context.stroke();

         //inside border
         context.beginPath();
         context.strokeStyle = "white";
         context.moveTo(insideRect.left, insideRect.bottom);
         context.lineTo(insideRect.left, insideRect.top); //left line
         context.lineTo(insideRect.right, insideRect.top); //top line
         context.stroke();

         context.beginPath();
         context.strokeStyle = "gray";
         context.moveTo(insideRect.right, insideRect.top);
         context.lineTo(insideRect.right, insideRect.bottom); //right line
         context.lineTo(insideRect.left, insideRect.bottom); //bottom line
         context.stroke();
      }
      else if(borderStyle === Enum.BORDER_STYLE.LOWERED)
      {
         //outside border
         context.beginPath();
         context.strokeStyle = "gray";
         context.moveTo(outsideRect.left, outsideRect.bottom);
         context.lineTo(outsideRect.left, outsideRect.top); //left line
         context.lineTo(outsideRect.right, outsideRect.top); //top line
         context.stroke();

         context.beginPath();
         context.strokeStyle = "white";
         context.moveTo(outsideRect.right, outsideRect.top);
         context.lineTo(outsideRect.right, outsideRect.bottom); //right line
         context.lineTo(outsideRect.left, outsideRect.bottom); //bottom line
         context.stroke();

         //inside border
         context.beginPath();
         context.strokeStyle = "darkgray";
         context.moveTo(insideRect.left, insideRect.bottom);
         context.lineTo(insideRect.left, insideRect.top); //left line
         context.lineTo(insideRect.right, insideRect.top); //top line
         context.stroke();

         context.beginPath();
         context.strokeStyle = "lightgray";
         context.moveTo(insideRect.right, insideRect.top);
         context.lineTo(insideRect.right, insideRect.bottom); //right line
         context.lineTo(insideRect.left, insideRect.bottom); //bottom line
         context.stroke();
      }
      else //Single
      {
         context.beginPath();
         context.strokeStyle = "black";
         context.moveTo(outsideRect.left, outsideRect.bottom);
         context.lineTo(outsideRect.left, outsideRect.top); //left line
         context.lineTo(outsideRect.right, outsideRect.top); //top line
         context.lineTo(outsideRect.right, outsideRect.bottom); //right line
         context.lineTo(outsideRect.left, outsideRect.bottom); //bottom line
         context.stroke();
      }
   }
}


CsiComponent.prototype.drawBorder = function (context, rect)
{
   drawBorder(context, rect, this.border_style);
};


CsiComponent.prototype.deactivate = function ()
{
   this.active = false;
};


CsiComponent.prototype.activate = function ()
{
   this.active = true;
};


function drawImageError(rect, context)
{
   context.save();
   context.translate(rect.left, rect.top); //move to location

   context.fillStyle = "white";
   context.fillRect(0, 0, rect.width, rect.height);

   //Draw image error
   context.lineWidth = 7;
   context.lineCap = "round";
   context.strokeStyle = "red";

   context.beginPath();
   context.moveTo(0, 0);
   context.lineTo(rect.width, rect.height);
   context.moveTo(rect.width, 0);
   context.lineTo(0, rect.height);
   context.stroke();

   context.restore();
}


CsiComponent.prototype.drawImageError = function (context)
{
   drawImageError(new Rect(this.left, this.top, this.width, this.height), context);
};


function drawSingleLineBorder(context, rect, borderStyle)
{
   if(borderStyle !== Enum.BORDER_STYLE.NONE)
   {
      if(borderStyle === Enum.BORDER_STYLE.RAISED)
      {
         context.beginPath();
         context.strokeStyle = "white";
         context.moveTo(rect.left, rect.bottom);
         context.lineTo(rect.left, rect.top); //left line
         context.lineTo(rect.right, rect.top); //top line
         context.stroke();

         context.beginPath();
         context.strokeStyle = "black";
         context.moveTo(rect.right, rect.top);
         context.lineTo(rect.right, rect.bottom); //right line
         context.lineTo(rect.left + 1, rect.bottom); //bottom line
         context.stroke();
      }
      else if(borderStyle === Enum.BORDER_STYLE.LOWERED)
      {
         context.beginPath();
         context.strokeStyle = "black";
         context.moveTo(rect.left, rect.bottom);
         context.lineTo(rect.left, rect.top); //left line
         context.lineTo(rect.right, rect.top); //top line
         context.stroke();

         context.beginPath();
         context.strokeStyle = "white";
         context.moveTo(rect.right, rect.top);
         context.lineTo(rect.right, rect.bottom); //right line
         context.lineTo(rect.left, rect.bottom); //bottom line
         context.stroke();
      }
      else //Single
      {
         context.beginPath();
         context.strokeStyle = "black";
         context.moveTo(rect.left, rect.bottom);
         context.lineTo(rect.left, rect.top); //left line
         context.lineTo(rect.right, rect.top); //top line
         context.lineTo(rect.right, rect.bottom); //right line
         context.lineTo(rect.left, rect.bottom); //bottom line
         context.stroke();
      }
   }
}


function drawStyledBorder(context, rect, styleOrColor, width)
{
   var tempRect = new Rect(rect.left, rect.top, rect.width, rect.height);
   tempRect.adjustForLines();

   context.save();
   context.beginPath();
   context.strokeStyle = styleOrColor;
   context.lineWidth = width;
   context.moveTo(tempRect.left, tempRect.bottom);
   context.lineTo(tempRect.left, tempRect.top); //left line
   context.lineTo(tempRect.right, tempRect.top); //top line
   context.lineTo(tempRect.right, tempRect.bottom); //right line
   context.lineTo(tempRect.left, tempRect.bottom); //bottom line
   context.stroke();
   context.restore();
}


function drawRoundedRect(context, rect, cornerRadius)
{
   context.beginPath();

   //Don't let the corners stick out of the rectangle.
   var radius = cornerRadius;
   if(rect.height < (radius * 2.0))
   {
      radius = rect.height / 2.0;
   }

   if(rect.width < (radius * 2.0))
   {
      radius = rect.width / 2.0;
   }

   //Top Left Corner
   context.moveTo(rect.left + radius, rect.top);
   context.quadraticCurveTo(rect.left, rect.top, rect.left, rect.top + radius);
   //Left Line
   context.lineTo(rect.left, rect.bottom - radius);
   //Bottom Left Line
   context.quadraticCurveTo(rect.left, rect.bottom, rect.left + radius, rect.bottom);
   //Bottom Line
   context.lineTo(rect.right - radius, rect.bottom);
   //Bottom Right Line
   context.quadraticCurveTo(rect.right, rect.bottom, rect.right, rect.bottom - radius);
   //Right Line
   context.lineTo(rect.right, rect.top + radius);
   //Top Right Corner
   context.quadraticCurveTo(rect.right, rect.top, rect.right - radius, rect.top);
   //Top line
   context.closePath();
}


function fillPolygon(context, points)
{
   context.beginPath();
   if(points.length > 0)
   {
      context.moveTo(points[0].x, points[0].y);
      var i = 0;
      for(i = 0; i < points.length; i++)
      {
         context.lineTo(points[i].x, points[i].y);
      }

      context.fill();
   }
}


function drawDot(context, x, y, size, borderStyle)
{
   //fill with color
   context.beginPath();
   context.arc(x, y, size, 0, 2 * Math.PI, false);
   context.fill();

   //draw border
   switch(borderStyle)
   {
      case Enum.BORDER_STYLE.RAISED:
         context.beginPath();
         context.arc(x, y, size, Math.PI * 3 / 4, Math.PI * 7 / 4, false);
         context.strokeStyle = "white";
         context.stroke();

         context.beginPath();
         context.arc(x, y, size, Math.PI * 3 / 4, Math.PI * 7 / 4, true);
         context.strokeStyle = "black";
         context.stroke();
         break;

      case Enum.BORDER_STYLE.LOWERED:
         context.beginPath();
         context.arc(x, y, size, Math.PI * 3 / 4, Math.PI * 7 / 4, false);
         context.strokeStyle = "black";
         context.stroke();

         context.beginPath();
         context.arc(x, y, size, Math.PI * 3 / 4, Math.PI * 7 / 4, true);
         context.strokeStyle = "white";
         context.stroke();
         break;
   }
}

function clipRect(context, x, y, width, height)
{
   context.beginPath();
   context.moveTo(x, y);
   context.lineTo(x + width + 0.5, y);
   context.lineTo(x + width + 0.5, y + height + 0.5);
   context.lineTo(x, y + height + 0.5);
   context.clip();
}


CsiComponent.prototype.getBadData = function ()
{
   return this.bad_data;
};


CsiComponent.prototype.getNanData = function ()
{
   return this.nan_data;
};


CsiComponent.prototype.reset_data = function ()
{
   this.bad_data = true;
   this.nan_data = false;
};


CsiComponent.prototype.on_touch_start = function (event)
{ return false; };


CsiComponent.prototype.on_touch_move = function (event)
{ return false; };


CsiComponent.prototype.on_touch_end = function (event)
{ return false; };


function drawNanData(comp, context)
{
   context.save();
   context.translate(comp.left + comp.width - 16, comp.top);

   context.fillStyle = "red";
   context.fillRect(0, 0, 16, 16);

   context.fillStyle = "yellow";
   context.beginPath();
   context.arc(8, 8, 7, 0, 2 * Math.PI, false);
   context.fill();

   context.strokeStyle = "black";
   context.lineJoin = "round";
   context.lineCap = "round";
   context.lineWidth = 3;
   context.beginPath();
   context.moveTo(5, 11);
   context.lineTo(5, 5);
   context.lineTo(11, 11);
   context.lineTo(11, 5);
   context.stroke();

   context.restore();
}


function drawBadData(comp, context)
{
   context.save();
   context.translate(comp.left + comp.width - 16, comp.top);

   context.fillStyle = "red";
   context.fillRect(0, 0, 16, 16);

   context.fillStyle = "yellow";
   context.beginPath();
   context.arc(8, 8, 7, 0, 2 * Math.PI, false);
   context.fill();

   context.fillStyle = "black";
   context.beginPath();
   context.arc(8, 13, 1.5, 0, 2 * Math.PI, false);
   context.fill();

   var tempRect = new Rect(6, 1, 3, 9);
   context.scale(1, tempRect.height / tempRect.width);
   var radius = tempRect.width / 2;
   context.beginPath();
   context.arc(8, 2, radius, 0, 2 * Math.PI, false);
   context.fill();

   context.restore();
}


function CsiGestureTap()
{
   // the page coordinates over which we will respond to touch events
   this.area = new Rect(0, 0, 100, 100);

   // the radius around the first coordinates.  If the touch moves outside this
   // radius, 
   this.tolerance = 50;

   // the object that will receive touch events
   this.client = null;

   // the maximum amount of time between the touch start and the touch end.
   this.interval = 500;

   // controls whether the component should prevent defaults for touch events
   this.prevent_defaults = false;
   
   // records the state of this recognizer
   this.state = CsiGestureTap.state_standby;
   this.first_touch = null;
   this.timer_tag = 0;
   this.move_rect = null;
   if(arguments.length >= 1)
   {
      this.client = arguments[0];
      if(arguments.length >= 2)
      {
         var arg1 = arguments[1]; 
         if(arg1 instanceof Rect)
         {
            this.area = new Rect(arg1);
         }
         if(arguments.length >= 3)
         {
            this.interval = Number(arguments[2]);
         }
      }
   }
}
CsiGestureTap.state_standby = 0;
CsiGestureTap.state_touched = 1;
CsiGestureTap.state_after_end = 2;


CsiGestureTap.prototype.on_touch_start = function(event)
{
   var rtn = (this.state !== CsiGestureTap.state_standby);
   if(this.state === CsiGestureTap.state_standby)
   {
      if(event.touches.length === 1)
      {
         var touch = event.touches[0];
         var touch_point = new Point(touch.pageX, touch.pageY);
         if(this.area.contains(touch_point))
         {
            this.first_touch = touch_point;
            ++this.timer_tag;
            oneShotTimer.setTimeout(this, this.timer_tag, this.interval);
            this.state = CsiGestureTap.state_touched;
            this.move_rect = new Rect(0, 0, this.tolerance, this.tolerance);
            this.move_rect.center(touch_point.x, touch_point.y);
            rtn = true;
            if(this.client && typeof this.client.on_single_tap_start === "function")
            {
               this.client.on_single_tap_start(this);
            }
            if(this.prevent_defaults)
            {
               event.preventDefault();
            }
         }
      }
   }
   else if(this.state === CsiGestureTap.state_touched)
   {
      if(this.client && typeof this.client.on_single_tap_cancelled === "function")
      {
         this.client.on_single_tap_cancelled(this);
      }
      oneShotTimer.clearTimeout(this, this.timer_tag);
      this.state = CsiGestureTap.state_standby;
   }
   return rtn;
};


CsiGestureTap.prototype.on_touch_move = function(event)
{
   var rtn = (this.state !== CsiGestureTap.state_standby);
   if(this.state === CsiGestureTap.state_touched)
   {
      var touch = event.touches[0];
      var touch_point = new Point(touch.pageX, touch.pageY);
      if(this.prevent_defaults)
      {
         event.preventDefault();
      }
      if(!this.move_rect.contains(touch_point))
      {
         if(this.client && typeof this.client.on_single_tap_cancelled === "function")
         {
            this.client.on_single_tap_cancelled(this);
         }
         oneShotTimer.clearTimeout(this, this.timer_tag);
         this.state = CsiGestureTap.state_standby;
      }
   }
   return rtn;
};


CsiGestureTap.prototype.on_touch_end = function(event)
{
   var rtn = (this.state !== CsiGestureTap.state_standby);
   if(this.state === CsiGestureTap.state_touched)
   {
      this.state = CsiGestureTap.state_after_end;
      event.preventDefault();
   }
   return rtn;
};


CsiGestureTap.prototype.onOneShotTimer = function(tag)
{
   if(this.timer_tag === tag)
   {
      if(this.state === CsiGestureTap.state_touched)
      {
         if(this.client && typeof this.client.on_single_tap_cancelled === "function")
         {
            this.client.on_single_tap_cancelled(this);
         }
         this.state = CsiGestureTap.state_standby;
      }
      else if(this.state === CsiGestureTap.state_after_end)
      {
         this.state = CsiGestureTap.state_standby;
         if(this.client && typeof this.client.on_single_tap_complete === "function")
         {
            this.client.on_single_tap_complete(this);
         }
      }
   }
};


CsiGestureTap.prototype.get_origin = function()
{
   return new Point(
      this.move_rect.left + this.move_rect.width / 2,
      this.move_rect.top + this.move_rect.height / 2);
};


function CsiGestureDoubleTap()
{
   // the page coordinates over which we will respond to touch events
   this.area = new Rect(0, 0, 100, 100);

   // the radius around the first coordinates.  If the touch moves outside this
   // radius, 
   this.tolerance = 50;

   // the object that will receive touch events
   this.client = null;

   // the maximum amount of time between the touch start and the touch end.
   this.interval = 700;

   // controls whether the component should prevent defaults for touch events
   this.prevent_defaults = false;
   
   // records the state of this recogniser
   this.state = CsiGestureDoubleTap.state_standby;
   this.first_touch = null;
   this.timer_tag = 0;
   this.move_rect = null;
   if(arguments.length >= 1)
   {
      this.client = arguments[0];
      if(arguments.length >= 2)
      {
         var arg1 = arguments[1]; 
         if(arg1 instanceof Rect)
         {
            this.area = new Rect(arg1);
         }
         if(arguments.length >= 3)
         {
            this.interval = Number(arguments[2]);
         }
      }
   }
}
CsiGestureDoubleTap.state_standby = 0;
CsiGestureDoubleTap.state_touched_1 = 1;
CsiGestureDoubleTap.state_released_1 = 2;
CsiGestureDoubleTap.state_touched_2 = 3;


CsiGestureDoubleTap.prototype.on_touch_start = function (event)
{
   var touch;
   var touch_point = null;
   var rtn = this.state !== CsiGestureDoubleTap.state_standby;
   var cancel = false;
   if(this.state === CsiGestureDoubleTap.state_standby)
   {
      if(event.touches.length === 1)
      {
         touch = event.touches[0];
         touch_point = new Point(touch.pageX, touch.pageY);
         if(this.area.contains(touch_point))
         {
            this.first_touch = touch_point;
            ++this.timer_tag;
            oneShotTimer.setTimeout(this, this.timer_tag, this.interval);
            this.state = CsiGestureDoubleTap.state_touched_1;
            this.move_rect = new Rect(0, 0, this.tolerance, this.tolerance);
            this.move_rect.center(touch_point.x, touch_point.y);
            rtn = true;
            csi_log("double tap start first: (" + touch_point.x + "," + touch_point.y + ")");
            if(this.client && typeof this.client.on_double_tap_start === "function")
            {
               this.client.on_double_tap_start(this);
            }
            if(this.prevent_defaults)
            {
               event.preventDefault();
            }
         }
      }
   }
   else if(this.state === CsiGestureDoubleTap.state_touched_1)
   {
      cancel = true;
   }
   else if(this.state === CsiGestureDoubleTap.state_released_1)
   {
      if(event.touches.length === 1)
      {
         touch = event.touches[0];
         touch_point = new Point(touch.pageX, touch.pageY);
         if(this.move_rect.contains(touch_point))
         {
            this.state = CsiGestureDoubleTap.state_touched_2;
            csi_log("double tap start second: (" + touch_point.x + "," + touch_point.y + ")");
            if(this.prevent_defaults)
            {
               event.preventDefault();
            }
         }
         else
         {
            cancel = true;
         }
      }
      else
      {
         cancel = true;
      }
   }
   if(cancel)
   {
      if(this.client && typeof this.client.on_double_tap_cancelled === "function")
      {
         this.client.on_double_tap_cancelled(this);
      }
      oneShotTimer.clearTimeout(this, this.timer_tag);
      this.state = CsiGestureDoubleTap.state_standby;
   }
   return rtn;
};


CsiGestureDoubleTap.prototype.on_touch_move = function(event)
{
   var rtn = this.state !== CsiGestureDoubleTap.state_standby;
   if(this.state === CsiGestureDoubleTap.state_touched_1 ||
      this.state === CsiGestureDoubleTap.state_touched_2)
   {
      var touch = event.touches[0];
      var touch_point = new Point(touch.pageX, touch.pageY);
      if(this.prevent_defaults)
      {
         event.preventDefault();
      }
      if(!this.move_rect.contains(touch_point))
      {
         if(this.client && typeof this.client.on_double_tap_cancelled === "function")
         {
            this.client.on_double_tap_cancelled(this);
         }
         oneShotTimer.clearTimeout(this, this.timer_tag);
         this.state = CsiGestureDoubleTap.state_standby;
      }
   }
   return rtn;
};


CsiGestureDoubleTap.prototype.on_touch_end = function(event)
{
   var rtn = this.state !== CsiGestureDoubleTap.state_standby;
   if(this.state === CsiGestureDoubleTap.state_touched_1)
   {
      if(this.prevent_defaults)
      {
         event.preventDefault();
      }
      this.state = CsiGestureDoubleTap.state_released_1;
   }
   if(this.state === CsiGestureDoubleTap.state_touched_2)
   {
      if(this.prevent_defaults)
      {
         event.preventDefault();
      }
      oneShotTimer.clearTimeout(this, this.timer_tag);
      this.state = CsiGestureDoubleTap.state_standby;
      rtn = false;
      if(this.client && typeof this.client.on_double_tap_complete === "function")
      {
         this.client.on_double_tap_complete(this);
      }
   }
   return rtn;
};


CsiGestureDoubleTap.prototype.onOneShotTimer = function(tag)
{
   if(this.state !== CsiGestureDoubleTap.state_standby && this.timer_tag === tag)
   {
      if(this.client && typeof this.client.on_double_tap_cancelled === "function")
      {
         this.client.on_double_tap_cancelled(this);
      }
      this.state = CsiGestureDoubleTap.state_standby;
   }
};


CsiGestureDoubleTap.prototype.get_origin = function()
{
   return new Point(
      this.move_rect.left + this.move_rect.width / 2,
      this.move_rect.top + this.move_rect.height / 2);
};


function CsiGestureSwipe()
{
   // the page coordinates over which we will respond to touch events
   this.area = new Rect(0, 0, 100, 100);

   // the object that will receive swipe events
   this.client = null;

   // controls whether the component should prevent defaults for touch events
   this.prevent_defaults = false;

   // initialize members that will maintain the state of this gesture
   this.state = CsiGestureSwipe.state_standby;
   this.origin = null;
   if(arguments.length >= 1)
   {
      this.client = arguments[0];
      if(arguments.length >= 2)
      {
         var arg1 = arguments[1];
         if(arg1 instanceof Rect)
         {
            this.area = new Rect(arg1);
         }
      }
   }
}
CsiGestureSwipe.state_standby = 0;
CsiGestureSwipe.state_touched = 1;
CsiGestureSwipe.state_moving = 2;


CsiGestureSwipe.prototype.on_touch_start = function(event)
{
   var rtn = (this.state !== CsiGestureSwipe.state_standby);
   if(this.state === CsiGestureSwipe.state_standby)
   {
      if(event.touches.length === 1)
      {
         var touch = event.touches[0];
         var touch_point = new Point(touch.pageX, touch.pageY);
         if(this.area.contains(touch_point))
         {
            this.origin = touch_point;
            this.state = CsiGestureSwipe.state_touched;
            rtn = true;
            if(this.client && typeof this.client.on_swipe_start === "function")
            {
               this.client.on_swipe_start(this);
            }
            if(this.prevent_defaults)
            {
               event.preventDefault();
            }
         }
      }
   }
   else
   {
      // any other touch start event when we are already started should reset us to a standby state.
      if(this.client && typeof this.client.on_swipe_cancelled === "function")
      {
         this.client.on_swipe_cancelled(this);
      }
      this.state = CsiGestureSwipe.state_standby;
   }
   return rtn;
};


CsiGestureSwipe.prototype.on_touch_move = function (event)
{
   var rtn = (this.state !== CsiGestureSwipe.state_standby);
   var touch = 0;
   var touch_point = 0;
   var delta_x = 0;
   var delta_y = 0;
   if(this.state === CsiGestureSwipe.state_touched)
   {
      var move_rect = new Rect(0, 0, this.tolerance, this.tolerance);
      touch = event.touches[0];
      touch_point = new Point(touch.pageX, touch.pageY);
      delta_x = touch_point.x - this.origin.x;
      delta_y = touch_point.y - this.origin.y;
      this.state = CsiGestureSwipe.state_moving;
      this.last_touch = touch_point;
      if(this.client && typeof this.client.on_swipe_moved === "function")
      {
         this.client.on_swipe_moved(this, delta_x, delta_y);
      }
      if(this.prevent_defaults)
      {
         event.preventDefault();
      }
   }
   else if(this.state === CsiGestureSwipe.state_moving)
   {
      touch = event.touches[0];
      touch_point = new Point(touch.pageX, touch.pageY);
      delta_x = touch.pageX - this.last_touch.x;
      delta_y = touch.pageY - this.last_touch.y;
      this.last_touch.x = touch.pageX;
      this.last_touch.y = touch.pageY;
      if(this.client && typeof this.client.on_swipe_moved === "function")
      {
         this.client.on_swipe_moved(this, delta_x, delta_y);
      }
      if(this.prevent_defaults)
      {
         event.preventDefault();
      }
   }
   return rtn;
};


CsiGestureSwipe.prototype.on_touch_end = function(event)
{
   var rtn = (this.state !== CsiGestureSwipe.state_standby);
   if(this.state === CsiGestureSwipe.state_touched)
   {
      this.state = CsiGestureSwipe.state_standby;
      if(this.client && typeof this.client.on_swipe_cancelled === "function")
      {
         this.client.on_swipe_cancelled(this);
      }
      if(this.prevent_defaults)
      {
         event.preventDefault();
      }
   }
   else if(this.state === CsiGestureSwipe.state_moving)
   {
      this.state = CsiGestureSwipe.state_standby;
      if(this.client && typeof this.client.on_swipe_complete === "function")
      {
         this.client.on_swipe_complete(this);
      }
      if(this.prevent_defaults)
      {
         event.preventDefault();
      }
   }  
   return rtn;
};


CsiGestureSwipe.prototype.get_origin = function ()
{
   return this.origin;
};


function CsiGesturePinch()
{
   // specifies the page coordinates for which this gesture will be recognized.
   this.area = new Rect(0, 0, 100, 100);

   // specifies the object that will receive event notifications from this gesture.
   this.client = null;

   // specifies whether this gesture should prevent defaults when it touch events.
   this.prevent_defaults = false;

   // set up the state for this gesture
   this.state = CsiGesturePinch.state_standby;
   this.last_distance = NaN;
   this.last_midpoint = null;
   if(arguments.length >= 1)
   {
      this.client = arguments[0];
      if(arguments.length >= 2)
      {
         var arg1 = arguments[1];
         if(arg1 instanceof Rect)
         {
            this.area = new Rect(arg1);
         }
      }
   }
}
CsiGesturePinch.state_standby = 0;
CsiGesturePinch.state_touched = 1;


CsiGesturePinch.prototype.on_touch_start = function(event)
{
   var rtn = (this.state !== CsiGesturePinch.state_standby);
   if(this.state === CsiGesturePinch.state_standby)
   {
      if(event.touches.length === 2)
      {
         var touch0 = event.touches[0];
         var touch1 = event.touches[1];
         var touch_point0 = new Point(touch0.pageX, touch0.pageY);
         var touch_point1 = new Point(touch1.pageX, touch1.pageY);
         if(this.area.contains(new Point(touch_point0.x, touch_point0.y)) &&
            this.area.contains(new Point(touch_point1.x, touch_point1.y)))
         {
            // we need to calculate the distance between the touch points
            var distance_x = touch_point1.x - touch_point0.x;
            var distance_y = touch_point1.y - touch_point0.y;
            this.last_distance = Math.sqrt(
               distance_x * distance_x + distance_y * distance_y);
            this.last_midpoint = new Point(
               (touch_point0.x + touch_point1.x) / 2,
               (touch_point0.y + touch_point1.y) / 2);
            this.state = CsiGesturePinch.state_touched;
            if(this.prevent_defaults)
            {
               event.preventDefault();
            }
            rtn = true;
            if(this.client && typeof this.client.on_pinch_start === "function")
            {
               this.client.on_pinch_start(this);
            }
         }
      }
   }
   else
   {
      this.state = CsiGesturePinch.state_standby;
      if(this.client && typeof this.client.on_pinch_cancelled === "function")
      {
         this.client.on_pinch_cancelled(this);
      }
   }
   return rtn;
};


CsiGesturePinch.prototype.on_touch_move = function(event)
{
   var rtn = (this.state !== CsiGesturePinch.state_standby);
   if(this.state == CsiGesturePinch.state_touched)
   {
      var touch0 = event.touches[0];
      var touch1 = event.touches[1];
      var touch_point0 = new Point(touch0.pageX, touch0.pageY);
      var touch_point1 = new Point(touch1.pageX, touch1.pageY);
      var distance_x = touch_point1.x - touch_point0.x;
      var distance_y = touch_point1.y - touch_point0.y;
      var new_distance = Math.sqrt(
         distance_x * distance_x + distance_y * distance_y);
      var scale = this.last_distance / new_distance;
      var midpoint = new Point(
         (touch_point0.x + touch_point1.x) / 2,
         (touch_point0.y + touch_point1.y) / 2);
      var delta_x = midpoint.x - this.last_midpoint.x;
      var delta_y = midpoint.y - this.last_midpoint.y;
      if(!isNaN(scale))
      {
         this.last_distance = new_distance;
         if(this.client && typeof this.client.on_pinch_moved === "function")
         {
            var canvas_offset = this.client.get_canvas_offset();
            this.client.on_pinch_moved(
               this, scale, new Point(midpoint.x - canvas_offset.x, midpoint.y - canvas_offset.y), delta_x, delta_y);
         }
         this.last_midpoint = midpoint;
      }
   }
   return rtn;
};


CsiGesturePinch.prototype.on_touch_end = function(event)
{
   var rtn = (this.state !== CsiGesturePinch.state_standby);
   if(this.state == CsiGesturePinch.state_touched)
   {
      this.state  = CsiGesturePinch.state_standby;
      if(this.client && typeof this.client.on_pinch_complete === "function")
      {
         this.client.on_pinch_complete(this);
      }
   }      
   return rtn;
};


function CsiGraph(left, top, width, height)
{
   //Do not add properties to prototype
   if(arguments.length === 0)
      return;
   CsiComponent.call(this, left, top, width, height);

   //rtmc props
   this.background_transparent = false;
   this.background_color = "rgba(255, 255, 255, 1)";
   this.background_gradient = new CsiGradient();
   this.graphWidth = 60000; //How much data to get
   this.displayWidth = 60000; //How much data to show
   this.fill_and_clear = false;
   this.real_time = false;
   this.image_name = "";
   this.title = "";
   this.title_font = "12pt Arial";
   this.restore_btn_font = "10pt Arial";
   this.title_font_color = "#000000";
   this.show_legend = true;
   this.image_name = "";
   this.inside_graph = 0;
   this.image_style = 0;
   this.image_visible = 0;
   this.use_pc_time = false;
   this.legend_visible = 1;
   this.legend_transparent = 0;
   this.legend_back_color = "#FFFFFF";
   this.legend_shadow_color = "#000000";
   this.legend_position = Enum.LEGEND_ALIGNMENT.BOTTOM;
   this.legend_position_offset = 0;
   this.legend_gradient = new CsiGradient();
   this.legend_font = "8pt Arial";
   this.legend_font_color = "#000000";
   this.enable_3d = 0;
   this.percent_3d = 15;
   this.rotation = 345;
   this.elevation = 345;
   this.orthogonal = 1;
   this.ortho_angle = 45;
   this.plot_area_transparent = false;
   this.plot_area_color = "#FFFFFF";
   this.plot_area_gradient = new CsiGradient();
   this.backgroundCsiImage = null;
   this.plotAreaCsiImage = null;
   this.plot_border_visible = 1;
   this.plot_border_style = 0;
   this.plot_border_color = "#000000";
   this.plot_border_width = 1;

   //misc
   this.bottomAxis = new CsiGraphAxis(this, CsiGraphAxis.axis_bottom);
   this.leftAxis = new CsiGraphAxis(this, CsiGraphAxis.axis_left);
   this.rightAxis = new CsiGraphAxis(this, CsiGraphAxis.axis_right);
   this.seriesarray = [];
   this.newestTimeStamp = 0;
   this.margin = 5;
   this.bar_count = 0;

   //cached positions
   this.positionsInvalid = true;
   this.legendBoxWidth = 0;
   this.legendBoxHeight = 0;
   this.legendMargin = 0;
   this.legendSpacing = 2;
   this.legend_rect = null;
   this.plotRect = null;
   this.backgroundRect = new Rect(0, 0, this.width, this.height);
   this.legendColumnCount = 1;
   this.legendRowCount = 1;
   this.widthForSeries = 0;

   //zoom/pan
   this.needs_mouse_events = true;
   this.zoomRect = new Rect();
   this.zoomOrigin = new Point(0, 0);
   this.zooming = false;
   this.left_down = false;
   this.right_down = false;
   this.show_restore_btn = false;
   this.show_zoom_btn = false;
   this.button_rect = null;

   // gesture parsers
   this.double_zoom_gesture = new CsiGestureDoubleTap(this, new Rect(left, top, width, height));
   this.double_zoom_gesture.prevent_defaults = true;
   this.pan_gesture = new CsiGestureSwipe(this, new Rect(left, top, width, height));
   this.pan_gesture.prevent_defaults = true;
   this.marks_gesture = new CsiGestureTap(this, new Rect(left, top, width, height));
   this.pinch_gesture = new CsiGesturePinch(this, new Rect(left, top, width, height));
   this.pinch_gesture.prevent_defaults = true;
   this.restore_gesture = null;
}
CsiGraph.prototype = new CsiComponent();


CsiGraph.prototype.getBadData = function ()
{
   var len = this.seriesarray.length;
   var i;
   for(i = 0; i < len; i++)
   {
      if(this.seriesarray[i].bad_data)
      {
         return true;
      }
   }
   return false;
};


CsiGraph.prototype.clear_data = function (series)
{
   this.bottomAxis.auto_time = false;
   this.bottomAxis.auto_max = false;
   this.bottomAxis.auto_min = false;
   this.bottomAxis.min = this.newestTimeStamp;
   this.bottomAxis.max = this.newestTimeStamp + this.displayWidth;
   var len = this.seriesarray.length;
   var i;
   for(i = 0; i < len; i++)
   {
      if(series !== this.seriesarray[i])
      {
         this.seriesarray[i].data = [];
      }
      else
      {
         series.data.splice(0, series.data.length - 2); //Leave the newest value in the trace since it was never displayed
      }
   }

   this.positionsInvalid = true;
};


CsiGraph.prototype.getNanData = function ()
{
   return false;
};


CsiGraph.prototype.csiImageOnLoad = function ()
{
   this.ready = true;
   this.invalidate();
};


CsiGraph.prototype.csiImageOnError = function ()
{
   this.ready = true;
   this.invalidate();
};


CsiGraph.prototype.draw = function (context)
{
   if(this.width * this.height === 0)
   {
      return;
   }

   if(this.positionsInvalid)
   {
      this.calculatePositions(context);
      this.positionsInvalid = false;
   }

   context.save();
   context.translate(this.left, this.top); //move to location
   clipRect(context, 0, 0, this.width, this.height);
   context.clearRect(0, 0, this.width, this.height);

   var series_count = this.seriesarray.length;

   this.drawBackground(context);
   this.drawTitle(context);
   if(series_count > 0)
      this.drawLegend(context);
   if(series_count > 0)
      this.drawPlotArea(context);
   this.drawBorder(context, new Rect(0, 0, this.width, this.height));
   if(series_count > 0)
      this.drawMarks(context);

   if(this.show_restore_btn || this.show_zoom_btn)
   {
      this.draw_button(context);
   }

   context.restore();
};


CsiGraph.prototype.calculatePositions = function (context)
{
   var titleBottom = this.margin;
   var title_height = 0;
   var title_size;
   var max_series_width;
   var graph = this;
   this.background_gradient.reset(new Rect(0, 0, this.width, this.height));
   if(this.title && this.title.length > 0)
   {
      context.font = this.title_font;
      title_size = measureText(context, this.title);
      this.title_rect = new Rect();
      this.title_rect.set_width(title_size.width);
      this.title_rect.set_height(title_size.height);
      this.title_rect.set_top(this.margin);
      this.title_rect.updateBottom();
      this.title_rect.center_x(this.width / 2);
      title_height = title_size.height;
      titleBottom = title_height + this.margin * 2;
   }
   else
      this.title_rect = null;

   //calculate legendRect and plotRect
   this.legend_rect = null;
   if(this.legend_visible && this.seriesarray.length > 0)
   {
      context.font = this.legend_font;
      this.legend_rect = new CsiNestedRect();
      this.seriesarray.forEach(function(trace) {
         var trace_rect = trace.make_legend_rect(context);
         if(trace_rect)
            graph.legend_rect.add(trace_rect);
      });
      if(this.legend_position === Enum.LEGEND_ALIGNMENT.RIGHT || this.legend_position === Enum.LEGEND_ALIGNMENT.LEFT)
         this.legend_rect.stack_grid(this.height - title_height, true);
      else
         this.legend_rect.stack_grid(this.width, false);
      switch(this.legend_position)
      {
      case Enum.LEGEND_ALIGNMENT.LEFT:
         this.legend_rect.move(margin, titleBottom);
         this.plotRect = new Rect(
            this.legend_rect.get_right(true) + this.margin,
            titleBottom,
            this.width - this.legend_rect.get_width() - this.margin * 3,
            this.height - title_height - this.margin);
         break;
         
      case Enum.LEGEND_ALIGNMENT.RIGHT:
         this.legend_rect.move(this.width - this.legend_rect.get_width() - this.margin, titleBottom);
         this.plotRect = new Rect(
            this.margin,
            titleBottom,
            this.width - this.legend_rect.get_width() - this.margin * 3,
            this.height - title_height - this.margin);
         break;
         
      case Enum.LEGEND_ALIGNMENT.TOP:
         this.legend_rect.move(this.width / 2, this.margin);
         this.plotRect = new Rect(
            this.margin,
            this.legend_rect.get_bottom() + this.margin,
            this.width - this.margin * 2,
            this.height - this.legend_rect.get_height() - this.margin - 2);
         break;
         
      case Enum.LEGEND_ALIGNMENT.BOTTOM:
         this.legend_rect.move(this.width / 2 - this.legend_rect.get_width() / 2, this.height - this.legend_rect.get_height() - this.margin);
         this.plotRect = new Rect(
            this.margin,
            titleBottom,
            this.width - this.margin * 2,
            this.height - this.legend_rect.get_height() - title_height - this.margin * 2);
         break;
      }
      this.legend_gradient.reset(this.legend_rect);
   }
   else
   {
      this.plotRect = new Rect(
         this.margin,
         titleBottom,
         this.width - this.margin * 2,
         this.height - title_height - this.margin * 2);
   }

   // the axes need to adjust the plot rectangle in order to account
   // for the space required by their labels.  There is a mutual
   // dependency between the height of the bottom axis and the width
   // of the left and right axes that makes this less than simple.
   var bottom_axis_rect = this.bottomAxis.generate_axis_rect(
      context, this.plotRect.width);
   var left_axis_rect;
   var right_axis_rect;

   bottom_axis_rect.height = bottom_axis_rect.get_height() + this.margin;
   left_axis_rect = this.leftAxis.generate_axis_rect(
      context, this.plotRect.height - bottom_axis_rect.get_height());
   right_axis_rect = this.rightAxis.generate_axis_rect(
      context, this.plotRect.height - bottom_axis_rect.get_height());
   if(this.bottomAxis.get_labels().length > 0 && this.bottomAxis.labels_visible)
   {
      var first_label = this.bottomAxis.get_labels()[0];
      var half_label_width = first_label.rect.get_width() / 2;
      var left_wprime = this.plotRect.left + left_axis_rect.width;
      var right_wprime = (this.width - this.plotRect.right) + right_axis_rect.width;

      var half_label_rect = null;
      if(half_label_width > left_wprime)
      {
         half_label_rect = new CsiNestedRect(0, 0, half_label_width - left_wprime, 2);
         left_axis_rect.shift_children(half_label_width - left_wprime, 0);
         left_axis_rect.add(half_label_rect);
      }
      if(half_label_width > right_wprime)
      {
         half_label_rect = new CsiNestedRect(0, 0, half_label_width - right_wprime, 2);
         right_axis_rect.add(half_label_rect);
      }
   }
   this.plotRect.set_height(this.plotRect.height - bottom_axis_rect.get_height());
   this.plotRect.set_width(
      this.plotRect.width - (left_axis_rect.get_width() + right_axis_rect.get_width()));
   this.plotRect.offset(left_axis_rect.width, 0);
   bottom_axis_rect = this.bottomAxis.generate_axis_rect(
      context, this.plotRect.width);
   bottom_axis_rect.move(this.plotRect.left, this.plotRect.bottom);
   left_axis_rect.set_right(this.plotRect.left);
   left_axis_rect.set_top(this.plotRect.top);
   right_axis_rect.set_left(this.plotRect.right);
   right_axis_rect.set_top(this.plotRect.top);
   this.plot_area_gradient.reset(this.plotRect);
   this.double_zoom_gesture.area = this.translate_page(this.plotRect);
   this.double_zoom_gesture.area.offset(this.left, this.top);
   this.pan_gesture.area = this.translate_page(this.plotRect);
   this.pan_gesture.area.offset(this.left, this.top);
   this.marks_gesture.area = this.translate_page(this.plotRect);
   this.marks_gesture.area.offset(this.left, this.top);
   this.pinch_gesture.area = this.translate_page(this.plotRect);
   this.pinch_gesture.area.offset(this.left, this.top);
};


CsiGraph.prototype.drawBackground = function (context)
{
   if(!this.background_transparent)
   {
      if(this.background_gradient.visible)
         this.background_gradient.draw(context);
      else
      {
         context.fillStyle = this.background_color;
         context.fillRect(0, 0, this.width, this.height);
      }
   }
   if(this.backgroundCsiImage)
      this.backgroundCsiImage.draw(context, this.backgroundRect, 0, this.imageDrawStyle);
};


CsiGraph.prototype.drawTitle = function (context)
{
   if(this.title !== "" && this.title_rect !== null)
   {
      context.fillStyle = this.title_font_color;
      context.textAlign = "center";
      context.textBaseline = "bottom";
      context.font = this.title_font;
      context.fillText(this.title, this.title_rect.get_center().x, this.title_rect.bottom);
   }
};


CsiGraph.prototype.drawLegend = function (context)
{
   if(this.legend_visible)
   {
      if(!this.legend_transparent)
         this.drawLegendBackground(context);
      this.drawLegendSeries(context);
   }
};


CsiGraph.prototype.drawLegendBackground = function (context)
{
   //draw shadow
   context.save(); //must save so we can reset the shadow props.
   context.shadowOffsetX = 3;
   context.shadowOffsetY = 3;
   context.shadowBlur = 5;
   context.shadowColor = this.legend_shadow_color;
   context.beginPath();
   context.rect(this.legend_rect.get_left(), this.legend_rect.get_top(), this.legend_rect.get_width(), this.legend_rect.get_height());
   context.closePath();
   context.strokeStyle = this.legend_shadow_color;
   context.stroke();
   context.restore();

   if(this.legend_gradient.visible)
   {
      this.legend_gradient.draw(context);

      //draw border
      context.beginPath();
      context.rect(this.legend_rect.get_left(), this.legend_rect.get_top(), this.legend_rect.get_width(), this.legend_rect.get_height());
      context.closePath();
      context.strokeStyle = "#000000";
      context.stroke();
   }
   else
   {
      //fill with background color and draw border
      context.beginPath();
      context.rect(this.legend_rect.get_left(), this.legend_rect.get_top(), this.legend_rect.get_width(), this.legend_rect.get_height());
      context.closePath();
      context.fillStyle = this.legend_back_color;
      context.fill();
      context.strokeStyle = "#000000";
      context.stroke();
   }
};


CsiGraph.prototype.drawLegendSeries = function (context)
{
   var graph = this;
   context.textAlign = "left";
   context.textBaseline = "middle";
   context.font = this.legend_font;
   this.seriesarray.forEach(function(trace) {
      trace.draw_legend(context, graph.legend_font_color);
   });
};


CsiGraph.prototype.drawPlotArea = function (context)
{
   context.save();
   //draw plot area background
   this.drawPlotAreaBackground(context);

   //draw axis labels and ticks
   this.bottomAxis.draw(context);
   this.leftAxis.draw(context);
   this.rightAxis.draw(context);

   //draw series
   clipRect(context, this.plotRect.left, this.plotRect.top, this.plotRect.width, this.plotRect.height);
   var len = this.seriesarray.length;
   var i;
   for(i = 0; i < len; i++)
   {
      this.seriesarray[i].draw(context);
   }
   context.restore();

   if(this.zooming)
   {
      context.save();
      context.lineWidth = 3;
      context.strokeStyle = "RGBA(64, 64, 64, 0.55)";
      context.strokeRect(this.zoomRect.left, this.zoomRect.top, this.zoomRect.width, this.zoomRect.height);
      context.restore();
   }
};


CsiGraph.prototype.drawPlotAreaBackground = function (context)
{
   if(!this.plot_area_transparent)
   {
      if(this.plot_area_gradient.visible)
      {
         this.plot_area_gradient.draw(context);
      }
      else
      {
         //fill background
         context.beginPath();
         context.rect(this.plotRect.left, this.plotRect.top, this.plotRect.width, this.plotRect.height);
         context.closePath();
         context.fillStyle = this.plot_area_color;
         context.fill();
      }
   }

   //draw image
   if(this.plotAreaCsiImage)
   {
      this.plotAreaCsiImage.draw(context, this.plotRect, 0, this.imageDrawStyle);
   }

   //draw border?
   if(this.plot_border_visible && this.plot_border_style !== Enum.LINE_TYPE.CLEAR && this.plot_border_style !== Enum.LINE_TYPE.IGNORE)
   {
      var previous = [0, 0];
      context.strokeStyle = this.plot_border_color;
      context.lineWidth = this.plot_border_width;
      context.beginPath();
      switch(this.plot_border_style)
      {
         case Enum.LINE_TYPE.SOLID:
            context.strokeRect(this.plotRect.left, this.plotRect.top, this.plotRect.width, this.plotRect.height);
            break;
         case Enum.LINE_TYPE.DASH:
            drawDashedLine(context, this.plotRect.left, this.plotRect.top, this.plotRect.right, this.plotRect.top, Csi.dash_pattern, previous);
            drawDashedLine(context, this.plotRect.right, this.plotRect.top, this.plotRect.right, this.plotRect.bottom, Csi.dash_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.right, this.plotRect.bottom, Csi.dash_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.left, this.plotRect.top, Csi.dash_pattern, previous);
            break;
         case Enum.LINE_TYPE.DOT:
            drawDashedLine(context, this.plotRect.left, this.plotRect.top, this.plotRect.right, this.plotRect.top, Csi.dot_pattern, previous);
            drawDashedLine(context, this.plotRect.right, this.plotRect.top, this.plotRect.right, this.plotRect.bottom, Csi.dot_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.right, this.plotRect.bottom, Csi.dot_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.left, this.plotRect.top, Csi.dot_pattern, previous);
            break;
         case Enum.LINE_TYPE.DASHDOT:
            drawDashedLine(context, this.plotRect.left, this.plotRect.top, this.plotRect.right, this.plotRect.top, Csi.dash_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.right, this.plotRect.top, this.plotRect.right, this.plotRect.bottom, Csi.dash_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.right, this.plotRect.bottom, Csi.dash_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.left, this.plotRect.top, Csi.dash_dot_pattern, previous);
            break;
         case Enum.LINE_TYPE.DASHDOTDOT:
            drawDashedLine(context, this.plotRect.left, this.plotRect.top, this.plotRect.right, this.plotRect.top, Csi.dash_dot_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.right, this.plotRect.top, this.plotRect.right, this.plotRect.bottom, Csi.dash_dot_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.right, this.plotRect.bottom, Csi.dash_dot_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.left, this.plotRect.top, Csi.dash_dot_dot_pattern, previous);
            break;
         //case Enum.LINE_TYPE.SMALLDOTS: 
         default:
            drawDashedLine(context, this.plotRect.left, this.plotRect.top, this.plotRect.right, this.plotRect.top, Csi.small_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.right, this.plotRect.top, this.plotRect.right, this.plotRect.bottom, Csi.small_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.right, this.plotRect.bottom, Csi.small_dot_pattern, previous);
            drawDashedLine(context, this.plotRect.left, this.plotRect.bottom, this.plotRect.left, this.plotRect.top, Csi.small_dot_pattern, previous);
            break;
      }
      context.stroke();
   }
};


var restore_btn_text = "Restore";
var zoom_btn_text = "Zoom/Pan";
CsiGraph.prototype.draw_button = function (context)
{
   var btn_text = "";
   if(this.show_restore_btn)
      btn_text = restore_btn_text;
   else if(this.show_zoom_btn)
      btn_text = zoom_btn_text;
   context.save();
   context.textAlign = "center";
   context.textBaseline = "middle";
   context.font = this.restore_btn_font;

   if(!this.button_rect || (this.plotRect.left != this.button_rect.left || this.plotRect.top != this.button_rect.top))
   {
      var size = measureText(context, zoom_btn_text); //Measure zoom_btn_text since it is longest
      this.button_rect = new Rect(this.plotRect.left, this.plotRect.top, size.width + 12, size.height + 12);
   }

   var grd = context.createLinearGradient(0, 0, 0, this.button_rect.height);
   grd.addColorStop(0, "#F8F8F8");
   grd.addColorStop(1, "#E0E0E0");
   context.fillStyle = grd;

   context.strokeStyle = "RGBA(85,85,85,.5)";
   context.lineWidth = 0.5;

   drawRoundedRect(context, this.button_rect, 5);
   context.fill();
   context.stroke();

   context.fillStyle = "#000000";
   context.fillText(btn_text, this.button_rect.left + this.button_rect.width / 2.0, this.button_rect.top + this.button_rect.height / 2.0);

   context.restore();
   if(!this.restore_gesture)
   {
      this.restore_gesture = new CsiGestureTap(
         this, this.translate_page(this.button_rect));
      this.restore_gesture.area.offset(this.left, this.top);
      csiMouseEvents.register_gesture(this.restore_gesture, 0);
   }
};


CsiGraph.prototype.drawMarks = function (context)
{
   var len = this.seriesarray.length;
   var i;
   for(i = 0; i < len; i++)
   {
      this.seriesarray[i].draw_marks(context);
   }
};


CsiGraph.prototype.createSeries = function (expression, axis)
{
   var series = new CsiGraphSeries(expression, this, axis);
   this.seriesarray.push(series);
   return series;
};


CsiGraph.prototype.OnLButtonDown = function (mouseX, mouseY)
{
   var local_x = mouseX - this.left;
   var local_y = mouseY - this.top;

   //Don't start zooming unless we are inside the plot area
   if(this.plotRect.contains(new Point(local_x, local_y)))
   {
      this.left_down = true;
      this.zoomRect = new Rect(local_x, local_y, 0, 0);
      this.zoomOrigin.x = local_x;
      this.zoomOrigin.y = local_y;
   }

   var len = this.seriesarray.length;
   var i;
   for(i = 0; i < len; i++)
   {
      if(this.seriesarray[i].marks_enabled && this.seriesarray[i].marks_show_on_click)
      {
         this.seriesarray[i].current_mark = null;
         this.invalidate();
      }
   }

   return this.left_down; //prevent default behavior
};


CsiGraph.prototype.OnRButtonDown = function (mouseX, mouseY)
{
   var local_x = mouseX - this.left;
   var local_y = mouseY - this.top;

   //Don't start zooming unless we are inside the plot area
   if(this.plotRect.contains(new Point(local_x, local_y)))
   {
      this.right_down = true;
      this.zoomOrigin.x = local_x;
      this.zoomOrigin.y = local_y;
      if(this.leftAxis.scale)
      {
         this.left_pan_max = this.leftAxis.scale.get_max_value();
         this.left_pan_min = this.leftAxis.scale.get_min_value();
      }

      if(this.rightAxis.scale)
      {
         this.right_pan_max = this.rightAxis.scale.get_max_value();
         this.right_pan_min = this.rightAxis.scale.get_min_value();
      }

      if(this.bottomAxis.scale)
      {
         this.bottom_pan_max = this.bottomAxis.scale.get_max_value();
         this.bottom_pan_min = this.bottomAxis.scale.get_min_value();
      }
   }

   var len = this.seriesarray.length;
   var i;
   for(i = 0; i < len; i++)
   {
      if(this.seriesarray[i].marks_enabled && this.seriesarray[i].marks_show_on_click)
      {
         this.seriesarray[i].current_mark = null;
         this.invalidate();
      }
   }
};


CsiGraph.prototype.OnMouseMove = function (mouseX, mouseY)
{
   var local_x = mouseX - this.left;
   var local_y = mouseY - this.top;

   if(this.left_down)
   {
      //Draw the zoom rect
      this.zooming = true;
      this.zoomRect.set_drag_point(local_x, local_y, this.zoomOrigin.x, this.zoomOrigin.y);
      this.refresh();
      return true; //Prevent default behavior
   }
   else if(this.right_down)
   {
      if(!this.show_restore_btn)
      {
         this.show_restore_btn = true;
         this.SaveState(false);
      }
      this.panning = true;

      //Calculate our offset from the zoomOrigin and set scales accordingly
      if(this.leftAxis.scale && this.leftAxis.scale_rect)
      {
         var left_pan_origin_y = this.leftAxis.pos_to_value(this.zoomOrigin.y);
         var left_pan_y = this.leftAxis.pos_to_value(local_y);
         var left_diff = left_pan_origin_y - left_pan_y;
         this.leftAxis.auto_max = false;
         this.leftAxis.auto_min = false;
         this.leftAxis.max = this.left_pan_max + left_diff;
         this.leftAxis.min = this.left_pan_min + left_diff;
         this.leftAxis.auto_label = true;
      }

      if(this.rightAxis.scale && this.rightAxis.scale_rect)
      {
         var right_pan_origin_y = this.rightAxis.pos_to_value(this.zoomOrigin.y);
         var right_pan_y = this.rightAxis.pos_to_value(local_y);
         var right_diff = right_pan_origin_y - right_pan_y;
         this.rightAxis.auto_max = false;
         this.rightAxis.auto_min = false;
         this.rightAxis.max = this.right_pan_max + right_diff;
         this.rightAxis.min = this.right_pan_min + right_diff;
         this.rightAxis.auto_label = true;
      }

      if(this.bottomAxis.scale)
      {
         var bottom_pan_origin_x = this.bottomAxis.pos_to_value(this.zoomOrigin.x);
         var bottom_pan_x = this.bottomAxis.pos_to_value(local_x);
         var bottom_diff = bottom_pan_origin_x - bottom_pan_x;
         this.bottomAxis.auto_time = false;
         this.bottomAxis.auto_min = this.bottomAxis.auto_max = false;
         this.bottomAxis.max = this.bottom_pan_max + bottom_diff;
         this.bottomAxis.min = this.bottom_pan_min + bottom_diff;
         this.bottomAxis.auto_label = true;
      }

      //Force graph to recalculate scales
      this.positionsInvalid = true;
      this.refresh();
   }
   return false; //Don't override default behavior
};


CsiGraph.prototype.OnLButtonUp = function (mouseX, mouseY)
{
   var local_x = mouseX - this.left;
   var local_y = mouseY - this.top;

   if(this.zooming && (local_x !== this.zoomOrigin.x && local_y !== this.zoomOrigin.y))
   {

      if(!this.show_restore_btn)
      {
         //Only do this on the first zoom
         this.show_restore_btn = true;
         this.SaveState(true);
      }
      else
      {
         //Drag from bottom right to upper left undoes zoom
         if(this.zoomOrigin.x > local_x && this.zoomOrigin.y > local_y)
         {
            this.left_down = false;
            this.zooming = false;
            this.RestoreState();
            return;
         }
      }

      //Set the scales according to the zoom rect
      this.leftAxis.auto_label = true;
      this.leftAxis.auto_max = false;
      this.leftAxis.auto_min = false;
      if(this.leftAxis.inverted)
      {
         this.leftAxis.max = this.leftAxis.pos_to_value(this.zoomRect.bottom);
         this.leftAxis.min = this.leftAxis.pos_to_value(this.zoomRect.top);
      }
      else
      {
         this.leftAxis.max = this.leftAxis.pos_to_value(this.zoomRect.top);
         this.leftAxis.min = this.leftAxis.pos_to_value(this.zoomRect.bottom);
      }

      this.rightAxis.auto_label = true;
      this.rightAxis.auto_max = false;
      this.rightAxis.auto_min = false;
      if(this.rightAxis.inverted)
      {
         this.rightAxis.max = this.rightAxis.pos_to_value(this.zoomRect.bottom);
         this.rightAxis.min = this.rightAxis.pos_to_value(this.zoomRect.top);
      }
      else
      {
         this.rightAxis.max = this.rightAxis.pos_to_value(this.zoomRect.top);
         this.rightAxis.min = this.rightAxis.pos_to_value(this.zoomRect.bottom);
      }

      this.bottomAxis.auto_label = true;
      this.bottomAxis.auto_time = false;
      this.bottomAxis.auto_max = false;
      this.bottomAxis.auto_min = false;
      if(this.bottomAxis.inverted)
      {
         this.bottomAxis.max = this.bottomAxis.pos_to_value(this.zoomRect.left);
         this.bottomAxis.min = this.bottomAxis.pos_to_value(this.zoomRect.right);
      }
      else
      {
         this.bottomAxis.max = this.bottomAxis.pos_to_value(this.zoomRect.right);
         this.bottomAxis.min = this.bottomAxis.pos_to_value(this.zoomRect.left);
      }

      //Force graph to recalculate scales
      this.positionsInvalid = true;
   }

   //End zoom rect draw
   this.left_down = false;
   this.zooming = false;
   this.refresh();
};


CsiGraph.prototype.OnRButtonUp = function (mouseX, mouseY)
{
   this.panning = false;
   this.right_down = false;
};


CsiGraph.prototype.OnRButtonClick = function (mouseX, mouseY)
{
   var local_x = mouseX - this.left;
   var local_y = mouseY - this.top;

   if(!this.panning && this.plotRect.contains(new Point(local_x, local_y)))
   {
      this.ShowMenu(mouseX, mouseY); //use screen coords here
   }
};


CsiGraph.prototype.OnLButtonClick = function (mouseX, mouseY)
{
   var local_x = mouseX - this.left;
   var local_y = mouseY - this.top;

   var point = new Point(local_x, local_y);
   //Handle the restore btn click
   if(this.show_restore_btn && this.button_rect && this.button_rect.contains(point))
      this.RestoreState();
   else if(!this.zooming)
   {
      var try_count = 1;
      var keep_trying = true;
      var any_series_has_marks = false;
      while(keep_trying && try_count <= 10) //Try 10 times
      {
         //We need to do a hit test on the series to see if we need to look for marks
         var len = this.seriesarray.length;
         var i;
         for(i = 0; i < len; i++)
         {
            if(this.seriesarray[i].marks_enabled)
            {
               any_series_has_marks = true;
               if(this.seriesarray[i].hit_test(point, 5 * try_count))
               {
                  keep_trying = false;
                  this.invalidate();
                  break;
               }
            }
         }

         if(!any_series_has_marks)
            keep_trying = false;
         else
            ++try_count;
      }
   }
};


CsiGraph.prototype.SaveState = function (clear_time_format)
{
   this.leftAxis.save_state(clear_time_format);
   this.rightAxis.save_state(clear_time_format);
   this.bottomAxis.save_state(clear_time_format);
};


CsiGraph.prototype.RestoreState = function ()
{
   this.show_restore_btn = false;
   this.leftAxis.restore_state();
   this.rightAxis.restore_state();
   this.bottomAxis.restore_state();
   this.positionsInvalid = true;
   if(this.restore_gesture)
   {
      csiMouseEvents.release_gesture(this.restore_gesture);
      this.restore_gesture = null;
   }
   this.invalidate();
};


CsiGraph.prototype.OnMouseExit = function ()
{
   if(this.zooming || this.left_down)
   {
      this.left_down = false;
      this.zooming = false;
      this.refresh();
      return true; //Prevent default behavior
   }
   else if(this.panning || this.right_down)
   {
      this.panning = false;
      this.right_down = false;
      return true; //Prevent default behavior
   }
   return false;
};


CsiGraph.prototype.on_double_tap_complete = function(gesture)
{
   var right_scale = this.rightAxis.scale;
   var left_scale = this.leftAxis.scale;
   var bottom_scale = this.bottomAxis.scale;
   var gesture_point = this.translate_canvas(gesture.get_origin());
   var canvas_offset = this.owner.get_canvas_offset();
   
   gesture_point.offset(-this.left, -this.top);
   gesture_point.offset(canvas_offset.x, canvas_offset.y);
   if(!this.show_restore_btn)
   {
      this.show_restore_btn = true;
      this.SaveState(true);
   }
   if(right_scale)
   {
      var right_width = (right_scale.get_max_value() - right_scale.get_min_value()) / 2;
      var right_y = this.rightAxis.pos_to_value(gesture_point.y);
      this.rightAxis.auto_min = this.rightAxis.auto_max = false;
      this.rightAxis.min = right_y - right_width / 2;
      this.rightAxis.max = right_y + right_width / 2;
   }
   if(left_scale)
   {
      var left_width = (left_scale.get_max_value() - left_scale.get_min_value()) / 2;
      var left_y = this.leftAxis.pos_to_value(gesture_point.y);
      this.leftAxis.auto_min = this.leftAxis.auto_max = false;
      this.leftAxis.min = left_y - left_width / 2;
      this.leftAxis.max = left_y + left_width / 2;
   }
   if(bottom_scale)
   {
      var bottom_width = (bottom_scale.get_max_value() - bottom_scale.get_min_value()) / 2;
      var bottom_x = this.bottomAxis.pos_to_value(gesture_point.x);
      this.bottomAxis.auto_min = this.bottomAxis.auto_max = false;
      this.bottomAxis.auto_time = false;
      this.bottomAxis.min = bottom_x - bottom_width / 2;
      this.bottomAxis.max = bottom_x + bottom_width / 2;
   }
   for(var i = 0; i < this.seriesarray.length; ++i)
   {
      var series = this.seriesarray[i];
      if(series.marks_enabled && series.marks_show_on_click)
      {
         series.current_mark = null;
      }
   }
   this.positionsInvalid = true;
   this.invalidate();
};


CsiGraph.prototype.on_single_tap_complete = function(gesture)
{
   if(gesture === this.restore_gesture)
   {
      this.RestoreState();
      this.invalidate();
   }
   else if(gesture == this.marks_gesture)
   {
      var local = this.translate_canvas(gesture.get_origin());
      this.OnLButtonClick(local.x, local.y);
   }
};


CsiGraph.prototype.on_swipe_moved = function(gesture, delta_x, delta_y)
{
   var right_scale = this.rightAxis.scale;
   var left_scale = this.leftAxis.scale;
   var bottom_scale = this.bottomAxis.scale;
   if(!this.show_restore_btn)
   {
      this.show_restore_btn = true;
      this.SaveState(false);
   }
   if(right_scale)
   {
      var right_delta = delta_y / right_scale.scale;
      this.rightAxis.auto_min = this.rightAxis.auto_max = false;
      this.rightAxis.min = right_scale.get_min_value() + right_delta;
      this.rightAxis.max = right_scale.get_max_value() + right_delta;
   }
   if(left_scale)
   {
      var left_delta = delta_y / left_scale.scale;
      this.leftAxis.auto_min = this.leftAxis.auto_max = false;
      this.leftAxis.min = left_scale.get_min_value() + left_delta;
      this.leftAxis.max = left_scale.get_max_value() + left_delta;
   }
   if(bottom_scale)
   {
      var bottom_delta = delta_x / bottom_scale.scale;
      this.bottomAxis.auto_min = this.bottomAxis.auto_max = false;
      this.bottomAxis.auto_time = false;
      this.bottomAxis.min = bottom_scale.get_min_value() - bottom_delta;
      this.bottomAxis.max = bottom_scale.get_max_value() - bottom_delta;
   }
   this.positionsInvalid = true;
   this.refresh();
};


CsiGraph.prototype.on_swipe_complete = function(gesture)
{
   var series_len = this.seriesarray.length;
   for(var i = 0; i < series_len; ++i)
   {
      var series = this.seriesarray[i];
      if(series.marks_enabled && series.marks_show_on_click)
      {
         series.current_mark = null;
      }
   }
   this.invalidate();
};


CsiGraph.prototype.on_pinch_start = function(gesture)
{
   var series_len = this.seriesarray.length;
   for(var i = 0; i < series_len; ++i)
   {
      var series = this.seriesarray[i];
      if(series.marks_enabled && series.marks_show_on_click)
      {
         series.current_mark = null;
      }
   }
};


CsiGraph.prototype.on_pinch_moved = function(gesture, scale, midpoint_, delta_x, delta_y)
{
   var right_scale = this.rightAxis.scale;
   var left_scale = this.leftAxis.scale;
   var bottom_scale = this.bottomAxis.scale;
   var midpoint = this.translate_canvas(midpoint_);
   var max;
   var min;
   var ratio;
   var new_width;
   var value;

   midpoint.offset(-this.left, -this.top);
   if(!this.show_restore_btn)
   {
      this.show_restore_btn = true;
      this.SaveState(true);
   }
   if(right_scale)
   {
      max = right_scale.get_max_value();
      min = right_scale.get_min_value();
      new_width = (max - min) * scale;
      value = this.rightScale.pos_to_value(midpoint.y);
      ratio = (value - min) / (max - min);
      this.rightAxis.auto_min = this.rightAxis.auto_max = false;
      this.rightAxis.min = value - new_width * ratio + delta_y / right_scale.scale;
      this.rightAxis.max = this.rightAxis.min + new_width;
   }
   if(left_scale)
   {
      max = left_scale.get_max_value();
      min = left_scale.get_min_value();
      new_width = (max - min) * scale;
      value = this.leftAxis.pos_to_value(midpoint.y);
      ratio = (value - min) / (max - min);
      this.leftAxis.auto_min = this.leftAxis.auto_max = false;
      this.leftAxis.min = value - new_width * ratio + delta_y / left_scale.scale;
      this.leftAxis.max = this.leftAxis.min + new_width;
   }
   if(bottom_scale)
   {
      max = bottom_scale.get_max_value();
      min = bottom_scale.get_min_value();
      new_width = (max - min) * scale;
      value = this.bottomAxis.pos_to_value(midpoint.x);
      ratio = (value - min) / (max - min);
      this.bottomAxis.auto_min = this.bottomAxis.auto_max = false;
      this.bottomAxis.auto_time = false;
      this.bottomAxis.min = value - new_width * ratio - delta_x / bottom_scale.scale;
      this.bottomAxis.max = this.bottomAxis.min + new_width;
   }
   this.positionsInvalid = true;
   this.refresh();
};


CsiGraph.prototype.on_pinch_complete = function(gesture)
{
   this.invalidate();
};


CsiGraph.prototype.activate = function(context)
{
   CsiComponent.prototype.activate.call(this);
   csiMouseEvents.register_gesture(this.double_zoom_gesture);
   csiMouseEvents.register_gesture(this.pan_gesture);
   csiMouseEvents.register_gesture(this.marks_gesture);
   csiMouseEvents.register_gesture(this.pinch_gesture);
   if(this.restore_gesture)
   {
      csiMouseEvents.register_gesture(this.restore_gesture, 0);
   }
};


CsiGraph.prototype.deactivate = function()
{
   CsiComponent.prototype.deactivate.call(this);
   CsiMouseEvents.remove_component(this);
   csiMouseEvents.release_gesture(this.double_zoom_gesture);
   csiMouseEvents.release_gesture(this.pan_gesture);
   csiMouseEvents.release_gesture(this.marks_gesture);
   csiMouseEvents.release_gesture(this.pinch_gesture);
   if(this.restore_gesture)
   {
      csiMouseEvents.release_gesture(this.restore_gesture);
   }
};


CsiGraph.prototype.clear_all_series = function()
{
   this.seriesarray = [];
   this.bottomAxis.series = [];
   this.leftAxis.series = [];
   this.rightAxis.series = [];
   this.positionsInvalid = true;
};


CsiGraph.prototype.remove_series = function(series)
{
   var index = this.seriesarray.indexOf(series);
   if(index >= 0)
   {
      series.domain_axis.remove_series(series);
      series.range_axis.remove_series(series);
      this.seriesarray.splice(index, 1);
      this.positionsInvalid = true;
   }
};


Enum.LEGEND_ALIGNMENT =
{
   LEFT: 0,
   RIGHT: 1,
   TOP: 2,
   BOTTOM: 3
};


function CsiWindData(timestamp)
{
   //Do not add properties to prototype
   if(arguments.length === 0)
      return;
   this.timestamp = timestamp;
   this.speed = null;
   this.windSpeedBin = null;
   this.direction = null;
   this.windDirectionIndex = -1;
}


CsiWindData.prototype.setWindSpeed = function (value, windSpeedBin)
{
   this.speed = value;
   this.windSpeedBin = windSpeedBin;
   if((this.windSpeedBin) && (this.windDirectionIndex >= 0))
      this.addToBin();
};


CsiWindData.prototype.setWindDirection = function (value, windDirectionIndex)
{
   this.direction = value;
   this.windDirectionIndex = windDirectionIndex;
   if((this.windSpeedBin) && (this.windDirectionIndex >= 0))
      this.addToBin();
};


CsiWindData.prototype.addToBin = function()
{
   this.windSpeedBin.increment(this.windDirectionIndex);
};


CsiWindData.prototype.removeFromBin = function()
{
   this.windSpeedBin.decrement(this.windDirectionIndex);
};


function CsiWindSpeedBin(windRose, maxWindSpeed, color)
{
   //Do not add properties to prototype
   if(arguments.length === 0)
      return;
   this.maxWindSpeed = maxWindSpeed;
   this.color = color;
   this.directionCounts = [];
   this.directionPercent = [];

   //initialize arrays
   this.directionCounts.length = windRose.windDirectionCount;
   this.directionPercent.length = windRose.windDirectionCount;
   var i;
   var len = windRose.windDirectionCount;
   for (i = 0; i < len; i++)
   {
      this.directionCounts[i] = 0;
      this.directionPercent[i] = 0;
   }
}


CsiWindSpeedBin.prototype.increment = function(dir_index)
{
   this.directionCounts[dir_index]++;
};


CsiWindSpeedBin.prototype.decrement = function (dir_index)
{
   this.directionCounts[dir_index]--;
   if(this.directionCounts[dir_index] < 0)
      csi_log('assertion(directionCount) < 0');
};


CsiWindSpeedBin.prototype.clearPercentages = function()
{
   var i;
   var len = this.directionPercent.length;
   for (i = 0; i < len; i++)
      this.directionPercent[i] = 0;
};


function CsiWindRose(left, top, width, height)
{
   //Do not add properties to prototype
   if(arguments.length === 0)
      return;
   CsiComponent.call(this, left, top, width, height);

   //wind rose properties
   this.displayRange = 86400000; // one day
   this.title = null;
   this.backgroundGradientStartColor = "white";
   this.backgroundGradientEndColor = "gray";
   this.transparent = true;
   this.lineColor = "black";
   this.titleFont = "12pt Arial bold";
   this.titleFontColor = "black";
   this.dirLabelFont = "10pt Arial";
   this.dirLabelFontColor = "black";
   this.legendFont = "8pt Arial";
   this.legendFontColor = "black";
   this.legendTitleFont = "8pt Arial bold";
   this.legendTitleFontColor = "black";
   this.scaleFont = "8pt Arial";
   this.scaleFontColor = "black";
   this.scaleAngle = 315;
   this.autoScale = true;
   this.scaleMax = 20;          //does not apply when autoScale is true
   this.margin = 14;            //outer margin
   this.roseMargin = 6;         //roseMargin
   this.legendTitle = "Wind (m/s)";
   this.calmInCenter = true;    //place calm in Center?
   this.innerRoseRadius = 0;    //does not apply when calmInCenter is true
   this.ringCount = 3;          //number of rings (excluding the center ring)
   this.petalSizePercent = 100.0; //100% = Completely fill petal
   this.scalePosition = Enum.WIND_ROSE_SCALE_POSITION.RING_OUTSIDE;
   this.windOrientation = Enum.WIND_ORIENTATION.BLOWING_FROM;
   this.windDirectionCount = 16; //number of wind directions
   this.windDirections = [];
   this.headings = ["E", "", "NE", "",  //headings. Independent from the wind directions
                    "N", "", "NW", "",
                    "W", "", "SW", "",
                    "S", "", "SE", ""];
   this.roseTransparent = true;
   this.roseGradientStartColor = "white";
   this.roseGradientEndColor = "gray";
   this.backgroundGradientOrientation = Enum.ORIENTATION.TOP_TO_BOTTOM;

   //data
   this.allWindData = [];
   this.bins = [];
   this.calmPercent = 0;
   this.calmBin = null;
   this.newestTimestamp = 0;

   //for drawing
   this.roseCenter = null;
   this.roseRadius = null;
   this.roseGradient = null;
   this.maxPetalLength = null;
   this.backgroundGradient = null;

   this.legendLeft = null;
   this.legendWidth = null;
   this.legendBoxSize = null;
}
CsiWindRose.prototype = new CsiComponent();


function CsiWindDirection()
{
   this.maxDirection = 0;  //maximum degrees that is included in this direction
   this.totalPercent = 0;  //total percentage that is pointing in this direction.  Used for autoScale
   this.petalStart = 0; //current petal start position.  increases as we move along petal
}


CsiWindRose.prototype.activate = function (context)
{
   CsiComponent.prototype.activate.call(this);
   this.calculatePositions(context);
};


CsiWindRose.prototype.calculatePositions = function (context)
{
   //legend position
   context.font = this.legendFont;
   var maxWidth = this.getLargestBinTextWidth(context);
   var maxHeight = context.measureText("W").width * 1.2; //assume height of text is width of "W"
   this.legendBoxSize = Math.ceil(maxHeight * 1.1);
   this.legendBoxBorder = Math.ceil(this.legendBoxSize * 0.2);
   maxWidth = maxWidth + this.legendBoxSize + this.legendBoxBorder;

   //take calm bin width into account
   maxWidth = Math.max(maxWidth, context.measureText("Calm: 99%").width);

   //take legendTitle width into account to determine legendWidth
   context.font = this.legendTitleFont;
   this.legendWidth = Math.max(maxWidth, context.measureText(this.legendTitle).width);
   this.legendLeft = this.width - this.margin - this.legendWidth;

   //rose Position
   var roseRect;
   if(this.title) 
   {
      context.font = this.titleFont;
      var titleBottom = this.margin + context.measureText("W").width; //assume height("W") == width("W");
      roseRect = new Rect(this.margin,
                          titleBottom + this.roseMargin,
                          this.legendLeft - this.margin - this.roseMargin,
                          (this.height - titleBottom) - this.margin - this.roseMargin);
   }
   else
   {
      roseRect = new Rect(this.margin,
                          this.margin,
                          this.legendLeft - this.margin - this.roseMargin,
                          this.height - this.margin * 2);
   }
   this.roseCenter = new Point(roseRect.left + roseRect.width / 2, roseRect.top + roseRect.height / 2);

   context.font = this.dirLabelFont;
   this.roseRadius = Math.min(roseRect.width / 2 - this.getLargestDirLabelWidth(context),
                              roseRect.height / 2 - context.measureText("W").width);

   if(this.calmInCenter)
   {
      context.font = this.scaleFont;
      this.innerRoseRadius = context.measureText("100%").width / 2;
   }

   this.maxPetalLength = this.roseRadius - this.innerRoseRadius;
};


CsiWindRose.prototype.getLargestDirLabelWidth = function (context)
{
   var result = 0;
   this.headings.forEach(function(heading) {
      result = Math.max(result, context.measureText(heading).width);
   });
   return result;
};


CsiWindRose.prototype.getLargestBinTextWidth = function (context)
{
   var result = 0;
   var comp = this;
   this.bins.forEach(function(bin, index) {
      result = Math.max(result, context.measureText(comp.getBinText(bin, index)).width);
   });
   return result;
};


CsiWindRose.prototype.addWindSpeedBin = function (maxWindSpeed, color)
{
   if(this.calmBin === null) 
      this.calmBin = new CsiWindSpeedBin(this, maxWindSpeed, color, 0);
   else
      this.bins.push(new CsiWindSpeedBin(this, maxWindSpeed, color));
};


CsiWindRose.prototype.on_new_data = function(for_speed, value, timestamp, expect_more)
{
   //get windData
   var windData = this.getWindDataAtTime(timestamp); //finds existing windData at timestamp
   if(windData === null) 
   {
      windData = new CsiWindData(timestamp);
      this.allWindData.push(windData);
   }

   //set WindSpeed or direction
   if(for_speed)
      windData.setWindSpeed(value, this.getWindSpeedBin(value));
   else //windDirHandler
   {
      //verify range
      if(value > 360)
         value = 360;
      else if(value < 0)
         value = 0;
      windData.setWindDirection(value, this.getCsiWindDirectionIndex(value));
   }

   // if more data is expected, we will let the values accumulate, otherwise, we will recalculate and force a redraw. 
   if(!expect_more)
   {
      if(timestamp > this.newestTimestamp)
         this.newestTimestamp = timestamp;
      this.removeOldData();
      this.computate();
      this.invalidate();
   }
};


CsiWindRose.prototype.getCsiWindDirectionIndex = function (direction)
{
   var rtn = 0;
   for(var i = 0; i < this.windDirections.length; i++)
   {
      if(direction < this.windDirections[i].maxDirection)
         return i;
   }
   return 0;
};


CsiWindRose.prototype.getWindSpeedBin = function (windSpeed)
{
   //Check for calm
   if(windSpeed <= this.calmBin.maxWindSpeed)
      return this.calmBin;

   //Check all of the bins except the last one, it get the hit if nothing
   //else does.
   var len = this.bins.length;
   var i;
   for(i = 0; i < len - 1; i++)
   {
      if(windSpeed <= this.bins[i].maxWindSpeed)
         return this.bins[i];
   }
   return this.bins[len - 1];   // last bin catches everything else
};


CsiWindRose.prototype.removeOldData = function ()
{
   //find and decrement bin counts for old data
   var cutoff = this.newestTimestamp - this.displayRange;
   this.allWindData = this.allWindData.filter(function(data) {
      var rtn = data.timestamp >= cutoff;
      if(!rtn)
         data.removeFromBin();
      return rtn;
   });
};


CsiWindRose.prototype.getWindDataAtTime = function (timestamp)
{
   var rtn = null;
   var len = this.allWindData.length;
   if(len > 0)
   {
      var i;
      var cont = true;
      for(i = len - 1; i >= 0 && cont; i--)
      {
         if(this.allWindData[i].timestamp.milliSecs === timestamp.milliSecs)
         {
            rtn = this.allWindData[i];
            cont = false;
         }
         else if(this.allWindData[i].timestamp.milliSecs < timestamp.milliSecs) //timestamp is older than what we are looking for
            cont = false;
      }
   }
   return rtn;
};


CsiWindRose.prototype.computate = function ()
{
   this.clearTotalPercentages();

   var total_data_points = this.allWindData.length;
   if(total_data_points > 0)
   {
      var i;
      var j;

      //Calculate the calm %
      var calm_hit_count_all_dirs = 0;
      var directionCount = this.windDirectionCount;
      for(i = 0; i < directionCount; ++i)
         calm_hit_count_all_dirs += this.calmBin.directionCounts[i];
      this.calmPercent = (calm_hit_count_all_dirs / total_data_points) * 100.0;

      //Calculate the % for each bin in each direction
      var percent = 0;
      var len = this.bins.length;
      for(i = 0; i < len; ++i) //for each bin
      {
         var bin = this.bins[i];
         bin.clearPercentages();
         for(j = 0; j < directionCount; ++j) //for each direction
         {
            percent = (bin.directionCounts[j] / total_data_points) * 100.0;
            bin.directionPercent[j] = percent;
            this.windDirections[j].totalPercent += percent;
         }
      }
      if(this.autoScale)
         this.calculateScaleMax();
   }
};


CsiWindRose.prototype.initializeArrays = function ()
{
   if(this.windDirections.length === 0)
   {
      this.windDirections.length = this.windDirectionCount;
      var diff = (360) / this.windDirectionCount;
      var direction = 0;
      var len = this.windDirectionCount;
      var i;
      for(i = 0; i < len; i++)
      {
         this.windDirections[i] = new CsiWindDirection();
         this.windDirections[i].maxDirection = direction + diff / 2;
         this.windDirections[i].totalPercent = 0;
         direction += diff;
      }
   }
};


CsiWindRose.prototype.clearTotalPercentages = function ()
{
   var i;
   var len = this.windDirectionCount;
   this.calmPercent = 0;
   for(i = 0; i < len; i++)
      this.windDirections[i].totalPercent = 0;
};


CsiWindRose.prototype.calculateScaleMax = function ()
{
   var i;
   var len = this.windDirectionCount;
   this.scaleMax = this.ringCount;
   for(i = 0; i < len; i++) //for each direction
      this.scaleMax = Math.max(this.windDirections[i].totalPercent, this.scaleMax);

   //ensure that the scaleMax is evenly divisible by the number of rings
   this.scaleMax = Math.ceil(this.scaleMax);
   var ringDiff = Math.ceil(this.scaleMax / this.ringCount);
   this.scaleMax = ringDiff * this.ringCount;
};


CsiWindRose.prototype.draw = function (context)
{
   if(this.width * this.height === 0)
   {
      return;
   }


   context.translate(this.left, this.top); //move to location
   clipRect(context, 0, 0, this.width, this.height);
   context.clearRect(0, 0, this.width, this.height);
   context.fillStyle = "RGBA(255, 255, 255, .5)";
   context.fillRect(0, 0, this.width, this.height);
   if(!this.transparent)
   {
      if(this.backgroundGradient === null) 
      {
         switch(this.backgroundGradientOrientation)
         {
         case Enum.ORIENTATION.TOP_TO_BOTTOM:
            this.backgroundGradient = context.createLinearGradient(0, 0, 0, this.height);
            this.backgroundGradient.addColorStop(0, this.backgroundGradientStartColor);
            this.backgroundGradient.addColorStop(1, this.backgroundGradientEndColor);
            break;

         case Enum.ORIENTATION.LEFT_TO_RIGHT:
            this.backgroundGradient = context.createLinearGradient(0, 0, this.width, 0);
            this.backgroundGradient.addColorStop(0, this.backgroundGradientStartColor);
            this.backgroundGradient.addColorStop(1, this.backgroundGradientEndColor);
            break;

         case Enum.ORIENTATION.RIGHT_TO_LEFT:
            this.backgroundGradient = context.createLinearGradient(0, 0, this.width, 0);
            this.backgroundGradient.addColorStop(0, this.backgroundGradientEndColor);
            this.backgroundGradient.addColorStop(1, this.backgroundGradientStartColor);
            break;

         default:
            this.backgroundGradient = context.createLinearGradient(0, 0, 0, this.height);
            this.backgroundGradient.addColorStop(0, this.backgroundGradientEndColor);
            this.backgroundGradient.addColorStop(1, this.backgroundGradientStartColor);
            break;
         }
      }
      context.fillStyle = this.backgroundGradient;
      context.fillRect(0, 0, this.width, this.height);
   }
   
   this.drawTitle(context);
   this.drawRose(context);
   this.drawLegend(context);
   this.drawBorder(context, new Rect(0, 0, this.width, this.height));
};


CsiWindRose.prototype.drawTitle = function (context)
{
   if(this.title) 
   {
      context.font = this.titleFont;
      context.fillStyle = this.titleFontColor;
      context.textBaseline = "top";
      context.textAlign = "center";
      context.fillText(this.title, this.width / 2, this.margin);
   }
};


CsiWindRose.prototype.drawRose = function (context)
{
   if(this.roseCenter) 
   {
      context.save();
      context.strokeStyle = this.lineColor;
      context.lineWidth = 0.5;
      context.translate(this.roseCenter.x, this.roseCenter.y);
      if(!this.roseTransparent)
         this.drawRoseGradient(context);
      if(this.calmInCenter)
         this.drawCalmInCenter(context);
      this.drawRings(context);
      this.drawSpokes(context);
      this.drawSpokeHeadings(context);
      this.drawPetals(context);
      this.drawScale(context);
      context.restore();
   }
};


CsiWindRose.prototype.drawRoseGradient = function (context)
{
   if(this.roseGradient === null) 
   {
      this.roseGradient = context.createRadialGradient(0, 0, 1, 0, 0, this.roseRadius*1.35);
      this.roseGradient.addColorStop(0, this.roseGradientStartColor);
      this.roseGradient.addColorStop(0.1, this.roseGradientStartColor);
      this.roseGradient.addColorStop(1, this.roseGradientEndColor);
   }
   context.fillStyle = this.roseGradient;
   context.beginPath();
   context.arc(0, 0, this.roseRadius, 0, Math.PI * 2, false);
   context.fill();
};


CsiWindRose.prototype.drawCalmInCenter = function (context)
{
   context.font = this.scaleFont;
   context.fillStyle = this.scaleFontColor;

   //calm percent
   context.textBaseline = "middle";
   context.textAlign = "center";
   context.fillText(Math.ceil(this.calmPercent) + "%", 0, 0);
};


CsiWindRose.prototype.drawRings = function (context)
{
   var ringDistanceDiff = (this.roseRadius - this.innerRoseRadius) / this.ringCount;
   if(ringDistanceDiff > 0)
   {
      var i;
      var distance = this.innerRoseRadius;
      var len = this.ringCount + 1;
      for(i = 0; i < len; i++)  //add 1 to include inner ring
      {
         context.beginPath();
         context.arc(0, 0, distance, 0, 2 * Math.PI, false);
         context.stroke();

         distance += ringDistanceDiff;
      }
   }
};


CsiWindRose.prototype.drawSpokes = function (context)
{
   var angleDiff = (Math.PI * 2) / this.headings.length;

   context.save();
   var i;
   var len = this.headings.length;
   for(i = 0; i < len; i++)
   {
      if(this.headings[i] !== "")
      {
         context.beginPath();
         context.moveTo(this.innerRoseRadius, 0);
         context.lineTo(this.roseRadius, 0);
         context.stroke();
      }

      context.rotate(-angleDiff);
   }
   context.restore();
};


CsiWindRose.prototype.drawSpokeHeadings = function (context)
{
   context.font = this.dirLabelFont;
   context.fillStyle = this.dirLabelFontColor;

   var angleDiff = (Math.PI * 2) / this.headings.length;
   var angle = 0;
   var labelRadius = this.roseRadius;
   var i;
   var len = this.headings.length;
   for(i = 0; i < len; i++)
   {
      var labelX = labelRadius * Math.cos(angle);
      var labelY = labelRadius * Math.sin(angle);

      //first check right angle axes
      if(Math.abs(angle - Math.PI / 2) < 0.00001) //south
      {
         context.textAlign = "center";
         context.textBaseline = "top";
      }
      else if(Math.abs(angle - Math.PI * 3 / 2) < 0.00001) //north
      {
         context.textAlign = "center";
         context.textBaseline = "bottom";
      }
      else if(Math.abs(angle < 0.00001)) //east
      {
         context.textAlign = "left";
         context.textBaseline = "middle";
      }
      else if(Math.abs(angle - Math.PI) < 0.00001) //west
      {
         context.textAlign = "right";
         context.textBaseline = "middle";
      }
      else //not a right angle axis
      {
         if((angle < Math.PI / 2) || (angle > Math.PI * 3 / 2)) //right side of rose
            context.textAlign = "left";
         else if((angle > Math.PI / 2) && (angle < Math.PI * 3 / 2)) //left side of rose
            context.textAlign = "right";
         if(angle < Math.PI) //bottom of rose
            context.textBaseline = "top";
         else //top of rose
            context.textBaseline = "bottom";
      }
      context.fillText(this.headings[i], labelX, labelY);
      angle -= angleDiff;
      if(angle < 0)
         angle += Math.PI * 2;
   }
};


CsiWindRose.prototype.drawLegend = function (context)
{
   //start at bottom of legend and move upward
   var x = Math.floor(this.legendLeft) + 0.5;
   var y = this.height - this.margin;

   //draw calm %
   context.font = this.legendFont;
   context.fillStyle = this.legendFontColor;
   context.textBaseline = "bottom";
   context.textAlign = "left";
   context.fillText("Calm: " + Math.ceil(this.calmPercent) + "%", x, y);

   //draw bins
   y -= (this.legendBoxSize + this.legendBoxBorder);
   var textX = this.legendLeft + this.legendBoxSize + this.legendBoxBorder;
   context.textBaseline = "middle";
   context.strokeStyle = "black";

   //start at last bin and move to calm bin
   var bin = null;
   var len = this.bins.length;
   var i;
   for(i = 0; i < len; i++)
   {
      bin = this.bins[i];
      context.fillStyle = bin.color;
      context.fillRect(x, y - this.legendBoxSize, this.legendBoxSize, this.legendBoxSize);
      context.strokeRect(x, Math.floor(y - this.legendBoxSize) + 0.5, this.legendBoxSize, this.legendBoxSize);
      context.fillStyle = this.legendFontColor;
      context.fillText(this.getBinText(bin, i), textX, y - this.legendBoxSize / 2);
      y -= (this.legendBoxSize + this.legendBoxBorder);
   }
   y -= this.legendBoxBorder;

   //draw legend title
   context.textAlign = "center";
   context.textBaseline = "bottom";
   context.font = this.legendTitleFont;
   context.fillStyle = this.legendTitleFontColor;
   context.fillText(this.legendTitle, x + this.legendWidth / 2, y);
};


CsiWindRose.prototype.getBinText = function (bin, index)
{
   var rtn = this.calmBin.maxWindSpeed + " - " + bin.maxWindSpeed;
   if(index === this.bins.length - 1) //last bin
   {
      if(this.bins.length === 1)
         rtn = "> " + this.calmBin.maxWindSpeed;
      else
         rtn = "> " + this.bins[index - 1].maxWindSpeed;
   }
   else if(index > 0)
      rtn = this.bins[index - 1].maxWindSpeed + " - " + bin.maxWindSpeed;
   return rtn;
};


CsiWindRose.prototype.drawPetals = function (context)
{
   var i;
   var j;
   var bin = null;
   var petalLength;
   var angleDiff = (Math.PI * 2.0) / this.windDirectionCount;
   var halfPetalWidth = (angleDiff * this.petalSizePercent / 100.0) / 2.0;

   context.save();
   context.strokeStyle = "black";
   context.rotate(3.0 / 2.0 * Math.PI); //start at north
   if(this.windOrientation === Enum.WIND_ORIENTATION.BLOWING_TO)
      context.rotate(Math.PI);

   //reset direction petalStarts
   var dirLen = this.windDirectionCount;
   var binsLen = this.bins.length;
   for(i = 0; i < dirLen; i++)
      this.windDirections[i].petalStart = this.innerRoseRadius;
   for(i = 0; i < binsLen; i++)
   {
      bin = this.bins[i];
      context.fillStyle = bin.color;
      dirLen = this.windDirections.length;
      for(j = 0; j < dirLen; j++)
      {
         petalLength = (bin.directionPercent[j] / this.scaleMax) * this.maxPetalLength;
         if(bin.directionPercent[j] > 0)
         {
            context.beginPath();
            context.arc(0, 0, this.windDirections[j].petalStart, halfPetalWidth, -halfPetalWidth, true);
            context.arc(0, 0, this.windDirections[j].petalStart + petalLength, -halfPetalWidth, halfPetalWidth, false);
            context.closePath();
            context.fill();
            context.stroke();
         }
         this.windDirections[j].petalStart += petalLength;
         context.rotate(angleDiff);
      }
   }
   context.restore();
};


CsiWindRose.prototype.drawScale = function (context)
{
   //make 0 north
   var angle = degreesToRadians(this.scaleAngle);
   context.fillStyle = this.scaleFontColor;
   context.font = this.scaleFont;
   angle -= Math.PI / 2;
   if(angle < 0)
      angle += Math.PI * 2;
   if(this.scalePosition === Enum.WIND_ROSE_SCALE_POSITION.RING_OUTSIDE)
   {
      if((angle > Math.PI / 2) && (angle < Math.PI * 3 / 2)) //left side of rose
         context.textAlign = "right";
      else
         context.textAlign = "left";
      if(angle < Math.PI) //bottom of rose
         context.textBaseline = "top";
      else //top of rose
         context.textBaseline = "bottom";
   }
   else
   {
      if((angle > Math.PI / 2) && (angle < Math.PI * 3 / 2)) //left side of rose
         context.textAlign = "left";
      else
         context.textAlign = "right";
      if(angle < Math.PI) //bottom of rose
         context.textBaseline = "bottom";
      else //top of rose
         context.textBaseline = "top";
   }

   var x;
   var y;
   var distanceDiff = (this.roseRadius - this.innerRoseRadius) / this.ringCount;
   var scaleDiff = (this.scaleMax / this.ringCount);
   var distance = this.roseRadius;
   var percent = this.scaleMax;
   var i = this.ringCount - 1;
   while(i >= 0)
   {
      x = distance * Math.cos(angle);
      y = distance * Math.sin(angle);
      context.fillText(Math.round(percent) + "%", x, y);
      distance -= distanceDiff;
      percent -= scaleDiff;
      i--;
   }
};


CsiWindRose.prototype.reset_data = function (reset_settings)
{

   var i;
   var j;
   var directionCount = this.windDirectionCount;
   var len = this.bins.length;
   this.allWindData = [];
   for(i = 0; i < directionCount; ++i)
   {
      this.calmBin.directionCounts[i] = 0;
      this.calmBin.directionPercent[i] = 0;
   }
   for(i = 0; i < len; ++i) //for each bin
   {
      var bin = this.bins[i];
      bin.clearPercentages();
      for(j = 0; j < directionCount; ++j) //for each direction
      {
         bin.directionCounts[j] = 0;
         bin.directionPercent[j] = 0;
      }
   }
   if(reset_settings)
      this.displayRange = Number.MAX_VALUE;
   this.newestTimestamp = 0;

   this.computate();
   this.invalidate();
};


Enum.WIND_ROSE_SCALE_POSITION =
{
   RING_OUTSIDE: 0,
   RING_INSIDE: 1
};

Enum.WIND_ORIENTATION =
{
   BLOWING_FROM: 0,
   BLOWING_TO: 1
};



var csiMouseEvents = null; //GLOBAL DECLARATION


function CsiMouseEvents()
{
   this.mouseDownPos = null; //Track this position to make sure the mouseUp event triggers a click only if needed
   this.leftMouseDownComp = null;  //left mouseDown occurred on component
   this.rightMouseDownComp = null; //right mouseDown occurred on component
   this.mouseOverComp = null; //last component which the mouse moved over
   this.dragComp = null; //component being dragged with left button
   this.lastTouchPos = null; //Track the last touch position
   this.gestures = [];
   this.components = [];
}


CsiMouseEvents.initialise = function()
{
   csiMouseEvents = new CsiMouseEvents();
   document.body.addEventListener("touchcancel", CsiMouseEvents.onTouchCancel, false);
};


CsiMouseEvents.add_component = function(component, canvas, priority)
{
   if(priority === undefined)
      component.mouse_priority = csiMouseEvents.components.length;
   else
      component.mouse_priority = priority;
   csiMouseEvents.components.push(component);
   csiMouseEvents.components.sort(function(comp1, comp2) {
      return comp1.mouse_priority - comp2.mouse_priority;
   });
   canvas.mousedown(function (evt) {
      return csiMouseEvents.onMouseDown(evt);
   });
   canvas.mouseup(function (evt) {
      return csiMouseEvents.onMouseUp(evt);
   });
   canvas.mousemove(function (evt) {
      return csiMouseEvents.onMouseMove(evt);
   });
   canvas.dblclick(function (evt) {
      return csiMouseEvents.onDblClick(evt);
   });
   canvas.mouseout(function (evt) {
      return csiMouseEvents.onMouseOut(evt);
   });
   canvas.on("touchstart", CsiMouseEvents.onTouchStart);
   canvas.on("touchend", CsiMouseEvents.onTouchEnd);
   canvas.on("touchmove", CsiMouseEvents.onTouchMove);
};


CsiMouseEvents.remove_component = function(component)
{
   var index = csiMouseEvents.components.indexOf(component);
   csiMouseEvents.components.splice(index, 1);
};


CsiMouseEvents.hit_test = function(x, y)
{
   var rtn = null;
   csiMouseEvents.components.every(function(component) {
      var canvas_offset = component.owner.get_canvas_offset();
      var comp_x = x - canvas_offset.x;
      var comp_y = y - canvas_offset.y;
      if(comp_x >= component.left && comp_x <= component.right &&
         comp_y >= component.top && comp_y <= component.bottom)
         rtn = component;
      return rtn === null;
   });
   return rtn;
};


CsiMouseEvents.find_touch_comp = function (event)
{
   var rtn =
   {
      component: null,
      touch_x: NaN,
      touch_y: NaN
   };
   var touch_test;
   var touch;
   var component;
   var i, j;
   
   for(i = 0; i < event.touches.length && rtn.component === null; ++i)
   {
      touch = event.touches[i];
      touch_point = new Point(touch.pageX, touch.pageY);
      rtn.touch_x = touch_point.x;
      rtn.touch_y = touch_point.y;
      touch_test = CsiMouseEvents.hit_test(rtn.touch_x, rtn.touch_y);
      if(!rtn.component && touch_test)
         rtn.component = touch_test;
      else if(rtn.component !== touch_test)
      {
         rtn.component = null;
         break;
      }
   }
   return rtn;
};


CsiMouseEvents.onTouchStart = function (evt)
{
   csiMouseEvents.gestures.forEach(function(gesture) {
      gesture.on_touch_start(evt);
   });
};


CsiMouseEvents.onTouchMove = function (evt)
{
   csiMouseEvents.gestures.forEach(function(gesture) {
      gesture.on_touch_move(evt);
   });
};


CsiMouseEvents.onTouchEnd = function (evt)
{
   csiMouseEvents.gestures.forEach(function(gesture) {
      gesture.on_touch_end(evt);
   });
};


CsiMouseEvents.onTouchCancel = function (event)
{
   CsiMouseEvents.onTouchEnd(event);
};


CsiMouseEvents.prototype.onDblClick = function (evt)
{
   var mouseX = evt.pageX;
   var mouseY = evt.pageY;
   var hit_comp = CsiMouseEvents.hit_test(mouseX, mouseY);
   if(hit_comp)
   {
      var canvas_offset = hit_comp.owner.get_canvas_offset();
      if(evt.button === Enum.BUTTON.LEFT)
      {
         if(typeof hit_comp.OnLButtonDblClk === "function")
            hit_comp.OnLButtonDblClk(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
      }
   }
};


CsiMouseEvents.prototype.onMouseDown = function (evt)
{
   var mouseX = evt.pageX;
   var mouseY = evt.pageY;
   var hit_comp = CsiMouseEvents.hit_test(mouseX, mouseY);
   var canvas_offset;
   
   this.mouseDownPos = new Point(mouseX, mouseY); //store the click pos
   if(evt.button === Enum.BUTTON.LEFT)
   {
      this.leftMouseDownComp = hit_comp;
      if(hit_comp)
      {
         canvas_offset = hit_comp.owner.get_canvas_offset();
         if(typeof hit_comp.OnLButtonDown === "function")
            hit_comp.OnLButtonDown(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
      }
   }
   else if(evt.button === Enum.BUTTON.RIGHT)
   {
      this.rightMouseDownComp = hit_comp;
      if(hit_comp)
      {
         canvas_offset = hit_comp.owner.get_canvas_offset();
         if(typeof hit_comp.OnRButtonDown === "function")
         {
            hit_comp.OnRButtonDown(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
            evt.preventDefault();
            return false; //Stop the right click event from moving to page
         }
      }
   }
   return hit_comp !== null;
};


CsiMouseEvents.prototype.onMouseUp = function (evt)
{
   var result = true;
   var mouseX = evt.pageX;
   var mouseY = evt.pageY;
   var hit_comp = CsiMouseEvents.hit_test(mouseX, mouseY);
   var canvas_offset;
   
   if(evt.button === Enum.BUTTON.LEFT)
   {
      if(hit_comp)
      {
         //LButtonUp
         canvas_offset = hit_comp.owner.get_canvas_offset();
         if(typeof hit_comp.OnLButtonUp === "function")
            hit_comp.OnLButtonUp(mouseX - canvas_offset.x, mouseY - canvas_offset.y);

         //LButtonClick
         if(hit_comp === this.leftMouseDownComp &&
            this.mouseDownPos.x === mouseX &&
            this.mouseDownPos.y === mouseY)
         {
            if(typeof hit_comp.OnLButtonClick === "function")
               hit_comp.OnLButtonClick(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
         }
      }

      if(this.dragComp)
      {
         if(typeof this.dragComp.OnMouseDragEnd === "function")
         {
            canvas_offset = this.dragComp.owner.get_canvas_offset();
            this.dragComp.OnMouseDragEnd(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
         }
      }

      if(this.leftMouseDownComp)
      {
         if(typeof this.leftMouseDownComp.OnMouseRelease === "function")
         {
            canvas_offset = this.leftMouseDownComp.owner.get_canvas_offset();
            this.leftMouseDownComp.OnMouseRelease(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
         }
      }

      this.leftMouseDownComp = null;
      this.dragComp = null;
   }
   else if(evt.button === Enum.BUTTON.RIGHT)
   {
      if(hit_comp)
      {
         canvas_offset = hit_comp.owner.get_canvas_offset();
         if(typeof hit_comp.OnRButtonUp === "function")
         {
            hit_comp.OnRButtonUp(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
            result = false; //Stop the right click event from moving to page
         }

         if(hit_comp === this.rightMouseDownComp)
         {
            if(typeof hit_comp.OnRButtonClick === "function")
            {
               if(this.mouseDownPos.x === mouseX - canvas_offset.x && this.mouseDownPos.y === mouseY - canvas_offset.y)
                  hit_comp.OnRButtonClick(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
            }
         }
      }

      this.rightMouseDownComp = null;
   }

   return result;
};


CsiMouseEvents.prototype.onMouseMove = function (evt)
{
   var mouseX = evt.pageX;
   var mouseY = evt.pageY;
   var hit_comp = CsiMouseEvents.hit_test(mouseX, mouseY);
   var canvas_offset;

   //mouseExit
   if((this.mouseOverComp) && (this.mouseOverComp !== hit_comp))
   {
      //allow selection in document after leaving component
      if(typeof this.mouseOverComp.OnMouseExit === "function")
         this.mouseOverComp.OnMouseExit();
      document.onselectstart = function () { return true; };
   }

   if(hit_comp)
   {
      //MouseEnter
      canvas_offset = hit_comp.owner.get_canvas_offset();
      if(hit_comp !== this.mouseOverComp)
      {
         if(typeof hit_comp.OnMouseEnter === "function")
            hit_comp.OnMouseEnter(mouseX - canvas_offset.x, mouseY - canvas_offset.y);

         //do not show the select cursor when dragging from a dragable component
         if(!hit_comp.showSelectCursor)
            document.onselectstart = function () { return false; };
      }

      //MouseMove
      if(typeof hit_comp.OnMouseMove === "function")
         hit_comp.OnMouseMove(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
   }

   //left drag
   if(this.leftMouseDownComp)
   {
      this.dragComp = this.leftMouseDownComp;
      canvas_offset = this.dragComp.owner.get_canvas_offset();
      if(typeof this.dragComp.OnMouseDrag === "function")
         this.dragComp.OnMouseDrag(mouseX - canvas_offset.x, mouseY - canvas_offset.y);
   }
   this.mouseOverComp = hit_comp;
   return true;
};


CsiMouseEvents.prototype.onMouseOut = function (evt)
{
   //mouseExit
   var mouseX = evt.pageX;
   var mouseY = evt.pageY;
   var hit_comp = CsiMouseEvents.hit_test(mouseX, mouseY);
   if((this.mouseOverComp) && (this.mouseOverComp !== hit_comp))
   {
      //allow selection in document after leaving component
      if(typeof this.mouseOverComp.OnMouseExit === "function")
         this.mouseOverComp.OnMouseExit();
      document.onselectstart = function () { return true; };
   }

   this.mouseDownPos = null; //Track this position to make sure the mouseUp event triggers a click only if needed
   this.leftMouseDownComp = null;  //left mouseDown occurred on component
   this.rightMouseDownComp = null; //right mouseDown occurred on component
   this.mouseOverComp = null; //last component which the mouse moved over
   this.dragComp = null; //component being dragged with left button
   this.lastTouchPos = null; //Track the last touch position
};


CsiMouseEvents.prototype.register_gesture = function (gesture, priority)
{
   if(arguments.length < 2)
   {
      priority = 10;
   }
   gesture.priority = priority;
   this.gestures.push(gesture);
   this.gestures.sort(function (first, second) { return first.priority - second.priority; });
};


CsiMouseEvents.prototype.release_gesture = function (gesture)
{
   var gesture_index = this.gestures.indexOf(gesture);
   if(gesture_index >= 0)
   {
      this.gestures.splice(gesture_index, 1);
   }
};


CsiComponent.prototype.page_rect = function ()
{
   return this.translate_page(
      new Rect(this.left, this.top, this.width, this.height));
};


CsiComponent.prototype.translate_page = function (arg)
{
   var rtn;
   var canvas_offset = this.owner.get_canvas_offset();
   if(arg instanceof Rect)
   {
      rtn = new Rect(
         arg.left + canvas_offset.x,
         arg.top + canvas_offset.y,
         arg.width,
         arg.height);
   }
   else if(arg instanceof Point)
      rtn = new Point(arg.x + canvas_offset, arg.y + canvas_offset);
   return rtn;
};


CsiComponent.prototype.translate_canvas = function (arg)
{
   var rtn;
   var canvas_offset = owner.get_canvas_offset(this);
   if(arg instanceof Rect)
   {
      rtn = new Rect(
         arg.left - canvas_offset.x,
         arg.top - canvas_offset.y,
         arg.width,
         arg.height);
   }
   else if(arg instanceof Point)
      rtn = new Point(arg.x - canvas_offset.x, arg.y - canvas_offset.y);
   return rtn;
};




