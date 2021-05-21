/* CsiUtility.js

Copyright (C) 2010, 2019 Campbell Scientific, Inc.

Written by: Jon Trauntvein
Date Begun: Thursday 12 August 2010
Last Change: Friday 01 March 2019
Last Commit: $Date: 2020-07-14 15:47:08 -0600 (Tue, 14 Jul 2020) $
Last Changed by: $Author: jbritt $

*/


function csi_log(msg)
{
   if(typeof console === "object")
   {
      console.log(msg);
   }
}


function addToStyle (value)
{
   getStyleTextNode().nodeValue += "\n" + value;
}


function getStyleTextNode ()
{
   //get styleElement
   var styleElement;
   var styleElementSelector = $('style');
   if(styleElementSelector.length === 0)
   {
      styleElement = document.createElement("style");
      $("head").append(styleElement);
      styleElement.setAttribute("type", "text/css");
   }
   else
   {
      styleElement = styleElementSelector.get(0);
   }

   //get styleTextNode from styleElement
   var styleTextNode = null;
   if(!styleElement.childNodes || (styleElement.childNodes.length === 0)) 
   {
      styleTextNode = document.createTextNode("");
      styleElement.appendChild(styleTextNode);
   }
   else
   {
      styleTextNode = styleElement.firstChild;
   }

   return styleTextNode;
}


//floating point mod: x mod y
//25.5 mod 10 = 5.5
function fmod(x, y)
{
   //25.5 - (floor(25.5/10) * 10)
   //25.5 - (2 * 10) = 5.5
   return x - (Math.floor(x / y) * y);
}


////////////////////////////////////////////////////////////
// class Point
////////////////////////////////////////////////////////////
function Point()
{
   if(arguments.length === 2)
   {
      this.x = Number(arguments[0]); 
      this.y = Number(arguments[1]); 
   }
   else if(arguments.length === 1)
   {
      var arg = arguments[0]; 
      if(arg instanceof Point)
      {
         this.x = arg.x;
         this.y = arg.y;
      }
   }
}


Point.prototype.adjustForLines = function ()
{
   this.x = Math.floor(this.x) + 0.5;
   this.y = Math.floor(this.y) + 0.5;
};


Point.prototype.adjustForFill = function ()
{
   this.x = Math.floor(this.x);
   this.y = Math.floor(this.y);
};


Point.prototype.offset = function(dx, dy)
{
   this.x += dx;
   this.y += dy;
};


////////////////////////////////////////////////////////////
// function distance
//
// Calculates the distance between two points 
////////////////////////////////////////////////////////////
Point.distance = function(p1, p2)
{
   var dx = p1.x - p2.x;
   var dy = p1.y - p2.y;
   return Math.sqrt(dx*dx + dy*dy);
};


////////////////////////////////////////////////////////////
// class Rect
////////////////////////////////////////////////////////////
function Rect()
{
   var initialised = false;
   if(arguments.length === 4)
   {
      this.left = Number(arguments[0]); 
      this.top = Number(arguments[1]); 
      this.width = Number(arguments[2]); 
      this.height = Number(arguments[3]); 
      this.right = this.left + this.width;
      this.bottom = this.top + this.height;
      initialised = true;
   }
   else if(arguments.length === 1)
   {
      var arg0 = arguments[0]; 
      if(arg0 instanceof Rect)
      {
         this.left = arg0.left;
         this.top = arg0.top;
         this.width = arg0.width;
         this.height = arg0.height;
         this.right = arg0.right;
         this.bottom = arg0.bottom;
         initialised = true;
      }
   }
   if(!initialised)
   {
      this.left = this.top = this.width = this.height = 0;
      this.right = this.bottom = 0;
   }
}


Rect.prototype.set_top = function (top)
{
   this.top = top;
   this.bottom = this.top + this.height;
};


Rect.prototype.get_top = function()
{ return this.top; };


Rect.prototype.set_bottom = function (bottom)
{
   this.bottom = bottom;
   this.top = this.bottom - this.height;
};


Rect.prototype.get_bottom = function()
{
   if(this.bottom != this.top + this.height)
      this.updateBottom();
   return this.bottom;
};


Rect.prototype.set_width = function (width)
{
   this.width = width;
   this.right = this.left + width;
};


Rect.prototype.get_width = function()
{ return this.width; };


Rect.prototype.set_height = function (height)
{
   this.height = height;
   this.bottom = this.top + height;
};


Rect.prototype.get_height = function()
{ return this.height; };


