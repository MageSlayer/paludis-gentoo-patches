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
#include <paludis/dep_atom.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_dep_atom;
    static VALUE c_package_dep_atom;

    VALUE
    dep_atom_init(VALUE self)
    {
        return self;
    }

    VALUE
    package_dep_atom_init(VALUE self, VALUE)
    {
        return self;
    }

    VALUE
    package_dep_atom_new(VALUE self, VALUE s)
    {
        PackageDepAtom::ConstPointer * ptr(0);
        try
        {
            ptr = new PackageDepAtom::ConstPointer(new PackageDepAtom(STR2CSTR(s)));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<PackageDepAtom::ConstPointer>::free, ptr));
            rb_obj_call_init(tdata, 1, &s);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    void do_register_dep_atom()
    {
        c_dep_atom = rb_define_class_under(master_class(), "DepAtom", rb_cObject);
        rb_funcall(c_dep_atom, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        c_package_dep_atom = rb_define_class_under(master_class(), "PackageDepAtom", c_dep_atom);
        rb_define_singleton_method(c_package_dep_atom, "new", RUBY_FUNC_CAST(&package_dep_atom_new), 1);
        rb_define_method(c_package_dep_atom, "initialize", RUBY_FUNC_CAST(&package_dep_atom_init), 1);
        rb_define_method(c_package_dep_atom, "to_s", RUBY_FUNC_CAST(&Common<PackageDepAtom::ConstPointer>::to_s_via_ptr), 0);
    }
}

PackageDepAtom::ConstPointer
paludis::ruby::value_to_package_dep_atom(VALUE v)
{
    if (T_STRING == TYPE(v))
        return PackageDepAtom::ConstPointer(new PackageDepAtom(STR2CSTR(v)));
    else
    {
        PackageDepAtom::ConstPointer * v_ptr;
        Data_Get_Struct(v, PackageDepAtom::ConstPointer, v_ptr);
        return *v_ptr;
    }
}

VALUE
paludis::ruby::dep_atom_to_value(DepAtom::ConstPointer m)
{
    DepAtom::ConstPointer * m_ptr(0);
    try
    {
        m_ptr = new DepAtom::ConstPointer(m);
        return Data_Wrap_Struct(c_dep_atom, 0, &Common<DepAtom::ConstPointer>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_dep_atom PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_atom);

