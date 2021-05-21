/* CsiColour.js

   Copyright (C) 2019, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 15 February 2019
   Last Change: Thursday 16 May 2019
   Last Commit: $Date: 2019-11-15 17:08:30 -0600 (Fri, 15 Nov 2019) $
   Last Changed by: $Author: rhyden $

*/

function CsiColour()
{
   this.red = 0;
   this.green = 0;
   this.blue = 0;
   this.alpha = 0xff;
}


CsiColour.parse = function(colour_str)
{
   var colour = colour_str.toLowerCase();
   var rtn = new CsiColour();
   switch(colour)
   {
   case "black":
      rtn.set_red(0).set_green(0x00).set_blue(0x00);
      break;

   case "silver":
      rtn.set_red(0xc0).set_green(0xc0).set_blue(0xc0);
      break;

   case "gray":
      rtn.set_red(0x80).set_green(0x80).set_blue(0x80);
      break;

   case "white":
      rtn.set_red(0xff).set_green(0xff).set_blue(0xff);
      break;

   case "maroon":
      rtn.set_red(0x80).set_green(0x80).set_blue(0x00);
      break;

   case "red":
      rtn.set_red(0xff).set_green(0x00).set_blue(0x00);
      break;

   case "purple":
      rtn.set_red(0x80).set_green(0x00).set_blue(0x80);
      break;

   case "fuchsia":
      rtn.set_red(0xff).set_green(0xff).set_blue(0xff);
      break;
      
   case "green":
      rtn.set_red(0x00).set_green(0x80).set_blue(0x00);
      break;

   case "lime":
      rtn.set_red(0x00).set_green(0xff).set_blue(0x00);
      break;

   case "olive":
      rtn.set_red(0x80).set_green(0x80).set_blue(0x80);
      break;

   case "yellow":
      rtn.set_red(0xff).set_green(0xff).set_blue(0x00);
      break;

   case "navy":
      rtn.set_red(0x00).set_green(0x00).set_blue(0x80);
      break;

   case "blue":
      rtn.set_red(0x00).set_green(0x00).set_blue(0xff);
      break;

   case "teal":
      rtn.set_red(0x00).set_green(0x80).set_blue(0x80);
      break;

   case "aqua":
      rtn.set_red(0x00).set_green(0xff).set_blue(0xff);
      break;

   case "orange":
      rtn.set_red(0xff).set_green(0xa5).set_blue(0x00);
      break;

   case "aliceblue":
      rtn.set_red(0xf0).set_green(0xf8).set_blue(0xff);
      break;

   case "antiquewhite":
      rtn.set_red(0xfa).set_green(0xeb).set_blue(0xd7);
      break;

   case "aquamarine":
      rtn.set_red(0x7f).set_green(0xff).set_blue(0xd4);
      break;

   case "azure":
      rtn.set_red(0xf0).set_green(0xff).set_blue(0xff);
      break;

   case "beige":
      rtn.set_red(0xf5).set_green(0xf5).set_blue(0xdc);
      break;

   case "bisque":
      rtn.set_red(0xff).set_green(0xe4).set_blue(0xc4);
      break;

   case "blanchedalmond":
      rtn.set_red(0xff).set_green(0xeb).set_blue(0xcd);
      break;

   case "blueviolet":
      rtn.set_red(0x8a).set_green(0x2b).set_blue(0xe2);
      break;

   case "brown":
      rtn.set_red(0xa5).set_green(0x2a).set_blue(0x2a);
      break;

   case "burlywood":
      rtn.set_red(0xde).set_green(0xb8).set_blue(0x87);
      break;

   case "cadetblue":
      rtn.set_red(0x5f).set_green(0x9e).set_blue(0xa0);
      break;

   case "chartreuse":
      rtn.set_red(0x7f).set_green(0xff).set_blue(0x00);
      break;

   case "chocolate":
      rtn.set_red(0xd2).set_green(0x69).set_blue(0x1e);
      break;

   case "coral":
      rtn.set_red(0xff).set_green(0x7f).set_blue(0x50);
      break;

   case "cornflowerblue":
      rtn.set_red(0x64).set_green(0x95).set_blue(0xed);
      break;

   case "cornsilk":
      rtn.set_red(0xff).set_green(0xf8).set_blue(0xdc);
      break;

   case "crimson":
      rtn.set_red(0xdc).set_green(0x14).set_blue(0x3c);
      break;

   case "darkblue":
      rtn.set_red(0x00).set_green(0x00).set_blue(0x8b);
      break;

   case "darkcyan":
      rtn.set_red(0x00).set_green(0x8b).set_blue(0x8b);
      break;

   case "darkgoldenrod":
      rtn.set_red(0xb8).set_green(0x86).set_blue(0x0b);
      break;

   case "darkgray":
   case "darkgey":
      rtn.set_red(0xa9).set_green(0xa9).set_blue(0xa9);
      break;

   case "darkgreen":
      rtn.set_red(0x00).set_green(0x64).set_blue(0x00);
      break;

   case "darkkhaki":
      rtn.set_red(0xbd).set_green(0xb7).set_blue(0x6b);
      break;

   case "darkmagenta":
      rtn.set_red(0x8b).set_green(0x00).set_blue(0x8b);
      break;

   case "darkolivegreen":
      rtn.set_red(0x55).set_green(0x6b).set_blue(0x2f);
      break;

   case "darkorange":
      rtn.set_red(0xff).set_green(0x8c).set_blue(0x00);
      break;

   case "darkorchid":
      rtn.set_red(0x99).set_green(0x32).set_blue(0xcc);
      break;

   case "darkred":
      rtn.set_red(0x8b).set_green(0x00).set_blue(0x00);
      break;

   case "darksalmon":
      rtn.set_red(0xe9).set_green(0x96).set_blue(0x7a);
      break;

   case "darkseagreen":
      rtn.set_red(0x8f).set_green(0xbc).set_blue(0x8f);
      break;

   case "darkslateblue":
      rtn.set_red(0x48).set_green(0x3d).set_blue(0x8b);
      break;

   case "darkslategray":
   case "darkslategrey":
      rtn.set_red(0x24).set_green(0x24).set_blue(0x24);
      break;

   case "darkturquoise":
      rtn.set_red(0x00).set_green(0xce).set_blue(0xd1);
      break;

   case "darkviolet":
      rtn.set_red(0x94).set_green(0x00).set_blue(0xd3);
      break;

   case "deeppink":
      rtn.set_red(0xff).set_green(0x14).set_blue(0x93);
      break;

   case "deepskyblue":
      rtn.set_red(0x00).set_green(0xbf).set_blue(0xff);
      break;

   case "dimgray":
   case "dimgrey":
      rtn.set_red(0x69).set_green(0x69).set_blue(0x69);
      break;

   case "dodgerblue":
      rtn.set_red(0x1e).set_green(0x90).set_blue(0xff);
      break;

   case "firebrick":
      rtn.set_red(0xb2).set_green(0x22).set_blue(0x22);
      break;

   case "floralwhite":
      rtn.set_red(0xff).set_green(0xfa).set_blue(0xf0);
      break;

   case "forestgreen":
      rtn.set_red(0x22).set_green(0x8b).set_blue(0x22);
      break;

   case "gainsboro":
      rtn.set_red(0xdc).set_green(0xdc).set_blue(0xdc);
      break;

   case "ghostwhite":
      rtn.set_red(0xf8).set_green(0xf8).set_blue(0xff);
      break;

   case "gold":
      rtn.set_red(0xff).set_green(0xd7).set_blue(0x00);
      break;

   case "goldenrod":
      rtn.set_red(0xda).set_green(0xa5).set_blue(0x20);
      break;

   case "greenyellow":
      rtn.set_red(0xad).set_green(0xff).set_blue(0x2f);
      break;

   case "grey":
   case "gray":
      rtn.set_red(0x80).set_green(0x80).set_blue(0x80);
      break;

   case "honeydew":
      rtn.set_red(0xf0).set_green(0xff).set_blue(0xf0);
      break;

   case "hotpink":
      rtn.set_red(0xff).set_green(0x69).set_blue(0xb4);
      break;

   case "indianred":
      rtn.set_red(0xcd).set_green(0x5c).set_blue(0x5c);
      break;

   case "indigo":
      rtn.set_red(0x4b).set_green(0x00).set_blue(0x82);
      break;

   case "ivory":
      rtn.set_red(0xff).set_green(0xff).set_blue(0xf0);
      break;

   case "khaki":
      rtn.set_red(0xf0).set_green(0xe6).set_blue(0x8c);
      break;

   case "lavender":
      rtn.set_red(0xe6).set_green(0xe6).set_blue(0xfa);
      break;

   case "lavenderblush":
      rtn.set_red(0xff).set_green(0xf0).set_blue(0xf5);
      break;

   case "lawngreen":
      rtn.set_red(0x7c).set_green(0xfc).set_blue(0x00);
      break;

   case "lemonchiffon":
      rtn.set_red(0xff).set_green(0xfa).set_blue(0xcd);
      break;

   case "lightblue":
      rtn.set_red(0xad).set_green(0xd8).set_blue(0xe6);
      break;

   case "lightcoral":
      rtn.set_red(0xf0).set_green(0x80).set_blue(0x80);
      break;

   case "lightcyan":
      rtn.set_red(0xe0).set_green(0xff).set_blue(0xff);
      break;

   case "lightgoldenrodyellow":
      rtn.set_red(0xfa).set_green(0xfa).set_blue(0xd2);
      break;

   case "lightgray":
   case "lightgrey":
      rtn.set_red(0xd3).set_green(0xd3).set_blue(0xd3);
      break;
      
   case "lightgreen":
      rtn.set_red(0x90).set_green(0xee).set_blue(0x90);
      break;

   case "lightpink":
      rtn.set_red(0xff).set_green(0xb6).set_blue(0xc1);
      break;

   case "lightsalmon":
      rtn.set_red(0xff).set_green(0xa0).set_blue(0x7a);
      break;

   case "lightseagreen":
      rtn.set_red(0x20).set_green(0xb2).set_blue(0xaa);
      break;

   case "lightskyblue":
      rtn.set_red(0x87).set_green(0xce).set_blue(0xfa);
      break;

   case "lightslategrey":
   case "lightslategray":
      rtn.set_red(0x77).set_green(0x88).set_blue(0x99);
      break;

   case "lightsteelblue":
      rtn.set_red(0xb0).set_green(0xc4).set_blue(0xde);
      break;

   case "lightyellow":
      rtn.set_red(0xff).set_green(0xff).set_blue(0xe0);
      break;

   case "limegreen":
      rtn.set_red(0x32).set_green(0xcd).set_blue(0x32);
      break;

   case "linen":
      rtn.set_red(0xfa).set_green(0xf0).set_blue(0xe6);
      break;

   case "mediumaquamarine":
      rtn.set_red(0x66).set_green(0xcd).set_blue(0xaa);
      break;

   case "mediumblue":
      rtn.set_red(0x00).set_green(0x00).set_blue(0xcd);
      break;

   case "mediumorchid":
      rtn.set_red(0xba).set_green(0x55).set_blue(0xd3);
      break;

   case "mediumpurple":
      rtn.set_red(0x93).set_green(0x70).set_blue(0xdb);
      break;

   case "mediumseagreen":
      rtn.set_red(0x3c).set_green(0xb3).set_blue(0x71);
      break;

   case "mediumslateblue":
      rtn.set_red(0x7b).set_green(0x68).set_blue(0xee);
      break;

   case "mediumspringgreen":
      rtn.set_red(0x00).set_green(0xfa).set_blue(0x9a);
      break;

   case "mediumturquoise":
      rtn.set_red(0x48).set_green(0xda).set_blue(0xcc);
      break;

   case "mediumvioletred":
      rtn.set_red(0xc7).set_green(0x15).set_blue(0x85);
      break;

   case "midnightblue":
      rtn.set_red(0x19).set_green(0x19).set_blue(0x70);
      break;

   case "mintcream":
      rtn.set_red(0xf5).set_green(0xff).set_blue(0xfa);
      break;

   case "mistyrose":
      rtn.set_red(0xff).set_green(0xe4).set_blue(0xe1);
      break;

   case "moccasin":
      rtn.set_red(0xff).set_green(0xe4).set_blue(0xb5);
      break;

   case "navajowhite":
      rtn.set_red(0xff).set_green(0xde).set_blue(0xb5);
      break;

   case "oldlace":
      rtn.set_red(0xfd).set_green(0xf5).set_blue(0xe6);
      break;

   case "olivedrab":
      rtn.set_red(0x6b).set_green(0x8e).set_blue(0x23);
      break;

   case "orangered":
      rtn.set_red(0xff).set_green(0x45).set_blue(0x00);
      break;

   case "orchid":
      rtn.set_red(0xda).set_green(0x70).set_blue(0xd6);
      break;

   case "palegoldenrod":
      rtn.set_red(0xee).set_green(0xe8).set_blue(0xaa);
      break;

   case "palegreen":
      rtn.set_red(0x98).set_green(0xfb).set_blue(0x98);
      break;

   case "paleturquoise":
      rtn.set_red(0xaf).set_green(0xee).set_blue(0xee);
      break;

   case "palevioletred":
      rtn.set_red(0xdb).set_green(0x70).set_blue(0x93);
      break;

   case "papayawhip":
      rtn.set_red(0xff).set_green(0xef).set_blue(0xd5);
      break;

   case "peachpuff":
      rtn.set_red(0xff).set_green(0xda).set_blue(0xb9);
      break;

   case "peru":
      rtn.set_red(0xcd).set_green(0x85).set_blue(0x3f);
      break;

   case "pink":
      rtn.set_red(0xff).set_green(0xc0).set_blue(0xcb);
      break;

   case "plum":
      rtn.set_red(0xdd).set_green(0xa0).set_blue(0xdd);
      break;

   case "powderblue":
      rtn.set_red(0xb0).set_green(0xe0).set_blue(0xe6);
      break;

   case "rosybrown":
      rtn.set_red(0xbc).set_green(0x8f).set_blue(0x8f);
      break;

   case "royalblue":
      rtn.set_red(0x41).set_green(0x69).set_blue(0xe1);
      break;

   case "saddlebrown":
      rtn.set_red(0x8b).set_green(0x45).set_blue(0x13);
      break;

   case "salmon":
      rtn.set_red(0xfa).set_green(0x80).set_blue(0x72);
      break;

   case "sandybrown":
      rtn.set_red(0xf4).set_green(0xa4).set_blue(0x60);
      break;

   case "seagreen":
      rtn.set_red(0x2e).set_green(0x8b).set_blue(0x57);
      break;

   case "seashell":
      rtn.set_red(0xff).set_green(0xf5).set_blue(0xee);
      break;

   case "sienna":
      rtn.set_red(0xa0).set_green(0x52).set_blue(0x2d);
      break;

   case "skyblue":
      rtn.set_red(0x87).set_green(0xce).set_blue(0xeb);
      break;

   case "slateblue":
      rtn.set_red(0x6a).set_green(0x5a).set_blue(0xcd);
      break;

   case "slategray":
   case "slategrey":
      rtn.set_red(0x70).set_green(0x80).set_blue(0x90);
      break;

   case "snow":
      rtn.set_red(0xff).set_green(0xfa).set_blue(0xfa);
      break;

   case "springgreen":
      rtn.set_red(0x00).set_green(0xff).set_blue(0x7f);
      break;

   case "steelblue":
      rtn.set_red(0x46).set_green(0x82).set_blue(0xb4);
      break;

   case "tan":
      rtn.set_red(0xd2).set_green(0xb4).set_blue(0x8c);
      break;

   case "thistle":
      rtn.set_red(0xd8).set_green(0xbf).set_blue(0xd8);
      break;

   case "tomato":
      rtn.set_red(0xff).set_green(0x63).set_blue(0x47);
      break;

   case "turquoise":
      rtn.set_red(0x40).set_green(0xe0).set_blue(0xd0);
      break;

   case "violet":
      rtn.set_red(0xee).set_green(0x82).set_blue(0xee);
      break;

   case "wheat":
      rtn.set_red(0xf5).set_green(0xde).set_blue(0xb3);
      break;

   case "whitesmoke":
      rtn.set_red(0xf5).set_green(0xf5).set_blue(0xf5);
      break;

   case "yellowgreen":
      rtn.set_red(0x9a).set_green(0xcd).set_blue(0x32);
      break;

   case "rebeccapurple":
      rtn.set_red(0x66).set_green(0x33).set_blue(0x99);
      break;

   default:
      if(colour.charAt(0) === '#')
         rtn = CsiColour.parse_hex(colour);
      else if(colour.indexOf("rgba(") === 0)
         rtn = CsiColour.parse_rgba(colour);
      else if(colour.indexOf("rgb(") === 0)
         rtn = CsiColour.parse_rgba(colour);
      break;
   }
   return rtn;
};


