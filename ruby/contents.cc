/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Richard Brown
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_contents;
    static VALUE c_contents_entry;
    static VALUE c_contents_file_entry;
    static VALUE c_contents_dir_entry;
    static VALUE c_contents_sym_entry;
    static VALUE c_contents_other_entry;

    struct V
    {
        VALUE value;
        std::shared_ptr<const ContentsEntry> mm;

        V(std::shared_ptr<const ContentsEntry> _m) :
            mm(_m)
        {
        }

        void visit(const ContentsFileEntry &)
        {
            value = Data_Wrap_Struct(c_contents_file_entry, 0, &Common<std::shared_ptr<const ContentsEntry> >::free,
                    new std::shared_ptr<const ContentsEntry>(mm));
        }

        void visit(const ContentsDirEntry &)
        {
            value = Data_Wrap_Struct(c_contents_dir_entry, 0, &Common<std::shared_ptr<const ContentsEntry> >::free,
                    new std::shared_ptr<const ContentsEntry>(mm));
        }

        void visit(const ContentsOtherEntry &)
        {
            value = Data_Wrap_Struct(c_contents_other_entry, 0, &Common<std::shared_ptr<const ContentsEntry> >::free,
                    new std::shared_ptr<const ContentsEntry>(mm));
        }

        void visit(const ContentsSymEntry &)
        {
            value = Data_Wrap_Struct(c_contents_sym_entry, 0, &Common<std::shared_ptr<const ContentsEntry> >::free,
                    new std::shared_ptr<const ContentsEntry>(mm));
        }
    };

    VALUE
    contents_entry_to_value(std::shared_ptr<const ContentsEntry> m)
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
        std::shared_ptr<Contents> * ptr(0);
        try
        {
            ptr = new std::shared_ptr<Contents>(new Contents());
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Contents> >::free, ptr));
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
                std::shared_ptr<Contents> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<Contents>, self_ptr);
                std::shared_ptr<const ContentsEntry> * v_ptr;
                Data_Get_Struct(v, std::shared_ptr<const ContentsEntry>, v_ptr);
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
        std::shared_ptr<Contents> * ptr;
        Data_Get_Struct(self, std::shared_ptr<Contents>, ptr);

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
            typename std::shared_ptr<const ContentsEntry> * ptr(0);
            try
            {
                if (1 == argc)
                {
                    ptr = new std::shared_ptr<const ContentsEntry>(std::make_shared<A_>(
                                FSPath(StringValuePtr(argv[0]))));
                }
                else
                {
                    rb_raise(rb_eArgError, "ContentsEntry expects one argument, but got %d",argc);
                }
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const ContentsEntry> >::free, ptr));
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
        std::shared_ptr<const ContentsEntry> * ptr(0);
        try
        {
            if (2 == argc)
            {
                ptr = new std::shared_ptr<const ContentsEntry>(std::make_shared<ContentsSymEntry>(
                            FSPath(StringValuePtr(argv[0])), StringValuePtr(argv[1])));
            }
            else
            {
                rb_raise(rb_eArgError, "ContentsSymEntry expects two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const ContentsEntry> >::free, ptr));
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
     * Document-method: location_key
     *
     * Returns our location
     */
    static VALUE
    contents_entry_location_key(VALUE self)
    {
        std::shared_ptr<const ContentsEntry> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const ContentsEntry>, ptr);
        return metadata_key_to_value((*ptr)->location_key());
    }

    /*
     * Document-method: target_key
     *
     * Returns our target (as per readlink)
     */
    static VALUE
    contents_sym_entry_target_key(VALUE self)
    {
        std::shared_ptr<const ContentsEntry> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const ContentsEntry>, ptr);
        return metadata_key_to_value((std::static_pointer_cast<const ContentsSymEntry>(*ptr))->target_key());
    }

    /*
     * call-seq:
     *     each_metadata {|key| block } -> Nil
     *
     * Our metadata.
     */
    VALUE
    contents_entry_each_metadata(VALUE self)
    {
        std::shared_ptr<const ContentsEntry> * self_ptr;
        Data_Get_Struct(self, std::shared_ptr<const ContentsEntry>, self_ptr);
        for (ContentsEntry::MetadataConstIterator it((*self_ptr)->begin_metadata()),
                it_end((*self_ptr)->end_metadata()); it_end != it; ++it)
        {
            VALUE val(metadata_key_to_value(*it));
            if (Qnil != val)
                rb_yield(val);
        }
        return Qnil;
    }

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
        rb_define_method(c_contents_entry, "location_key", RUBY_FUNC_CAST(&contents_entry_location_key), 0);
        rb_define_method(c_contents_entry, "initialize", RUBY_FUNC_CAST(&contents_entry_init),-1);
        rb_define_method(c_contents_entry, "each_metadata", RUBY_FUNC_CAST(&contents_entry_each_metadata), 0);

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
         * Document-class: Paludis::ContentsOtherEntry
         *
         *  An 'other' ContentsEntry
         */
        c_contents_other_entry = rb_define_class_under(paludis_module(), "ContentsOtherEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_other_entry, "new", RUBY_FUNC_CAST((&ContentsNew<ContentsOtherEntry>::contents_entry_new)), -1);


        /*
         * Document-class: Paludis::ContentsSymEntry
         *
         * A symlink ContentsEntry
         */
        c_contents_sym_entry = rb_define_class_under(paludis_module(), "ContentsSymEntry", c_contents_entry);
        rb_define_singleton_method(c_contents_sym_entry, "new", RUBY_FUNC_CAST(&contents_sym_entry_new), -1);
        rb_define_method(c_contents_sym_entry, "target_key", RUBY_FUNC_CAST(&contents_sym_entry_target_key), 0);
    }
}

VALUE
paludis::ruby::contents_to_value(std::shared_ptr<const Contents> m)
{
    std::shared_ptr<const Contents> * m_ptr(0);
    try
    {
        m_ptr = new std::shared_ptr<const Contents>(m);
        return Data_Wrap_Struct(c_contents, 0, &Common<std::shared_ptr<const Contents> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_contents PALUDIS_ATTRIBUTE((used))
    (&do_register_contents);