Rect.prototype.set_left = function (left)
{
   this.left = left;
   this.right = left + this.width;
};


Rect.prototype.get_left = function()
{ return this.left; };


Rect.prototype.set_right = function (right)
{
   this.right = right;
   this.left = right - this.width;
};


Rect.prototype.get_right = function()
{
   if(this.right != this.left + this.width)
      this.right = this.left + this.width;
   return this.right;
};


Rect.prototype.updateRight = function ()
{
   this.right = this.left + this.width;
};


Rect.prototype.updateBottom = function ()
{
   this.bottom = this.top + this.height;
};


Rect.prototype.updateHeight = function ()
{
   this.height = this.bottom - this.top;
   if(this.height < 0)
   {
      this.height = 0;
      this.top = this.bottom;
   }
};


Rect.prototype.updateWidth = function ()
{
   this.width = this.right - this.left;
   if(this.width < 0)
   {
      this.width = 0;
      this.left = this.right;
   }
};


Rect.prototype.adjustForFill = function ()
{
   this.left = Math.floor(this.left);
   this.top = Math.floor(this.top);
   this.right = Math.floor(this.right);
   this.bottom = Math.floor(this.bottom);
   this.width = this.right - this.left;
   this.height = this.bottom - this.top;
};


Rect.prototype.adjustForLines = function ()
{
   this.left = Math.floor(this.left) + 0.5;
   this.top = Math.floor(this.top) + 0.5;
   this.right = Math.floor(this.right) + 0.5;
   this.bottom = Math.floor(this.bottom) + 0.5;
   this.width = this.right - this.left;
   this.height = this.bottom - this.top;
};


Rect.prototype.contains = function (point)
{
   var rtn = false;
   if(point.x >= this.left && point.x <= this.right && point.y >= this.top && point.y <= this.bottom)
   {
      rtn = true;
   }
   return rtn;
};


Rect.prototype.deflate = function (dx, dy)
{ this.inflate(-dx, -dy); };


Rect.prototype.inflate = function (dx, dy)
{
   if(-2 * dx > this.width)
   {
      // don't allow to deflate to eat more width than available
      this.left += this.width / 2;
      this.width = 0;
   }
   else
   {
      this.left -= dx;
      this.width += 2 * dx;
   }
   if(-2 * dy > this.height)
   {
      this.top += this.height / 2;
      this.height = 0;
   }
   else
   {
      this.top -= dy;
      this.height += 2 * dy;
   }
   this.updateRight();
   this.updateBottom();
};


Rect.prototype.intersect = function (other)
{
   var x2 = this.right;
   var y2 = this.bottom;
   if(this.left < other.left)
   {
      this.left = other.left;
   }
   if(this.top < other.top)
   {
      this.y = other.y;
   }
   if(x2 > other.right)
   {
      x2 = other.right;
   }
   if(y2 > other.bottom)
   {
      y2 = other.bottom;
   }
   this.width = x2 - this.left;
   this.height = y2 - this.top;
   if(this.width <= 0 || this.height <= 0)
   {
      this.width = this.height = 0.0;
   }
   this.updateRight();
   this.updateBottom();
   return this;
};


Rect.prototype.is_empty = function ()
{ return this.width <= 0 || this.height <= 0; };


Rect.prototype.offset = function (dx, dy)
{
   this.left += dx;
   this.top += dy;
   this.updateRight();
   this.updateBottom();
};


Rect.prototype.move = function (x, y)
{
   this.left = x;
   this.top = y;
   this.updateRight();
   this.updateBottom();
};


Rect.prototype.union = function (other)
{
   if(this.is_empty())
   {
      this.top = other.top;
      this.left = other.left;
      this.width = other.width;
      this.height = other.height;
      this.right = other.right;
      this.bottom = other.bottom;
   }
   else if(!other.is_empty())
   {
      var x1 = Math.min(this.left, other.left);
      var y1 = Math.min(this.top, other.top);
      var x2 = Math.max(this.right, other.right);
      var y2 = Math.max(this.bottom, other.bottom);

      this.left = x1;
      this.top = y1;
      this.width = x2 - x1;
      this.height = y2 - y1;
      this.right = this.left + this.width;
      this.bottom = this.top + this.height;
   }
   return this;
};


