/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/action.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_supports_action_test_base;
    static VALUE c_supports_fetch_action_test;

    static VALUE c_action;
    static VALUE c_fetch_action;
    static VALUE c_fetch_action_options;

    const FetchActionOptions
    value_to_fetch_action_options(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_fetch_action_options))
        {
            FetchActionOptions * v_ptr;
            Data_Get_Struct(v, FetchActionOptions, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into FetchActionOptions", rb_obj_classname(v));
        }
    }

    VALUE
    fetch_action_options_to_value(const FetchActionOptions & m)
    {
        FetchActionOptions * m_ptr(new FetchActionOptions(m));
        try
        {
            return Data_Wrap_Struct(c_fetch_action_options, 0, &Common<FetchActionOptions>::free, m_ptr);
        }
        catch (const std::exception & e)
        {
            delete m_ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    supports_fetch_action_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    fetch_action_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    fetch_action_options_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     SupportsFetchActionTest.new -> SupportsFetchActionTest
     */
    VALUE
    supports_fetch_action_test_new(VALUE self)
    {
        tr1::shared_ptr<const SupportsActionTestBase> * a(
                new tr1::shared_ptr<const SupportsActionTestBase>(new SupportsActionTest<FetchAction>));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const SupportsActionTestBase> >::free, a));
        rb_obj_call_init(tdata, 0, &self);
        return tdata;
    }

    /*
     * call-seq:
     *     FetchActionOptions.new(fetch_unneeded, safe_resume) -> FetchActionOptions
     *     FetchActionOptions.new(Hash) -> FetchActionOptions
     *
     * FetchActionOptions.new can either be called with all parameters in order, or with one hash
     * parameter, where the hash keys are symbols with the names above.
     */
    VALUE
    fetch_action_options_new(int argc, VALUE *argv, VALUE self)
    {
        FetchActionOptions * ptr(0);
        try
        {
            bool v_fetch_unneeded;
            bool v_safe_resume;

            if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("fetch_unneeded"))))
                    rb_raise(rb_eArgError, "Missing Parameter: fetch_unneeded");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("safe_resume"))))
                    rb_raise(rb_eArgError, "Missing Parameter: safe_resume");
                v_fetch_unneeded =
                    rb_hash_aref(argv[0], ID2SYM(rb_intern("fetch_unneeded"))) != Qfalse;
                v_safe_resume =
                    rb_hash_aref(argv[0], ID2SYM(rb_intern("safe_resume"))) != Qfalse;
            }
            else if (2 == argc)
            {
                v_fetch_unneeded          = argv[0] != Qfalse;
                v_safe_resume             = argv[1] != Qfalse;
            }
            else
            {
                rb_raise(rb_eArgError, "FetchActionOptions expects one or two arguments, but got %d",argc);
            }

            ptr = new FetchActionOptions(FetchActionOptions::create()
                    .fetch_unneeded(v_fetch_unneeded)
                    .safe_resume(v_safe_resume)
                    );

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<FetchActionOptions>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: fetch_unneeded
     *
     * call-seq:
     *     fetch_unneeded -> true or false
     */
    VALUE
    fetch_action_options_fetch_unneeded(VALUE self)
    {
        FetchActionOptions * p;
        Data_Get_Struct(self, FetchActionOptions, p);
        return p->fetch_unneeded ? Qtrue : Qfalse;
    }

    /*
     * Document-method: safe_resume
     *
     * call-seq:
     *     safe_resume -> true or false
     */
    VALUE
    fetch_action_options_safe_resume(VALUE self)
    {
        FetchActionOptions * p;
        Data_Get_Struct(self, FetchActionOptions, p);
        return p->safe_resume ? Qtrue : Qfalse;
    }

    /*
     * call-seq:
     *     FetchAction.new -> FetchAction
     */
    VALUE
    fetch_action_new(VALUE self, VALUE opts)
    {
        const FetchActionOptions opts_ptr(value_to_fetch_action_options(opts));
        tr1::shared_ptr<Action> * a(
                new tr1::shared_ptr<Action>(new FetchAction(opts_ptr)));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<Action> >::free, a));
        rb_obj_call_init(tdata, 1, &self);
        return tdata;
    }

    /*
     * call-seq:
     *     options -> FetchActionOptions
     *
     * Our FetchActionOptions.
     */
    VALUE
    fetch_action_options(VALUE self)
    {
        tr1::shared_ptr<Action> * p;
        Data_Get_Struct(self, tr1::shared_ptr<Action>, p);
        return fetch_action_options_to_value(tr1::static_pointer_cast<FetchAction>(*p)->options);
    }

    void do_register_action()
    {
        /*
         * Document-class: Paludis::SupportsActionTestBase
         *
         * Base class for action tests, used by Paludis::PackageID#supports_action.
         */
        c_supports_action_test_base = rb_define_class_under(paludis_module(), "SupportsActionTestBase", rb_cObject);
        rb_funcall(c_supports_action_test_base, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::SupportsFetchActionTest
         *
         * Tests whether a Paludis::PackageID supports a Paludis::FetchAction.
         */
        c_supports_fetch_action_test = rb_define_class_under(paludis_module(), "SupportsFetchActionTest", c_supports_action_test_base);
        rb_define_singleton_method(c_supports_fetch_action_test, "new", RUBY_FUNC_CAST((&supports_fetch_action_test_new)), 0);
        rb_define_method(c_supports_fetch_action_test, "initialize", RUBY_FUNC_CAST(&supports_fetch_action_init), -1);

        /*
         * Document-class: Paludis::Action
         *
         * Base class for actions, used by Paludis::PackageID#perform_action.
         */
        c_action = rb_define_class_under(paludis_module(), "Action", rb_cObject);
        rb_funcall(c_action, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::FetchAction
         *
         * An action for fetching.
         */
        c_fetch_action = rb_define_class_under(paludis_module(), "FetchAction", c_action);
        rb_define_singleton_method(c_fetch_action, "new", RUBY_FUNC_CAST(&fetch_action_new), 1);
        rb_define_method(c_fetch_action, "initialize", RUBY_FUNC_CAST(&fetch_action_init), -1);
        rb_define_method(c_fetch_action, "options", RUBY_FUNC_CAST(&fetch_action_options), 0);

        /*
         * Document-class: Paludis::FetchActionOptions
         *
         * Options for Paludis::FetchAction.
         */
        c_fetch_action_options = rb_define_class_under(paludis_module(), "FetchActionOptions", rb_cObject);
        rb_define_singleton_method(c_fetch_action_options, "new", RUBY_FUNC_CAST(&fetch_action_options_new), -1);
        rb_define_method(c_fetch_action_options, "initialize", RUBY_FUNC_CAST(&fetch_action_options_init), -1);
        rb_define_method(c_fetch_action_options, "fetch_unneeded", RUBY_FUNC_CAST(&fetch_action_options_fetch_unneeded), 0);
        rb_define_method(c_fetch_action_options, "safe_resume", RUBY_FUNC_CAST(&fetch_action_options_safe_resume), 0);
    }
}

tr1::shared_ptr<const SupportsActionTestBase>
paludis::ruby::value_to_supports_action_test_base(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_supports_action_test_base))
    {
        tr1::shared_ptr<const SupportsActionTestBase> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<const SupportsActionTestBase>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into SupportsActionTestBase", rb_obj_classname(v));
    }

}

tr1::shared_ptr<Action>
paludis::ruby::value_to_action(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_action))
    {
        tr1::shared_ptr<Action> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<Action>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Action", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_action PALUDIS_ATTRIBUTE((used))
    (&do_register_action);

