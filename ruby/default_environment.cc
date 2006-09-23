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
#include <paludis/default_environment.hh>
#include <paludis/util/compare.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_default_environment;

    VALUE
    default_environment_query_use(int argc, VALUE * argv, VALUE)
    {
        try
        {
            if (1 == argc || 2 == argc)
            {
                UseFlagName * use_flag_ptr;
                Data_Get_Struct(argv[0], UseFlagName, use_flag_ptr);

                PackageDatabaseEntry * pde_ptr(0);
                if (2 == argc)
                    Data_Get_Struct(argv[1], PackageDatabaseEntry, pde_ptr);

                return DefaultEnvironment::get_instance()->query_use(*use_flag_ptr, pde_ptr) ? Qtrue : Qfalse;
            }
            else
                rb_raise(rb_eArgError, "DefaultEnvironment.query_use expects one or two arguments, but got %d", argc);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_default_environment()
    {
        rb_require("singleton");

        c_default_environment = rb_define_class("DefaultEnvironment", rb_cObject);
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_default_environment);
        rb_define_method(c_default_environment, "query_use", RUBY_FUNC_CAST(&default_environment_query_use), -1);
    }
}

RegisterRubyClass::Register paludis_ruby_register_default_environment PALUDIS_ATTRIBUTE((used)) (&do_register_default_environment);

