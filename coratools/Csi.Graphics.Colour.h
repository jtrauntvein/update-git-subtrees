/* Csi.Graphics.Colour.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 14 January 2015
   Last Change: Wednesday 16 December 2015
   Last Commit: $Date: 2018-10-25 11:16:21 -0600 (Thu, 25 Oct 2018) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_Colour_h
#define Csi_Graphics_Colour_h

#include "CsiTypeDefs.h"
#include <iostream>


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines an object that describes a colour using the RGB/A model.
       */
      class Colour
      {
      private:
         /**
          * Holds the colour components in a packed integer.
          */
         uint4 components;

         /**
          * Specifies the mask used for the alpha channel.
          */
         static uint4 const alpha_mask = 0xff000000;
         
         /**
          * Specifies the mask used for red channel.
          */
         static uint4 const red_mask = 0x00ff0000;

         /**
          * Specifies the mask used for the green channel.
          */
         static uint4 const green_mask = 0x0000ff00;
         
         /**
          * Specifies the mask used for the blue channel.
          */
         static uint4 const blue_mask = 0x000000ff;

      public:
         /**
          * Default Constructor
          */
         Colour():
            components(0)
         { }

         /**
          * Construct from a bitmap for the components.
          *
          * @param components_  Specifies the new components value.
          *
          * @param set_alpha Set to true if the alpha portion of the components should be accepted/ 
          */
         Colour(uint4 components_, bool use_alpha = false):
            components(components_)
         {
            if(!use_alpha)
               set_alpha(0xff);
         }

         /**
          * Copy constructor
          *
          * @param other Specifies the colour to copy.
          */
         Colour(Colour const &other):
            components(other.components)
         { }

         /**
          * Construct from individual colour components.
          *
          * @param red Specifies the red intensity as an integer between 0 and 255.
          *
          * @param green Specifies the green intensity as an integer between 0 and 255.
          *
          * @param blue Specifies the blue intensity as an integer between 0 and 255.
          *
          * @param alpha Specifies the alpha channel intensity as an integer between 0 and 255.
          */
         Colour(byte red, byte green, byte blue, byte alpha = 0xff):
            components(0)
         {
            set_red(red);
            set_green(green);
            set_blue(blue);
            set_alpha(alpha);
         }

         /**
          * Construct the colour with the floating point intensities of each shade.  Each intensity
          * is expected to be a value between zero and one.
          *
          * @param red Specifies the intensity of red.
          *
          * @param green Specifies the intensity of green.
          *
          * @param blue Specifies the intensity of blue.
          *
          * @param alpha Specifies the intensity of the alpha channel.
          */
         Colour(double red, double green, double blue, double alpha = 1)
         {
            if(red < 0)
               red = 0;
            if(red > 1)
               red = 1;
            if(green < 0)
               green = 0;
            if(green > 1)
               green = 1;
            if(blue < 0)
               blue = 0;
            if(blue > 1)
               blue = 1;
            if(alpha < 0)
               alpha = 0;
            if(alpha > 1)
               alpha = 1;
            set_red(static_cast<byte>((red * 255) + 0.5));
            set_green(static_cast<byte>((green * 255) + 0.5));
            set_blue(static_cast<byte>((blue * 255) + 0.5));
            set_alpha(static_cast<byte>((alpha * 255) + 0.5));
         }

         /**
          * Construct from a string.
          *
          * @param s Specifies a string that will express the string as a
          * decimal integer or as a Javascript RGBA(r, g, b, a) string.
          */
         Colour(char const *s):
            components(0)
         { parse(s); }
         Colour(wchar_t const *s):
            components(0)
         { parse(s); }

         /**
          * Copy operator
          *
          * @param other Specifies the colour to copy.
          *
          * @return Returns a reference to this colour.
          */
         Colour &operator =(Colour const &other)
         {
            components = other.components;
            return *this;
         }

         /**
          * Copy from components
          */
         Colour &operator =(uint4 components_)
         {
            components = components_;
            return *this;
         }

         /**
          * Copy from a string expressed as either RGBA(r, g, b, a) or as a
          * decimal integer.
          */
         Colour &operator =(char const *s)
         {
            parse(s);
            return *this;
         }
         Colour &operator =(wchar_t const *s)
         {
            parse(s);
            return *this;
         }

         /**
          * @return Returns the red intensity as an integer between 0 and 255.
          */
         byte get_red() const
         {
            return static_cast<byte>((components & red_mask) >> 16);
         }

         /**
          * @return Returns the green intensity as an integer between 0 and 255.
          */
         byte get_green() const
         {
            return static_cast<byte>((components & green_mask) >> 8);
         }

         /**
          * @return Returns the blue intensity as an integer between 0 and 255.
          */
         byte get_blue() const
         {
            return static_cast<byte>(components & blue_mask);
         }

         /**
          * @return Returns the alpha channel intensity as an integer between 0 and 255.
          */
         byte get_alpha() const
         {
            return static_cast<byte>((components & alpha_mask) >> 24);
         }

         /**
          * Sets the red colour intensity.
          *
          * @param red Specifies the red colour intensity as an integer between 0 and 255.
          */
         Colour &set_red(byte red)
         {
            uint4 red_val(red);
            components = (components & ~red_mask) | ((red_val << 16) & red_mask);
            return *this;
         }

         /**
          * Sets the green colour component.
          *
          * @param green Specifies the green colour intensity as an integer between 0 and 255.
          */
         Colour &set_green(byte green)
         {
            uint4 green_val(green);
            components = (components & ~green_mask) | ((green_val << 8) & green_mask);
            return *this;
         }

         /**
          * Sets the blue colour component.
          *
          * @param blue Specifies the blue intensity as an integer between 0 and 255.
          */
         Colour &set_blue(byte blue)
         {
            uint4 blue_val(blue);
            components = (components & ~blue_mask) | (blue_val & blue_mask);
            return *this;
         }

         /**
          * Sets the alpha colour component.
          *
          * @param alpha Specifies the alpha intensity as an integer between 0 and 255.
          */
         Colour &set_alpha(byte alpha)
         {
            uint4 alpha_val(alpha);
            components = (components & ~alpha_mask) | ((alpha_val << 24) & alpha_mask);
            return *this;
         }

         /**
          * @return Returns the grayscale equivalent of this colour as a floating point value
          * between 0 and 1 where 0 represents black and 1 represents white.
          */
         double to_grayscale() const
         {
            double red(get_red() / 255.0);
            double green(get_green() / 255.0);
            double blue(get_blue() / 255.0);
            return red * 0.21 + green * 0.71 + blue * 0.07;
         }

         /**
          * @return Returns the components encoded in an unsigned integer.
          */
         uint4 get_components() const
         { return components; }

         /**
          * @return Returns true if the other colour is equal.
          */
         bool operator ==(Colour const &other) const
         { return components == other.components; }

         /**
          * @return Returns true if the other colour is not equal to this colour.
          */
         bool operator !=(Colour const &other) const
         { return components != other.components; }

         /**
          * @return Returns true if the other colour is less than this colour.
          */
         bool operator <(Colour const &other) const
         { return components < other.components; }

         /**
          * Parse the colour from the specified string.
          *
          * @param s Specifies the string representation of the colour.
          */
         void parse(char const *s);
         void parse(wchar_t const *s);

         /**
          * @return Returns the colour encoded as a COLORREF.
          *
          * @param include_alpha Set to true if the alpha channel should be encoded.
          */
         uint4 to_colorref(bool include_alpha = false) const
         {
            uint4 red_cor(get_red());
            uint4 blue_cor(get_blue());
            uint4 green_cor(get_green());
            uint4 rtn((blue_cor << 16) | (green_cor << 8) | red_cor);
            if(include_alpha)
               rtn |= static_cast<uint4>(get_alpha()) << 24;
            return rtn;
         }

         /**
          * @return Returns transparency as a ratio between zero and one with
          * one being most transparent (alpha = 0) and zero being least
          * transparent (alpha = 255).
          */
         double get_transparency() const
         { return (255.0 - get_alpha()) / 255.0; }

         /**
          * @param transparency Specifies the transparency of the colour as a
          * ratio between zero and one with zero being the least transparent
          * (alpha = 255) and one being the most transparent (alpha = 0).
          */
         Colour &set_transparency(double transparency)
         {
            set_alpha(static_cast<byte>((1 - transparency) * 255));
            return *this;
         }
      };
      typedef Colour Color;


      /**
       * @return Returns a red colour.
       */
      inline Colour red_colour()
      { return Colour(0xff0000); }

      /**
       * @return Returns a green colour.
       */
      inline Colour green_colour()
      { return Colour(0x00ff00); }

      /**
       * @return Returns a forest green colour.
       */
      inline Colour forest_green_colour()
      { return Colour(0x228b22); }

      /**
       * @return Returns a cyan colour.
       */
      inline Colour cyan_colour()
      { return Colour(0x00ffff); }

      /**
       * @return Returns a blue colour.
       */
      inline Colour blue_colour()
      { return Colour(0x0000ff); }

      /**
       * @return Returns a dark blue colour.
       */
      inline Colour dark_blue_colour()
      { return Colour(0x0000a0); }

      /**
       * @return Returns a magenta colour.
       */
      inline Colour magenta_colour()
      { return Colour(0xff00ff); }

      /**
       * @return Returns a brown colour.
       */
      inline Colour brown_colour()
      { return Colour(0xa52a2a); }

      /**
       * @return Returns a line colour.
       */
      inline Colour lime_colour()
      { return Colour(0x00ff00); }

      /**
       * @return Returns an orange colour.
       */
      inline Colour orange_colour()
      { return Colour(0xffa500); }

      /**
       * @return Returns the purple colour.
       */
      inline Colour purple_colour()
      { return Colour(0x8e35ef); }

      /**
       * @return Returns a white colour.
       */
      inline Colour white_colour()
      { return Colour(0xffffff); }

      /**
       * @return Returns a milk white colour.
       */
      inline Colour milk_white_colour()
      { return Colour(0xfefcff); }

      /**
       * @return Returns a gray colour.
       */
      inline Colour gray_colour()
      { return Colour(0x808080); }

      /**
       * @return Returns a light gray colour.
       */
      inline Colour light_gray_colour()
      { return Colour(0xd3d3d3); }

      /**
       * @return Returns a dark gray colour.
       */
      inline Colour dark_gray_colour()
      { return Colour(0x0c090a); }

      /**
       * @return Returns a white smoke (very light) gray colour.
       */
      inline Colour whitesmoke_gray_colour()
      { return Colour(0xf5f5f5); }

      /**
       * @return Returns a black colour.
       */
      inline Colour black_colour()
      { return Colour(0, false); }

      /**
       * @return Returns a completely transparent colour.
       */
      inline Colour transparent_colour()
      { return Colour(0, true); }

      /**
       * @return Returns the RGB equivalent of the specified grayscale.
       *
       * @param grayscale Specifies the grayscale as a range between 0 and 1.
       */
      inline Colour grayscale_colour(double grayscale)
      {
         byte channel_range(static_cast<byte>(0xff * grayscale));
         return Colour(channel_range, channel_range, channel_range); 
      }
   };
};


/**
 * Overloads the insertion operators to output a colour to a stream.
 *
 * @param out Specifies the output stream.
 *
 * @param colour Specifies the colour to output.
 */
std::ostream &operator <<(std::ostream &out, Csi::Graphics::Colour const &colour);
std::wostream &operator <<(std::wostream &out, Csi::Graphics::Colour const &colour);


#endif
