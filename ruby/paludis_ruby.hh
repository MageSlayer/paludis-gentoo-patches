/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006, 2007 Richard Brown <mynamewasgone@gmail.com>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_RUBY_RUBY_PALUDIS_RUBY_HH
#define PALUDIS_GUARD_RUBY_RUBY_PALUDIS_RUBY_HH 1

#include "config.h"

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/compare.hh>
#include <paludis/package_database.hh>
#include <paludis/mask_reasons.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/environment.hh>
#include <paludis/environment/no_config/no_config_environment.hh>
#include <paludis/repositories/gentoo/portage_repository.hh>
#include <paludis/query.hh>

#ifdef ENABLE_RUBY_QA
#include <paludis/qa/qa.hh>
#endif

#include <ruby.h>

namespace paludis
{

    namespace ruby
    {

        class EnvironmentData
        {
            private:
                Environment * _e;

            public:
                Environment * const env_ptr;

                EnvironmentData(Environment * const ee, Environment * const free_e = 0) :
                    _e(free_e),
                    env_ptr(ee)
                {
                }

                ~EnvironmentData()
                {
                    delete _e;
                }
        };
        /* general utilities */

        void exception_to_ruby_exception(const std::exception &) PALUDIS_ATTRIBUTE((noreturn));

        std::string value_case_to_RubyCase(const std::string & s);

        VALUE paludis_module();
        VALUE environment_class();
        VALUE no_config_environment_class();

        bool is_kind_of_query(VALUE query);

        /* constructors */

        VALUE mask_reasons_to_value(const MaskReasons &);
        VALUE package_database_to_value(std::tr1::shared_ptr<PackageDatabase>);
        VALUE package_database_entry_to_value(const PackageDatabaseEntry &);
        VALUE repository_to_value(std::tr1::shared_ptr<const Repository>);
        VALUE version_spec_to_value(const VersionSpec &);
        VALUE version_metadata_to_value(std::tr1::shared_ptr<const VersionMetadata>);
        VALUE dep_atom_to_value(std::tr1::shared_ptr<const DepAtom>);
        VALUE qualified_package_name_to_value(const QualifiedPackageName &);
        VALUE contents_to_value(std::tr1::shared_ptr<const Contents>);
        VALUE contents_entry_to_value(std::tr1::shared_ptr<const ContentsEntry>);
        VALUE portage_repository_profiles_desc_line_to_value(const PortageRepositoryProfilesDescLine &);
        VALUE dep_tag_to_value(std::tr1::shared_ptr<const DepTag>);

        VersionSpec value_to_version_spec(VALUE v);
        std::tr1::shared_ptr<const VersionMetadata> value_to_version_metadata(VALUE);
        std::tr1::shared_ptr<const PackageDepAtom> value_to_package_dep_atom(VALUE v);
        std::tr1::shared_ptr<const DepAtom> value_to_dep_atom(VALUE v);
        QualifiedPackageName value_to_qualified_package_name(VALUE v);
        PackageDatabaseEntry value_to_package_database_entry(VALUE v);
        EnvironmentData* value_to_environment_data(VALUE v);
        NoConfigEnvironment* value_to_no_config_environment(VALUE v);
        PortageRepositoryProfilesDescLine value_to_portage_repository_profiles_desc_line(VALUE v);
        MaskReasons value_to_mask_reasons(VALUE v);
        Query value_to_query(VALUE v);

#ifdef ENABLE_RUBY_QA
        VALUE paludis_qa_module();
        qa::Message value_to_message(VALUE v);
        qa::EbuildCheckData value_to_ebuild_check_data(VALUE v);
        qa::PerProfileEbuildCheckData value_to_per_profile_ebuild_check_data(VALUE v);
        qa::ProfileCheckData value_to_profile_check_data(VALUE v);
        qa::QAEnvironment* value_to_qa_environment(VALUE v);
        VALUE ebuild_check_data_to_value(const qa::EbuildCheckData &);
        VALUE per_profile_ebuild_check_data_to_value(const qa::PerProfileEbuildCheckData &);
        VALUE profile_check_data_to_value(const qa::ProfileCheckData &);
        VALUE check_result_to_value(const qa::CheckResult &);
        VALUE package_dir_check_to_value(std::tr1::shared_ptr<qa::PackageDirCheck>);
        VALUE file_check_to_value(std::tr1::shared_ptr<qa::FileCheck>);
        VALUE ebuild_check_to_value(std::tr1::shared_ptr<qa::EbuildCheck>);
        VALUE per_profile_ebuild_check_to_value(std::tr1::shared_ptr<qa::PerProfileEbuildCheck>);
        VALUE profiles_check_to_value(std::tr1::shared_ptr<qa::ProfilesCheck>);
        VALUE profile_check_to_value(std::tr1::shared_ptr<qa::ProfileCheck>);
        VALUE message_to_value(const qa::Message &);
        VALUE metadata_file_to_value(const qa::MetadataFile &);
#endif

        /* registration */

        class RegisterRubyClass :
            public InstantiationPolicy<RegisterRubyClass, instantiation_method::SingletonTag>,
            private PrivateImplementationPattern<RegisterRubyClass>
        {
            friend class InstantiationPolicy<RegisterRubyClass, instantiation_method::SingletonTag>;

            private:
                RegisterRubyClass();
                ~RegisterRubyClass();

            public:
                class Register;
                friend class Register;

                void execute() const;
        };

        class RegisterRubyClass::Register :
            public InstantiationPolicy<RegisterRubyClass, instantiation_method::NonCopyableTag>
        {
            public:
                Register(void (* func)());
        };

        template <typename T_>
        struct Common
        {
            static void free(void * p)
            {
                delete static_cast<T_ *>(p);
            }

            static VALUE compare(VALUE left, VALUE right)
            {
                T_ * left_ptr, * right_ptr;
                Data_Get_Struct(left, T_, left_ptr);
                Data_Get_Struct(right, T_, right_ptr);
                return INT2FIX(paludis::compare(*left_ptr, *right_ptr));
            }

            static VALUE equal(VALUE left, VALUE right)
            {
                T_ * left_ptr, * right_ptr;
                Data_Get_Struct(left, T_, left_ptr);
                Data_Get_Struct(right, T_, right_ptr);
                return (*left_ptr == *right_ptr) ? Qtrue : Qfalse;
            }

            static VALUE to_s(VALUE self)
            {
                T_ * self_ptr;
                Data_Get_Struct(self, T_, self_ptr);

                return rb_str_new2(stringify(*self_ptr).c_str());
            }

            static VALUE to_s_via_ptr(VALUE self)
            {
                T_ * self_ptr;
                Data_Get_Struct(self, T_, self_ptr);

                return rb_str_new2(stringify(**self_ptr).c_str());
            }
        };

        void init();
    }
}

#endif