Rect.prototype.rotate = function (degrees)
{
   if(degrees === 90 || degrees === 270)
   {
      var temp = this.width;
      this.width = this.height;
      this.height = temp;
      this.updateRight();
      this.updateBottom();
   }
   else if(degrees !== 0 && degrees !== 180)
   {
      var theta = degreesToRadians(degrees);
      var old_width = this.width;
      var old_height = this.height;
      this.width = Math.abs(old_width * Math.cos(theta)) + Math.abs(old_height * Math.cos(Math.PI / 2 - theta));
      this.height = Math.abs(old_width * Math.sin(theta)) + Math.abs(old_height * Math.sin(Math.PI / 2 - theta));
      this.right = this.left + this.width;
      this.bottom = this.top + this.height;
   }
};


Rect.prototype.center = function (centre_x, centre_y)
{
   this.left = centre_x - this.width / 2;
   this.top = centre_y - this.height / 2;
   this.right = this.left + this.width;
   this.bottom = this.top + this.height;
};


Rect.prototype.center_x = function (center_x)
{
   this.left = center_x - this.width / 2;
   this.right = this.left + this.width;
};


Rect.prototype.center_y = function (center_y)
{
   this.top = center_y - this.height / 2;
   this.bottom = this.top + this.height;
};


Rect.prototype.get_center = function ()
{ return new Point(this.left + this.width / 2, this.top + this.height / 2); };


Rect.prototype.get_top_left = function ()
{ return new Point(this.left, this.top); };


Rect.prototype.get_bottom_left = function ()
{ return new Point(this.left, this.bottom); };


Rect.prototype.get_top_right = function ()
{ return new Point(this.right, this.top); };


Rect.prototype.get_bottom_right = function ()
{ return new Point(this.right, this.bottom); };


Rect.prototype.set_drag_point = function (drag_x, drag_y, org_x, org_y)
{
   if(drag_x < org_x)
   {
      this.left = drag_x;
      this.width = org_x - drag_x;
      this.right = org_x;
   }
   else
   {
      this.right = drag_x;
      this.width = drag_x - org_x;
      this.left = org_x;
   }
   if(drag_y < org_y)
   {
      this.top = drag_y;
      this.height = org_y - drag_y;
      this.bottom = org_y;
   }
   else
   {
      this.bottom = drag_y;
      this.height = drag_y - org_y;
      this.top = org_y;
   }
};


////////////////////////////////////////////////////////////
// line_intersect
//
// Calculates the points at which the line specified by the specified
// points will intersect this rectangle.  If both points lie within the
// rectangle, they will be copied in the return object.  If the line does not
// intersect, a null reference will be returned.
////////////////////////////////////////////////////////////
Rect.prototype.line_intersect = function(p1, p2)
{
   var rtn = { "p1":p1, "p2":p2 };
   if(!this.contains(p1) || !this.contains(p2))
   {
      // we need to calculate all possible intersects with this rectangle
      var intersects = [];
      if(p1.x === p2.x)
      {
         intersects.push(new Point(p1.x, this.top));
         intersects.push(new Point(p1.x, this.bottom));
      }
      else if(p1.y === p2.y)
      {
         intersects.push(new Point(this.left, p1.y));
         intersects.push(new Point(this.right, p1.y));
      }
      else
      {
         // we need to calculate the slope intercept form of the connecting line
         var m = (p1.y - p2.y) / (p1.x - p2.x);
         var b = p1.y - m*p1.x;

         // we will now apply the line equation to calculate the intercept for each side of the rectangle
         var top_intersect = new Point((this.top - b)/m, this.top);
         var bottom_intersect = new Point((this.bottom - b)/m, this.bottom);
         var left_intersect = new Point(this.left, m*this.left + b);
         var right_intersect = new Point(this.right, m*this.right + b);
         
         if(this.contains(top_intersect))
         {
            intersects.push(top_intersect);
         }
         if(this.contains(bottom_intersect))
         {
            intersects.push(bottom_intersect);
         }
         if(this.contains(left_intersect))
         {
            intersects.push(left_intersect);
         }
         if(this.contains(right_intersect))
         {
            intersects.push(right_intersect);
         }
      }

      // we will now choose the coordinates that are closest to the specified points
      if(intersects.length > 0)
      {
         var closest_distance = 1E38;
         var len = intersects.length;
         var candidate;
         var candidate_distance;
         var i;
         
         if(!this.contains(p1))
         {
            for(i = 0; i < len; ++i)
            {
               candidate = intersects[i];
               candidate_distance = Point.distance(candidate, p1);
               if(candidate_distance < closest_distance)
               {
                  rtn.p1 = candidate;
                  closest_distance = candidate_distance;
               }
            }
         }
         if(!this.contains(p2))
         {
            closest_distance = 1E38;
            for(i = 0; i < len; ++i)
            {
               candidate = intersects[i];
               candidate_distance = Point.distance(candidate, p2);
               if(candidate_distance  < closest_distance)
               {
                  rtn.p2 = candidate;
                  closest_distance = candidate_distance;
               }
            }
         }
      }
      else
      {
         rtn = null;
      }
   }
   return rtn;
};


