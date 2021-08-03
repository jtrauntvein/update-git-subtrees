/* Csi.Graphics.Font.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 14 January 2015
   Last Change: Monday 21 December 2015
   Last Commit: $Date: 2016-06-28 16:50:43 -0600 (Tue, 28 Jun 2016) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_Font_h
#define Csi_Graphics_Font_h

#include "CsiTypeDefs.h"
#include "StrUni.h"
#include "Csi.Xml.Element.h"


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines an object that can be used to select a font.
       */
      class FontInfo
      {
      public:
         /**
          * Specifies the point size of this font in units of points (1/72 inch).
          */
         double point_size;

         /**
          * Specifies the angle in 1/10 degrees between the escapement vector
          * and the device x axis.  The escapement vector is parallel to the
          * text base line.
          */
         int4 escapement;

         /**
          * Specifies the angle in 1/10 degrees between the each character
          * baseline and the device x-axis.
          */
         int4 orientation;

         /**
          * Specifies the weight of the font as an integer between 0 and 1000.
          * A value of zero implies that the default width will be used.  This
          * can be one of the value defined in the weights enumeration.
          */
         int4 weight;

         /**
          * Enumerates common weight values.
          */
         enum common_weights
         {
            weight_default = 0,
            weight_thin = 100,
            weight_extra_light = 200,
            weight_light = 300,
            weight_normal = 400,
            weight_medium = 500,
            weight_semi_bold = 600,
            weight_bold = 700,
            weight_extra_bold = 800,
            weight_heavy = 900
         };

         /**
          * Set to true if the font is in italics.
          */
         bool italics;

         /**
          * Set to true if the font is to be underlined.
          */
         bool underline;

         /**
          * Set to true if the font is to be struck out.
          */
         bool strikeout;

         /**
          * Specifies the character set as an implementation dependent code.
          */
         byte character_set;

         /**
          * Specifies the precision at which the selected values must match the
          * selected font.
          */
         enum precision_type
         {
            precision_default = 0,
            precision_raster = 6,
            precision_string = 1
         } precision;

         /**
          * Specifies the clipping precision.
          */
         enum clip_precision_type
         {
            clip_default = 0,
            clip_character = 1,
            clip_stroke = 2
         } clip_precision;

         /**
          * Specifies the font quality
          */
         enum quality_type
         {
            quality_antialiased = 4,
            quality_nonantialiased = 3,
            quality_cleartype_compat = 6,
            quality_cleartype = 5,
            quality_default = 0,
            quality_draft = 1
         } quality;

         /**
          * Specifies the pitch of this font.
          */
         enum pitch_type
         {
            pitch_default = 0,
            pitch_fixed = 1,
            pitch_mono = 2,
            pitch_variable = 8
         } pitch;

         /**
          * Specifies the family for this font
          */
         enum family_type
         {
            family_decorative = 80,
            family_dont_care = 1,
            family_modern = 48,
            family_roman = 16,
            family_script = 64,
            family_swiss = 32
         } family;

         /**
          * Specifies the font face name.  If empty, the first font that
          * matches the other attributes is selected.
          */
         StrUni face_name;
         
      public:
         /**
          * Default constructor
          */
         FontInfo():
            face_name(L"Arial"),
            point_size(10),
            escapement(0),
            orientation(0),
            weight(weight_default),
            pitch(pitch_variable),
            family(family_swiss),
            italics(false),
            underline(false),
            strikeout(false),
            character_set(0),
            precision(precision_default),
            clip_precision(clip_default),
            quality(quality_default)
         { }

         /**
          * Copy constructor
          */
         FontInfo(FontInfo const &other):
            point_size(other.point_size),
            escapement(other.escapement),
            orientation(other.orientation),
            weight(other.weight),
            pitch(other.pitch),
            family(other.family),
            face_name(other.face_name),
            italics(other.italics),
            underline(other.underline),
            strikeout(other.strikeout),
            character_set(other.character_set),
            precision(other.precision),
            clip_precision(other.clip_precision),
            quality(other.quality)
         { }

         /**
          * Copy operator
          */
         FontInfo &operator =(FontInfo const &other)
         {
            do_copy(other);
            return *this;
         }
         
         /**
          * @return Returns the point size.
          */
         double get_point_size() const
         { return point_size; }

         /**
          * @param size Specifies the point size.
          */
         FontInfo &set_point_size(double size)
         {
            point_size = size;
            return *this;
         }

         /**
          * @return Returns the escapement value.
          */
         int4 get_escapement() const
         { return escapement; }
         
         /**
          * @param escapement_ Specifies the new value for the escapement.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_escapement(int4 escapement_)
         {
            escapement = escapement_;
            return *this;
         }

         /**
          * @return Returns the orientation.
          */
         int4 get_orientation() const
         { return orientation; }
         
         /**
          * @param orientation_ Speciifes the new value for the orientation angle.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_orientation(int4 orientation_)
         {
            orientation = orientation_;
            return *this;
         }

         /**
          * @param weight_ Specifies the new value for the weight.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_weight(int4 weight_)
         {
            weight = weight_;
            return *this;
         }

         /**
          * @return Returns the font weight.
          */
         int4 get_weight() const
         { return weight; }

         /**
          * @param italics_ Specifies the new value for italics.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_italics(bool italics_)
         {
            italics = italics_;
            return *this;
         }

         /**
          * @return Returns true if the font should use italics.
          */
         bool get_italics() const
         { return italics; }

         /**
          * @param underline_ Specifies the new value for underline.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_underline(bool underline_)
         {
            underline = underline_;
            return *this;
         }

         /**
          * @return Returns true if the text should be underlined.
          */
         bool get_underline() const
         { return underline; }

         /**
          * @param strikeout_ Specifies the new value for strikeout.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_strikeout(bool strikeout_)
         {
            strikeout = strikeout_;
            return *this;
         }

         /**
          * @return Returns tru if the text should be struck through.
          */
         bool get_strikeout() const
         { return strikeout; }

         /**
          * @return Returns the character set code.
          */
         byte get_character_set() const
         { return character_set; }
            
         /**
          * @param charset Specifies the new character set
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_character_set(byte charset)
         {
            character_set = charset;
            return *this;
         }

         /**
          * @return Returns the precision code.
          */
         precision_type get_precision() const
         { return precision; }

         /**
          * @param precision_ Specifies the new value for precision.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_precision(precision_type precision_)
         {
            precision = precision_;
            return *this;
         }

         /**
          * @return Returns the clip precision.
          */
         clip_precision_type get_clip_precision() const
         { return clip_precision; }

         /**
          * @param clip_precision_ Specifies the new value for clip_precision.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_clip_precision(clip_precision_type clip_precision_)
         {
            clip_precision = clip_precision_;
            return *this;
         }

         /**
          * @return Returns the quality.
          */
         quality_type get_quality() const
         { return quality; }

         /**
          * @param quality_ Specifies the new value for quality.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_quality(quality_type quality_)
         {
            quality = quality_;
            return *this;
         }

         /**
          * @return Returns the pitch.
          */
         pitch_type get_pitch() const
         { return pitch; }

         /**
          * @param pitch_ Specifies the new value for pitch.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_pitch(pitch_type pitch_)
         {
            pitch = pitch_;
            return *this;
         }

         /**
          * @return Returns the font family code.
          */
         family_type get_family() const
         { return family; }
         
         /**
          * @param family_ Specifies the new value for family.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_family(family_type family_)
         {
            family = family_;
            return *this;
         }

         /**
          * @param face_name_ Specifies the new value for the face name.
          *
          * @return Returns a reference to this structure.
          */
         FontInfo &set_face_name(StrUni const &face_name_)
         {
            face_name = face_name_;
            return *this;
         }

         /**
          * @return Returns the face name.
          */
         StrUni const get_face_name() const
         { return face_name; }

         /**
          * Writes the font description to the specified XML object.
          *
          * @param elem Specifies the destination XML object.
          */
         void write(Xml::Element &elem) const
         {
            elem.set_attr_double(point_size, L"point-size");
            elem.set_attr_int4(escapement, L"escapement");
            elem.set_attr_int4(orientation, L"orientation");
            elem.set_attr_int4(weight, L"weight");
            elem.set_attr_bool(italics, L"italics");
            elem.set_attr_bool(underline, L"underline");
            elem.set_attr_bool(strikeout, L"strikeout");
            elem.set_attr_uint4(precision, L"precision");
            elem.set_attr_uint4(clip_precision, L"clip-precision");
            elem.set_attr_uint4(quality, L"quality");
            elem.set_attr_uint4(pitch, L"pitch");
            elem.set_attr_uint4(family, L"family");
            elem.set_attr_wstr(face_name, L"face-name");
         }

         /**
          * Reads the font information from the specified XML object.
          *
          * @param elem Specifies the source XML object.
          */
         void read(Xml::Element &elem)
         {
            point_size = elem.get_attr_double(L"point-size");
            escapement = elem.get_attr_int4(L"escapement");
            orientation = elem.get_attr_int4(L"orientation");
            weight = elem.get_attr_int4(L"weight");
            italics = elem.get_attr_bool(L"italics");
            underline = elem.get_attr_bool(L"underline");
            strikeout = elem.get_attr_bool(L"strikeout");
            precision = static_cast<precision_type>(elem.get_attr_uint4(L"precision"));
            clip_precision = static_cast<clip_precision_type>(elem.get_attr_uint4(L"clip-precision"));
            pitch = static_cast<pitch_type>(elem.get_attr_uint4(L"pitch"));
            family = static_cast<family_type>(elem.get_attr_uint4(L"family"));
            face_name = elem.get_attr_wstr(L"face-name");
         }

      private:
         /**
          * Performs the copy operations
          */
         void do_copy(FontInfo const &other)
         {
            point_size = other.point_size;
            escapement = other.escapement;
            orientation = other.orientation;
            weight = other.weight;
            pitch = other.pitch;
            family = other.family;
            face_name = other.face_name;
            precision = other.precision;
            clip_precision = other.clip_precision;
            quality = other.quality;
            underline = other.underline;
            strikeout = other.strikeout;
            italics = other.italics;
         }
      };


      /**
       * Defines an object that represents a created font resource.  Specific
       * graphic libraries will have associated subclasses whose
       * implementations contain the specific font handle that can be
       * selected.
       */
      class Font
      {
      public:
         /**
          * Constructor.
          *
          * @param desc_ Specifies the descriptor for this font.
          */
         Font(FontInfo const &desc_):
            desc(desc_)
         { }
         
         /**
          * Defines a virtual destructor.
          */
         virtual ~Font()
         { }

         /**
          * @return Returns the information needed to create this font.
          */
         FontInfo const &get_desc() const
         { return desc; }

      protected:
         /**
          * Specifies the description required to build this font.
          */
         FontInfo desc;
      };
   };
};


#endif
