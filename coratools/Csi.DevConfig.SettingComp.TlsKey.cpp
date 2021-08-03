/* Csi.DevConfig.SettingComp.TlsKey.cpp

   Copyright (C) 2011, 2020 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Monday 06 June 2011
   Last Change: Thursday 02 July 2020
   Last Commit: $Date: 2020-07-02 09:50:06 -0600 (Thu, 02 Jul 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.TlsKey.h"
#include "Csi.Base64.h"
#include "Csi.BuffStream.h"
#include "Csi.StrAscStream.h"
#include "coratools.strings.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         CompBase *TlsKeyDesc::make_component(
            SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous_comp)
         { return new TlsKey(desc); }

         
         TlsKey::TlsKey(SharedPtr<DescBase> &desc_):
            CompBase(desc_)
         { }


         TlsKey::~TlsKey()
         { }
         
         
         namespace
         {
            StrAsc const begin_rsa_name("-----BEGIN RSA PRIVATE KEY-----");
            StrAsc const end_rsa_name("-----END RSA PRIVATE KEY-----");
            StrAsc const begin_ec_name("-----BEGIN EC PRIVATE KEY-----");
            StrAsc const end_ec_name("-----END EC PRIVATE KEY-----");
            StrAsc const begin_pkcs8_encrypted_name("-----BEGIN ENCRYPTED PRIVATE KEY-----");
            StrAsc const end_pkcs8_encrypted_name("-----END ENCRYPTED PRIVATE KEY-----");
            StrAsc const begin_pkcs8_name("-----BEGIN PRIVATE KEY-----");
            StrAsc const end_pkcs8_name("-----END PRIVATE KEY-----");
         };
         
         
         void TlsKey::read(SharedPtr<Message> &message)
         {
            uint2 content_len(message->readUInt2());
            StrBin bytes;
            message->readBytes(bytes, content_len, false);
            content.setContents(bytes.getContents(), bytes.length());
         } // read
         
         
         void TlsKey::write(SharedPtr<Message> &message)
         {
            message->addUInt2(static_cast<uint2>(content.length()));
            message->addBytes(content.c_str(), (uint4)content.length());
         } // write


         void TlsKey::write(Json::Array &json)
         {
            OStrAscStream temp;
            output(temp, true);
            json.push_back(new Json::String(temp.str()));
         } // write


         Json::Array::iterator TlsKey::read(Json::Array::iterator current)
         {
            Json::StringHandle value_json(*current);
            IBuffStream temp(value_json->get_value().c_str(), value_json->get_value().length());
            input(temp, true);
            return ++current;
         } // read
         
         
         void TlsKey::output(std::ostream &out, bool translate)
         { out << content; }
         
         
         void TlsKey::output(std::wostream &out, bool translate)
         { out << content; }
         
         
         void TlsKey::input(std::istream &in, bool translate)
         {
            // we will use a state machine to parse and validate the contents of the stream.
            using namespace TlsKeyStrings;
            StrAsc temp;
            enum state_type
            {
               state_before_header,
               state_in_header,
               state_rsa_header,
               state_pkcs8_header,
               state_pkcs8_encrypted_header,
               state_ec_header,
               state_in_footer,
               state_after_footer
            } state = state_before_header;
            char ch;
            char prev_ch = 0;
            state_type header_state = state_before_header;
            StrAsc value;

            while(state != state_after_footer && in.get(ch))
            {
               switch(state)
               {
               case state_before_header:
                  if(ch == '-')
                  {
                     temp.append(ch);
                     value.append(ch);
                     state = state_in_header;
                  }
                  else if(!isspace(ch))
                     throw std::invalid_argument(my_strings[strid_invalid_pem_format].c_str());
                  break;

               case state_in_header:
                  value.append(ch);
                  if(ch == '\n' || ch == '\r')
                  {
                     if(temp == begin_rsa_name)
                        state = state_rsa_header;
                     else if(temp == begin_pkcs8_name)
                        state = state_pkcs8_header;
                     else if(temp == begin_pkcs8_encrypted_name)
                        state = state_pkcs8_encrypted_header;
                     else if(temp == begin_ec_name)
                        state = state_ec_header;
                     else
                        throw std::invalid_argument(my_strings[strid_invalid_pem_format].c_str());
                     header_state = state;
                     temp.cut(0);
                  }
                  else
                     temp.append(ch);
                  break;

               case state_rsa_header:
               case state_pkcs8_header:
               case state_pkcs8_encrypted_header:
               case state_ec_header:
                  value.append(ch);
                  if(ch == '-' && (prev_ch == '\r' || prev_ch == '\n'))
                  {
                     temp.cut(0);
                     temp.append(ch);
                     state = state_in_footer;
                  }
                  break;

               case state_in_footer:
                  value.append(ch);
                  if(ch == '\n' || ch == '\r')
                     state = state_after_footer;
                  else
                     temp.append(ch);
                  break;
               }
               prev_ch = ch;
            }

            // if we made it all the way through, we will need to compare the footer against the
            // header
            if(state >= state_in_footer)
            {
               switch(header_state)
               {
               case state_rsa_header:
                  if(temp != end_rsa_name)
                     throw std::invalid_argument(my_strings[strid_invalid_pem_format].c_str());
                  break;

               case state_ec_header:
                  if(temp != end_ec_name)
                     throw std::invalid_argument(my_strings[strid_invalid_pem_format].c_str());
                  break;

               case state_pkcs8_header:
                  if(temp != end_pkcs8_name)
                     throw std::invalid_argument(my_strings[strid_invalid_pem_format].c_str());
                  break;

               case state_pkcs8_encrypted_header:
                  if(temp != end_pkcs8_encrypted_name)
                     throw std::invalid_argument(my_strings[strid_invalid_pem_format].c_str());
                  break;
               }
            }
            else if(state != state_before_header)
               throw std::invalid_argument(my_strings[strid_invalid_pem_format].c_str());
            content = value;
            set_has_changed(true);
         } // input
         
         
         void TlsKey::set_val_str(StrAsc const &s, bool check)
         {
            Csi::IBuffStream temp(s.c_str(), s.length());
            temp.imbue(std::locale::classic());
            input(temp, false);
         } // set_val_str
         
         
         StrAsc TlsKey::get_val_str()
         { return content; }
         
         
         void TlsKey::do_copy(CompBase *other_)
         {
            TlsKey *other(dynamic_cast<TlsKey *>(other_));
            if(other)
               content = other->content;
         } // do_copy
      };
   };
};
