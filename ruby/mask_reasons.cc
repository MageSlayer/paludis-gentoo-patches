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
#include <paludis/mask_reasons.hh>
#include <paludis/util/stringify.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_mask_reasons;
    static VALUE c_mask_reason;

    VALUE
    mask_reasons_init(VALUE self)
    {
        return self;
    }

    VALUE
    mask_reasons_new(VALUE self)
    {
        MaskReasons * ptr(0);
        try
        {
            ptr = new MaskReasons;
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<MaskReasons>::free, ptr));
            rb_obj_call_init(tdata, 0, &self);
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
     *     each {|mask_reason| block } -> Nil
     *
     * Iterate through the mask reasons.
     */
    VALUE
    mask_reasons_each(VALUE self)
    {
        MaskReasons * m_ptr;
        Data_Get_Struct(self, MaskReasons, m_ptr);
        for (MaskReason i(static_cast<MaskReason>(0)), i_end(last_mr) ; i != i_end ;
                i = static_cast<MaskReason>(static_cast<int>(i) + 1))
            if ((*m_ptr)[i])
                rb_yield(INT2FIX(i));
        return Qnil;
    }

    /*
     * call-seq:
     *     empty? -> true or false
     *
     * Is the collection empty.
     */
    VALUE
    mask_reasons_empty(VALUE self)
    {
        MaskReasons * m_ptr;
        Data_Get_Struct(self, MaskReasons, m_ptr);
        return m_ptr->any() ? Qfalse : Qtrue;
    }

    /*
     * call-seq:
     *     set(mask_reason) -> Nil
     *
     * Add MaskReason to collection.
     */
    VALUE
    mask_reasons_set(VALUE self, VALUE mask_reason)
    {
        MaskReasons * m_ptr;
        Data_Get_Struct(self, MaskReasons, m_ptr);
        try
        {
            int mr = NUM2INT(mask_reason);
            if (mr < 0 || mr >= last_mr)
                rb_raise(rb_eArgError, "MaskReason out of range");
            m_ptr->set(mr);
            return Qnil;

        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     == other_mask_reason -> True or False
     *
     * Are two MaskReasons equal
     */
    VALUE
    mask_reasons_equal(VALUE self, VALUE other)
    {
        MaskReasons * m_ptr;
        Data_Get_Struct(self, MaskReasons, m_ptr);
        try
        {
            MaskReasons mr = value_to_mask_reasons(other);
            return (*m_ptr) == mr ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_mask_reasons()
    {
        /*
         * Document-class: Paludis::MaskReasons
         *
         * A collection of reasons for why a package is masked. Includes
         * Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html]
         * but not Comparable.
         */
        c_mask_reasons = rb_define_class_under(paludis_module(), "MaskReasons", rb_cObject);
        rb_define_singleton_method(c_mask_reasons, "new", RUBY_FUNC_CAST(&mask_reasons_new), 0);
        rb_define_method(c_mask_reasons, "initialize", RUBY_FUNC_CAST(&mask_reasons_init), 0);
        rb_define_method(c_mask_reasons, "each", RUBY_FUNC_CAST(&mask_reasons_each), 0);
        rb_include_module(c_mask_reasons, rb_mEnumerable);
        rb_define_method(c_mask_reasons, "empty?", RUBY_FUNC_CAST(&mask_reasons_empty), 0);
        rb_define_method(c_mask_reasons, "set", RUBY_FUNC_CAST(&mask_reasons_set), 1);
        rb_define_method(c_mask_reasons, "==", RUBY_FUNC_CAST(&mask_reasons_equal), 1);

        /*
         * Document-module: Paludis::MaskReason
         *
         * Each value represents one reason for a package being masked.
         */
        c_mask_reason = rb_define_module_under(paludis_module(), "MaskReason");
        for (MaskReason l(static_cast<MaskReason>(0)), l_end(last_mr) ; l != l_end ;
                l = static_cast<MaskReason>(static_cast<int>(l) + 1))
            rb_define_const(c_mask_reason, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/mask_reasons.hh, MaskReason, c_mask_reason>
    }
}

VALUE
paludis::ruby::mask_reasons_to_value(const MaskReasons & m)
{
    return Data_Wrap_Struct(c_mask_reasons, 0, &Common<MaskReasons>::free, new MaskReasons(m));
}

MaskReasons
paludis::ruby::value_to_mask_reasons(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_mask_reasons))
    {
        MaskReasons * v_ptr;
        Data_Get_Struct(v, MaskReasons, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into MaskReasons", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_mask_reasons PALUDIS_ATTRIBUTE((used))
    (&do_register_mask_reasons);


