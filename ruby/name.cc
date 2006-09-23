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
#include <paludis/name.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    template <typename T_, typename E_>
    struct NameWrapper
    {
        static VALUE c_class;
        static VALUE c_class_except;

        static VALUE
        name_part_error_new(VALUE the_class, VALUE value)
        {
            VALUE argv[1];
            std::string * ptr(new std::string(STR2CSTR(value)));
            VALUE tdata(Data_Wrap_Struct(the_class, 0, &Common<std::string>::free, ptr));
            argv[0] = value;
            rb_obj_call_init(tdata, 1, argv);
            rb_call_super(1, &value);
            return tdata;
        }

        static VALUE
        name_part_error_init(VALUE self, VALUE)
        {
            return self;
        }

        static VALUE
        name_part_new(VALUE the_class, VALUE value)
        {
            VALUE argv[1];
            T_ * ptr(0);
            try
            {
                ptr = new T_(std::string(STR2CSTR(value)));
                VALUE tdata(Data_Wrap_Struct(the_class, 0, &Common<T_>::free, ptr));
                argv[0] = value;
                rb_obj_call_init(tdata, 1, argv);
                return tdata;
            }
            catch (const NameError & e)
            {
                delete ptr;
                rb_raise(c_class_except, e.message().c_str());
            }
            catch (const std::exception & e)
            {
                delete ptr;
                exception_to_ruby_exception(e);
            }
        }

        static VALUE
        name_part_init(VALUE self, VALUE)
        {
            return self;
        }

        static void do_register(const std::string & name)
        {
            c_class = rb_define_class(name.c_str(), rb_cObject);
            rb_define_singleton_method(c_class, "new", RUBY_FUNC_CAST(&name_part_new), 1);
            rb_define_method(c_class, "initialize", RUBY_FUNC_CAST(&name_part_init), 1);
            rb_define_method(c_class, "<=>", RUBY_FUNC_CAST(&Common<T_>::compare), 1);
            rb_include_module(c_class, rb_mComparable);
            rb_define_method(c_class, "to_s", RUBY_FUNC_CAST(&Common<T_>::to_s), 0);

            c_class_except = rb_define_class((name + "Error").c_str(), rb_eRuntimeError);
            rb_define_singleton_method(c_class_except, "new", RUBY_FUNC_CAST(&name_part_error_new), 1);
            rb_define_method(c_class_except, "initialize", RUBY_FUNC_CAST(&name_part_error_init), 1);
        }
    };

    template <typename T_, typename E_> VALUE NameWrapper<T_, E_>::c_class;
    template <typename T_, typename E_> VALUE NameWrapper<T_, E_>::c_class_except;

    static VALUE c_qualified_package_name;
    static VALUE c_qualified_package_name_error;

    VALUE
    category_name_part_plus(VALUE left, VALUE right)
    {
        return rb_funcall(c_qualified_package_name, rb_intern("new"), 2, left, right);
    }

    VALUE
    qualified_package_name_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    qualified_package_name_new(int argc, VALUE * argv, VALUE self)
    {
        QualifiedPackageName * ptr(0);
        try
        {
            if (1 == argc)
                ptr = new QualifiedPackageName(std::string(STR2CSTR(argv[0])));
            else if (2 == argc)
            {
                CategoryNamePart * cat_ptr;
                Data_Get_Struct(argv[0], CategoryNamePart, cat_ptr);
                PackageNamePart * pkg_ptr;
                Data_Get_Struct(argv[1], PackageNamePart, pkg_ptr);
                ptr = new QualifiedPackageName(*cat_ptr, *pkg_ptr);
            }
            else
                rb_raise(rb_eArgError, "QualifiedPackageName.new expects one or two arguments, but got %d", argc);

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<QualifiedPackageName>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const NameError & e)
        {
            delete ptr;
            rb_raise(c_qualified_package_name_error, e.message().c_str());
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    qualified_package_name_error_new(VALUE the_class, VALUE value)
    {
        VALUE argv[1];
        std::string * ptr(new std::string(STR2CSTR(value)));
        VALUE tdata(Data_Wrap_Struct(the_class, 0, &Common<std::string>::free, ptr));
        argv[0] = value;
        rb_obj_call_init(tdata, 1, argv);
        rb_call_super(1, &value);
        return tdata;
    }

    VALUE
    qualified_package_name_error_init(VALUE self, VALUE)
    {
        return self;
    }

    void do_register_names()
    {
        NameWrapper<PackageNamePart, PackageNamePartError>::do_register("PackageNamePart");
        NameWrapper<CategoryNamePart, CategoryNamePartError>::do_register("CategoryNamePart");
        NameWrapper<UseFlagName, UseFlagNameError>::do_register("UseFlagName");
        NameWrapper<SlotName, SlotNameError>::do_register("SlotName");
        NameWrapper<RepositoryName, RepositoryNameError>::do_register("RepositoryName");
        NameWrapper<KeywordName, KeywordNameError>::do_register("KeywordName");

        rb_define_method(NameWrapper<CategoryNamePart, CategoryNamePartError>::c_class,
                "+", RUBY_FUNC_CAST(&category_name_part_plus), 1);

        c_qualified_package_name = rb_define_class("QualifiedPackageName", rb_cObject);
        rb_define_singleton_method(c_qualified_package_name, "new", RUBY_FUNC_CAST(&qualified_package_name_new), -1);
        rb_define_method(c_qualified_package_name, "initialize", RUBY_FUNC_CAST(&qualified_package_name_init), -1);
        rb_define_method(c_qualified_package_name, "<=>", RUBY_FUNC_CAST(&Common<QualifiedPackageName>::compare), 1);
        rb_include_module(c_qualified_package_name, rb_mComparable);
        rb_define_method(c_qualified_package_name, "to_s", RUBY_FUNC_CAST(&Common<QualifiedPackageName>::to_s), 0);

        c_qualified_package_name_error = rb_define_class("QualifiedPackageNameError", rb_eRuntimeError);
        rb_define_singleton_method(c_qualified_package_name_error, "new", RUBY_FUNC_CAST(&qualified_package_name_error_new), 1);
        rb_define_method(c_qualified_package_name_error, "initialize", RUBY_FUNC_CAST(&qualified_package_name_error_init), 1);
    }
}

RegisterRubyClass::Register paludis_ruby_register_name PALUDIS_ATTRIBUTE((used)) (&do_register_names);