CsiColour.parse_hex = function(colour)
{
   var components = Number.parseInt(colour.substring(1), 16);
   var rtn = new CsiColour();
   rtn.set_red((components & 0xff0000) >> 16);
   rtn.set_green((components & 0x00ff00) >> 8);
   rtn.set_blue(components & 0x0000ff);
   return rtn;
};


CsiColour.parse_rgba = function(colour)
{
   const state_before_colour = 1;
   const state_read_r = 4;
   const state_read_g = 5;
   const state_read_b = 6;
   const state_read_a = 7;
   const state_before_red = 8;
   const state_read_red = 9;
   const state_before_green = 10;
   const state_read_green = 11;
   const state_before_blue = 12;
   const state_read_blue = 13;
   const state_before_alpha = 14;
   const state_read_alpha = 15;
   const state_end = 16;
   const state_error = 17;
   var state = state_before_colour;
   var pos = 0;
   var temp = "";
   var ch;
   var rtn = new CsiColour();
   var alpha_val;
   var do_incr;

   while(pos < colour.length && state < state_end)
   {
      do_incr = true;
      ch = colour.charAt(pos);
      if(state === state_before_colour)
      {
         if(ch === 'R' || ch === 'r')
            state = state_read_r;
         else if(ch >= '0' && ch <= '9')
         {
            temp = ch;
            state = state_error;
         }
         else if(ch == '#')
            state = state_error;
      }
      else if(state === state_read_r)
      {
         if(ch === 'g' || ch === 'G')
            state = state_read_g;
         else
            state = state_error;
      }
      else if(state === state_read_g)
      {
         if(ch === 'b' || ch === 'B')
            state = state_read_b;
         else
            state = state_error;
      }
      else if(state === state_read_b)
      {
         if(ch === 'a' || ch == 'A')
            state = state_read_a;
         else if(ch === '(')
            state = state_read_red;
         else
            state = state_error;
      }
      else if(state === state_read_a)
      {
         if(ch === '(')
            state = state_before_red;
         else
            state = state_error;
      }
      else if(state === state_before_red)
      {
         if(ch >= '0' && ch <= '9')
         {
            state = state_read_red;
            do_incr = false;
            temp = "";
         }
         else if(ch !== ' ')
            state = state_error;
      }
      else if(state == state_read_red)
      {
         if(ch >= '0' && ch <= '9')
            temp += ch;
         else if(ch === ',')
         {
            if(temp.length > 0)
            {
               rtn.set_red(Number.parseInt(temp));
               state = state_before_green;
            }
            else
               state = state_error;
         }
         else
            state = state_error;
      }
      else if(state === state_before_green)
      {
         if(ch >= '0' && ch <= '9')
         {
            state = state_read_green;
            do_incr = false;
            temp = "";
         }
         else if(ch !== ' ')
            state = state_error;
      }
      else if(state === state_read_green)
      {
         if(ch >= '0' && ch <= '9')
            temp += ch;
         else if(ch === ',')
         {
            if(temp.length > 0)
            {
               rtn.set_green(Number.parseInt(temp));
               state = state_before_blue;
            }
            else
               state = state_error;
         }
         else
            state = state_error;
      }
      else if(state === state_before_blue)
      {
         if(ch >= '0' && ch <= '9')
         {
            state = state_read_blue;
            do_incr = false;
            temp = "";
         }
         else if(ch !== ' ')
            state = state_error;
      }
      else if(state === state_read_blue)
      {
         if(ch >= '0' && ch <= '9')
            temp += ch;
         else if(ch === ')' || ch === ',')
         {
            if(temp.length > 0)
            {
               rtn.set_blue(Number.parseInt(temp));
               if(ch === ',')
                  state = state_before_alpha;
               else
                  state = state_end;
            }
            else
               state = state_error;
         }
         else
            state = state_error;
      }
      else if(state === state_before_alpha)
      {
         if((ch >= '0' && ch <= '9') || ch === '.')
         {
            state = state_read_alpha;
            do_incr = false;
            temp = "";
         }
         else if(ch !== ' ')
            state = state_error;
      }
      else if(state === state_read_alpha)
      {
         if((ch >= '0' && ch <= '9') || ch === '.')
            temp += ch;
         else if(ch === ')')
         {
            if(temp.length > 0)
            {
               alpha_val = Number.parseFloat(temp);
			   rtn.set_alpha(Math.trunc(alpha_val * 100)/100);
               if(alpha_val < 0)
                  alpha_val = 0;
               if(alpha_val > 1)
                  alpha_val = 1;               
               state = state_end;
            }
            else
               state = state_error;
            temp = "";
         }
         else
            state = state_error;
      }
      if(do_incr)
         ++pos;
   }
   if(state === state_error)
      rtn = new CsiColour();
   return rtn;
};


