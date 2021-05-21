/* Csi.DevConfig.SettingComp.TlsCertificate.h

   Copyright (C) 2011, 2019 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Tuesday 07 June 2011
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_TlsCertificate_h
#define Csi_DevConfig_SettingComp_TlsCertificate_h

#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include <list>


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines an object that acts as a description for a TLS certificate component.
          */
         class TlsCertificateDesc: public DescBase
         {
         public:
            /**
             * Constructor
             */
            TlsCertificateDesc():
               DescBase(Components::comp_tls_cert),
               requires_der(false)
            { }

            /**
             * @return Overloads the base class to create the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);
            
            /**
             * Overloads the base class version to initialise properties from the XML structure.
             */
            virtual void init_from_xml(
               Csi::Xml::Element &xml_data, StrAsc const &library_dir);
            
            /**
             * @return Returns true if the certificate should be sent in DER format.
             */
            bool get_requires_der() const
            { return requires_der; }

            /**
             * Overloads the base class version to write specific properties
             */
            virtual void describe_json(Csi::Json::Object &desc)
            {
               DescBase::describe_json(desc);
               desc.set_property_bool("requires_der", requires_der);
            }
            
         private:
            /**
             * Set to true if the certificate should be sent in DER format.
             */
            bool requires_der;
         };


         /**
          * Defines a component object that works with a TLS certificate.
          */
         class TlsCertificate: public CompBase
         {
         public:
            /**
             * Constructor
             */
            TlsCertificate(SharedPtr<DescBase> &desc_);

            /**
             * Destructor
             */
            virtual ~TlsCertificate();

            /**
             * Overloads the base class version to read the message.
             */
            virtual void read(SharedPtr<Message> &message);

            /**
             * Overloads the base class version to write to the specified message.
             */
            virtual void write(SharedPtr<Message> &message);

            /**
             * Overloads the base class version to add to the specified JSON components array.
             */
            virtual void write(Json::Array &json);

            /**
             * Overloads the base class version to read the value from the specified array iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current);
            
            /**
             * Overloads the base class version to format the component to the specified stream.
             */
            virtual void output(std::ostream &out, bool translate);
            virtual void output(std::wostream &out, bool translate);

            /**
             * Overloads the base class version to parse the component from the specified stream.
             */
            virtual void input(std::istream &in, bool translate);

            virtual StrAsc get_val_str();

            virtual void set_val_str(StrAsc const &s, bool check);

            // @group: methods to act as a container for chained certificates

            /**
             * @return Returns an iterator to the first certificate in the chain.
             */
            typedef StrBin value_type;
            typedef std::list<value_type> certs_type;
            typedef certs_type::iterator iterator;
            typedef certs_type::const_iterator const_iterator;
            iterator begin()
            { return certs.begin(); }
            const_iterator begin() const
            { return certs.begin(); }

            /**
             * @return Returns an iterator beyond the first certitficate in the chain.
             */
            iterator end()
            { return certs.end(); }
            const_iterator end() const
            { return certs.end(); }

            /**
             * @return Returns true if there are no certificates in the chain.
             */
            bool empty() const
            { return certs.empty(); }
            
            /**
             * Removes all certificates from the chain.
             */
            void clear()
            { certs.clear(); }

            /**
             * Adds a new certificate to the end of the chain.
             */
            void push_back(value_type const &cert)
            { certs.push_back(cert); }

            /**
             * Adds a new certificate at the top of the chain.
             */
            void push_front(value_type const &cert)
            { certs.push_front(cert); }

            /**
             * @return Returns the first certificate in the chain.
             */
            value_type &front()
            { return certs.front(); }
            value_type const &front() const
            { return certs.front(); }

            /**
             * @return Returns the last certificate in the chain.
             */
            value_type &back()
            { return certs.back(); }
            value_type const &back() const
            { return certs.back(); }

            /**
             * Removes the first certificate in the chain.
             */
            void pop_front()
            { certs.pop_front(); }

            /**
             * Removes the last certificate in the chain.
             */
            void pop_back()
            { certs.pop_back(); }
            
            // @endgroup:

            /**
             * Parses the input stream in PEM format.
             */
            void read_pem(std::istream &in);

            /**
             * @return Returns the length of all of the certificates that are stored.
             */
            size_t get_content_len() const
            {
               size_t rtn(0);
               for(const_iterator ci = begin(); ci != end(); ++ci)
                  rtn += ci->length();
               return rtn;
            }
            
         protected:
            /**
             * Overloads the base class version to copy the other component.
             */
            virtual void do_copy(CompBase *other);

         private:
            /**
             * Specifies the collection of certificates managed by this setting.
             */
            certs_type certs;
         };
      };
   };
};


#endif

