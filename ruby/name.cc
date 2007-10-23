/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown
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
#include <paludis/name.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_iuse_flag_parse_mode;

    void do_register_names()
    {
        /*
         * Document-module: Paludis::IUseFlagParseMode
         *
         * How to parse an IUSE flag string.
         */
        c_iuse_flag_parse_mode = rb_define_module_under(paludis_module(), "IUseFlagParseMode");
        for (IUseFlagParseMode l(static_cast<IUseFlagParseMode>(0)), l_end(last_iuse_pm) ; l != l_end ;
                l = static_cast<IUseFlagParseMode>(static_cast<int>(l) + 1))
            rb_define_const(c_iuse_flag_parse_mode, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/name-se.hh, IUseFlagParseMode , c_iuse_flag_parse_mode>
    }
}

RegisterRubyClass::Register paludis_ruby_register_names PALUDIS_ATTRIBUTE((used)) (&do_register_names);