CsiColour.from_int = function(red, green, blue, alpha)
{
   var rtn = new CsiColour();
   if(red !== undefined)
      rtn.red = red;
   if(green !== undefined)
      rtn.green = green;
   if(blue !== undefined)
      rtn.blue = blue;
   if(alpha !== undefined)
      rtn.alpha = alpha;
   return rtn;
};


CsiColour.from_float = function(red, green, blue, alpha)
{
   var rtn = new CsiColour();
   if(red !== undefined)
   {
      if(red < 0)
         red = 0;
      if(red > 1)
         red = 1;
      rtn.red = Math.floor(red * 255);
   }
   if(green !== undefined)
   {
      if(green < 0)
         green = 0;
      if(green > 1)
         green = 1;
      rtn.green = Math.floor(green * 255);
   }
   if(blue != undefined)
   {
      if(blue < 0)
         blue = 0;
      if(blue > 1)
         blue = 1;
      rtn.blue = Math.floor(blue * 255);
   }
   if(alpha !== undefined)
   {
      if(alpha < 0)
         alpha = 0;
      if(alpha > 1)
         alpha = 1;
      rtn.alpha = Math.floor(alpha * 255);
   }
   return rtn;
};


CsiColour.prototype.get_red = function()
{
   return this.red;
};


CsiColour.prototype.set_red = function(red)
{
   this.red = red;
   return this;
};


