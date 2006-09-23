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

    void do_register_version_spec()
    {
        c_version_spec = rb_define_class_under(master_class(), "VersionSpec", rb_cObject);
        rb_define_singleton_method(c_version_spec, "new", RUBY_FUNC_CAST(&version_spec_new), 1);
        rb_define_method(c_version_spec, "initialize", RUBY_FUNC_CAST(&version_spec_init), 1);
        rb_define_method(c_version_spec, "<=>", RUBY_FUNC_CAST(&Common<VersionSpec>::compare), 1);
        rb_include_module(c_version_spec, rb_mComparable);
        rb_define_method(c_version_spec, "to_s", RUBY_FUNC_CAST(&Common<VersionSpec>::to_s), 0);
    }
}

RegisterRubyClass::Register paludis_ruby_register_version_spec PALUDIS_ATTRIBUTE((used))
    (&do_register_version_spec);