function degreesToRadians(degrees)
{
   return degrees * Math.PI / 180;
}


function radiansToDegrees(radians)
{
   return (radians * 180) / Math.PI;
}


function getUrlVars()
{
   var vars = [], hash;
   var hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');
   var i;
   for(i = 0; i < hashes.length; i++)
   {
      hash = hashes[i].split('=');
      vars.push(hash[0]);
      vars[hash[0]] = hash[1];
   }
   return vars;
}

// pass a canvas context and a string
function getTextHeight(context)
{
   return context.measureText("W").width * 1.5;
//   var body = document.getElementsByTagName("body")[0];
//   var tempDiv = document.createElement("div");
//   var divText = document.createTextNode(text);
//   tempDiv.appendChild(divText);
//   tempDiv.setAttribute("style", "font-family: " + context.font.split('px ')[1] + "; font-size: " + parseInt(context.font.match(/\d+px/)) + "px");
//   body.appendChild(tempDiv);
//   var ret = tempDiv.offsetHeight;
//   body.removeChild(tempDiv);
//   return ret;
}



function measureText(context, text, textDecorations = 0)
{
   var result = context.measureText(text);
   if(!result.height) 
   {
      if (context.font !== "Arial")
      {
         let origFont = context.font;
         context.font = setFontFamily(origFont, "Arial");
         result.height = context.measureText("W").width * 1.5; //assume height("W") == width("W") * 1.5;
         context.font = origFont;
      }
      else
      {
         result.height = context.measureText("W").width * 1.5; //assume height("W") == width("W") * 1.5;
      }
   }

   if (textDecorations & 0x1)
   {
      result.height = Math.floor(result.height * 1.02 + 0.5);
   }

   return result;
}

function drawTextWithDecorations(context, textStr, textLoc_x, textLoc_y, textSize, textDecorations = 0)
{

   // use fillrect, not lineto, because of strokestyle vs. fillstyle.  color is in fillstyle.
   // offset y by baseLine.. should be either 1/2 textSize.h if context.textBaseline === midle , 
   context.fillText(textStr, textLoc_x, textLoc_y);

   if (textDecorations !== null && textDecorations !== 0)
   {
      if (textSize === null) {
         textSize = measureText(context, textStr);
         textSize.height = textSize.height * 0.65; //Necessary for the correct positioning of strikethrough and underline
      }

      var x = textLoc_x;
      var y = textLoc_y;

      if (context.textBaseline == "middle")
         y -= textSize.height / 2;
      else if (context.textBaseline == "bottom")
         y -= textSize.height;

      if (context.textAlign === "center")
         x -= textSize.width / 2;
      else if (context.textAlign === "right")
         x -= textSize.width;

      if (textDecorations & 0x1)
      {
         //context.fillRect(x, y,                   textSize.width, 2);
         //context.fillRect(x, y + textSize.height, textSize.width, 2);
         context.lineWidth = 1;
         context.fillRect(x, y + textSize.height, textSize.width, 1 + textSize.height * 0.02);
      }

      if (textDecorations & 0x2)
      {
         context.lineWidth = 1;
         context.fillRect(x, y + textSize.height/2, textSize.width, 2);
      }
   }
      
}


// Given a shorthand css font, returns a dictionary with each of
// the font property attributes
function parseCSSFont(font)
{
   var $font = $('<span />');
   $font.css('font', font);
   var fontDict = {};
   fontDict['font-style'] = $font.css('fontStyle');
   fontDict['font-variant'] = $font.css('fontVariant');
   fontDict['font-weight'] = $font.css('fontWeight');
   fontDict['font-size'] = $font.css('fontSize');
   fontDict['line-height'] = $font.css('lineHeight');
   fontDict['font-family'] = $font.css('fontFamily');
   return fontDict;
}


