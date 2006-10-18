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

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/compare.hh>
#include <paludis/package_database.hh>
#include <paludis/mask_reasons.hh>
#include <ruby.h>

namespace paludis
{
    namespace ruby
    {
        /* general utilities */

        void exception_to_ruby_exception(const std::exception &) PALUDIS_ATTRIBUTE((noreturn));

        std::string value_case_to_RubyCase(const std::string & s);

        VALUE master_class();

        /* constructors */

        VALUE mask_reasons_to_value(const MaskReasons &);
        VALUE package_database_to_value(PackageDatabase::Pointer);
        VALUE package_database_entry_to_value(const PackageDatabaseEntry &);
        VALUE repository_to_value(Repository::ConstPointer);
        VALUE version_spec_to_value(const VersionSpec &);
        VALUE version_metadata_to_value(VersionMetadata::ConstPointer);
        VALUE dep_atom_to_value(DepAtom::ConstPointer);

        VersionSpec value_to_version_spec(VALUE v);
        VersionMetadata::ConstPointer value_to_version_metadata(VALUE);
        PackageDepAtom::ConstPointer value_to_package_dep_atom(VALUE v);

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
