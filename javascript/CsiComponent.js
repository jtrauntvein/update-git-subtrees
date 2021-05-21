/* CsiComponent.js

   Copyright (C) 2010, 2016 Campbell Scientific, Inc.

   Written by: Kevin Westwood
   Date Begun: Thursday 12 August 2010
   Last Change: Friday 22 April 2016
   Last Commit: $Date: 2020-07-14 15:47:08 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

/* global CsiColour: true */
/* global getGradient: true */


////////////////////////////////////////////////////////////
// class CsiComponent
//
// Defines a base class for all visual components
////////////////////////////////////////////////////////////
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

   this.bHasBackground = false;
   this.bUseClip = false;
   this.backgroundMargin = 0;
   this.eBackgroundBorderStyle = Enum.BORDER_STYLE.NONE;

   this.eBackgroundColorStyle = Enum.BACKGROUND_STYLE.USE_GRADIENT;
   this.eBackgroundGradientDirection = Enum.GRADIENT.LinearGradientModeForwardDiagonal;
   this.backgroundGradientStartColor = "white";
   this.backgroundGradientMidColor   = "white";
   this.backgroundGradientEndColor   = "silver";
   this.backgroundGradientUseMid     = false;

   this.bBackgroundRoundedCorners = true;
   this.backgroundRoundedRadius = 5;
   this.backgroundSolidColor = "Silver";

   this.tipX = this.left + this.width + 10;
   this.tipY = this.top + 40;

   this.RotationAngle = 0;

   this.expression = null;

   this.valueSetter = null;
   this.isAlarm = false;
}

//must be implemented by each component
//draw(context) //called to draw component

//implement to support animation
//updateAnimation(), component should update animation values (only called if animating = true)

//implement to support data
//newValue(value, timestamp, expectMore)
//newNanValue(value, timestamp, expectMore)
//newRecord(jsonRecord, timestamp, expectMore)

//Optional for mouse events - just implement in needed class and set
//needs_mouse_events = true;
//
//CsiComponent.prototype.OnLButtonDblClk = function(mouseX, mouseY)
//CsiComponent.prototype.OnLButtonDown = function(mouseX, mouseY)
//CsiComponent.prototype.OnLButtonUp = function(mouseX, mouseY)
//CsiComponent.prototype.OnRButtonDown = function(mouseX, mouseY)
//CsiComponent.prototype.OnRButtonUp = function(mouseX, mouseY)

//Implement to handle when parent tab is initially shown
//CsiComponent.prototype.deactivate = function()
//CsiComponent.prototype.activate = function()

//When invalidate is called notify the CsiGraphicsManager
CsiComponent.prototype.invalidate = function ()
{
   this.valid = false;
   graphicsManager.componentInvalidate(this);
};


CsiComponent.prototype.refresh = function ()
{
   this.valid = false;
   graphicsManager.componentRefresh(this);
};


CsiComponent.prototype.getAnimating = function ()
{
   return this.animating;
};