// Convert a dictionary containing all css font properties into a
// shorthand css font. To be used with parseCSSFont. Do not use unless
// all six properties are defined.
function expandFontDict(fontDict)
{
   if (
      fontDict['font-style'] === undefined ||
      fontDict['font-variant'] === undefined ||
      fontDict['font-weight'] === undefined ||
      fontDict['font-size'] === undefined ||
      fontDict['line-height'] === undefined ||
      fontDict['font-family'] === undefined
   )
   {
      csi_log('WARNING: Undefined values found while trying to convert ' +
         'dictionary to CSS shorthand font.');
      return '10pt Arial';
   }
   var ret = '';
   ret += fontDict['font-style'] + ' ';
   ret += fontDict['font-variant'] + ' ';
   ret += fontDict['font-weight'] + ' ';
   ret += fontDict['font-size'] + '/' + fontDict['line-height'] + ' ';
   ret += fontDict['font-family'];
   return ret;
}


// Returns the maximum font pt that will fit within the provided width and height
// Use measureVertically = true if text will be displayed vertically
function getMaxFontPt(context, font, str, width, height, margin, measureVertically = false)
{
   context.save();

   var fontDict = parseCSSFont(font);

   fontDict['font-size'] = '10pt';
   context.font = expandFontDict(fontDict);
   var dims1 = measureVertically ?
      {
         width: context.measureText('W').width,
         height: context.measureText('W').width * str.length * 1.5
      } :
      measureText(context, str);

   fontDict['font-size'] = '11pt';
   context.font = expandFontDict(fontDict);
   var dims2 = measureVertically ?
      {
         width: context.measureText('W').width,
         height: context.measureText('W').width * str.length * 1.5
      } :
      measureText(context, str);

   // dwdx is the amount the width increases when we increase the font pt by 1
   var dwdx = dims2.width - dims1.width;
   // dhdx is the amount the height increases when we increase the font pt by 1
   var dhdx = dims2.height - dims1.height;

   var widthpt = Math.floor((width - 2 * margin) / dwdx);
   var heightpt = Math.floor((height - 2 * margin) / dhdx);

   context.restore();

   return Math.max(Math.min(widthpt, heightpt), 0) + 'pt';
}


// Compare the size of two fonts; returns true if fontA < fontB
function compareFonts(context, str, fontA, fontB)
{
   context.save();
   context.font = fontA;
   var a = measureText(context, str);
   context.font = fontB;
   var b = measureText(context, str);
   context.restore();
   return a.width < b.width; // Fonts should vary only by their font-size property
}

// Returns a css font that is either the original font or a font that has been scaled
// down to fit inside the provided dimensions. Use measureVertically = true if text
// will be displayed vertically
function adjustFontForRect(context, font, str, width, height, margin = 0, measureVertically = false)
{
   var fontDict = parseCSSFont(font);
   var origSize = fontDict['font-size'];
   var maxSize = getMaxFontPt(context, font, str, width, height, margin, measureVertically);
   fontDict['font-size'] = maxSize;
   var fontSize = compareFonts(context, str, font, expandFontDict(fontDict)) ? origSize : maxSize;
   fontDict['font-size'] = fontSize;
   return expandFontDict(fontDict);
}

   // Returns a font's point size
function getFontSize(font)
{
   var fontDict = parseCSSFont(font);
   return parseInt(fontDict['font-size'], 10);
}

function setFontSize(font, size)
{
   var fontDict = parseCSSFont(font);
   fontDict['font-size'] = String(size) + "pt";
   return expandFontDict(fontDict);
}

function setFontFamily(font, name)
{
   var fontDict = parseCSSFont(font);
   fontDict['font-family'] = name;
   return expandFontDict(fontDict);
}

