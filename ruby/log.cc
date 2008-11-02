/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2008 Ciaran McCreesh
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

namespace
{
    static VALUE c_log;
    static VALUE c_log_level;

    /*
     * call-seq:
     *     log_level -> LogLevel
     *
     * Fetch the current log level.
     */
    VALUE
    log_log_level(VALUE)
    {
        return INT2FIX(Log::get_instance()->log_level());
    }

    /*
     * call-seq:
     *     log_level=
     *
     * Only display messages of at least this level.
     */
    VALUE
    log_log_level_set(VALUE self, VALUE value)
    {
        int l = NUM2INT(value);
        if (l < 0 || l >= last_ll)
            rb_raise(rb_eTypeError, "Log.level= expects a valid LogLevel");
        Log::get_instance()->set_log_level(static_cast<LogLevel>(l));
        return self;
    }

    /*
     * call-seq:
     *     set_program_name(name)
     *
     * Set our program name.
     */
    VALUE
    log_set_program_name(VALUE self, VALUE name)
    {
        Log::get_instance()->set_program_name(stringify(StringValuePtr(name)));
        return self;
    }

    /*
     * call-seq:
     *     message(id, log_level, message)
     *
     * Log a message at the specified level.
     */
    VALUE
    log_message(VALUE self, VALUE id, VALUE log_level, VALUE message)
    {
        int l = NUM2INT(log_level);
        if (l < 0 || l >= last_ll)
            rb_raise(rb_eTypeError, "Log.log_message expects a valid LogLevel as the first parameter");
        Log::get_instance()->message(stringify(StringValuePtr(id)), static_cast<LogLevel>(l), lc_no_context)
            << stringify(StringValuePtr(message));
        return self;
    }

    void do_register_log()
    {
        rb_require("singleton");

        /*
         * Document-class: Paludis::Log
         *
         * Singleton class that handles log messages.
         */
        c_log = rb_define_class_under(paludis_module(), "Log", rb_cObject);
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_log);
        rb_define_method(c_log, "log_level", RUBY_FUNC_CAST(&log_log_level), 0);
        rb_define_method(c_log, "log_level=", RUBY_FUNC_CAST(&log_log_level_set), 1);
        rb_define_method(c_log, "program_name=", RUBY_FUNC_CAST(&log_set_program_name), 1);
        rb_define_method(c_log, "message", RUBY_FUNC_CAST(&log_message), 3);

        /*
         * Document-module: Paludis::LogLevel
         *
         * Specifies the level of a log message
         */
        c_log_level = rb_define_module_under(paludis_module(), "LogLevel");
        for (LogLevel l(static_cast<LogLevel>(0)), l_end(last_ll) ; l != l_end ;
                l = static_cast<LogLevel>(static_cast<int>(l) + 1))
            rb_define_const(c_log_level, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/util/log.hh, LogLevel, c_log_level>
    }
}

RegisterRubyClass::Register paludis_ruby_register_log PALUDIS_ATTRIBUTE((used)) (&do_register_log);


