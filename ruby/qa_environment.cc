/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Richard Brown <rbrown@gentoo.org>
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
#include <paludis/qa/qa_environment.hh>
#include <paludis/util/fs_entry.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::qa;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_qa_environment;

    VALUE
    qa_environment_init(int, VALUE*, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     QAEnvironment.new(dir) -> QAEnvironment
     *     QAEnvironment.new(dir, write_cache_dir) -> QAEnvironment
     *     QAEnvironment.new(dir, write_cache_dir, master_repository_dir) -> QAEnvironment
     *
     * Creates a new QAEnvironment from the specified directory. A write cache and master repository
     * may also be specified.
     */
    VALUE
    qa_environment_new(int argc, VALUE* argv, VALUE self)
    {
        std::string write_cache, master_repository_dir;
        try
        {
            if (1 == argc)
            {
                write_cache = "/var/empty";
                master_repository_dir = "/var/empty";
            }
            else if (2 == argc)
            {
                write_cache = StringValuePtr(argv[1]);
                master_repository_dir = "/var/empty";
            }
            else if (3 == argc)
            {
                write_cache = StringValuePtr(argv[1]);
                master_repository_dir = StringValuePtr(argv[2]);
            }
            else
            {
                rb_raise(rb_eArgError, "QAEnvironment expects one to three arguments, but got %d",argc);
            }
            tr1::shared_ptr<QAEnvironment> * e = new tr1::shared_ptr<QAEnvironment>(new QAEnvironment(FSEntry(StringValuePtr(argv[0])), FSEntry(write_cache),
                    FSEntry(master_repository_dir)));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common< tr1::shared_ptr<QAEnvironment> >::free, e));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_qa_environment()
    {
        /*
         * Document-class: Paludis::QA::QAEnvironment
         *
         * The QAEnvironment is an environment fo running QA checks in.
         */
        c_qa_environment = rb_define_class_under(paludis_qa_module(), "QAEnvironment",
                no_config_environment_class());
        rb_define_singleton_method(c_qa_environment, "new", RUBY_FUNC_CAST(&qa_environment_new),-1);
        rb_define_method(c_qa_environment, "initialize", RUBY_FUNC_CAST(&qa_environment_init),-1);
    }
}

tr1::shared_ptr<QAEnvironment>
paludis::ruby::value_to_qa_environment(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_qa_environment))
    {
        tr1::shared_ptr<QAEnvironment> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<QAEnvironment>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into QAEnvironment", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_qa_environment PALUDIS_ATTRIBUTE((used))
    (&do_register_qa_environment);
