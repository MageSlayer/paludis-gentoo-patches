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
#include <paludis/package_database.hh>
#include <paludis/util/stringify.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_package_database;
    static VALUE c_package_database_install_state;

    VALUE
    package_database_favourite_repository(VALUE self)
    {
        try
        {
            PackageDatabase::Pointer * self_ptr;
            Data_Get_Struct(self, PackageDatabase::Pointer, self_ptr);
            return rb_str_new2(stringify((*self_ptr)->favourite_repository()).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    package_database_fetch_unique_qualified_package_name(VALUE self, VALUE pkg)
    {
        try
        {
            PackageDatabase::Pointer * self_ptr;
            Data_Get_Struct(self, PackageDatabase::Pointer, self_ptr);
            return rb_str_new2(stringify((*self_ptr)->fetch_unique_qualified_package_name(
                            PackageNamePart(STR2CSTR(pkg)))).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    package_database_query(VALUE self, VALUE atom, VALUE state)
    {
        try
        {
            PackageDatabase::Pointer * self_ptr;
            Data_Get_Struct(self, PackageDatabase::Pointer, self_ptr);

            PackageDatabaseEntryCollection::ConstPointer items((*self_ptr)->query(
                        value_to_package_dep_atom(atom),
                        static_cast<InstallState>(NUM2INT(state))));

            VALUE result(rb_ary_new());
            for (PackageDatabaseEntryCollection::Iterator i(items->begin()),
                    i_end(items->end()) ; i != i_end ; ++i)
                rb_ary_push(result, create_package_database_entry(*i));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_package_database()
    {
        c_package_database = rb_define_class_under(master_class(), "PackageDatabase", rb_cObject);
        rb_funcall(c_package_database, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_package_database, "favourite_repository",
                RUBY_FUNC_CAST(&package_database_favourite_repository), 0);
        rb_define_method(c_package_database, "fetch_unique_qualified_package_name",
                RUBY_FUNC_CAST(&package_database_fetch_unique_qualified_package_name), 1);
        rb_define_method(c_package_database, "query",
                RUBY_FUNC_CAST(&package_database_query), 2);

        c_package_database_install_state = rb_define_class_under(master_class(), "InstallState", rb_cObject);
        for (InstallState l(static_cast<InstallState>(0)), l_end(last_install_state) ; l != l_end ;
                l = static_cast<InstallState>(static_cast<int>(l) + 1))
            rb_define_const(c_package_database_install_state, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));
    }
}

VALUE
paludis::ruby::create_package_database(PackageDatabase::Pointer m)
{
    PackageDatabase::Pointer * m_ptr(0);
    try
    {
        m_ptr = new PackageDatabase::Pointer(m);
        return Data_Wrap_Struct(c_package_database, 0, &Common<PackageDatabase::Pointer>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_package_database PALUDIS_ATTRIBUTE((used))
    (&do_register_package_database);

