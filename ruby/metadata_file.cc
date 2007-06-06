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
#include <paludis/qa/metadata_file.hh>
#include <paludis/util/stringify.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::qa;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_metadata_file;

    VALUE
    metadata_file_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     MetadataFile.new(file_name) -> MetadataFile
     *
     * Create a new MetadataFile instance from the specified file.
     */
    VALUE
    metadata_file_new(int argc, VALUE *argv, VALUE self)
    {
        MetadataFile * ptr(0);
        try
        {
            if (1 == argc)
            {
                std::string s = StringValuePtr(argv[0]);
                FSEntry fse = FSEntry(s);
                if (fse.is_regular_file())
                    ptr = new MetadataFile(fse);
                else
                    ptr = new MetadataFile(s);
            }
            else
            {
                rb_raise(rb_eArgError, "MetadataFile expects one argument, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<MetadataFile>::free, ptr));
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
     *     herds -> Array
     *
     * Array of herds specified in file.
     */
    VALUE
    metadata_file_herds(VALUE self)
    {
        MetadataFile * ptr;
        Data_Get_Struct(self, MetadataFile, ptr);

        VALUE result(rb_ary_new());
        for (MetadataFile::HerdsIterator i (ptr->begin_herds()),
                i_end(ptr->end_herds()) ; i != i_end ; ++i)
            rb_ary_push(result, rb_str_new2(i->c_str()));
        return result;
    }

    /*
     * call-seq:
     *     maintainers -> Hash
     *
     * Hash of maintainers specified in file, keys are :email and :name.
     */
    VALUE
    metadata_file_maintainers(VALUE self)
    {
        MetadataFile * ptr;
        Data_Get_Struct(self, MetadataFile, ptr);
        VALUE result(rb_ary_new());
        VALUE result_hash;
        for (MetadataFile::MaintainersIterator i(ptr->begin_maintainers()),
                i_end(ptr->end_maintainers()) ; i != i_end ; ++i)
        {
            if (i->first.empty() && i->second.empty())
                continue;

            result_hash = rb_hash_new();
            if (!i->first.empty())
                rb_hash_aset(result_hash, ID2SYM(rb_intern("email")), rb_str_new2(i->first.c_str()));
            if (!i->second.empty())
                rb_hash_aset(result_hash, ID2SYM(rb_intern("name")), rb_str_new2(i->second.c_str()));
            rb_ary_push(result, result_hash);
        }
        return result;
    }
    void do_register_metadata_file()
    {
        /*
         * Document-class: Paludis::QA::MetadataFile
         *
         * Wrapper around metadata.xml files.
         */
        c_metadata_file = rb_define_class_under(paludis_qa_module(), "MetadataFile", rb_cObject);
        rb_define_singleton_method(c_metadata_file, "new", RUBY_FUNC_CAST(&metadata_file_new),-1);
        rb_define_method(c_metadata_file, "initialize", RUBY_FUNC_CAST(&metadata_file_init),-1);
        rb_define_method(c_metadata_file, "herds", RUBY_FUNC_CAST(&metadata_file_herds),0);
        rb_define_method(c_metadata_file, "maintainers", RUBY_FUNC_CAST(&metadata_file_maintainers),0);
    }
}

VALUE
paludis::ruby::metadata_file_to_value(const MetadataFile & v)
{
    MetadataFile * vv(new MetadataFile(v));
    return Data_Wrap_Struct(c_metadata_file, 0, &Common<MetadataFile>::free, vv);
}

RegisterRubyClass::Register paludis_ruby_register_metadata_file PALUDIS_ATTRIBUTE((used))
    (&do_register_metadata_file);
