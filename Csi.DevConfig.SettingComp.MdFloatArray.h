/* Csi.DevConfig.SettingComp.MdFloatArray.h

   Copyright (C) 2011, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 21 December 2011
   Last Change: Thursday 01 March 2018
   Last Commit: $Date: 2018-03-01 14:40:39 -0600 (Thu, 01 Mar 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_MdFloatArray_h
#define Csi_DevConfig_SettingComp_MdFloatArray_h


#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.ArrayDimensions.h"
#include <vector>


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines an object that describes an MD Float array component.
          */
         class MdFloatArrayDesc: public DescBase
         {
         public:
            /**
             * Constructor
             */
            MdFloatArrayDesc():
               DescBase(Components::comp_md_float_array)
            { }

            /**
             * @return Overloads the base class version to create the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);
         };


         /**
          * Defines a component object that represents a dynamically dimensioned multi-dimensional
          * array of floating point values.
          */
         class MdFloatArray: public CompBase
         {
         public:
            /**
             * Constructor
             */
            MdFloatArray(SharedPtr<DescBase> &desc);

            /**
             * Destructor
             */
            virtual ~MdFloatArray();

            /**
             * Overloads the base class version to read component values from the message.
             */
            virtual void read(SharedPtr<Message> &message);

            /**
             * Overloads the base class verion to write the component values to the message.
             */
            virtual void write(SharedPtr<Message> &message);

            /**
             * Overloads the base class version to write the component values to the JSON array as
             * an object.
             */
            virtual void write(Json::Array &json);

            /**
             * Overloads the base class version to read the component from the specified array
             * iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current);
            
            /**
             * Overloads the base class version to format the component values to the specified
             * stream.  The format used will match that used by the HP48 calculator to represent a
             * matrix.
             */
            virtual void output(std::ostream &out, bool translate);
            virtual void output(std::wostream &out, bool translate);

            /**
             * Overloads the base class version to parse the component values from the specified
             * stream.
             */
            virtual void input(std::istream &in, bool translate);

            // @group: declarations to act as a container of values

            /**
             * @return Returns an iterator to the first value.
             */
            typedef std::vector<float> values_type;
            typedef values_type::iterator iterator;
            typedef values_type::const_iterator const_iterator;
            iterator begin()
            { return values.begin(); }
            const_iterator begin() const
            { return values.begin(); }

            /**
             * @return Returns an iterator beyond the last value.
             */
            iterator end()
            { return values.end(); }
            const_iterator end() const
            { return values.end(); }

            /**
             * @return Returns true if there are no values stored.
             */
            bool empty() const
            { return values.empty(); }

            /**
             * @return Returns the number of values in the array.
             */
            typedef values_type::size_type size_type;
            size_type size() const
            { return values.size(); }

            /**
             * @return Returns the value stored at the specified address.
             *
             * @param address Specifies the address of the value as a row major format index.
             */
            typedef std::vector<uint4> address_type;
            iterator value_at_address(address_type const &address);
            const_iterator value_at_address(address_type const &address) const;
            
            // @endgroup:

            /**
             * @return Returns the dimensions for this array.
             */
            ArrayDimensions const &get_dimensions() const
            { return dimensions; }

         protected:
            /**
             * Overloads the base class version to copy the component.
             */
            virtual void do_copy(CompBase *other);
            
         private:
            /**
             * Specifies the array of values.  This is kept as a linear array that is indexed using
             * row major convention.
             */
            values_type values;

            /**
             * Specifies the size of each dimension.
             */
            ArrayDimensions dimensions;
         };
      };
   };
};


#endif
