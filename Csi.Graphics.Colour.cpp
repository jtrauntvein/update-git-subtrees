/* Csi.Graphics.Colour.cpp

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 17 December 2015
   Last Change: Tuesday 29 December 2015
   Last Commit: $Date: 2015-12-29 14:40:19 -0600 (Tue, 29 Dec 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Graphics.Colour.h"
#include "StrUni.h"
#include "CsiTypes.h"
#include <cctype>
#include <cwctype>


namespace Csi
{
   namespace Graphics
   {
      void Colour::parse(char const *s)
      {
         enum state_type
         {
            state_before_colour,
            state_in_integer,
            state_in_hex_integer,
            state_read_r,
            state_read_g,
            state_read_b,
            state_read_a,
            state_read_red,
            state_read_green,
            state_read_blue,
            state_read_alpha,
            state_end,
            state_error
         } state = state_before_colour;
         size_t pos(0);
         StrAsc temp;
         
         while(s[pos] != 0 && state < state_end)
         {
            char ch = s[pos];
            switch(state)
            {
            case state_before_colour:
               if(ch == 'R' || ch == 'r')
                  state = state_read_r;
               else if(ch >= '0' && ch <= '9')
               {
                  temp.append(ch);
                  state = state_in_integer;
               }
               break;

            case state_read_r:
               if(ch == 'g' || ch == 'G')
                  state = state_read_g;
               else
                  state = state_error;
               break;

            case state_read_g:
               if(ch == 'b' || ch == 'B')
                  state = state_read_b;
               else
                  state = state_error;
               break;

            case state_read_b:
               if(ch == 'a' || ch == 'A')
                  state = state_read_a;
               else
                  state = state_error;
               break;

            case state_read_a:
               if(ch == '(')
                  state = state_read_red;
               else if(!std::isspace(ch))
                  state = state_error;
               break;

            case state_read_red:
               if(ch >= '0' && ch <= '9')
                  temp.append(ch);
               else if(ch == ',')
               {
                  uint4 red_val(strtoul(temp.c_str(), 0, 10));
                  set_red(static_cast<byte>(red_val & 0xff));
                  temp.cut(0);
                  state = state_read_green;
               }
               else if(!std::isspace(ch))
                  state = state_error;
               break;

            case state_read_green:
               if(ch >= '0' && ch <= '9')
                  temp.append(ch);
               else if(ch == ',')
               {
                  uint4 green_val(strtoul(temp.c_str(), 0, 10));
                  set_green(static_cast<byte>(green_val & 0xff));
                  temp.cut(0);
                  state = state_read_blue;
               }
               else if(!std::isspace(ch))
                  state = state_error;
               break;

            case state_read_blue:
               if(ch >= '0' && ch <= '9')
                  temp.append(ch);
               else if(ch == ',' || ch == ')')
               {
                  uint4 blue_val(strtoul(temp.c_str(), 0, 10));
                  set_blue(static_cast<byte>(blue_val & 0xff));
                  temp.cut(0);
                  if(ch == ',')
                     state = state_read_alpha;
                  else
                     state = state_end;
               }
               else if(!isspace(ch))
                  state = state_error;
               break;

            case state_read_alpha:
               if((ch >= '0' && ch <= '9') || ch == '.')
                  temp.append(ch);
               else if(ch == ')' || isspace(ch))
               {
                  double alpha_val(csiStringToFloat(temp.c_str(), Csi::StringLoader::make_locale(0)));
                  if(alpha_val < 0)
                     alpha_val = 0;
                  if(alpha_val > 1)
                     alpha_val = 1;
                  set_alpha(static_cast<byte>(alpha_val * 255));
                  state = state_end;
               }
               break;

            case state_in_integer:
               if(ch >= '0' && ch <= '9')
                  temp.append(ch);
               else if(ch == 'x' || ch == 'X')
               {
                  if(temp == "0")
                  {
                     temp.cut(0);
                     state = state_in_hex_integer;
                  }
                  else
                     state = state_error;
               }
               break;

            case state_in_hex_integer:
               if((ch >= '0' && ch <= '9') || (ch >= 'a' || ch <= 'f') || (ch >= 'A' && ch <= 'F'))
                  temp.append(ch);
               break;
            }
            ++pos;
         }

         // if we are parsing a decimal or hex string, we will assume that the alpha channel is on
         // all the way.
         if(state == state_in_integer)
         {
            // the colorref structure defines the order of colours differently than those used by
            // this class.  Its components are ordered 00bbggrr where we expect 00rrggbb.
            uint4 value(strtoul(temp.c_str(), 0, 10));
            set_red(static_cast<byte>(value & 0xff));
            set_green(static_cast<byte>((value & 0xff00) >> 8));
            set_blue(static_cast<byte>((value & 0xff0000) >> 16));
            set_alpha(0xff);
         }
         else if(state == state_in_hex_integer)
            components = strtoul(temp.c_str(), 0, 16) | 0xff000000;
      } // parse (single byte)


      void Colour::parse(wchar_t const *s)
      {
         enum state_type
         {
            state_before_colour,
            state_in_integer,
            state_in_hex_integer,
            state_read_r,
            state_read_g,
            state_read_b,
            state_read_a,
            state_read_red,
            state_read_green,
            state_read_blue,
            state_read_alpha,
            state_end,
            state_error
         } state = state_before_colour;
         size_t pos(0);
         StrUni temp;
         
         while(s[pos] != 0 && state < state_end)
         {
            wchar_t ch = s[pos];
            switch(state)
            {
            case state_before_colour:
               if(ch == L'R' || ch == L'r')
                  state = state_read_r;
               else if(ch >= L'0' && ch <= L'9')
               {
                  temp.append(ch);
                  state = state_in_integer;
               }
               break;

            case state_read_r:
               if(ch == L'g' || ch == L'G')
                  state = state_read_g;
               else
                  state = state_error;
               break;

            case state_read_g:
               if(ch == L'b' || ch == L'B')
                  state = state_read_b;
               else
                  state = state_error;
               break;

            case state_read_b:
               if(ch == L'a' || ch == 'A')
                  state = state_read_a;
               else
                  state = state_error;
               break;

            case state_read_a:
               if(ch == L'(')
                  state = state_read_red;
               else if(!std::iswspace(ch))
                  state = state_error;
               break;

            case state_read_red:
               if(ch >= L'0' && ch <= L'9')
                  temp.append(ch);
               else if(ch == L',')
               {
                  uint4 red_val(wcstoul(temp.c_str(), 0, 10));
                  set_red(static_cast<byte>(red_val & 0xff));
                  temp.cut(0);
                  state = state_read_green;
               }
               else if(!std::iswspace(ch))
                  state = state_error;
               break;

            case state_read_green:
               if(ch >= L'0' && ch <= L'9')
                  temp.append(ch);
               else if(ch == L',')
               {
                  uint4 green_val(wcstoul(temp.c_str(), 0, 10));
                  set_green(static_cast<byte>(green_val & 0xff));
                  temp.cut(0);
                  state = state_read_blue;
               }
               else if(!std::iswspace(ch))
                  state = state_error;
               break;

            case state_read_blue:
               if(ch >= L'0' && ch <= L'9')
                  temp.append(ch);
               else if(ch == L',' || ch == L')')
               {
                  uint4 blue_val(wcstoul(temp.c_str(), 0, 10));
                  set_blue(static_cast<byte>(blue_val & 0xff));
                  temp.cut(0);
                  if(ch == L',')
                     state = state_read_alpha;
                  else
                     state = state_end;
               }
               else if(!isspace(ch))
                  state = state_error;
               break;

            case state_read_alpha:
               if((ch >= L'0' && ch <= L'9') || ch == L'.')
                  temp.append(ch);
               else if(ch == L')' || iswspace(ch))
               {
                  double alpha_val(csiStringToFloat(temp.c_str(), Csi::StringLoader::make_locale(0)));
                  if(alpha_val < 0)
                     alpha_val = 0;
                  if(alpha_val > 1)
                     alpha_val = 1;
                  set_alpha(static_cast<byte>(alpha_val * 255));
                  state = state_end;
               }
               break;

            case state_in_integer:
               if(ch >= L'0' && ch <= L'9')
                  temp.append(ch);
               else if(ch == L'x' || ch == L'X')
               {
                  if(temp == L"0")
                  {
                     temp.cut(0);
                     state = state_in_hex_integer;
                  }
                  else
                     state = state_error;
               }
               break;

            case state_in_hex_integer:
               if((ch >= L'0' && ch <= L'9') || (ch >= L'a' || ch <= L'f') || (ch >= L'A' && ch <= L'F'))
                  temp.append(ch);
               break;
            }
            ++pos;
         }
         
         // if we are parsing a decimal or hex integer, we will assume that the alpha
         // channel is turned up all the way.
         if(state == state_in_integer)
         {
            // the colorref structure defines the order of colours differently than those used by
            // this class.  Its components are ordered 00bbggrr where we expect 00rrggbb.
            uint4 value(wcstoul(temp.c_str(), 0, 10));
            set_red(static_cast<byte>(value & 0xff));
            set_green(static_cast<byte>((value & 0xff00) >> 8));
            set_blue(static_cast<byte>((value & 0xff0000) >> 16));
            set_alpha(0xff);
         }
         else if(state == state_in_hex_integer)
            components = wcstoul(temp.c_str(), 0, 16) | 0xff000000;
      } // parse (wide)
   };
};


std::ostream &operator <<(std::ostream &out, Csi::Graphics::Colour const &colour)
{
   out << "RGBA("
       << (int)colour.get_red() << ","
       << (int)colour.get_green() << ","
       << (int)colour.get_blue() << ",";
   csiFloatToStream(out, colour.get_alpha() / 255.0);
   out << ")";
   return out;
}


std::wostream &operator <<(std::wostream &out, Csi::Graphics::Colour const &colour)
{
   out << L"RGBA("
       << (int)colour.get_red() << L","
       << (int)colour.get_green() << L","
       << (int)colour.get_blue() << L",";
   csiFloatToStream(out, colour.get_alpha() / 255.0);
   out << L")";
   return out;
}
