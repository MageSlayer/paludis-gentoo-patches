/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/repositories/portage/portage_repository.hh>

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

        /* constructors */

        VALUE mask_reasons_to_value(const MaskReasons &);
        VALUE package_database_to_value(PackageDatabase::Pointer);
        VALUE package_database_entry_to_value(const PackageDatabaseEntry &);
        VALUE repository_to_value(Repository::ConstPointer);
        VALUE portage_repository_to_value(PortageRepository::ConstPointer);
        VALUE version_spec_to_value(const VersionSpec &);
        VALUE version_metadata_to_value(VersionMetadata::ConstPointer);
        VALUE dep_atom_to_value(DepAtom::ConstPointer);
        VALUE qualified_package_name_to_value(const QualifiedPackageName &);
        VALUE contents_to_value(Contents::ConstPointer);
        VALUE contents_entry_to_value(ContentsEntry::ConstPointer);
        VALUE portage_repository_profiles_desc_line_to_value(const PortageRepositoryProfilesDescLine &);

        VersionSpec value_to_version_spec(VALUE v);
        VersionMetadata::ConstPointer value_to_version_metadata(VALUE);
        PackageDepAtom::ConstPointer value_to_package_dep_atom(VALUE v);
        QualifiedPackageName value_to_qualified_package_name(VALUE v);
        PackageDatabaseEntry value_to_package_database_entry(VALUE v);
        EnvironmentData* value_to_environment_data(VALUE v);
        NoConfigEnvironment* value_to_no_config_environment(VALUE v);

#ifdef ENABLE_RUBY_QA
        VALUE paludis_qa_module();
        qa::Message value_to_message(VALUE v);
        qa::EbuildCheckData value_to_ebuild_check_data(VALUE v);
        qa::PerProfileEbuildCheckData value_to_per_profile_ebuild_check_data(VALUE v);
        qa::QAEnvironment* value_to_qa_environment(VALUE v);
        VALUE ebuild_check_data_to_value(const qa::EbuildCheckData &);
        VALUE per_profile_ebuild_check_data_to_value(const qa::PerProfileEbuildCheckData &);
        VALUE check_result_to_value(const qa::CheckResult &);
        VALUE package_dir_check_to_value(qa::PackageDirCheck::Pointer);
        VALUE file_check_to_value(qa::FileCheck::Pointer);
        VALUE ebuild_check_to_value(qa::EbuildCheck::Pointer);
        VALUE per_profile_ebuild_check_to_value(qa::PerProfileEbuildCheck::Pointer);
        VALUE message_to_value(const qa::Message &);
        VALUE metadata_file_to_value(const qa::MetadataFile &);
#endif

        /* registration */

        class RegisterRubyClass :
            public InstantiationPolicy<RegisterRubyClass, instantiation_method::SingletonAsNeededTag>,
            private PrivateImplementationPattern<RegisterRubyClass>
        {
            friend class InstantiationPolicy<RegisterRubyClass, instantiation_method::SingletonAsNeededTag>;

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
    }
}

#endif
