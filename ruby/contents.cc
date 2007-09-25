/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Richard Brown <rbrown@gentoo.org>
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
#include <paludis/util/visitor-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
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
    static VALUE c_contents_dev_entry;
    static VALUE c_contents_fifo_entry;

    struct V :
            ConstVisitor<ContentsVisitorTypes>
        {
            VALUE value;
            tr1::shared_ptr<const ContentsEntry> mm;

            V(tr1::shared_ptr<const ContentsEntry> _m) :
                mm(_m)
            {
            }

            void visit(const ContentsFileEntry &)
            {
                value = Data_Wrap_Struct(c_contents_file_entry, 0, &Common<tr1::shared_ptr<const ContentsFileEntry> >::free,
                        new tr1::shared_ptr<const ContentsFileEntry>(tr1::static_pointer_cast<const ContentsFileEntry>(mm)));
            }

            void visit(const ContentsDirEntry &)
            {
                value = Data_Wrap_Struct(c_contents_dir_entry, 0, &Common<tr1::shared_ptr<const ContentsDirEntry> >::free,
                        new tr1::shared_ptr<const ContentsDirEntry>(tr1::static_pointer_cast<const ContentsDirEntry>(mm)));
            }

            void visit(const ContentsMiscEntry &)
            {
                value = Data_Wrap_Struct(c_contents_misc_entry, 0, &Common<tr1::shared_ptr<const ContentsMiscEntry> >::free,
                        new tr1::shared_ptr<const ContentsMiscEntry>(tr1::static_pointer_cast<const ContentsMiscEntry>(mm)));
            }

            void visit(const ContentsSymEntry &)
            {
                value = Data_Wrap_Struct(c_contents_sym_entry, 0, &Common<tr1::shared_ptr<const ContentsSymEntry> >::free,
                        new tr1::shared_ptr<const ContentsSymEntry>(tr1::static_pointer_cast<const ContentsSymEntry>(mm)));
            }

            void visit(const ContentsFifoEntry &)
            {
                value = Data_Wrap_Struct(c_contents_fifo_entry, 0, &Common<tr1::shared_ptr<const ContentsFifoEntry> >::free,
                        new tr1::shared_ptr<const ContentsFifoEntry>(tr1::static_pointer_cast<const ContentsFifoEntry>(mm)));
            }

            void visit(const ContentsDevEntry &)
            {
                value = Data_Wrap_Struct(c_contents_dev_entry, 0, &Common<tr1::shared_ptr<const ContentsDevEntry> >::free,
                        new tr1::shared_ptr<const ContentsDevEntry>(tr1::static_pointer_cast<const ContentsDevEntry>(mm)));
            }
        };


    VALUE
    contents_entry_to_value(tr1::shared_ptr<const ContentsEntry> m)
    {
        try
        {
            V v(m);
            m->accept(v);
            return v.value;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    contents_init(VALUE self)
    {
        return self;
    }

    VALUE
    contents_new(VALUE self)
    {
        tr1::shared_ptr<Contents> * ptr(0);
        try
        {
            ptr = new tr1::shared_ptr<Contents>(new Contents());
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<Contents> >::free, ptr));
            rb_obj_call_init(tdata, 0, 0);
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
     *     add(contents_entry)
     *
     * Add a new entry.
    */
    VALUE contents_add(VALUE self, VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_contents_entry))
        {
            try
            {
                tr1::shared_ptr<Contents> * self_ptr;
                Data_Get_Struct(self, tr1::shared_ptr<Contents>, self_ptr);
                tr1::shared_ptr<const ContentsEntry> * v_ptr;
                Data_Get_Struct(v, tr1::shared_ptr<const ContentsEntry>, v_ptr);
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

    /*
     * call-seq:
     *     each {|contents_entry| block}
     *
     * Iterate through our entries.
     */
    VALUE
    contents_each(VALUE self)
    {
        tr1::shared_ptr<Contents> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<Contents>, ptr);

        for (Contents::ConstIterator i ((*ptr)->begin()), i_end((*ptr)->end()) ; i != i_end; ++i)
            rb_yield(contents_entry_to_value(*i));
        return self;
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
            typename tr1::shared_ptr<const A_> * ptr(0);
            try
            {
                if (1 == argc)
                {
                    ptr = new tr1::shared_ptr<const A_>(new A_(StringValuePtr(argv[0])));
                }
                else
                {
                    rb_raise(rb_eArgError, "ContentsEntry expects one argument, but got %d",argc);
                }
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const A_> >::free, ptr));
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
        tr1::shared_ptr<const ContentsSymEntry> * ptr(0);
        try
        {
            if (2 == argc)
            {
                ptr = new tr1::shared_ptr<const ContentsSymEntry>(new ContentsSymEntry(StringValuePtr(argv[0]), StringValuePtr(argv[1])));
            }
            else
            {
                rb_raise(rb_eArgError, "ContentsSymEntry expects two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const ContentsSymEntry> >::free, ptr));
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
     * Document-method: name
     *
     * Returns our name
     */
    /*
     * Document-method: target
     *
     * Returns the target
     */
    template <typename T_, std::string (T_::* m_) () const>
    struct ContentsThings
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<const T_> * ptr;
            Data_Get_Struct(self, tr1::shared_ptr<const T_>, ptr);
            return rb_str_new2((((**ptr).*m_)()).c_str());
        }
    };

    void do_register_contents()
    {
        /*
         * Document-class: Paludis::Contents
         *
         * A package's contents. Includes Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html],
         * but not Comparable.
         */
        c_contents = rb_define_class_under(paludis_module(), "Contents", rb_cObject);
        rb_include_module(c_contents, rb_mEnumerable);
        rb_define_singleton_method(c_contents, "new", RUBY_FUNC_CAST(&contents_new), 0);
        rb_define_method(c_contents, "initialize", RUBY_FUNC_CAST(&contents_init), 0);
        rb_define_method(c_contents, "each", RUBY_FUNC_CAST(&contents_each), 0);
        rb_define_method(c_contents, "add", RUBY_FUNC_CAST(&contents_add), 1);

        /*
         * Document-class: Paludis::ContentsEntry
         *
         * Base class for a ContentsEntry
         */
        c_contents_entry = rb_define_class_under(paludis_module(), "ContentsEntry", rb_cObject);
        rb_funcall(c_contents_entry, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_contents_entry, "name", RUBY_FUNC_CAST((&ContentsThings<ContentsEntry,&ContentsEntry::name>::fetch)), 0);
        rb_define_method(c_contents_entry, "initialize", RUBY_FUNC_CAST(&contents_entry_init),-1);
        rb_define_method(c_contents_entry, "to_s", RUBY_FUNC_CAST(&Common<tr1::shared_ptr<const ContentsEntry> >::to_s_via_ptr), 0);

        /*
         * Document-class: Paludis::ContentsFileEntry
         *
         * A file ContentsEntry
         */
        c_contents_file_entry = rb_define_class_under(paludis_module(), "ContentsFileEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_file_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsFileEntry>::contents_entry_new)), -1);

        /*
         * Document-class: Paludis::ContentsDirEntry
         *
         * A directory ContentsEntry
         */
        c_contents_dir_entry = rb_define_class_under(paludis_module(), "ContentsDirEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_dir_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsDirEntry>::contents_entry_new)), -1);

        /*
         * Document-class: Paludis::ContentsMiscEntry
         *
         *  A miscellaneous ContentsEntry
         */
        c_contents_misc_entry = rb_define_class_under(paludis_module(), "ContentsMiscEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_misc_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsMiscEntry>::contents_entry_new)), -1);


        /*
         * Document-class: Paludis::ContentsFifoEntry
         *
         *  A fifo ContentsEntry
         */
        c_contents_fifo_entry = rb_define_class_under(paludis_module(), "ContentsFifoEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_fifo_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsFifoEntry>::contents_entry_new)), -1);


        /*
         * Document-class: Paludis::ContentsDevEntry
         *
         *  A device ContentsEntry
         */
        c_contents_dev_entry = rb_define_class_under(paludis_module(), "ContentsDevEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_dev_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsDevEntry>::contents_entry_new)), -1);

        /*
         * Document-class: Paludis::ContentsSymEntry
         *
         * A symlink ContentsEntry
         */
        c_contents_sym_entry = rb_define_class_under(paludis_module(), "ContentsSymEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_sym_entry, "new", RUBY_FUNC_CAST(&contents_sym_entry_new), -1);
        rb_define_method(c_contents_sym_entry, "target", RUBY_FUNC_CAST((&ContentsThings<ContentsSymEntry,&ContentsSymEntry::target>::fetch)), 0);
        rb_define_method(c_contents_sym_entry, "to_s", RUBY_FUNC_CAST(&Common<tr1::shared_ptr<const ContentsSymEntry> >::to_s_via_ptr), 0);
    }
}

VALUE
paludis::ruby::contents_to_value(tr1::shared_ptr<const Contents> m)
{
    tr1::shared_ptr<const Contents> * m_ptr(0);
    try
    {
        m_ptr = new tr1::shared_ptr<const Contents>(m);
        return Data_Wrap_Struct(c_contents, 0, &Common<tr1::shared_ptr<const Contents> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_contents PALUDIS_ATTRIBUTE((used))
    (&do_register_contents);

