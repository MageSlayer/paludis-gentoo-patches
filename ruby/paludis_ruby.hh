/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
 * Copyright (c) 2006, 2007 Richard Brown
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
#include <paludis/util/fs_entry.hh>
#include <paludis/environment.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/query.hh>
#include <paludis/repository.hh>
#include <paludis/contents.hh>
#include <paludis/dep_tag.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/stringify.hh>

#ifdef ENABLE_RUBY_QA
#include <paludis/qa.hh>
#endif

#include <ruby.h>

namespace paludis
{

    namespace ruby
    {

#ifdef ENABLE_RUBY_QA
        class RubyQAReporter : public QAReporter
        {
            VALUE* reporter;

            public:
                RubyQAReporter(VALUE*);
                void message(const QAMessage &);
                void status(const std::string &);
        };
#endif

        /* general utilities */

        void exception_to_ruby_exception(const std::exception &) PALUDIS_ATTRIBUTE((noreturn));

        std::string value_case_to_RubyCase(const std::string & s);

        VALUE paludis_module();
        VALUE environment_class();
        VALUE no_config_environment_class();

        bool is_kind_of_query(VALUE query);

        /* constructors */

        VALUE package_database_to_value(tr1::shared_ptr<PackageDatabase>);
        VALUE repository_to_value(tr1::shared_ptr<Repository>);
        VALUE version_spec_to_value(const VersionSpec &);
        VALUE package_id_to_value(tr1::shared_ptr<const PackageID>);
#if CIARANM_REMOVED_THIS
        VALUE dep_spec_to_value(tr1::shared_ptr<const DepSpec>);
#endif
        VALUE dep_tag_to_value(tr1::shared_ptr<const DepTag>);
        VALUE qualified_package_name_to_value(const QualifiedPackageName &);
        VALUE contents_to_value(tr1::shared_ptr<const Contents>);
        VALUE repository_mask_info_to_value(tr1::shared_ptr<const RepositoryMaskInfo>);
        VALUE metadata_key_to_value(tr1::shared_ptr<const MetadataKey> m);
        VALUE fetch_action_failure_to_value(const FetchActionFailure &);
#ifdef ENABLE_RUBY_QA
        VALUE qa_message_to_value(const QAMessage &);
#endif

        VersionSpec value_to_version_spec(VALUE v);
        tr1::shared_ptr<const PackageID> value_to_package_id(VALUE);
        tr1::shared_ptr<const PackageDepSpec> value_to_package_dep_spec(VALUE v);
        tr1::shared_ptr<const DepSpec> value_to_dep_spec(VALUE v);
        QualifiedPackageName value_to_qualified_package_name(VALUE v);
        tr1::shared_ptr<Environment> value_to_environment(VALUE v);
        tr1::shared_ptr<NoConfigEnvironment> value_to_no_config_environment(VALUE v);
        RepositoryEInterface::ProfilesDescLine value_to_profiles_desc_line(VALUE v);
        Query value_to_query(VALUE v);
        tr1::shared_ptr<Repository> value_to_repository(VALUE);
        tr1::shared_ptr<const SupportsActionTestBase> value_to_supports_action_test_base(VALUE v);
        tr1::shared_ptr<Action> value_to_action(VALUE v);

#ifdef ENABLE_RUBY_QA
        QACheckProperties value_to_qa_check_properties(VALUE);
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
                if (*left_ptr < *right_ptr)
                    return INT2FIX(-1);
                if (*left_ptr > *right_ptr)
                    return INT2FIX(1);
                return INT2FIX(0);
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