function getLinesAndFont(context, phrase, allowed_width, allowed_height, margin, word_wrap = true)
{
   // Break up the entire string into linefeeds using all possible line endings
   var linefeeds = phrase.split("\r\n");
   if (linefeeds.length === 1)
   {
      linefeeds = phrase.split("\r");
   }

   if (linefeeds.length === 1)
   {
      linefeeds = phrase.split("\n");
   }

   if (!word_wrap) {
      var maxLineWidth = 0;
      var minFont = context.font;
      // Iterate through all lines. Set a new minimal font when we find a line that won't fit
      for (let i = 0; i < linefeeds.length; ++i) {
         if (linefeeds[i].length != 0) {
            minFont = adjustFontForRect(context, minFont, linefeeds[i], allowed_width, allowed_height, margin);
         }
      }
      // We also shrink the font if the lines are too tall. To do this, make a string with linefeeds.length W's
      // and measure it vertically. Shrink only if needed.
      var verticalString = new Array(linefeeds.length + 1).join("W");
      minFont = adjustFontForRect(context, minFont, verticalString, allowed_width, allowed_height, margin, true);
      return { lines: linefeeds, font: minFont };
   } else {
      if (linefeeds.length === 1) {
         linefeeds = getLines(context, linefeeds, allowed_width, margin);
      }
   }

   // Create list of words
   var allWords = [];
   for (let i = 0; i < linefeeds.length; ++i)
   {
      allWords = allWords.concat(linefeeds[i].split(" "));  // word boundaries should consist of any non-letter/non-number character
   }


   // Shrink font size so that the longest word always fits within the rectangle
   for (let i = 0; i < allWords.length; ++i)
   {
      if (allWords[i].length > 0)
         context.font = adjustFontForRect(context, context.font, allWords[i], allowed_width, allowed_height, margin);
   }

   var lines = linefeeds;
   if (context.measureText("W").width * 1.5 * lines.length > allowed_height - 2 * margin) {
      // If the current text at the specified font size doesn't fit then we will
      // perform a binary search in which we repeatedly choose a font size, get the lines
      // associated with that font size, and determine if their summed height will fit within
      // the rectangle height. If they do fit, we advance the binary search pointer.
      // Otherwise, it remains stationary.
      var origFont = context.font;
      var bsPtr = 0;
      var bsIncr = 1024; // Maximal reasonable font size, must be of form 2^n
      for (; bsIncr >= 1; bsIncr /= 2) {
         var fDict = parseCSSFont(context.font);
         fDict['font-size'] = bsPtr + bsIncr + 'pt';
         context.font = expandFontDict(fDict);
         if (compareFonts(context, "W", context.font, origFont)) {
            var tempLines = getLines(context, linefeeds, allowed_width, margin);
            if (context.measureText("W").width * 1.5 * tempLines.length < allowed_height - 2 * margin) {
               bsPtr += bsIncr;
               lines = tempLines;
            }
         }
      }
   }

   return { lines: lines, font: context.font };
}


function getLines(context, linefeeds, allowed_width, margin)
{
   var rtn = [];
   var linefeed_count = linefeeds.length;
   var i, j;
   for (i = 0; i < linefeed_count; i++)
   {
      var cur_phrase = "";
      var measure = 0;
      var cur_line = linefeeds[i];
      var words = cur_line.split(" "); //Break line feeds up into words one line at a time
      var word_count = words.length;
      if (word_count > 1)
      {
         for (j = 0; j < word_count; j++)
         {
            //Add the next word onto the phrase and see if it fits
            var test_phrase = cur_phrase;
            if (test_phrase.length)
            {
               test_phrase += " ";
            }
            test_phrase += words[j];
            measure = context.measureText(test_phrase).width;
            if (measure < allowed_width - 2 * margin) //It fits
            {
               cur_phrase = test_phrase;
            }
            else //Didn't fit, so end the line we had and start a new one
            {
               rtn.push(cur_phrase);
               cur_phrase = words[j];
            }
         }

         //Add the last phrase to the rtn
         rtn.push(cur_phrase);
      }
      else //Only one word, so just push it on the rtn array
      {
         rtn.push(words[0]);
      }
   }
   return rtn;
}



function scaleContextForEllipse(context, rect)
{
   if(rect.width > rect.height)
   {
      context.scale(rect.width / rect.height, 1);
   }
   else
   {
      context.scale(1, rect.height / rect.width);
   }
}


function draw_circle(context, rect)
{
   var radius = Math.min(rect.width / 2, rect.height / 2);
   context.beginPath();
   context.arc(0, 0, radius, 0, 2 * Math.PI, false);
}


function draw_ellipse(context, rect)
{
   context.save();
   context.translate(rect.left + rect.width / 2, rect.top + rect.height / 2);
   scaleContextForEllipse(context, rect);
   draw_circle(context, rect);
   context.restore();
}