/******************************************************************************
* drawBackground
* 
* Draw the background that appears behind each component.  It can be transparent,
* or use a solid color, or a gradient.  This will also draw a border around it.
*
******************************************************************************/
CsiComponent.prototype.drawBackground = function (context, rect)
{
   // Return if background is not to be drawn
   if (!this.bHasBackground)
   {
      return rect;
   }

   var rtnRect = new Rect(rect.left, rect.top, rect.width, rect.height);

   var borderWidth = this.getBorderWidth(this.eBackgroundBorderStyle);
   var fillZone = new Rect(rtnRect.left + borderWidth / 2, rtnRect.top + borderWidth / 2,
      rtnRect.width - borderWidth, rtnRect.height - borderWidth);

   context.save();

   switch (this.eBackgroundColorStyle)
   {
      case Enum.BACKGROUND_STYLE.USE_SOLID_COLOR:
         context.fillStyle = this.backgroundSolidColor;
         break;

      case Enum.BACKGROUND_STYLE.USE_GRADIENT:
         // For a radial gradient, we must use a transform to obtain an ellipse.
         if (this.eBackgroundGradientDirection === Enum.GRADIENT.Radial)
         {
            if (this.bBackgroundRoundedCorners)
            {
               context.fillStyle = this.backgroundGradientEndColor;
               fillRoundedRect(context, fillZone, this.backgroundRoundedRadius);
            }

            var XYRatio = fillZone.height / fillZone.width;
            if (XYRatio >= 1)
            {
               fillZone.top /= XYRatio;
               fillZone.height /= XYRatio;
               fillZone.updateBottom();
               context.transform(1, 0, 0, XYRatio, 0, 0);
            }
            else
            {
               fillZone.left *= XYRatio;
               fillZone.width *= XYRatio;
               fillZone.updateRight();
               context.transform(1.0/XYRatio, 0, 0, 1, 0, 0);          
            }
         }
         context.fillStyle = getGradient(context, fillZone, this.eBackgroundGradientDirection, this.backgroundGradientStartColor,
            this.backgroundGradientMidColor, this.backgroundGradientEndColor, this.backgroundGradientUseMid);
         break;

      case Enum.BACKGROUND_STYLE.USE_TRANSPARENT:
         break;
   }

   if (this.eBackgroundColorStyle !== Enum.BACKGROUND_STYLE.USE_TRANSPARENT)
   {
      if (this.bBackgroundRoundedCorners)
         fillRoundedRect(context, fillZone, this.backgroundRoundedRadius);
      else
         context.fillRect(fillZone.left, fillZone.top, fillZone.width, fillZone.height);
   }

   context.restore();

   this.drawBorder(context, rect, this.eBackgroundBorderStyle);

   var edgeWidth = borderWidth + this.backgroundMargin;

   rtnRect.deflate(edgeWidth, edgeWidth);

   if (rtnRect.left > rtnRect.right -4)
   {
      var centerX = rect.lef + rect.width / 2;
      rtnRect.left = centerX - 2;
      rtnRect.right = centerX + 2;
   }
   if (rtnRect.top > rtnRect.bottom - 4)
   {
      var centerY = rect.top + rect.height / 2;
      rtnRect.top = centerY - 2;
      rtnRect.bottom = centerY + 2;
   }

   return rtnRect;
};


function adjustColorDarkness(color, shift, lighten)
{
   var c = CsiColour.parse(color);
   return lighten ?
      "rgba(" + (c.red * (1 - shift) + 255 * shift) + "," + (c.green * (1 - shift) + 255 * shift) + "," + (c.blue * (1 - shift) + 255 * shift) + "," + c.alpha + ")" :
      "rgba(" + (c.red * (1 - shift) + 0 * shift)   + "," + (c.green * (1 - shift) + 0 * shift)   + "," + (c.blue * (1 - shift) + 0 * shift)   + "," + c.alpha + ")";
}

function getColorAlpha(color)
{
   var c = CsiColour.parse(color);
   return c.alpha;
}


function setColorAlpha(color, alpha)
{
   var c = CsiColour.parse(color);
   return "rgba(" + c.red + "," + c.green + "," + c.blue + "," + alpha + ")";
}



CsiComponent.prototype.getBorderWidth = function (borderstyle)
{
   switch (borderstyle)
   {
      case Enum.BORDER_STYLE.NONE:
         return 0;

      case Enum.BORDER_STYLE.LOWERED:
      case Enum.BORDER_STYLE.RAISED:
         return 5;

      case Enum.BORDER_STYLE.SINGLE:
      case Enum.BORDER_STYLE.LOWERED_BEVEL:
      case Enum.BORDER_STYLE.RAISED_BEVEL:
         return this.backgroundBorderThickness;

      default:
         csi_log("WARNING: Default case reached in getBorderWidth");
         return 0;
   }
};


function getUpperBevelGeometry(rect, borderThickness)
{
   return [
      { x: rect.left, y: rect.bottom },
      { x: rect.left, y: rect.top },
      { x: rect.right, y: rect.top },
      { x: rect.right - borderThickness, y: rect.top + borderThickness },
      { x: rect.left + borderThickness, y: rect.top + borderThickness },
      { x: rect.left + borderThickness, y: rect.bottom - borderThickness }
   ];
}


function getLowerBevelGeometry(rect, borderThickness)
{
   return [
      { x: rect.right, y: rect.top },
      { x: rect.right, y: rect.bottom },
      { x: rect.left, y: rect.bottom },
      { x: rect.left + borderThickness, y: rect.bottom - borderThickness },
      { x: rect.right - borderThickness, y: rect.bottom - borderThickness },
      { x: rect.right - borderThickness, y: rect.top + borderThickness }
   ];
}


