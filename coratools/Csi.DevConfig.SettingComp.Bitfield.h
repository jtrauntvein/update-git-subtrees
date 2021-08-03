/* Csi.DevConfig.SettingComp.Bitfield.h

   Copyright (C) 2004, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 24 January 2004
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_Bitfield_h
#define Csi_DevConfig_SettingComp_Bitfield_h

#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.MsgExcept.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines an object that describes a bit-field component.  This type of component shares
          * storage with adjacent bitfields.
          */
         class BitfieldDesc: public DescBase
         {
         private:
            /**
             * Specifies the size of this field in terms of number of bits.
             */
            uint4 size;

            /**
             * Specifies the mask for this bitfield.
             */
            uint4 mask;

            /**
             * Set to true if this bitfield grows from the most significant bit rather than the
             * least significant bit.
             */
            bool reverse;

            /**
             * Set to true of this bitfield should be hidden.
             */
            bool hidden;

            /**
             * Specifies the minimum minor version required for this bitfield to be shown.
             */
            byte min_minor;
            
         public:
            /**
             * Default constructor.
             */
            BitfieldDesc():
               DescBase(Components::comp_bitfield),
               size(0),
               mask(0),
               reverse(false),
               hidden(false),
               min_minor(0)
            { }

            /**
             * @return Returns the size of this field in terms of number of bits.
             */
            uint4 get_size() const
            { return size; }

            /**
             * @return Returns the mask for this field in the shared storage.
             */
            uint4 get_mask() const
            { return mask; }

            /**
             * @return Returns the maximum value allowed for this field.
             */
            uint4 get_max_value() const
            { return mask; }

            /**
             * @return Returns true if this bitfield has been configured to count from the most
             * significant bit rather than the least significant.
             */
            bool get_reverse() const
            { return reverse; }

            /**
             * @return Returns true if the device description declares this bit field as hidden.
             */
            bool get_hidden() const
            { return hidden; }

            /**
             * @return Overloads the base class version to let the hidden flag control the presence
             * in the summary.
             */
            virtual bool present_in_summary() const
            { return !hidden; }

            /**
             * @return Returns the required minimum minor version for this bitfield to be shown.
             */
            byte get_min_minor() const
            { return min_minor; }
            
            /**
             * @return Overrides the base class version to generate the bitfield component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * Overloads the base class version to parse specified definition parameters.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data,
               StrAsc const &library_dir)
            {
               DescBase::init_from_xml(xml_data,library_dir);
               size = xml_data.get_attr_uint4(L"size");
               if(size >= 32)
                  throw std::out_of_range("unsupported bitfield size");
               mask = (1 << size) - 1;
               reverse = hidden = false;
               if(xml_data.has_attribute(L"reverse"))
                  reverse = xml_data.get_attr_bool(L"reverse");
               if(xml_data.has_attribute(L"hidden"))
                  hidden = xml_data.get_attr_bool(L"hidden");
               if(xml_data.has_attribute(L"min-minor"))
                  min_minor = static_cast<byte>(xml_data.get_attr_uint2(L"min-minor"));
            }

            /**
             * @return Overloaded to return true to indicate that there is a maximum value for this
             * bitfield.
             */
            virtual bool has_maxima() const
            { return true; }

            /**
             * Overloaded to return the maximum size for the number of bits.
             */
            virtual void format_max(std::ostream &out) const
            {
               uint4 rtn(1);
               for(uint4 i = 0; i < size; ++i)
                  rtn *= 2;
               out << (rtn - 1);
            }

            /**
             * Overloaded to print out 0 as the minimum value.
             */
            virtual void format_min(std::ostream &out) const
            { out << 0; }

            /**
             * Overloads the base class version to write the properties of this component type.
             */
            virtual void describe_json(Csi::Json::Object &desc_json)
            {
               DescBase::describe_json(desc_json);
               desc_json.set_property_number("size", size);
               desc_json.set_property_number("mask", mask);
               desc_json.set_property_bool("reverse", reverse);
               desc_json.set_property_bool("hidden", hidden);
               desc_json.set_property_number("min_minor", min_minor);
            }
         };


         /**
          * Defines a buffer object that is shared between adjacent bitfields and represents the
          * value a read from or written to the device.
          */
         class BitfieldBuffer
         {
         private:
            /**
             * Specifies the total number of bits that that are actually used.
             */
            uint4 overall_size;

            /**
             * Represents the value that was read from the device.
             */
            uint4 storage;

         public:
            /**
             * @param overall_size_ Specifies the total number of bits that this buffer will
             * represent.
             */
            BitfieldBuffer(uint4 overall_size_):
               overall_size(overall_size_),
               storage(0)
            {
               if(overall_size > 32)
                  throw std::out_of_range("Unsupported bitfield size");
            }

            /**
             * Increases the allocation of bits being used by the specified factor.
             *
             * @param additional Specifies the number of bits to increase.
             */
            void increase_overall_size(uint4 additional)
            {
               if(overall_size + additional > 32)
                  throw std::out_of_range("Unsupported bitfield_size");
               overall_size += additional;
            }

            /**
             * @return Returns the value associated with the specified mask and shift.
             */
            uint4 extract_field(uint4 mask, uint4 shift)
            {
               uint4 rtn;
               uint4 read_mask = mask << shift;
               rtn = (storage & read_mask) >> shift;
               return rtn;
            }

            /**
             * Sets the bits for the specified value at the position signified by the mask and
             * shift.
             */
            void insert_field(uint4 value, uint4 mask, uint4 shift)
            {
               uint4 write_mask = mask << shift;
               if((value & ~mask) != 0)
                  throw std::out_of_range("bitfield value out of range");
               storage &= ~write_mask;
               storage |= (value << shift);
            }

            /**
             * Overloaded to write the actual representation of this buffer.
             */
            void write(SharedPtr<Message> &out)
            {
               if(overall_size <= 8)
                  out->addByte(static_cast<uint1>(storage));
               else if(overall_size <= 16)
                  out->addUInt2(static_cast<uint2>(storage));
               else
                  out->addUInt4(storage);
            }

            /**
             * Overloaded to read the actual representation of this buffer.
             */
            void read(SharedPtr<Message> &in)
            {
               if(overall_size <= 8)
                  storage = in->readByte();
               else if(overall_size <= 16)
                  storage = in->readUInt2();
               else
                  storage = in->readUInt4();
            }

            /**
             * @return Returns the shared value.
             */
            uint4 get_storage() const
            { return storage; }

            /**
             * @param value Specifies the new shared value.
             */
            void set_storage(uint4 value)
            { storage = value; }
         };
         

         /**
          * Defines an object that represents a bitfield setting component.
          */
         class Bitfield: public CompBase
         {
         private:
            /**
             * Specifies the field immediately preceding this field.
             */
            PolySharedPtr<CompBase, Bitfield> previous_field;

            /**
             * Specifies the description for this bitfield..
             */
            PolySharedPtr<DescBase, BitfieldDesc> desc;

            /**
             * Specifies the number of bits that need to be shifted to reach this field's value.
             */
            uint4 shift;

            /**
             * Specifies the bitfield buffer shared with adjacent bitfields.
             */
            SharedPtr<BitfieldBuffer> buffer;
            
         public:
            /**
             * @param desc Specifies the description for this bitfield.
             *
             * @param previous_component Specifies the component that preceded this component.
             */
            Bitfield(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * Destructor
             */
            virtual ~Bitfield()
            {
               buffer.clear();
               previous_field.clear();
            }

            /**
             * @return Returns true if this bit field should be hidden based upon the description
             * and the specified minor version.
             */
            bool get_hidden() const
            {
               bool rtn(desc->get_hidden());
               if(!rtn && desc->get_min_minor() > minor_version)
                  rtn = true;
               return rtn;
            }
            
            /**
             * Reads the buffer from the message only if this is the first bitfield in the sequence.
             */
            virtual void read(SharedPtr<Message> &in)
            {
               if(previous_field == 0)
                  buffer->read(in);
            }

            /**
             * Writes the buffer to the message only if this is the first bitfield in the sequence.
             */
            virtual void write(SharedPtr<Message> &out)
            {
               if(previous_field == 0)
                  buffer->write(out);
            }

            /**
             * Overloaded to read the value for this field from the specified stream.'
             */
            virtual void input(std::istream &in, bool translate)
            {
               uint4 temp;
               if(in >> temp)
               {
                  buffer->insert_field(temp,desc->get_mask(),shift);
                  has_changed = true;
               }
               else
                  throw MsgExcept("stream read error");
            }

            /**
             * Overloaded to write the value of this bitfield to the specified stream.
             */
            virtual void output(std::ostream &out, bool translate)
            {
               out << buffer->extract_field(desc->get_mask(),shift);
            }
            virtual void output(std::wostream &out, bool translate)
            { out << buffer->extract_field(desc->get_mask(),shift); }

            /**
             * Overloads the base class version to add to the array only if this is the first
             * bitfield value in the sequence
             */
            virtual void write(Json::Array &json)
            {
               if(previous_field == 0)
                  json.push_back(new Json::Number(buffer->get_storage())); 
            }

            /**
             * Overloads the base class to load the shared value from the array only if there is no
             * previous bitfield.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               Json::Array::iterator rtn(current);
               if(previous_field == 0)
               {
                  Json::NumberHandle value_json(*current);
                  buffer->set_storage((uint4)value_json->get_value());
                  ++rtn;
               }
               return rtn;
            }
            
            virtual byte get_val_byte()
            { return static_cast<byte>(get_val_uint4()); }

            virtual void set_val_byte(byte val)
            { set_val_uint4(val); }
            
            virtual int2 get_val_int2()
            { return static_cast<int2>(get_val_uint4()); }

            virtual void set_val_int2(int2 val)
            { set_val_uint4(static_cast<uint4>(val)); }

            virtual uint2 get_val_uint2()
            { return static_cast<uint2>(get_val_uint4()); }

            virtual void set_val_uint2(uint2 val)
            { set_val_uint4(val); }

            virtual int4 get_val_int4()
            { return static_cast<int4>(get_val_uint4()); }

            virtual void set_val_int4(int4 val)
            { set_val_uint4(static_cast<uint4>(val)); }

            virtual uint4 get_val_uint4()
            { return buffer->extract_field(desc->get_mask(), shift); }

            virtual void set_val_uint4(uint4 val)
            {
               if(val >= 0 && val <= desc->get_max_value())
               {
                  buffer->insert_field(val, desc->get_mask(),shift);
                  has_changed = true;
               }
               else
                  throw std::out_of_range(get_name().c_str());
            }
            
            virtual float get_val_float()
            { return static_cast<float>(get_val_uint4()); }

            virtual void set_val_float(float val)
            { set_val_uint4(static_cast<uint4>(val)); }
            
            virtual double get_val_double()
            { return buffer->extract_field(desc->get_mask(), shift); }

            virtual void set_val_double(double val)
            { set_val_uint4(static_cast<uint4>(val)); }
            
         protected:
            /**
             * Overloads the copy method tocopy the other field.
             */
            virtual void do_copy(CompBase *other_)
            {
               // the invoking code should have already checked the
               // description, this method also needs to check that the shift
               // is the same as well.
               Bitfield *other = static_cast<Bitfield *>(other_);
               if(other->shift == shift)
               {
                  buffer->insert_field(
                     other->buffer->extract_field(
                        desc->get_mask(),
                        shift),
                     desc->get_mask(),
                     shift);
               }
               else
                  throw std::invalid_argument("Wrong bitfield shift value");
            } // do_copy
         };


         /**
          * @return Returns a bitfield component based upon the description and previous component.
          */
         inline CompBase *BitfieldDesc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new Bitfield(desc,previous_component); }
      };
   };
};


#endif