function drawDashedLine(context, x_, y_, x2_, y2_, dashArray, previous)
{
   // we will assign our own coordinates based on the parameters.  The
   // dashing algorithm also assumes that x increases from x to x2.
   // In order to accomodate this, we may have to reverse the points.
   var x, y, x2, y2;
   if(x2_ > x_)
   {
      x = Number(x_);
      y = Number(y_);
      x2 = Number(x2_);
      y2 = Number(y2_);
   }
   else
   {
      x = Number(x2_);
      y = Number(y2_);
      x2 = Number(x_);
      y2 = Number(y_);
   }

   // we can now draw the dashes
   var dashCount = dashArray.length;
   context.moveTo(x, y);
   var dx = (x2 - x);
   var dy = (y2 - y);
   var slope = dy / dx;
   var distRemaining = Math.sqrt(dx * dx + dy * dy);
   var dashIndex = 0;
   var draw = true;

   if(slope === Infinity)
   {
      slope = 1E38;
   }
   else if(slope === -Infinity)
   {
      slope = -1E38;
   }

   if(!previous || previous.length === 0) 
   {
      previous = [0, 0];
   }
   dashIndex = previous[1];
   while(distRemaining >= 0.1)
   {
      draw = dashIndex % 2 === 0;
      var dashLength;

      if(previous[0] === 0)
      {
         dashLength = dashArray[dashIndex % dashCount];
         if(!draw)
         {
            dashLength += context.lineWidth;
         }
      }
      else
      {
         dashLength = previous[0];
      }

      if(dashLength > distRemaining)
      {
         previous[0] = dashLength - distRemaining;
         dashLength = distRemaining;
      }
      else
      {
         ++dashIndex;
         previous[0] = 0;
      }
      var xStep = Math.sqrt(dashLength * dashLength / (1 + slope * slope));
      x += xStep;
      y += slope * xStep;

      context[draw ? 'lineTo' : 'moveTo'](x, y);
      distRemaining -= dashLength;
   }
   previous[1] = dashIndex;
}

function getGradient(context, rect, mode, color1, colorMid, color2, bUseMid) 
{
   var x0, y0, x1, y1;
   var r0, r1;
   switch(mode)
   {
      case Enum.GRADIENT.LinearGradientModeHorizontal:
         x0 = rect.left;
         x1 = rect.left + rect.width;
         y0 = y1 = 0;
         break;
      case Enum.GRADIENT.LinearGradientModeVertical:
         y0 = rect.top;
         y1 = rect.top + rect.height;
         x0 = x1 = 0;
         break;
      case Enum.GRADIENT.LinearGradientModeForwardDiagonal:
         x0 = rect.left;
         y0 = rect.top;
         x1 = rect.left + rect.width;
         y1 = rect.top + rect.height;
         break;
      case Enum.GRADIENT.LinearGradientModeBackwardDiagonal:
         x0 = rect.left + rect.width;
         y0 = rect.top + rect.height;
         x1 = rect.left;
         y1 = rect.top;
         break;
      case Enum.GRADIENT.Radial:
         x0 = rect.left + rect.width / 2;
         y0 = rect.top  + rect.height / 2;
         r0 = 0;
         r1 = rect.width / 2;
   }

   var gradient = mode == Enum.GRADIENT.Radial ?
      context.createRadialGradient(x0, y0, r0, x0, y0, r1) :
      context.createLinearGradient(x0, y0, x1, y1);

   gradient.addColorStop(0, color1);
   if (bUseMid) gradient.addColorStop(0.5, colorMid);
   gradient.addColorStop(1, color2);

   return gradient;
}



//Enum
var Enum = {}; //Global Enum Array
Enum.BUTTON =
{
   LEFT: 0,
   MIDDLE: 1,
   RIGHT: 2
};
Enum.LOGICAL_OPERAND =
   {
      NONE: 0,
      AND: 1,
      OR: 2
   };
Enum.COMPARATOR =
{
   GREATER_THAN: 0,
   LESS_THAN: 1,
   GREATER_THAN_EQUAL: 2,
   LESS_THAN_EQUAL: 3,
   EQUAL: 4,
   NOT_EQUAL: 5
};


//component enumerations
Enum.BORDER_STYLE =
{
   NONE: 0,
   RAISED: 1,
   LOWERED: 2,
   SINGLE: 3,
   RAISED_BEVEL: 4,
   LOWERED_BEVEL: 5
};

Enum.BACKGROUND_STYLE =
{
   USE_SOLID_COLOR: 0,
   USE_GRADIENT:    1,
   USE_TRANSPARENT: 2
};

Enum.GRADIENT = 
{
   LinearGradientModeHorizontal        : 0,
   LinearGradientModeVertical          : 1,
   LinearGradientModeForwardDiagonal   : 2,
   LinearGradientModeBackwardDiagonal  : 3,
   Radial                              : 4
};

