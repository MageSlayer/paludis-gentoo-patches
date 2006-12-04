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
#include <paludis/contents.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_contents;
    static VALUE c_contents_entry;
    static VALUE c_contents_file_entry;
    static VALUE c_contents_dir_entry;
    static VALUE c_contents_sym_entry;
    static VALUE c_contents_misc_entry;

    VALUE
    contents_init(VALUE self)
    {
        return self;
    }

    VALUE
    contents_new(VALUE self)
    {
        Contents::Pointer * ptr(0);
        try
        {
            ptr = new Contents::Pointer(new Contents());
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<Contents::Pointer>::free, ptr));
            rb_obj_call_init(tdata, 0, 0);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE contents_add(VALUE self, VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_contents_entry))
        {
            try
            {
                Contents::Pointer * self_ptr;
                Data_Get_Struct(self, Contents::Pointer, self_ptr);
                ContentsEntry::ConstPointer * v_ptr;
                Data_Get_Struct(v, ContentsEntry::ConstPointer, v_ptr);
                (*self_ptr)->add(*v_ptr);
                return self;

            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into ContentsEntry", rb_obj_classname(v));
        }
    }

    VALUE
    contents_entries(VALUE self)
    {
        Contents::Pointer * ptr;
        Data_Get_Struct(self, Contents::Pointer, ptr);

        VALUE result(rb_ary_new());
        for (Contents::Iterator i ((*ptr)->begin()), i_end((*ptr)->end()) ; i != i_end; ++i)
            rb_ary_push(result, contents_entry_to_value(*i));
        return result;
    }

    VALUE
    contents_entry_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    template <typename A_>
    struct ContentsNew
    {
        static VALUE
        contents_entry_new(int argc, VALUE * argv, VALUE self)
        {
            typename A_::ConstPointer * ptr(0);
            try
            {
                if (1 == argc)
                {
                    ptr = new typename A_::ConstPointer(new A_(StringValuePtr(argv[0])));
                }
                else
                {
                    rb_raise(rb_eArgError, "ContentsEntry expects one argument, but got %d",argc);
                }
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<typename A_::ConstPointer>::free, ptr));
                rb_obj_call_init(tdata, argc, argv);
                return tdata;
            }
            catch (const std::exception & e)
            {
                delete ptr;
                exception_to_ruby_exception(e);
            }
        }
    };

    VALUE contents_sym_entry_new(int argc, VALUE * argv, VALUE self)
    {
        ContentsSymEntry::ConstPointer * ptr(0);
        try
        {
            if (2 == argc)
            {
                ptr = new ContentsSymEntry::ConstPointer(new ContentsSymEntry(StringValuePtr(argv[0]), StringValuePtr(argv[1])));
            }
            else
            {
                rb_raise(rb_eArgError, "ContentsSymEntry expects two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<ContentsSymEntry::ConstPointer>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    template <typename T_, std::string (T_::* m_) () const>
    struct ContentsThings
    {
        static VALUE
        fetch(VALUE self)
        {
            typename T_::ConstPointer * ptr;
            Data_Get_Struct(self,typename T_::ConstPointer, ptr);
            return rb_str_new2((((**ptr).*m_)()).c_str());
        }
    };

    void do_register_contents()
    {
        c_contents = rb_define_class_under(paludis_module(), "Contents", rb_cObject);
        rb_define_singleton_method(c_contents, "new", RUBY_FUNC_CAST(&contents_new), 0);
        rb_define_method(c_contents, "initialize", RUBY_FUNC_CAST(&contents_init), 0);
        rb_define_method(c_contents, "entries", RUBY_FUNC_CAST(&contents_entries), 0);
        rb_define_method(c_contents, "add", RUBY_FUNC_CAST(&contents_add), 1);

        c_contents_entry = rb_define_class_under(paludis_module(), "ContentsEntry", rb_cObject);
        rb_funcall(c_contents_entry, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_contents_entry, "name", RUBY_FUNC_CAST((&ContentsThings<ContentsEntry,&ContentsEntry::name>::fetch)), 0);
        rb_define_method(c_contents_entry, "initialize", RUBY_FUNC_CAST(&contents_entry_init),-1);
        rb_define_method(c_contents_entry, "to_s", RUBY_FUNC_CAST(&Common<ContentsEntry::ConstPointer>::to_s_via_ptr), 0);

        c_contents_file_entry = rb_define_class_under(paludis_module(), "ContentsFileEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_file_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsFileEntry>::contents_entry_new)), -1);
        c_contents_dir_entry = rb_define_class_under(paludis_module(), "ContentsDirEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_dir_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsDirEntry>::contents_entry_new)), -1);
        c_contents_misc_entry = rb_define_class_under(paludis_module(), "ContentsMiscEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_misc_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsMiscEntry>::contents_entry_new)), -1);
        c_contents_sym_entry = rb_define_class_under(paludis_module(), "ContentsSymEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_sym_entry, "new", RUBY_FUNC_CAST(&contents_sym_entry_new), -1);
        rb_define_method(c_contents_sym_entry, "to_s", RUBY_FUNC_CAST(&Common<ContentsSymEntry::ConstPointer>::to_s_via_ptr), 0);
        rb_define_method(c_contents_sym_entry, "target", RUBY_FUNC_CAST((&ContentsThings<ContentsSymEntry,&ContentsSymEntry::target>::fetch)), 0);
    }
}

VALUE
paludis::ruby::contents_entry_to_value(ContentsEntry::ConstPointer m)
{
    struct V :
        ContentsVisitorTypes::ConstVisitor
    {
        VALUE value;
        ContentsEntry::ConstPointer mm;

        V(ContentsEntry::ConstPointer _m) :
            mm(_m)
        {
        }

        void visit(const ContentsFileEntry *)
        {
            value = Data_Wrap_Struct(c_contents_file_entry, 0, &Common<ContentsFileEntry::ConstPointer>::free,
                    new ContentsFileEntry::ConstPointer(mm));
        }

        void visit(const ContentsDirEntry *)
        {
            value = Data_Wrap_Struct(c_contents_dir_entry, 0, &Common<ContentsDirEntry::ConstPointer>::free,
                    new ContentsDirEntry::ConstPointer(mm));
        }

        void visit(const ContentsMiscEntry *)
        {
            value = Data_Wrap_Struct(c_contents_misc_entry, 0, &Common<ContentsMiscEntry::ConstPointer>::free,
                    new ContentsMiscEntry::ConstPointer(mm));
        }

        void visit(const ContentsSymEntry *)
        {
            value = Data_Wrap_Struct(c_contents_sym_entry, 0, &Common<ContentsSymEntry::ConstPointer>::free,
                    new ContentsSymEntry::ConstPointer(mm));
        }
    };

    ContentsEntry::ConstPointer * m_ptr(0);
    try
    {
        V v(m);
        m->accept(&v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::contents_to_value(Contents::ConstPointer m)
{
    Contents::ConstPointer * m_ptr(0);
    try
    {
        m_ptr = new Contents::ConstPointer(m);
        return Data_Wrap_Struct(c_contents, 0, &Common<Contents::ConstPointer>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_contents PALUDIS_ATTRIBUTE((used))
    (&do_register_contents);

