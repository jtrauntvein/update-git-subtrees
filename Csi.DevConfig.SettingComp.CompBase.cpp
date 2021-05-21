/* Csi.DevConfig.SettingComp.CompBase.cpp

   Copyright (C) 2003, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 24 December 2003
   Last Change: Monday 19 April 2021
   Last Commit: $Date: 2021-04-20 12:42:24 -0600 (Tue, 20 Apr 2021) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         CompBase::CompBase(SharedPtr<DescBase> &desc_):
            desc(desc_),
            has_changed(true),
            read_only(desc_->get_read_only()),
            repeat_count(0),
            major_version(0xFF),
            minor_version(0xFF)
         { }

         CompBase::~CompBase()
         { }

         SharedPtr<DescBase> CompBase::get_desc() const
         { return desc; }

         bool CompBase::get_read_only() const
         { return read_only || desc->get_read_only(); }
         
         StrAsc const &CompBase::get_name() const
         { return desc->get_name(); }

         StrAsc const &CompBase::get_desc_text() const
         { return desc->get_description(); }

         bool CompBase::get_desc_text_is_file() const
         { return desc->get_description_is_file(); }

         StrAsc const &CompBase::get_format_prefix() const
         { return desc->get_format_prefix(); }

         StrAsc const &CompBase::get_format_postfix() const
         { return desc->get_format_postfix(); }

         void CompBase::configure_editor(EditorBase *editor)
         { desc->configure_editor(editor); }

         bool CompBase::has_maxima() const
         { return desc->has_maxima(); }

         void CompBase::format_max(std::ostream &out) const
         {  desc->format_max(out); }

         void CompBase::format_min(std::ostream &out) const
         { desc->format_min(out); }

         int CompBase::get_significant_digits() const
         { return desc->get_significant_digits(); }

         uint4 CompBase::get_component_type() const
         { return desc->get_component_type(); }

         StrAsc const &CompBase::get_enable_expr() const
         { return desc->get_enable_expr(); }

         bool CompBase::has_protected_value() const
         { return desc->has_protected_value(); }

         CompBase::rule_sources_type const &CompBase::get_rule_sources() const
         { return desc->get_rules(); }

         StrAsc CompBase::get_val_str()
         {
            OStrAscStream rtn;
            rtn.imbue(std::locale::classic());
            output(rtn, false);
            return rtn.str();
         } // get_val_str

         void CompBase::set_val_str(StrAsc const &val, bool apply_checks)
         {
            IBuffStream temp(val.c_str(), val.length());
            input(temp, false);
         } // set_val_str
      };
   };
};

