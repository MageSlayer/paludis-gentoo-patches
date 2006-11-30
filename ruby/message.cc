/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Richard Brown <mynamewasgone@gmail.com>
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
#include <paludis/qa/message.hh>
#include <paludis/util/stringify.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::qa;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_message;
    static VALUE c_qa_level;

    VALUE
    message_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    message_new(int argc, VALUE *argv, VALUE self)
    {
        Message * ptr(0);
        try
        {
            if (2 == argc)
            {
                int i = NUM2INT(argv[0]);
                if (i < 0|| i >= last_qal)
                    rb_raise(rb_eArgError, "QALevel out of range");
                ptr = new Message(static_cast<QALevel>(i), StringValuePtr(argv[1]));
            }
            else
            {
                rb_raise(rb_eArgError, "Message expects two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<Message>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    message_msg(VALUE self)
    {
        Message * ptr;
        Data_Get_Struct(self, Message, ptr);
        return rb_str_new2((ptr)->msg.c_str());
    }

    VALUE
    message_level(VALUE self)
    {
        Message * ptr;
        Data_Get_Struct(self, Message, ptr);
        return INT2NUM((ptr)->level);
    }
    void do_register_message()
    {
        c_qa_level = rb_define_module_under(paludis_qa_module(), "QALevel");
        for (QALevel l(static_cast<QALevel>(0)), l_end(last_qal) ; l != l_end ;
                l = static_cast<QALevel>(static_cast<int>(l) + 1))
            rb_define_const(c_qa_level, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        c_message = rb_define_class_under(paludis_qa_module(), "Message", rb_cObject);
        rb_define_singleton_method(c_message, "new", RUBY_FUNC_CAST(&message_new),-1);
        rb_define_method(c_message, "initialize", RUBY_FUNC_CAST(&message_init),-1);
        rb_define_method(c_message, "to_s", RUBY_FUNC_CAST(&Common<Message>::to_s),0);
        rb_define_method(c_message, "msg", RUBY_FUNC_CAST(&message_msg),0);
        rb_define_method(c_message, "level", RUBY_FUNC_CAST(&message_level),0);
    }
}

Message
paludis::ruby::value_to_message(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_message))
    {
        Message * v_ptr;
        Data_Get_Struct(v, Message, v_ptr);
        return *v_ptr;
    }
    else
    {
        std::string message = "TypeError: can't convert " + std::string(rb_obj_classname(v)) + " into Message";
        rb_raise(rb_eTypeError, message.c_str());
    }
}

VALUE
paludis::ruby::message_to_value(const Message & v)
{
    Message * vv(new Message(v));
    return Data_Wrap_Struct(c_message, 0, &Common<Message>::free, vv);
}

RegisterRubyClass::Register paludis_ruby_register_message PALUDIS_ATTRIBUTE((used))
    (&do_register_message);
