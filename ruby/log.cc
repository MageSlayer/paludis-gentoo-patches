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
#include <paludis/util/log.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_log;
    static VALUE c_log_level;

    VALUE
    log_log_level(VALUE)
    {
        return INT2FIX(Log::get_instance()->log_level());
    }

    VALUE
    log_log_level_set(VALUE self, VALUE value)
    {
        Log::get_instance()->set_log_level(static_cast<LogLevel>(NUM2INT(value)));
        return self;
    }

    VALUE
    log_set_program_name(VALUE self, VALUE name)
    {
        Log::get_instance()->set_program_name(stringify(StringValuePtr(name)));
        return self;
    }

    VALUE
    log_message(VALUE self, VALUE log_level, VALUE message)
    {
        Log::get_instance()->message(static_cast<LogLevel>(NUM2INT(log_level)), lc_no_context,
                stringify(StringValuePtr(message)));
        return self;
    }

    void do_register_log()
    {
        rb_require("singleton");

        c_log = rb_define_class_under(paludis_module(), "Log", rb_cObject);
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_log);
        rb_define_method(c_log, "log_level", RUBY_FUNC_CAST(&log_log_level), 0);
        rb_define_method(c_log, "log_level=", RUBY_FUNC_CAST(&log_log_level_set), 1);
        rb_define_method(c_log, "program_name=", RUBY_FUNC_CAST(&log_set_program_name), 1);
        rb_define_method(c_log, "message", RUBY_FUNC_CAST(&log_message), 2);

        c_log_level = rb_define_class_under(paludis_module(), "LogLevel", rb_cObject);
        for (LogLevel l(static_cast<LogLevel>(0)), l_end(last_ll) ; l != l_end ;
                l = static_cast<LogLevel>(static_cast<int>(l) + 1))
            rb_define_const(c_log_level, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));
    }
}

RegisterRubyClass::Register paludis_ruby_register_log PALUDIS_ATTRIBUTE((used)) (&do_register_log);


