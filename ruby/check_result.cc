/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Richard Brown <mynamewasgone@gmail.com>
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
#include <paludis/qa/check_result.hh>
#include <paludis/util/stringify.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::qa;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_check_result;

    VALUE
    check_result_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    check_result_new(int argc, VALUE *argv, VALUE self)
    {
        CheckResult * ptr(0);
        try
        {
            if (2 == argc)
            {
                ptr = new CheckResult(StringValuePtr(argv[0]), StringValuePtr(argv[1]));
            }
            else
            {
                rb_raise(rb_eArgError, "CheckResult expects two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<CheckResult>::free, ptr));
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
     * call-seq:
     *     empty? -> true of false
     *
     * Are there any messages?
     */
    VALUE
    check_result_empty(VALUE self)
    {
        CheckResult * ptr;
        Data_Get_Struct(self, CheckResult, ptr);
        return (ptr)->empty() ? Qtrue : Qfalse;
    }

    /*
     * call-seq:
     *     most_severe_level -> QALevel
     *
     * Most severe message level in the CheckResult.
     */
    VALUE
    check_result_most_severe_level(VALUE self)
    {
        CheckResult * ptr;
        Data_Get_Struct(self, CheckResult, ptr);
        return INT2NUM((ptr)->most_severe_level());
    }

    VALUE
    check_result_cat_message(VALUE self, VALUE message)
    {
        try
        {
            CheckResult * ptr;
            Data_Get_Struct(self, CheckResult, ptr);
            *ptr << value_to_message(message);
            return self;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

    }

    /*
     * call-seq:
     *     messages -> Array
     *     messages -> {|message| block } -> Qnil
     *
     * Returns all the Messages in the CheckResult, either as an Array, or as the parameters to a block.
     */
    VALUE
    check_result_messages(VALUE self)
    {
        CheckResult * ptr;
        Data_Get_Struct(self, CheckResult, ptr);

        if (rb_block_given_p())
        {
            for (CheckResult::Iterator i (ptr->begin()), i_end(ptr->end()) ; i != i_end; ++i)
                rb_yield(message_to_value(*i));
            return Qnil;
        }
        VALUE result(rb_ary_new());
        for (CheckResult::Iterator i (ptr->begin()), i_end(ptr->end()) ; i != i_end; ++i)
            rb_ary_push(result, message_to_value(*i));
        return result;
    }

    /*
     * Document-method: item
     *
     * call-seq:
     *     item -> String
     *
     * What was checked
     */
    /*
     * Document-method: rule
     *
     * call-seq:
     *     rule -> String
     *
     * Check class that produced this result.
     */
    template <const std::string & (CheckResult::* m_) () const>
    struct CheckResultValue
    {
        static VALUE
        fetch(VALUE self)
        {
            CheckResult * ptr;
            Data_Get_Struct(self, CheckResult, ptr);
            return rb_str_new2((((*ptr).*m_)()).c_str());
        }
    };

    void do_register_check_result()
    {
        /*
         * Document-class: Paludis::QA::CheckResult
         *
         * The result of a QA check.
         */
        c_check_result = rb_define_class_under(paludis_qa_module(), "CheckResult", rb_cObject);
        rb_define_singleton_method(c_check_result, "new", RUBY_FUNC_CAST(&check_result_new),-1);
        rb_define_method(c_check_result, "initialize", RUBY_FUNC_CAST(&check_result_init),-1);
        rb_define_method(c_check_result, "empty?", RUBY_FUNC_CAST(&check_result_empty),0);
        rb_define_method(c_check_result, "most_severe_level", RUBY_FUNC_CAST(&check_result_most_severe_level),0);
        rb_define_method(c_check_result, "messages", RUBY_FUNC_CAST(&check_result_messages),0);
        rb_define_method(c_check_result, "<<", RUBY_FUNC_CAST(&check_result_cat_message),1);
        rb_define_method(c_check_result, "item", RUBY_FUNC_CAST((&CheckResultValue<&CheckResult::item>::fetch)),0);
        rb_define_method(c_check_result, "rule", RUBY_FUNC_CAST((&CheckResultValue<&CheckResult::rule>::fetch)),0);
    }
}

VALUE
paludis::ruby::check_result_to_value(const CheckResult & v)
{
    CheckResult * vv(new CheckResult(v));
    return Data_Wrap_Struct(c_check_result, 0, &Common<CheckResult>::free, vv);
}

RegisterRubyClass::Register paludis_ruby_register_check_result PALUDIS_ATTRIBUTE((used))
    (&do_register_check_result);