/******************************************************************************
* drawSquareBorder / drawRoundedBorder
* 
* Draw a border at the given rectangle with the given border style.
* drawSquareBorder draws a squared edged border, and drawRoundedBorder will have rounded corners
*
******************************************************************************/
function drawSquareBorder(context, rect, borderStyle, color, borderThickness = 0)
{
   var small_rect = new Rect(rect.left + borderThickness / 4, rect.top + borderThickness / 4,
      rect.width - borderThickness, rect.height - borderThickness);
   var large_rect = new Rect(rect.left + borderThickness / 2, rect.top + borderThickness / 2,
      rect.width - borderThickness, rect.height - borderThickness);

   var lowerGeometry = getLowerBevelGeometry(rect, borderThickness);
   var upperGeometry = getUpperBevelGeometry(rect, borderThickness);

   switch (borderStyle)
   {
      case Enum.BORDER_STYLE.NONE:
         return;

      case Enum.BORDER_STYLE.SINGLE:
         context.lineWidth = borderThickness;
         context.strokeStyle = color;
         var thisRect = new Rect(
            rect.left + borderThickness / 2.0,
            rect.top + borderThickness / 2.0,
            rect.width - borderThickness,
            rect.height - borderThickness);
         context.strokeRect(thisRect.left, thisRect.top, thisRect.width, thisRect.height);
         break;

      case Enum.BORDER_STYLE.LOWERED:
         context.lineWidth = borderThickness;
         context.strokeStyle = adjustColorDarkness(color, 0.5, true);
         context.strokeRect(large_rect.left, large_rect.top, large_rect.width, large_rect.height);
         context.lineWidth = borderThickness / 2;
         context.strokeStyle = adjustColorDarkness(color, 0.5, false);
         context.strokeRect(small_rect.left, small_rect.top, small_rect.width, small_rect.height);
         break;

      case Enum.BORDER_STYLE.RAISED:
         context.lineWidth = borderThickness;
         context.strokeStyle = adjustColorDarkness(color, 0.5, false);
         context.strokeRect(large_rect.left, large_rect.top, large_rect.width, large_rect.height);
         context.lineWidth = borderThickness / 2;
         context.strokeStyle = adjustColorDarkness(color, 0.5, true);
         context.strokeRect(small_rect.left, small_rect.top, small_rect.width, small_rect.height);
         break;

      case Enum.BORDER_STYLE.LOWERED_BEVEL:
         context.fillStyle = adjustColorDarkness(color, 0.5, false);
         fillPolygon(context, upperGeometry);
         context.fillStyle = adjustColorDarkness(color, 0.5, true);
         fillPolygon(context, lowerGeometry);
         break;

      case Enum.BORDER_STYLE.RAISED_BEVEL:
         context.fillStyle = adjustColorDarkness(color, 0.5, true);
         fillPolygon(context, upperGeometry);
         context.fillStyle = adjustColorDarkness(color, 0.5, false);
         fillPolygon(context, lowerGeometry);
         break;

      default:
         csi_log("WARNING: Default case reached in drawSquareBorder");
   }
}

function drawRoundedBorder(context, rect, borderStyle, cornerRadius, color, borderThickness = 0)
{
   var small_rect = new Rect(rect.left + borderThickness / 4, rect.top + borderThickness / 4,
      rect.width - borderThickness, rect.height - borderThickness);
   var large_rect = new Rect(rect.left + borderThickness / 2, rect.top + borderThickness / 2,
      rect.width - borderThickness, rect.height - borderThickness);

   switch (borderStyle)
   {
      case Enum.BORDER_STYLE.NONE:
         return;

      case Enum.BORDER_STYLE.SINGLE:
         context.lineWidth = borderThickness;
         context.strokeStyle = color;
         var thisRect = new Rect(
            rect.left + borderThickness / 2.0,
            rect.top + borderThickness / 2.0,
            rect.width - borderThickness,
            rect.height - borderThickness);
         drawRoundedRect(context, thisRect, cornerRadius);
         context.stroke();
         break;

      case Enum.BORDER_STYLE.LOWERED:
         context.lineWidth = borderThickness / 2;
         large_rect.top -= 1;
         large_rect.left -= 1;
         small_rect.top += 1;
         small_rect.left += 1;
         context.strokeStyle = adjustColorDarkness(color, 0.5, true);
         drawRoundedRect(context, small_rect, cornerRadius);
         context.stroke();
         context.lineWidth = borderThickness / 2;
         context.strokeStyle = adjustColorDarkness(color, 0.5, false);
         drawRoundedRect(context, large_rect, cornerRadius);
         context.stroke();
         break;

      case Enum.BORDER_STYLE.RAISED:
         context.lineWidth = borderThickness;
         context.strokeStyle = adjustColorDarkness(color, 0.5, false);
         drawRoundedRect(context, large_rect, cornerRadius);
         context.stroke();
         context.lineWidth = borderThickness / 2;
         context.strokeStyle = adjustColorDarkness(color, 0.5, true);
         drawRoundedRect(context, small_rect, cornerRadius);
         context.stroke();
         break;

      default:
         csi_log("WARNING: Default case reached in drawRoundedBorder");
   }
}


