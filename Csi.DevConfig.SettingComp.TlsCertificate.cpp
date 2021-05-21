/* Csi.DevConfig.SettingComp.TlsCertificate.cpp

   Copyright (C) 2011, 2018 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Tuesday 07 June 2011
   Last Change: Thursday 01 March 2018
   Last Commit: $Date: 2018-03-01 14:40:39 -0600 (Thu, 01 Mar 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.TlsCertificate.h"
#include "Csi.StrAscStream.h"
#include "Csi.BuffStream.h"
#include "Csi.Base64.h"
#include "Csi.PolySharedPtr.h"
#include "coratools.strings.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         CompBase *TlsCertificateDesc::make_component(
            SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous)
         { return new TlsCertificate(desc); }


         void TlsCertificateDesc::init_from_xml(
            Csi::Xml::Element &xml_data, StrAsc const &library_dir)
         {
            static StrUni const requires_der_name(L"requires-der");
            if(xml_data.has_attribute(requires_der_name))
               requires_der = xml_data.get_attr_bool(requires_der_name);
            else
               requires_der = false;
            DescBase::init_from_xml(xml_data, library_dir);
         } // init_from_xml


         TlsCertificate::TlsCertificate(SharedPtr<DescBase> &desc):
            CompBase(desc)
         { }


         TlsCertificate::~TlsCertificate()
         { certs.clear(); }


         namespace
         {
            StrAsc const begin_cert_name("-----BEGIN CERTIFICATE-----");
            StrAsc const end_cert_name("-----END CERTIFICATE-----");
            StrAsc const begin_name("-----BEGIN");
         };

         
         void TlsCertificate::read(SharedPtr<Message> &message)
         {
            uint2 cert_len = message->readUInt2();
            StrBin cert;
            
            certs.clear();
            while(cert_len > 0)
            {
               cert.cut(0);
               message->readBytes(cert, cert_len);
               if(cert.find(begin_cert_name.c_str(), begin_cert_name.length()) < cert.length())
               {
                  IBuffStream temp(cert.getContents(), cert.length());
                  read_pem(temp);
                  break;
               }
               else
                  certs.push_back(cert);
               if(message->whatsLeft() >= 2)
                  cert_len = message->readUInt2();
               else
                  cert_len = 0;
            }
         } // read


         void TlsCertificate::write(SharedPtr<Message> &message)
         {
            PolySharedPtr<DescBase, TlsCertificateDesc> cert_desc(get_desc());
            if(cert_desc->get_requires_der())
            {
               for(const_iterator ci = begin(); ci != end(); ++ci)
               {
                  value_type const &cert(*ci);
                  message->addUInt2(static_cast<uint2>(cert.length()));
                  message->addBytes(cert.getContents(), (uint4)cert.length());
               }
               message->addUInt2(0); // add the terminator
            }
            else
            {
               Csi::OStrAscStream temp;
               output(temp, false);
               if(temp.str().length() > 0)
               {
                  message->addUInt2(static_cast<uint2>(temp.str().length()));
                  message->addBytes(temp.str().c_str(), (uint4)temp.str().length());
               }
               message->addUInt2(0); // add the terminator
            }
         } // write


         void TlsCertificate::write(Json::Array &json)
         {
            OStrAscStream temp;
            output(temp, true);
            json.push_back(new Json::String(temp.str()));
         } // write


         Json::Array::iterator TlsCertificate::read(Json::Array::iterator current)
         {
            Json::StringHandle value_json(*current);
            IBuffStream temp(value_json->get_value().c_str(), value_json->get_value().length());
            input(temp, true);
            return ++current;
         } // read

         
         void TlsCertificate::output(std::ostream &out, bool translate)
         {
            for(const_iterator ci = begin(); ci != end(); ++ci)
            {
               value_type const &cert(*ci);
               out << begin_cert_name << "\n";
               Base64::encode(out, cert.getContents(), cert.length());
               out << "\n" << end_cert_name << "\n";
            }
         } // output


         void TlsCertificate::output(std::wostream &out, bool translate)
         {
            for(const_iterator ci = begin(); ci != end(); ++ci)
            {
               value_type const &cert(*ci);
               out << begin_cert_name << "\n";
               Base64::encode(out, cert.getContents(), cert.length());
               out << end_cert_name << "\n";
            }
         } // output

         
         void TlsCertificate::input(std::istream  &in, bool translate)
         {
            read_pem(in);
            set_has_changed(true);
         } // input


         StrAsc TlsCertificate::get_val_str()
         {
            OStrAscStream temp;
            output(temp, false);
            return temp.str();
         }

         void TlsCertificate::set_val_str(StrAsc const &s, bool check)
         {
            IBuffStream temp(s.c_str(), s.length());
            input(temp, false); 
         } // set_val_str


         void TlsCertificate::do_copy(CompBase *other_)
         {
            TlsCertificate *other(dynamic_cast<TlsCertificate *>(other_));
            if(other)
               certs = other->certs;
         } // do_copy
         

         void TlsCertificate::read_pem(std::istream &in)
         {
            // we will input the stream into a buffer which can then be parsed.
            using namespace TlsCertificateStrings;
            StrBin content;
            char temp[1024];
            while(in)
            {
               in.read(temp, sizeof(temp));
               if(in.gcount() > 0)
                  content.append(temp, static_cast<size_t>(in.gcount()));
            }

            // we will now clear our list of certificates and iterate through the content
            size_t cert_begin_pos(0);
            StrBin cert;
            
            certs.clear();
            while(cert_begin_pos < content.length())
            {
               // search for the beginning of this cert
               cert_begin_pos = content.find(
                  begin_cert_name.c_str(), begin_cert_name.length(), cert_begin_pos);
               if(cert_begin_pos < content.length())
               {
                  size_t content_start_pos(cert_begin_pos + begin_cert_name.length());
                  size_t cert_end_pos(
                     content.find(
                        end_cert_name.c_str(),
                        end_cert_name.length(),
                        content_start_pos));
                  size_t content_len;
                  
                  if(cert_end_pos >= content.length())
                     throw std::invalid_argument(my_strings[strid_begin_cert_without_end].c_str());
                  content_len = cert_end_pos - content_start_pos;
                  cert.cut(0);
                  Base64::decode(cert, content.getContents() + content_start_pos, content_len);
                  certs.push_back(cert);
                  cert_begin_pos = cert_end_pos + end_cert_name.length();
               }
               else
                  break;
            }
            if(certs.empty() && content.length() > 0)
            {
               size_t begin_pos(content.find(begin_name.c_str(), begin_name.length()));
               if(begin_pos < content.length())
                  throw std::invalid_argument(my_strings[strid_invalid_pem_format].c_str());
               else
                  throw std::invalid_argument(my_strings[strid_not_pem_encoded].c_str());
            }
         } // read_pem
         
      };
   };
};
