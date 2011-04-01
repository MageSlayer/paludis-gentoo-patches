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
    static VALUE c_package_name_part_constraint;
    static VALUE c_category_name_part_constraint;
    static VALUE c_in_repository_constraint;
    static VALUE c_from_repository_constraint;
    static VALUE c_installed_at_path_constraint;
    static VALUE c_installable_to_path_constraint;
    static VALUE c_installable_to_repository_constraint;
    static VALUE c_any_slot_constraint;
    static VALUE c_exact_slot_constraint;
    static VALUE c_key_constraint;

    static VALUE c_key_constraint_operation;

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

        void visit(const CategoryNamePartConstraint &)
        {
            value = Data_Wrap_Struct(c_category_name_part_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const PackageNamePartConstraint &)
        {
            value = Data_Wrap_Struct(c_package_name_part_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const InRepositoryConstraint &)
        {
            value = Data_Wrap_Struct(c_in_repository_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const FromRepositoryConstraint &)
        {
            value = Data_Wrap_Struct(c_from_repository_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const InstalledAtPathConstraint &)
        {
            value = Data_Wrap_Struct(c_installed_at_path_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const InstallableToPathConstraint &)
        {
            value = Data_Wrap_Struct(c_installable_to_path_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const InstallableToRepositoryConstraint &)
        {
            value = Data_Wrap_Struct(c_installable_to_repository_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const AnySlotConstraint &)
        {
            value = Data_Wrap_Struct(c_any_slot_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const ExactSlotConstraint &)
        {
            value = Data_Wrap_Struct(c_exact_slot_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
                    new std::shared_ptr<const PackageDepSpecConstraint>(mm));
        }

        void visit(const KeyConstraint &)
        {
            value = Data_Wrap_Struct(c_key_constraint, 0, &Common<std::shared_ptr<const PackageDepSpecConstraint> >::free,
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

    /*
     * Document-method: name_part
     *
     * The name part constraint.
     */
    static VALUE
    package_name_part_constraint_name_part(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const PackageNamePartConstraint>(*ptr))->name_part()).c_str());
    }

    /*
     * Document-method: name_part
     *
     * The name part constraint.
     */
    static VALUE
    category_name_part_constraint_name_part(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const CategoryNamePartConstraint>(*ptr))->name_part()).c_str());
    }

    /*
     * Document-method: name
     *
     * The name constraint.
     */
    static VALUE
    in_repository_constraint_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const InRepositoryConstraint>(*ptr))->name()).c_str());
    }

    /*
     * Document-method: name
     *
     * The name constraint.
     */
    static VALUE
    from_repository_constraint_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const FromRepositoryConstraint>(*ptr))->name()).c_str());
    }

    /*
     * Document-method: path
     *
     * The path constraint.
     */
    static VALUE
    installed_at_path_constraint_path(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const InstalledAtPathConstraint>(*ptr))->path()).c_str());
    }

    /*
     * Document-method: path
     *
     * The path constraint.
     */
    static VALUE
    installable_to_path_constraint_path(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const InstallableToPathConstraint>(*ptr))->path()).c_str());
    }

    /*
     * Document-method: include_masked?
     *
     * The include-masked constraint.
     */
    static VALUE
    installable_to_path_constraint_include_masked(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return (std::static_pointer_cast<const InstallableToPathConstraint>(*ptr))->include_masked() ? Qtrue : Qfalse;
    }

    /*
     * Document-method: name
     *
     * The name constraint.
     */
    static VALUE
    installable_to_repository_constraint_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const InstallableToRepositoryConstraint>(*ptr))->name()).c_str());
    }

    /*
     * Document-method: include_masked?
     *
     * The include-masked constraint.
     */
    static VALUE
    installable_to_repository_constraint_include_masked(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return (std::static_pointer_cast<const InstallableToRepositoryConstraint>(*ptr))->include_masked() ? Qtrue : Qfalse;
    }

    /*
     * Document-method: locking?
     *
     * The locking constraint.
     */
    static VALUE
    any_slot_constraint_locking(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return (std::static_pointer_cast<const AnySlotConstraint>(*ptr))->locking() ? Qtrue : Qfalse;
    }

    /*
     * Document-method: locked?
     *
     * The locked constraint.
     */
    static VALUE
    exact_slot_constraint_locked(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return (std::static_pointer_cast<const ExactSlotConstraint>(*ptr))->locked() ? Qtrue : Qfalse;
    }

    /*
     * Document-method: name
     *
     * The name constraint.
     */
    static VALUE
    exact_slot_constraint_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const ExactSlotConstraint>(*ptr))->name()).c_str());
    }

    /*
     * Document-method: key
     *
     * The key constraint.
     */
    static VALUE
    key_constraint_key(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const KeyConstraint>(*ptr))->key()).c_str());
    }

    /*
     * Document-method: pattern
     *
     * The pattern constraint.
     */
    static VALUE
    key_constraint_pattern(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const KeyConstraint>(*ptr))->pattern()).c_str());
    }

    /*
     * Document-method: operation
     *
     * The operation constraint.
     */
    static VALUE
    key_constraint_operation(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecConstraint> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecConstraint>, ptr);
        return INT2FIX((std::static_pointer_cast<const KeyConstraint>(*ptr))->operation());
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

        /*
         * Document-class: Paludis::PackageNamePartConstraint
         *
         * Represents a /pkg name constraint in a PackageDepSpec.
         */
        c_package_name_part_constraint = rb_define_class_under(
                paludis_module(), "PackageNamePartConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_package_name_part_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_package_name_part_constraint, "name_part", RUBY_FUNC_CAST(
                    &package_name_part_constraint_name_part), 0);

        /*
         * Document-class: Paludis::CategoryNamePartConstraint
         *
         * Represents a /pkg name constraint in a PackageDepSpec.
         */
        c_category_name_part_constraint = rb_define_class_under(
                paludis_module(), "CategoryNamePartConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_category_name_part_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_category_name_part_constraint, "name_part", RUBY_FUNC_CAST(
                    &category_name_part_constraint_name_part), 0);

        /*
         * Document-class: Paludis::InRepositoryConstraint
         *
         * Represents a /pkg name constraint in a PackageDepSpec.
         */
        c_in_repository_constraint = rb_define_class_under(
                paludis_module(), "InRepositoryConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_in_repository_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_in_repository_constraint, "name", RUBY_FUNC_CAST(
                    &in_repository_constraint_name), 0);

        /*
         * Document-class: Paludis::FromRepositoryConstraint
         *
         * Represents a /pkg name constraint in a PackageDepSpec.
         */
        c_from_repository_constraint = rb_define_class_under(
                paludis_module(), "InRepositoryConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_from_repository_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_from_repository_constraint, "name", RUBY_FUNC_CAST(
                    &from_repository_constraint_name), 0);

        /*
         * Document-class: Paludis::InstalledAtPathConstraint
         *
         * Represents a ::/ path constraint in a PackageDepSpec.
         */
        c_installed_at_path_constraint = rb_define_class_under(
                paludis_module(), "InRepositoryConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_installed_at_path_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_installed_at_path_constraint, "path", RUBY_FUNC_CAST(
                    &installed_at_path_constraint_path), 0);

        /*
         * Document-class: Paludis::InstallableToPathConstraint
         *
         * Represents a ::/? path constraint in a PackageDepSpec.
         */
        c_installable_to_path_constraint = rb_define_class_under(
                paludis_module(), "InRepositoryConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_installable_to_path_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_installable_to_path_constraint, "path", RUBY_FUNC_CAST(
                    &installable_to_path_constraint_path), 0);
        rb_define_method(c_installable_to_path_constraint, "include_masked?", RUBY_FUNC_CAST(
                    &installable_to_path_constraint_include_masked), 0);

        /*
         * Document-class: Paludis::InstallableToRepositoryConstraint
         *
         * Represents a ::repo? repository constraint in a PackageDepSpec.
         */
        c_installable_to_repository_constraint = rb_define_class_under(
                paludis_module(), "InstallableToRepositoryConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_installable_to_repository_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_installable_to_repository_constraint, "name", RUBY_FUNC_CAST(
                    &installable_to_repository_constraint_name), 0);
        rb_define_method(c_installable_to_repository_constraint, "include_masked?", RUBY_FUNC_CAST(
                    &installable_to_repository_constraint_include_masked), 0);

        /*
         * Document-class: Paludis::AnySlotConstraint
         *
         * Represents a :* or := constraint in a PackageDepSpec.
         */
        c_any_slot_constraint = rb_define_class_under(
                paludis_module(), "AnySlotConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_any_slot_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_any_slot_constraint, "locking?", RUBY_FUNC_CAST(
                    &any_slot_constraint_locking), 0);

        /*
         * Document-class: Paludis::ExactSlotConstraint
         *
         * Represents a :slot or :=slot constraint in a PackageDepSpec.
         */
        c_exact_slot_constraint = rb_define_class_under(
                paludis_module(), "ExactSlotConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_exact_slot_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_exact_slot_constraint, "locked?", RUBY_FUNC_CAST(
                    &exact_slot_constraint_locked), 0);
        rb_define_method(c_exact_slot_constraint, "name", RUBY_FUNC_CAST(
                    &exact_slot_constraint_name), 0);

        /*
         * Document-class: Paludis::KeyConstraint
         *
         * Represents a [.key=value] constraint in a PackageDepSpec.
         */
        c_key_constraint = rb_define_class_under(
                paludis_module(), "KeyConstraint", c_package_dep_spec_constraint);
        rb_funcall(c_key_constraint, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_key_constraint, "key", RUBY_FUNC_CAST(
                    &key_constraint_key), 0);
        rb_define_method(c_key_constraint, "pattern", RUBY_FUNC_CAST(
                    &key_constraint_pattern), 0);
        rb_define_method(c_key_constraint, "operation", RUBY_FUNC_CAST(
                    &key_constraint_operation), 0);

        /*
         * Document-module: Paludis::KeyConstraintOperation
         *
         * The operation for a KeyConstraint.
         */
        c_key_constraint_operation = rb_define_module_under(paludis_module(), "KeyConstraintOperation");
        for (KeyConstraintOperation l(static_cast<KeyConstraintOperation>(0)), l_end(last_kco) ; l != l_end ;
                l = static_cast<KeyConstraintOperation>(static_cast<int>(l) + 1))
            rb_define_const(c_key_constraint_operation, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/package_dep_spec_constraint-se.hh, KeyConstraint, c_key_constraint_operation>
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