CsiColour.prototype.get_green = function()
{
   return this.green;
};


CsiColour.prototype.set_green = function(green)
{
   this.green = green;
   return this;
};


CsiColour.prototype.get_blue = function()
{
   return this.blue;
};


CsiColour.prototype.set_blue = function(blue)
{
   this.blue = blue;
   return this;
};


CsiColour.prototype.get_alpha = function()
{
   return this.alpha;
};


CsiColour.prototype.set_alpha = function(alpha)
{
   this.alpha = alpha;
   return this;
};


CsiColour.prototype.get_transparency = function()
{
   return (255.0 - this.alpha) / 255.0;
};


CsiColour.prototype.set_transparency = function(transparency)
{
   if(transparency < 0)
      transparency = 0;
   if(transparency > 1)
      transparency = 1;
   this.alpha = Math.floor((1 - transparency) * 255);
   return this;
};


CsiColour.prototype.to_grayscale = function()
{
   var red = this.red / 255;
   var green = this.green / 255;
   var blue = this.blue / 255;
   return red * 0.21 + green * 0.71 + blue * 0.07;
};


CsiColour.prototype.background_complement = function()
{
   var grayscale = this.to_grayscale();
   var rtn = new CsiColour();
   if(grayscale < 0.5)
      rtn.set_red(0xFF).set_green(0xFF).set_blue(0xFF);
   return rtn;
};


CsiColour.prototype.format = function()
{
   var rtn = "RGBA(" + this.red + "," + this.green + "," + this.blue + "," + (this.alpha / 255) + ")";
   return rtn;
};




