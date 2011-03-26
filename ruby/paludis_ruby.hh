/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2006, 2007, 2008 Richard Brown
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

#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/singleton.hh>
#include <paludis/environment.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/repository.hh>
#include <paludis/contents.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/metadata_key.hh>
#include <paludis/selection.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/dep_label-fwd.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>

#ifdef ENABLE_RUBY_QA
#include <paludis/qa.hh>
#endif

#include <ruby.h>

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)
#define RDOC_IS_STUPID(x, y) RUBY_FUNC_CAST((y))
#define FAKE_RDOC_METHOD(x) //

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

        /* constructors */

        VALUE repository_to_value(std::shared_ptr<Repository>);
        VALUE version_spec_to_value(const VersionSpec &);
        VALUE package_id_to_value(std::shared_ptr<const PackageID>);
        VALUE qualified_package_name_to_value(const QualifiedPackageName &);
        VALUE contents_to_value(std::shared_ptr<const Contents>);
        VALUE metadata_key_to_value(std::shared_ptr<const MetadataKey> m);
        VALUE fetch_action_failure_to_value(const FetchActionFailure &);
        VALUE generator_to_value(const Generator &);
        VALUE filter_to_value(const Filter &);
        VALUE filtered_generator_to_value(const FilteredGenerator &);
#ifdef ENABLE_RUBY_QA
        VALUE qa_message_to_value(const QAMessage &);
#endif
        template <typename T_> VALUE dep_tree_to_value(const std::shared_ptr<const T_> &);
        template <typename T_> std::shared_ptr<const T_> value_to_dep_tree(VALUE);
        VALUE package_dep_spec_to_value(const PackageDepSpec &);
        VALUE uri_label_to_value(const std::shared_ptr<const URILabel> &);
        VALUE dependencies_label_to_value(const std::shared_ptr<const DependenciesLabel> &);
        VALUE mask_to_value(std::shared_ptr<const Mask>);
        VALUE overridden_mask_to_value(std::shared_ptr<const OverriddenMask>);
        VALUE choices_to_value(const std::shared_ptr<const Choices> & c);
        VALUE choice_to_value(const std::shared_ptr<const Choice> & c);
        VALUE choice_value_to_value(const std::shared_ptr<const ChoiceValue> & c);
        VALUE match_package_options_to_value(const MatchPackageOptions & c);
        VALUE bool_to_value(bool b);

        VersionSpec value_to_version_spec(VALUE v);
        std::shared_ptr<const PackageID> value_to_package_id(VALUE, bool nil_ok = false);
        std::shared_ptr<const PackageDepSpec> value_to_package_dep_spec(VALUE v);
        std::shared_ptr<const DependenciesLabelsDepSpec> value_to_dependencies_labels_dep_spec(VALUE v);
        std::shared_ptr<const DepSpec> value_to_dep_spec(VALUE v);
        QualifiedPackageName value_to_qualified_package_name(VALUE v);
        std::shared_ptr<Environment> value_to_environment(VALUE v);
        std::shared_ptr<NoConfigEnvironment> value_to_no_config_environment(VALUE v);
        std::shared_ptr<Repository> value_to_repository(VALUE);
        std::shared_ptr<const SupportsActionTestBase> value_to_supports_action_test_base(VALUE v);
        std::shared_ptr<Action> value_to_action(VALUE v);
        std::shared_ptr<const Choices> value_to_choices(VALUE v);
        std::shared_ptr<const Choice> value_to_choice(VALUE v);
        std::shared_ptr<const ChoiceValue> value_to_choice_value(VALUE v);
        std::shared_ptr<const DependenciesLabel> value_to_dependencies_label(VALUE v);
        MatchPackageOptions value_to_match_package_options(VALUE v);
        bool value_to_bool(VALUE v);

        Filter value_to_filter(VALUE v);
        Selection value_to_selection(VALUE v);
        FilteredGenerator value_to_filtered_generator(VALUE v);
        Generator value_to_generator(VALUE v);

        VALUE * install_action_value_ptr();
        VALUE * fetch_action_value_ptr();
        VALUE * info_action_value_ptr();
        VALUE * config_action_value_ptr();
        VALUE * uninstall_action_value_ptr();
        VALUE * pretend_action_value_ptr();
        VALUE * pretend_fetch_action_value_ptr();
        VALUE * dependencies_labels_dep_spec_value_ptr();

#ifdef ENABLE_RUBY_QA
        QACheckProperties value_to_qa_check_properties(VALUE);
#endif

        /* registration */

        class RegisterRubyClass :
            public Singleton<RegisterRubyClass>
        {
            friend class Singleton<RegisterRubyClass>;

            private:
                Pimp<RegisterRubyClass> _imp;

                RegisterRubyClass();
                ~RegisterRubyClass();

            public:
                class Register;
                friend class Register;

                void execute() const;
        };

        class RegisterRubyClass::Register
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

            static VALUE equal_via_ptr(VALUE left, VALUE right)
            {
                T_ * left_ptr, * right_ptr;
                Data_Get_Struct(left, T_, left_ptr);
                Data_Get_Struct(right, T_, right_ptr);
                return (**left_ptr == **right_ptr) ? Qtrue : Qfalse;
            }

            static VALUE to_s(VALUE self)
            {
                T_ * self_ptr;
                Data_Get_Struct(self, T_, self_ptr);
                return rb_str_new2(stringify(*self_ptr).c_str());

            }

            static VALUE hash(VALUE self)
            {
                T_ * self_ptr;
                Data_Get_Struct(self, T_, self_ptr);
                return INT2FIX(Hash<T_>()(*self_ptr));
            }

            static VALUE hash_via_ptr(VALUE self)
            {
                T_ * self_ptr;
                Data_Get_Struct(self, T_, self_ptr);
                return INT2FIX(Hash<typename std::remove_const<typename T_::element_type>::type>()(**self_ptr));
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
