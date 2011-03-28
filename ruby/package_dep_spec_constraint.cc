/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/package_dep_spec_constraint.hh>

#include <algorithm>
#include <list>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_package_dep_spec_constraint;
    static VALUE c_name_constraint;

    struct V
    {
        VALUE value;
        std::shared_ptr<const PackageDepSpecConstraint> mm;

        V(const std::shared_ptr<const PackageDepSpecConstraint> & m) :
            mm(m)
        {
        }

        void visit(const NameConstraint &)
        {
            value = Data_Wrap_Struct(c_name_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }
    };

    /*
     * Document-method: name
     *
     * The name constraint.
     */
    static VALUE
    name_constraint_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return qualified_package_name_to_value((std::static_pointer_cast<const NameConstraint>(*ptr))->name());
    }

    void do_register_package_dep_spec_constraint()
    {
        /*
         * Document-class: Paludis::PackageDepSpecConstraint
         *
         * Represents a constraint in a PackageDepSpec.
         */
        c_package_dep_spec_constraint = rb_define_class_under(paludis_module(), "PackageDepSpecConstraint", rb_cObject);
        rb_funcall(c_package_dep_spec_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::NameConstraint
         *
         * Represents a cat/pkg name constraint in a PackageDepSpec.
         */
        c_name_constraint = rb_define_class_under(
                paludis_module(), "NameConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_name_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_package_dep_spec_constraint, "name", RUBY_FUNC_CAST(&name_constraint_name), 0);
    }
}

VALUE
paludis::ruby::package_dep_spec_constraint_to_value(const std::shared_ptr<const PackageDepSpecConstraint> & m)
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

RegisterRubyClass::Register paludis_ruby_register_package_dep_spec_constraint PALUDIS_ATTRIBUTE((used))
    (&do_register_package_dep_spec_constraint);