Enum.ORIENTATION =
{
   BOTTOM_TO_TOP: 0,
   TOP_TO_BOTTOM: 1,
   LEFT_TO_RIGHT: 2,
   RIGHT_TO_LEFT: 3
};
Enum.ALIGNMENT =
{
   LEFT: 0,
   CENTER: 1,
   RIGHT: 2
};

Enum.LINE_TYPE =
{
   SOLID: 0,
   DASH: 1,
   DOT: 2,
   DASHDOT: 3,
   DASHDOTDOT: 4,
   CLEAR: 5,
   IGNORE: 6,
   SMALLDOTS: 7
};



////////////////////////////////////////////////////////////
// Csi namespace
//
// Provides namespace on which to hang various functions
////////////////////////////////////////////////////////////
function Csi()
{ }


/**
 * @return Returns an array where each element is a character from the string.
 *
 * @param s Specifies the string to split.
 */
Csi.string_to_array = function(s)
{ return s.split(""); };


Csi.dash_pattern = [17, 5];
Csi.small_dot_pattern = [2, 2];
Csi.dot_pattern = [3, 3];
Csi.dash_dot_pattern = [8, 5, 2, 5];
Csi.dash_dot_dot_pattern = [8, 2, 2, 2, 2, 2];


Csi.draw_line = function (context, x1_, y1_, x2_, y2_, style)
{
   var x1 = Math.floor(x1_) + 0.5;
   var y1 = Math.floor(y1_) + 0.5;
   var x2 = Math.floor(x2_) + 0.5;
   var y2 = Math.floor(y2_) + 0.5;

   var pattern = null;
   context.beginPath();
   switch(style)
   {
      case Enum.LINE_TYPE.SOLID:
         pattern = null;
         context.moveTo(x1, y1);
         context.lineTo(x2, y2);
         break;

      case Enum.LINE_TYPE.DASH:
         pattern = Csi.dash_pattern;
         break;

      case Enum.LINE_TYPE.DOT:
         pattern = Csi.dot_pattern;
         break;

      case Enum.LINE_TYPE.DASHDOT:
         pattern = Csi.dash_dot_pattern;
         break;

      case Enum.LINE_TYPE.DASHDOTDOT:
         pattern = Csi.dash_dot_dot_pattern;
         break;

      case Enum.LINE_TYPE.CLEAR:
      case Enum.LINE_TYPE.IGNORE:
         pattern = null;
         break;

      //case Enum.LINE_TYPE.SMALLDOTS: 
      default:
         pattern = Csi.small_dot_pattern;
         context.lineWidth = 1;
         break;
   }
   if(pattern) 
   {
      drawDashedLine(context, x1, y1, x2, y2, pattern, [0, 0]);
   }
   context.stroke();
};


/**
 * Performs a binary search on the provided array for a match to the provided value.
 * Assumes that the provided array has already been sorted.
 *
 * @return Returns the index of the value if it was found or -1 if it was not found.
 *
 * @oaram {array} values Specifies the array in which the search fill take place.
 *
 * @param {any} value Specifies the value to search for.
 *
 * @param {number} start_ Specifies the starting position in the array for the search.
 * If specified as null, this value will default to zero.
 *
 * @param {number} end_ Specifies the ending position in the array for the search.
 * If specified as null, this value will default to the index of the last element.
 *
 * @param {function} comparator Specifies a function that will compare an array value with
 * the provided value.  This function expects two arguments that specify the value to be
 * compared and is expected to return an integer that is negative if the first value
 * orders before the second, 0 if the values order the same, and a positive number if the second
 * orders after the first value.
 */
Csi.binary_search = function(values, value, start_, end_, comparator)
{
   var start, middle, end, candidate;
   var rtn = -1;
   var result;
   
   if(start === null || start === undefined)
      start = 0;
   if(end === null || end === undefined)
      end = values.length - 1;
   if(comparator === null || comparator === undefined)
   {
      comparator = function(v1, v2) {
         if(v1 < v2)
            return -1;
         else if(v1 > v2)
            return 1;
         else
            return 0;
      };
   }
   while(start < end && rtn === -1)
   {
      middle = Math.floor(start + (end - start) / 2);
      candidate = values[middle];
      result = comparator(value, candidate);
      if(result > 0)
      {
         if(start !== middle)
            start = middle;
         else
            break;
      }
      else if(result < 0)
      {
         if(end !== middle)
            end = middle;
         else
            break;
      }
      else
         rtn = middle;
   }
   return rtn;
};
