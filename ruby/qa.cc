/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown <rbrown@gentoo.org>
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
#include <paludis/qa.hh>
#include <paludis/util/options.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_qa_message_level;
    static VALUE c_qa_check_property;
    static VALUE c_qa_check_properties;
    static VALUE c_qa_message;
    static VALUE c_qa_reporter;

    VALUE
    qa_check_properties_init(VALUE self)
    {
        return self;
    }

    VALUE
    qa_check_properties_new(VALUE self)
    {
        QACheckProperties * ptr(0);
        try
        {
            ptr = new QACheckProperties;
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<QACheckProperties>::free, ptr));
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
     *     each {|qa_check_property| block } -> Nil
     *
     * Iterate through the qa check properties.
     */
    VALUE
    qa_check_properties_each(VALUE self)
    {
        QACheckProperties * m_ptr;
        Data_Get_Struct(self, QACheckProperties, m_ptr);
        for (QACheckProperty i(static_cast<QACheckProperty>(0)), i_end(last_qacp) ; i != i_end ;
                i = static_cast<QACheckProperty>(static_cast<int>(i) + 1))
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
    qa_check_properties_empty(VALUE self)
    {
        QACheckProperties * m_ptr;
        Data_Get_Struct(self, QACheckProperties, m_ptr);
        return m_ptr->any() ? Qfalse : Qtrue;
    }

    /*
     * call-seq:
     *     set(qa_check_property) -> Nil
     *
     * Add QACheckProperty to collection.
     */
    VALUE
    qa_check_properties_set(VALUE self, VALUE qa_check_property)
    {
        QACheckProperties * m_ptr;
        Data_Get_Struct(self, QACheckProperties, m_ptr);
        try
        {
            int mr = NUM2INT(qa_check_property);
            if (mr < 0 || mr >= last_mro)
                rb_raise(rb_eArgError, "QACheckProperty out of range");
            *m_ptr += static_cast<QACheckProperty>(mr);
            return Qnil;

        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     message(qa_message)
     *
     * Process a qa error message
     */
    VALUE
    ruby_qa_reporter_message(VALUE)
    {
        return Qnil;
    }

    VALUE
    qa_message_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     QAMessage.new(fs_entry_string, qa_message_level, name_string, message_string) -> QAMessage
     *
     * Creates a new QAMessage.
     */
    VALUE
    qa_message_new(int argc, VALUE *argv, VALUE self)
    {
        QAMessage * ptr(0);
        try
        {
            if (4 == argc)
            {
                int ml = NUM2INT(argv[1]);
                if (ml < 0 || ml >= last_qaml)
                    rb_raise(rb_eArgError, "QAMessageLevel out of range");

                ptr = new QAMessage(FSEntry(StringValuePtr(argv[0])), static_cast<QAMessageLevel>(ml),
                        StringValuePtr(argv[2]), StringValuePtr(argv[3]));
            }
            else
            {
                rb_raise(rb_eArgError, "QAMessage expects four arguments, but got %d", argc);
            }

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<QAMessage>::free, ptr));
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
     *     level -> QAMessageLevel
     *
     * Fetch QAMessage level.
     */
    VALUE
    qa_message_level(VALUE self)
    {
        QAMessage * m_ptr;
        Data_Get_Struct(self, QAMessage, m_ptr);
        return INT2FIX(m_ptr->level);
    }

    /*
     * call-seq:
     *     level=
     *
     * Set QAMessage level.
     */
    VALUE
    qa_message_level_set(VALUE self, VALUE qa_message_level)
    {
        QAMessage * m_ptr;
        Data_Get_Struct(self, QAMessage, m_ptr);
        try
        {
            int ml = NUM2INT(qa_message_level);
            if (ml < 0 || ml >= last_qaml)
                rb_raise(rb_eArgError, "QAMessageLevel out of range");
            m_ptr->level = static_cast<QAMessageLevel>(ml);
            return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    template <typename T_, typename M_, M_ T_::*m_>
    struct FetchSetString
    {
        static VALUE
        fetch(VALUE self)
        {
            T_ * p;
            Data_Get_Struct(self, T_, p);
            return rb_str_new2(stringify(p->*m_).c_str());
        }

        static VALUE
        set(VALUE self, VALUE str)
        {
            try
            {
                T_ * p;
                Data_Get_Struct(self, T_, p);
                p->*m_ = M_ ((StringValuePtr(str)));
                return Qnil;
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    void do_register_qa()
    {
        /*
         * Document-classs: Paludis::QAReporter
         *
         * Base class for QAReporter, create a new sublclass that implements a message function.
         */
        c_qa_reporter = rb_define_class_under(paludis_module(), "QAReporter", rb_cObject);
        rb_define_method(c_qa_reporter, "message", RUBY_FUNC_CAST(&ruby_qa_reporter_message), 1);

        /*
         * Document-class: Paludis::QACheckProperties
         *
         * A collection of properties for a QACheck. Includes
         * Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html]
         * but not Comparable.
         */
        c_qa_check_properties = rb_define_class_under(paludis_module(), "QACheckProperties", rb_cObject);
        rb_define_singleton_method(c_qa_check_properties, "new", RUBY_FUNC_CAST(&qa_check_properties_new), 0);
        rb_define_method(c_qa_check_properties, "initialize", RUBY_FUNC_CAST(&qa_check_properties_init), 0);
        rb_define_method(c_qa_check_properties, "each", RUBY_FUNC_CAST(&qa_check_properties_each), 0);
        rb_include_module(c_qa_check_properties, rb_mEnumerable);
        rb_define_method(c_qa_check_properties, "empty?", RUBY_FUNC_CAST(&qa_check_properties_empty), 0);
        rb_define_method(c_qa_check_properties, "set", RUBY_FUNC_CAST(&qa_check_properties_set), 1);
        rb_define_method(c_qa_check_properties, "add", RUBY_FUNC_CAST(&qa_check_properties_set), 1);

        /*
         * Document-module: Paludis::QAMessageLevel
         *
         * The importance of a QA notice.
         *
         */
        c_qa_message_level = rb_define_module_under(paludis_module(), "QAMessageLevel");
        for (QAMessageLevel l(static_cast<QAMessageLevel>(0)), l_end(last_qaml) ; l != l_end ;
                l = static_cast<QAMessageLevel>(static_cast<int>(l) + 1))
            rb_define_const(c_qa_message_level, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        /*
         * Document-module: Paludis::QACheckProperty
         *
         * Properties of a QA check.
         *
         */
        c_qa_check_property = rb_define_module_under(paludis_module(), "QACheckProperty");
        for (QACheckProperty l(static_cast<QACheckProperty>(0)), l_end(last_qacp) ; l != l_end ;
                l = static_cast<QACheckProperty>(static_cast<int>(l) + 1))
            rb_define_const(c_qa_check_property, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        /*
         * Document-class: Paludis::QAMessage
         *
         * QA message.
         *
         */
        c_qa_message = rb_define_class_under(paludis_module(), "QAMessage", rb_cObject);
        rb_define_singleton_method(c_qa_message, "new", RUBY_FUNC_CAST(&qa_message_new), -1);
        rb_define_method(c_qa_message, "initialize", RUBY_FUNC_CAST(&qa_message_init), -1);
        rb_define_method(c_qa_message, "entry",
                RUBY_FUNC_CAST((&FetchSetString<QAMessage, FSEntry, &QAMessage::entry>::fetch)), 0);
        rb_define_method(c_qa_message, "entry=",
                RUBY_FUNC_CAST((&FetchSetString<QAMessage, FSEntry, &QAMessage::entry>::set)), 1);
        rb_define_method(c_qa_message, "level", RUBY_FUNC_CAST(&qa_message_level), 0);
        rb_define_method(c_qa_message, "level=", RUBY_FUNC_CAST(&qa_message_level_set), 1);
        rb_define_method(c_qa_message, "name",
                RUBY_FUNC_CAST((&FetchSetString<QAMessage, std::string, &QAMessage::name>::fetch)), 0);
        rb_define_method(c_qa_message, "name=",
                RUBY_FUNC_CAST((&FetchSetString<QAMessage, std::string, &QAMessage::name>::set)), 1);
        rb_define_method(c_qa_message, "message",
                RUBY_FUNC_CAST((&FetchSetString<QAMessage, std::string, &QAMessage::message>::fetch)), 0);
        rb_define_method(c_qa_message, "message=",
                RUBY_FUNC_CAST((&FetchSetString<QAMessage, std::string, &QAMessage::message>::set)), 1);
    }
}

RegisterRubyClass::Register paludis_ruby_register_qa PALUDIS_ATTRIBUTE((used))
    (&do_register_qa);

QACheckProperties
paludis::ruby::value_to_qa_check_properties(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_qa_check_properties))
    {
        QACheckProperties * v_ptr;
        Data_Get_Struct(v, QACheckProperties, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into QACheckProperties", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::qa_message_to_value(const QAMessage & qamsg)
{
    QAMessage * qamsg2(new QAMessage(qamsg));
    return Data_Wrap_Struct(c_qa_message, 0, &Common<QAMessage>::free, qamsg2);
}
