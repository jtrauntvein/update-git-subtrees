/* Csi.DevConfig.SettingComp.TlsKey.h

   Copyright (C) 2011, 2018 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Monday 06 June 2011
   Last Change: Thursday 01 March 2018
   Last Commit: $Date: 2018-03-01 14:40:39 -0600 (Thu, 01 Mar 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_TlsKey_h
#define Csi_DevConfig_SettingComp_TlsKey_h

#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.PolySharedPtr.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines a description for a TLS private key setting component.
          */
         class TlsKeyDesc: public DescBase
         {
         public:
            /**
             * Default Constructor
             */
            TlsKeyDesc():
               DescBase(Components::comp_tls_key)
            { }

            /**
             * @return Overloads the base class version to create the appropriate component object.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);
         };


         /**
          * Defines a setting component that represents the contents of a TLS private key.
          */
         class TlsKey: public CompBase
         {
         public:
            /**
             * Constructor.
             *
             * @param desc_ Specifies the component description.
             */
            TlsKey(SharedPtr<DescBase> &desc_);

            /**
             * Destructor
             */
            virtual ~TlsKey();

            /**
             * Overloads the base class version to read the content from the specified message.
             */
            virtual void read(SharedPtr<Message> &message);

            /**
             * Overloads the base class version to write the content to the specified message.
             */
            virtual void write(SharedPtr<Message> &message);

            /**
             * Overloads the base class version to append the component to the specified array.
             */
            virtual void write(Json::Array &json);

            /**
             * Overloads the base class version to read the component from the specified array
             * iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current);
            
            /**
             * Overloads the base class version to write the content as text to the specified
             * stream.
             */
            virtual void output(std::ostream &out, bool translate);
            virtual void output(std::wostream &out, bool translate);

            /**
             * Overloads the base class version to read the content from the specified stream.
             */
            virtual void input(std::istream &in, bool translate);

            /**
             * @return Returns the key formatted in PEM format.
             */
            virtual StrAsc get_val_str();

            /**
             * Overloads the base class version to set the component value as a string.
             */
            virtual void set_val_str(StrAsc const &s, bool check);

            /**
             * @return Returns the key as PEM format.
             */
            StrAsc const &get_content() const
            { return content; }
            
         protected:
            /**
             * Overloads the base class version to copy the content from other.
             */
            virtual void do_copy(CompBase *other);

         private:
            /**
             * Specifies the content of the private key.
             */
            StrAsc content;
         };
      };
   };
};


#endif