/******************************************************************************
* drawBorder
* 
* Draw the border around the component.  The color may change, based on whether
* we are setting a value or not.
*
******************************************************************************/
CsiComponent.prototype.drawBorder = function (context, rect, borderStyle)
{
   context.save();

   if (this.bBackgroundRoundedCorners && borderStyle != Enum.BORDER_STYLE.LOWERED_BEVEL && borderStyle != Enum.BORDER_STYLE.RAISED_BEVEL)
      drawRoundedBorder(context, rect, borderStyle, this.backgroundRoundedRadius, this.backgroundBorderColor, this.getBorderWidth(borderStyle));
   else
      drawSquareBorder(context, rect, borderStyle, this.backgroundBorderColor, this.getBorderWidth(borderStyle));

   if (this.valueSetter !== null)
      this.valueSetter.drawSetValueBorder(context, this.valueSetter.state, rect);

   context.restore();
};


// Copies all background props from a to b
CsiComponent.prototype.copyBackgroundProps = function (a, b)
{
   b.bHasBackground               = a.bHasBackground;
   b.bUseClip                     = a.bUseClip;
   b.backgroundMargin             = a.backgroundMargin;
   b.eBackgroundBorderStyle       = a.eBackgroundBorderStyle;
   b.backgroundBorderColor        = a.backgroundBorderColor;
   b.backgroundBorderThickness    = a.backgroundBorderThickness;
   b.eBackgroundColorStyle        = a.eBackgroundColorStyle;
   b.backgroundSolidColor         = a.backgroundSolidColor;
   b.eBackgroundGradientDirection = a.eBackgroundGradientDirection;
   b.backgroundGradientStartColor = a.backgroundGradientStartColor;
   b.backgroundGradientMidColor   = a.backgroundGradientMidColor;
   b.backgroundGradientEndColor   = a.backgroundGradientEndColor;
   b.backgroundGradientUseMid     = a.backgroundGradientUseMid;
   b.bBackgroundRoundedCorners    = a.bBackgroundRoundedCorners;
   b.backgroundRoundedRadius      = a.backgroundRoundedRadius;
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

   context.translate(rect.left, rect.top);

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


function drawImage(context, img, rect, drawStyle = Enum.DrawStyleType.stretch)
{
   if(img) 
   {
      context.save();
      context.translate(rect.left, rect.top); //move to location

      if (drawStyle === Enum.DrawStyleType.stretch)
      {
         context.drawImage(img, 0, 0, rect.width, rect.height);
      }
      else if (drawStyle === Enum.DrawStyleType.tile)
      {
         var currLeft = 0;
         while (currLeft <= rect.width)
         {
            var currTop = 0;
            while (currTop <= rect.height)
            {
               context.drawImage(img, currLeft, currTop);
               currTop += img.height;
            }
            currLeft += img.width;
         }
      }
      else if (drawStyle === Enum.DrawStyleType.center)
      {
         context.drawImage(img, (rect.width - img.width) / 2.0,
            (rect.height - img.height) / 2.0);
      }
      else //Enum.DrawStyleType.best_fit
      {
         var im_w = img.width;
         var im_h = img.height;
         var comp_w = rect.width;
         var comp_h = rect.height;
         var image_aspect_ratio = 0;
         //Scale the image
         if (im_w < im_h)
         {
            image_aspect_ratio = im_w / im_h;
            im_h = comp_h;
            im_w = im_h * image_aspect_ratio;

            if (im_w > comp_w) //Keep the image inside the comp bounds
            {
               im_w = comp_w;
               im_h = im_w / image_aspect_ratio;
            }
         }
         else
         {
            image_aspect_ratio = im_h / im_w;
            im_w = comp_w;
            im_h = im_w * image_aspect_ratio;

            if (im_h > comp_h) //Keep the image inside the comp bounds
            {
               im_h = comp_h;
               im_w = im_h / image_aspect_ratio;
            }
         }
         var im_x = (rect.width / 2) - (im_w / 2);
         var im_y = (rect.height / 2) - (im_h / 2);
         context.drawImage(img, im_x, im_y, im_w, im_h);
      }
      context.restore();
   }
}




CsiComponent.prototype.drawImageError = function (context)
{
   drawImageError(new Rect(this.left, this.top, this.width, this.height), context);
};


/**
 * @return Returns true if this component is on the screen that is currently visible
 * or if none of the currently visible screen's components have bad_data flags.
 */
CsiComponent.prototype.needs_data_now = function()
{
   return graphicsManager.component_needs_data(this);
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

function fillRoundedRect(context, rect, cornerRadius)
{
   drawRoundedRect(context, rect, cornerRadius);
   context.fill();
}

function drawRoundedRect(context, rect, cornerRadius)
{
   context.beginPath();

   //Don't let the corners stick out of the rectangle.
   var radiusH = cornerRadius;
   var radiusW = cornerRadius;
   if(rect.height < (radiusH * 2.0))
   {
      radiusH = rect.height / 2.0;
   }

   if(rect.width < (radiusW * 2.0))
   {
      radiusW = rect.width / 2.0;
   }

   //Top Left Corner
   context.moveTo(rect.left + radiusW, rect.top);
   context.quadraticCurveTo(rect.left, rect.top, rect.left, rect.top + radiusH);
   //Left Line
   context.lineTo(rect.left, rect.bottom - radiusH);
   //Bottom Left Line
   context.quadraticCurveTo(rect.left, rect.bottom, rect.left + radiusW, rect.bottom);
   //Bottom Line
   context.lineTo(rect.right - radiusW, rect.bottom);
   //Bottom Right Line
   context.quadraticCurveTo(rect.right, rect.bottom, rect.right, rect.bottom - radiusH);
   //Right Line
   context.lineTo(rect.right, rect.top + radiusH);
   //Top Right Corner
   context.quadraticCurveTo(rect.right, rect.top, rect.right - radiusW, rect.top);
   //Top line
   context.closePath();
}


function fillPolygon(context, points)
{
   if (points.length === 0)
      return;

   context.beginPath();
   context.moveTo(points[0].x, points[0].y);
   for(let i = 1; i < points.length; i++)
   {
      context.lineTo(points[i].x, points[i].y);
   }
   context.closePath();
   context.fill();
}

function clipRect(context, x, y, width, height)
{
   context.beginPath();
   context.moveTo(x, y);
   context.lineTo(x + width, y);
   context.lineTo(x + width, y + height);
   context.lineTo(x, y + height);
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


CsiComponent.prototype.OnMouseExit = function ()
{
   if(this.IncludeHoverCaption)
   {
      oneShotTimer.clearTimeout(this, "ShowTooltip_Add");
      oneShotTimer.clearTimeout(this, "ShowTooltip_Remove");
      $("#tooltip_div").remove();
   }
   document.body.style.cursor = "default";
   return true; //Prevent default behavior
};


CsiComponent.prototype.OnMouseMove = function (mouseX, mouseY)
{
   if(this.IncludeHoverCaption)
   {
      oneShotTimer.clearTimeout(this, "ShowTooltip_Add");
      // Base the tip position off of the hotspot rectangle
      //this.tipX = mouseX + 5;
      //this.tipY = mouseY + 15;
      oneShotTimer.setTimeout(this, "ShowTooltip_Add", 1000); //Show if mouse hovers for 1 sec
   }
   return true; //Prevent default behavior
};


CsiComponent.prototype.onOneShotTimer = function (tag)
{
   if(tag === "ShowTooltip_Add")
   {
      $("#tooltip_div").remove(); //Fresh start

      var $tip = $('<div />', {
         id: 'tooltip_div',
      });
      $tip.html(this.HoverCaption);
      $tip.css({
         display: 'none', top: this.tipY, left: this.tipX, position: 'absolute', border: '1px solid black', color: 'black',
         'background-color': '#FFFFE1', padding: '4px', 'border-radius': '0px', 'z-index': '9', 'font-size' : '12pt', 'font-weight' : '500'
      });
      $tip.appendTo($('#canvas_container'));
      $tip.fadeIn("slow");
   }
   else if(tag === "ShowTooltip_Remove")
   {
      $("#tooltip_div").hide();
      $("#tooltip_div").remove();
   }
};


function drawNanData(comp, context)
{
   context.save();

   context.translate(comp.left + comp.width / 2, comp.top + comp.height / 2);
   if (typeof comp.RotationAngle === "number" && comp.RotationAngle !== 0)
   {
      context.rotate(degreesToRadians(comp.RotationAngle));
   }
   context.translate(comp.width / 2 - 16, -comp.height / 2);

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

   context.translate(comp.left + comp.width / 2, comp.top + comp.height / 2);
   if (typeof comp.RotationAngle === "number" && comp.RotationAngle !== 0)
   {
      context.rotate(degreesToRadians(comp.RotationAngle));
   }
   context.translate(comp.width / 2 - 16, -comp.height / 2);

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
