/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
 * Copyright (c) 2008 Richard Brown
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
#include <paludis/version_spec.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_qualified_package_name;

    VALUE
    qualified_package_name_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     QualifiedPackageName.new(category_and_package_string) -> QualifiedPackageName
     *     QualifiedPackageName.new(category_name, package_name) -> QualifiedPackageName
     *
     * Creates a new QualifiedPackageName.
     */
    VALUE
    qualified_package_name_new(int argc, VALUE *argv, VALUE self)
    {
        QualifiedPackageName * ptr(0);
        try
        {
            if (1 == argc)
            {
                ptr = new QualifiedPackageName(StringValuePtr(argv[0]));
            }
            else if (2 == argc)
            {
                ptr = new QualifiedPackageName(CategoryNamePart(StringValuePtr(argv[0])), PackageNamePart(StringValuePtr(argv[1])));
            }
            else
            {
                rb_raise(rb_eArgError, "QualifiedPackageName expects one or two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<QualifiedPackageName>::free, ptr));
            rb_obj_call_init(tdata,argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: category
     *
     * call-seq:
     *     category -> String
     *
     * Fetch the category part of the QualifiedPackageName.
     */
    /*
     * Document-method: package
     *
     * call-seq:
     *     package -> String
     *
     * Fetch the package name part of the QualifiedPackageName.
     */
    template <typename T_, typename M_, const M_ (QualifiedPackageName::* m_)() const>
    struct QPNMember
    {
        static VALUE
        fetch(VALUE self)
        {
            QualifiedPackageName * p;
            Data_Get_Struct(self, QualifiedPackageName, p);
            return rb_str_new2(stringify((p->*m_)()).c_str());
        }

        static VALUE
        set(VALUE self, VALUE str)
        {
            try
            {
                QualifiedPackageName * p;
                Data_Get_Struct(self, QualifiedPackageName, p);
                (p->*m_)() =  T_ ((StringValuePtr(str)));
                return self;
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    VALUE
    qualified_package_name_compare(VALUE left, VALUE right)
    {
        try
        {
            QualifiedPackageName l = value_to_qualified_package_name(left);
            QualifiedPackageName r = value_to_qualified_package_name(right);
            if (l < r)
                return INT2FIX(-1);
            if (l > r)
                return INT2FIX(1);
            return INT2FIX(0);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_qualified_package_name()
    {
        /*
         * Document-class: Paludis::QualifiedPackageName
         *
         * Represents a category plus package name. Includes
         * Comparable[http://www.ruby-doc.org/core/classes/Comparable.html]
         */
        c_qualified_package_name = rb_define_class_under(paludis_module(), "QualifiedPackageName", rb_cObject);
        rb_define_singleton_method(c_qualified_package_name, "new", RUBY_FUNC_CAST(&qualified_package_name_new), -1);
        rb_define_method(c_qualified_package_name, "initialize", RUBY_FUNC_CAST(&qualified_package_name_init), -1);
        rb_define_method(c_qualified_package_name, "<=>", RUBY_FUNC_CAST(&qualified_package_name_compare), 1);
        rb_include_module(c_qualified_package_name, rb_mComparable);
        rb_define_method(c_qualified_package_name, "to_s", RUBY_FUNC_CAST(&Common<QualifiedPackageName>::to_s), 0);
        rb_define_alias(c_qualified_package_name, "to_str", "to_s");
        rb_define_method(c_qualified_package_name, "hash", RUBY_FUNC_CAST(&Common<QualifiedPackageName>::hash), 0);
        rb_define_method(c_qualified_package_name, "eql?", RUBY_FUNC_CAST(&Common<QualifiedPackageName>::equal), 1);
        rb_define_method(c_qualified_package_name, "category",
                RUBY_FUNC_CAST((&QPNMember<CategoryNamePart, CategoryNamePart, &QualifiedPackageName::category>::fetch)), 0);
        rb_define_method(c_qualified_package_name, "package",
                RUBY_FUNC_CAST((&QPNMember<PackageNamePart, PackageNamePart, &QualifiedPackageName::package>::fetch)), 0);
    }
}

QualifiedPackageName
paludis::ruby::value_to_qualified_package_name(VALUE v)
{
    if (T_STRING == TYPE(v))
        return QualifiedPackageName(StringValuePtr(v));
    else if (rb_obj_is_kind_of(v, c_qualified_package_name))
    {
        QualifiedPackageName * v_ptr;
        Data_Get_Struct(v, QualifiedPackageName, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into QualifiedPackageName", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::qualified_package_name_to_value(const QualifiedPackageName & qpn)
{
    QualifiedPackageName * qpn2(new QualifiedPackageName(qpn));
    return Data_Wrap_Struct(c_qualified_package_name, 0, &Common<QualifiedPackageName>::free, qpn2);
}

RegisterRubyClass::Register paludis_ruby_register_qualified_package_name PALUDIS_ATTRIBUTE((used))
    (&do_register_qualified_package_name);
