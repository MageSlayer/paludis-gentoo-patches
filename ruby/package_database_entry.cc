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

#include <paludis_ruby.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/util/compare.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_package_database_entry;

    VALUE
    package_database_entry_init(VALUE self, VALUE, VALUE, VALUE)
    {
        return self;
    }

    VALUE
    package_database_entry_new(VALUE self, VALUE q, VALUE v, VALUE n)
    {
        VALUE argv[3] = { q, v, n };
        PackageDatabaseEntry * ptr(0);
        try
        {
            QualifiedPackageName * q_ptr;
            Data_Get_Struct(q, QualifiedPackageName, q_ptr);
            VersionSpec * v_ptr;
            Data_Get_Struct(v, VersionSpec, v_ptr);
            RepositoryName * n_ptr;
            Data_Get_Struct(n, RepositoryName, n_ptr);

            ptr = new PackageDatabaseEntry(*q_ptr, *v_ptr, *n_ptr);
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<PackageDatabaseEntry>::free, ptr));
            rb_obj_call_init(tdata, 3, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    void do_register_package_database_entry()
    {
        c_package_database_entry = rb_define_class_under(master_class(), "PackageDatabaseEntry", rb_cObject);
        rb_define_singleton_method(c_package_database_entry, "new", RUBY_FUNC_CAST(&package_database_entry_new), 3);
        rb_define_method(c_package_database_entry, "initialize", RUBY_FUNC_CAST(&package_database_entry_init), 3);
        rb_define_method(c_package_database_entry, "<=>", RUBY_FUNC_CAST(&Common<PackageDatabaseEntry>::compare), 1);
        rb_include_module(c_package_database_entry, rb_mComparable);
        rb_define_method(c_package_database_entry, "to_s", RUBY_FUNC_CAST(&Common<PackageDatabaseEntry>::to_s), 0);
    }
}

RegisterRubyClass::Register paludis_ruby_register_package_database_entry PALUDIS_ATTRIBUTE((used))
    (&do_register_package_database_entry);


