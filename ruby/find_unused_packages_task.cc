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
#include <paludis/legacy/find_unused_packages_task.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence-impl.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_find_unused_packages_task;

    VALUE
    find_unused_packages_task_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    find_unused_packages_task_new(int argc, VALUE *argv, VALUE self)
    {
        FindUnusedPackagesTask * ptr(0);
        try
        {
            if (2 == argc)
            {
                ptr = new FindUnusedPackagesTask(value_to_environment(argv[0]).get(),
                    value_to_repository(argv[1]).get());
            }
            else
            {
                rb_raise(rb_eArgError, "FindUnusedPackagesTask expects two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<FindUnusedPackagesTask>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE find_unused_packages_task_execute(VALUE self, VALUE qpn)
    {
        try
        {
            FindUnusedPackagesTask * ptr;
            Data_Get_Struct(self, FindUnusedPackagesTask, ptr);
            std::shared_ptr<const PackageIDSequence> c(ptr->execute(value_to_qualified_package_name(qpn)));
            VALUE result(rb_ary_new());
            for (PackageIDSequence::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, package_id_to_value(*i));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_find_unused_packages_task()
    {
        /* Document-class: Paludis::QA::Message
         *
         * A QA message
         */
        c_find_unused_packages_task = rb_define_class_under(paludis_module(),
                "FindUnusedPackagesTask", rb_cObject);
        rb_define_singleton_method(c_find_unused_packages_task,
                "new", RUBY_FUNC_CAST(&find_unused_packages_task_new),-1);
        rb_define_method(c_find_unused_packages_task,
                "initialize", RUBY_FUNC_CAST(&find_unused_packages_task_init),-1);
        rb_define_method(c_find_unused_packages_task,
                "execute", RUBY_FUNC_CAST(&find_unused_packages_task_execute),1);
    }
}

RegisterRubyClass::Register paludis_ruby_register_find_unused_packages_task PALUDIS_ATTRIBUTE((used))
    (&do_register_find_unused_packages_task);
