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
#include <paludis/version_spec.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_version_spec;

    VALUE
    version_spec_init(VALUE self, VALUE)
    {
        return self;
    }

    VALUE
    version_spec_new(VALUE self, VALUE s)
    {
        VersionSpec * ptr(0);
        try
        {
            ptr = new VersionSpec(std::string(STR2CSTR(s)));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<VersionSpec>::free, ptr));
            rb_obj_call_init(tdata, 1, &s);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    version_spec_remove_revision(VALUE self)
    {
        try
        {
            return version_spec_to_value(value_to_version_spec(self).remove_revision());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    version_spec_revision_only(VALUE self)
    {
        try
        {
            return rb_str_new2((value_to_version_spec(self).revision_only().c_str()));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_version_spec()
    {
        c_version_spec = rb_define_class_under(master_class(), "VersionSpec", rb_cObject);
        rb_define_singleton_method(c_version_spec, "new", RUBY_FUNC_CAST(&version_spec_new), 1);
        rb_define_method(c_version_spec, "initialize", RUBY_FUNC_CAST(&version_spec_init), 1);
        rb_define_method(c_version_spec, "<=>", RUBY_FUNC_CAST(&Common<VersionSpec>::compare), 1);
        rb_include_module(c_version_spec, rb_mComparable);
        rb_define_method(c_version_spec, "to_s", RUBY_FUNC_CAST(&Common<VersionSpec>::to_s), 0);
        rb_define_method(c_version_spec, "remove_revision", RUBY_FUNC_CAST(&version_spec_remove_revision), 0);
        rb_define_method(c_version_spec, "revision_only", RUBY_FUNC_CAST(&version_spec_revision_only), 0);
    }
}

VersionSpec
paludis::ruby::value_to_version_spec(VALUE v)
{
    if (T_STRING == TYPE(v))
        return VersionSpec(STR2CSTR(v));
    else
    {
        VersionSpec * v_ptr;
        Data_Get_Struct(v, VersionSpec, v_ptr);
        return *v_ptr;
    }
}

VALUE
paludis::ruby::version_spec_to_value(const VersionSpec & v)
{
    VersionSpec * vv(new VersionSpec(v));
    return Data_Wrap_Struct(c_version_spec, 0, &Common<VersionSpec>::free, vv);
}

RegisterRubyClass::Register paludis_ruby_register_version_spec PALUDIS_ATTRIBUTE((used))
    (&do_register_version_spec);



