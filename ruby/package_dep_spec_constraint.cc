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

#include <paludis/package_dep_spec_requirement.hh>

#include <algorithm>
#include <list>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_package_dep_spec_requirement;
    static VALUE c_name_requirement;
    static VALUE c_package_name_part_requirement;
    static VALUE c_category_name_part_requirement;
    static VALUE c_version_requirement;
    static VALUE c_in_repository_requirement;
    static VALUE c_from_repository_requirement;
    static VALUE c_installed_at_path_requirement;
    static VALUE c_installable_to_path_requirement;
    static VALUE c_installable_to_repository_requirement;
    static VALUE c_any_slot_requirement;
    static VALUE c_exact_slot_requirement;
    static VALUE c_key_requirement;
    static VALUE c_choice_requirement;

    static VALUE c_key_requirement_operation;

    struct V
    {
        VALUE value;
        std::shared_ptr<const PackageDepSpecRequirement> mm;

        V(const std::shared_ptr<const PackageDepSpecRequirement> & m) :
            mm(m)
        {
        }

        void visit(const NameRequirement &)
        {
            value = Data_Wrap_Struct(c_name_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const CategoryNamePartRequirement &)
        {
            value = Data_Wrap_Struct(c_category_name_part_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const PackageNamePartRequirement &)
        {
            value = Data_Wrap_Struct(c_package_name_part_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const VersionRequirement &)
        {
            value = Data_Wrap_Struct(c_version_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const InRepositoryRequirement &)
        {
            value = Data_Wrap_Struct(c_in_repository_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const FromRepositoryRequirement &)
        {
            value = Data_Wrap_Struct(c_from_repository_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const InstalledAtPathRequirement &)
        {
            value = Data_Wrap_Struct(c_installed_at_path_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const InstallableToPathRequirement &)
        {
            value = Data_Wrap_Struct(c_installable_to_path_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const InstallableToRepositoryRequirement &)
        {
            value = Data_Wrap_Struct(c_installable_to_repository_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const AnySlotRequirement &)
        {
            value = Data_Wrap_Struct(c_any_slot_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const ExactSlotRequirement &)
        {
            value = Data_Wrap_Struct(c_exact_slot_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const KeyRequirement &)
        {
            value = Data_Wrap_Struct(c_key_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }

        void visit(const ChoiceRequirement &)
        {
            value = Data_Wrap_Struct(c_choice_requirement, 0, &Common<std::shared_ptr<const PackageDepSpecRequirement> >::free,
                    new std::shared_ptr<const PackageDepSpecRequirement>(mm));
        }
    };

    /*
     * Document-method: name
     *
     * The name requirement.
     */
    static VALUE
    name_requirement_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return qualified_package_name_to_value((std::static_pointer_cast<const NameRequirement>(*ptr))->name());
    }

    /*
     * Document-method: name_part
     *
     * The name part requirement.
     */
    static VALUE
    package_name_part_requirement_name_part(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const PackageNamePartRequirement>(*ptr))->name_part()).c_str());
    }

    /*
     * Document-method: name_part
     *
     * The name part requirement.
     */
    static VALUE
    category_name_part_requirement_name_part(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const CategoryNamePartRequirement>(*ptr))->name_part()).c_str());
    }

    /*
     * Document-method: name
     *
     * The name requirement.
     */
    static VALUE
    in_repository_requirement_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const InRepositoryRequirement>(*ptr))->name()).c_str());
    }

    /*
     * Document-method: name
     *
     * The name requirement.
     */
    static VALUE
    from_repository_requirement_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const FromRepositoryRequirement>(*ptr))->name()).c_str());
    }

    /*
     * Document-method: path
     *
     * The path requirement.
     */
    static VALUE
    installed_at_path_requirement_path(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const InstalledAtPathRequirement>(*ptr))->path()).c_str());
    }

    /*
     * Document-method: path
     *
     * The path requirement.
     */
    static VALUE
    installable_to_path_requirement_path(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const InstallableToPathRequirement>(*ptr))->path()).c_str());
    }

    /*
     * Document-method: include_masked?
     *
     * The include-masked requirement.
     */
    static VALUE
    installable_to_path_requirement_include_masked(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return (std::static_pointer_cast<const InstallableToPathRequirement>(*ptr))->include_masked() ? Qtrue : Qfalse;
    }

    /*
     * Document-method: name
     *
     * The name requirement.
     */
    static VALUE
    installable_to_repository_requirement_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const InstallableToRepositoryRequirement>(*ptr))->name()).c_str());
    }

    /*
     * Document-method: include_masked?
     *
     * The include-masked requirement.
     */
    static VALUE
    installable_to_repository_requirement_include_masked(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return (std::static_pointer_cast<const InstallableToRepositoryRequirement>(*ptr))->include_masked() ? Qtrue : Qfalse;
    }

    /*
     * Document-method: locking?
     *
     * The locking requirement.
     */
    static VALUE
    any_slot_requirement_locking(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return (std::static_pointer_cast<const AnySlotRequirement>(*ptr))->locking() ? Qtrue : Qfalse;
    }

    /*
     * Document-method: locked?
     *
     * The locked requirement.
     */
    static VALUE
    exact_slot_requirement_locked(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return (std::static_pointer_cast<const ExactSlotRequirement>(*ptr))->locked() ? Qtrue : Qfalse;
    }

    /*
     * Document-method: name
     *
     * The name requirement.
     */
    static VALUE
    exact_slot_requirement_name(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const ExactSlotRequirement>(*ptr))->name()).c_str());
    }

    /*
     * Document-method: key
     *
     * The key requirement.
     */
    static VALUE
    key_requirement_key(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const KeyRequirement>(*ptr))->key()).c_str());
    }

    /*
     * Document-method: pattern
     *
     * The pattern requirement.
     */
    static VALUE
    key_requirement_pattern(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return rb_str_new2(stringify((std::static_pointer_cast<const KeyRequirement>(*ptr))->pattern()).c_str());
    }

    /*
     * Document-method: operation
     *
     * The operation requirement.
     */
    static VALUE
    key_requirement_operation(VALUE self)
    {
        std::shared_ptr<const PackageDepSpecRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const PackageDepSpecRequirement>, ptr);
        return INT2FIX((std::static_pointer_cast<const KeyRequirement>(*ptr))->operation());
    }

    void do_register_package_dep_spec_requirement()
    {
        /*
         * Document-class: Paludis::PackageDepSpecRequirement
         *
         * Represents a requirement in a PackageDepSpec.
         */
        c_package_dep_spec_requirement = rb_define_class_under(paludis_module(), "PackageDepSpecRequirement", rb_cObject);
        rb_funcall(c_package_dep_spec_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::NameRequirement
         *
         * Represents a cat/pkg name requirement in a PackageDepSpec.
         */
        c_name_requirement = rb_define_class_under(
                paludis_module(), "NameRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_name_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_package_dep_spec_requirement, "name", RUBY_FUNC_CAST(&name_requirement_name), 0);

        /*
         * Document-class: Paludis::PackageNamePartRequirement
         *
         * Represents a /pkg name requirement in a PackageDepSpec.
         */
        c_package_name_part_requirement = rb_define_class_under(
                paludis_module(), "PackageNamePartRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_package_name_part_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_package_name_part_requirement, "name_part", RUBY_FUNC_CAST(
                    &package_name_part_requirement_name_part), 0);

        /*
         * Document-class: Paludis::CategoryNamePartRequirement
         *
         * Represents a /pkg name requirement in a PackageDepSpec.
         */
        c_category_name_part_requirement = rb_define_class_under(
                paludis_module(), "CategoryNamePartRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_category_name_part_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_category_name_part_requirement, "name_part", RUBY_FUNC_CAST(
                    &category_name_part_requirement_name_part), 0);

        /*
         * Document-class: Paludis::InRepositoryRequirement
         *
         * Represents a /pkg name requirement in a PackageDepSpec.
         */
        c_in_repository_requirement = rb_define_class_under(
                paludis_module(), "InRepositoryRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_in_repository_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_in_repository_requirement, "name", RUBY_FUNC_CAST(
                    &in_repository_requirement_name), 0);

        /*
         * Document-class: Paludis::FromRepositoryRequirement
         *
         * Represents a /pkg name requirement in a PackageDepSpec.
         */
        c_from_repository_requirement = rb_define_class_under(
                paludis_module(), "FromRepositoryRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_from_repository_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_from_repository_requirement, "name", RUBY_FUNC_CAST(
                    &from_repository_requirement_name), 0);

        /*
         * Document-class: Paludis::InstalledAtPathRequirement
         *
         * Represents a ::/ path requirement in a PackageDepSpec.
         */
        c_installed_at_path_requirement = rb_define_class_under(
                paludis_module(), "InstalledAtPathRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_installed_at_path_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_installed_at_path_requirement, "path", RUBY_FUNC_CAST(
                    &installed_at_path_requirement_path), 0);

        /*
         * Document-class: Paludis::InstallableToPathRequirement
         *
         * Represents a ::/? path requirement in a PackageDepSpec.
         */
        c_installable_to_path_requirement = rb_define_class_under(
                paludis_module(), "InstallableToPathRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_installable_to_path_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_installable_to_path_requirement, "path", RUBY_FUNC_CAST(
                    &installable_to_path_requirement_path), 0);
        rb_define_method(c_installable_to_path_requirement, "include_masked?", RUBY_FUNC_CAST(
                    &installable_to_path_requirement_include_masked), 0);

        /*
         * Document-class: Paludis::InstallableToRepositoryRequirement
         *
         * Represents a ::repo? repository requirement in a PackageDepSpec.
         */
        c_installable_to_repository_requirement = rb_define_class_under(
                paludis_module(), "InstallableToRepositoryRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_installable_to_repository_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_installable_to_repository_requirement, "name", RUBY_FUNC_CAST(
                    &installable_to_repository_requirement_name), 0);
        rb_define_method(c_installable_to_repository_requirement, "include_masked?", RUBY_FUNC_CAST(
                    &installable_to_repository_requirement_include_masked), 0);

        /*
         * Document-class: Paludis::AnySlotRequirement
         *
         * Represents a :* or := requirement in a PackageDepSpec.
         */
        c_any_slot_requirement = rb_define_class_under(
                paludis_module(), "AnySlotRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_any_slot_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_any_slot_requirement, "locking?", RUBY_FUNC_CAST(
                    &any_slot_requirement_locking), 0);

        /*
         * Document-class: Paludis::ExactSlotRequirement
         *
         * Represents a :slot or :=slot requirement in a PackageDepSpec.
         */
        c_exact_slot_requirement = rb_define_class_under(
                paludis_module(), "ExactSlotRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_exact_slot_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_exact_slot_requirement, "locked?", RUBY_FUNC_CAST(
                    &exact_slot_requirement_locked), 0);
        rb_define_method(c_exact_slot_requirement, "name", RUBY_FUNC_CAST(
                    &exact_slot_requirement_name), 0);

        /*
         * Document-class: Paludis::KeyRequirement
         *
         * Represents a [.key=value] requirement in a PackageDepSpec.
         */
        c_key_requirement = rb_define_class_under(
                paludis_module(), "KeyRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_key_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_key_requirement, "key", RUBY_FUNC_CAST(
                    &key_requirement_key), 0);
        rb_define_method(c_key_requirement, "pattern", RUBY_FUNC_CAST(
                    &key_requirement_pattern), 0);
        rb_define_method(c_key_requirement, "operation", RUBY_FUNC_CAST(
                    &key_requirement_operation), 0);

        /*
         * Document-module: Paludis::KeyRequirementOperation
         *
         * The operation for a KeyRequirement.
         */
        c_key_requirement_operation = rb_define_module_under(paludis_module(), "KeyRequirementOperation");
        for (KeyRequirementOperation l(static_cast<KeyRequirementOperation>(0)), l_end(last_kro) ; l != l_end ;
                l = static_cast<KeyRequirementOperation>(static_cast<int>(l) + 1))
            rb_define_const(c_key_requirement_operation, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/package_dep_spec_requirement-se.hh, KeyRequirement, c_key_requirement_operation>

        /*
         * Document-class: Paludis::ChoiceRequirement
         *
         * Represents a [flag] requirement in a PackageDepSpec.
         */
        c_choice_requirement = rb_define_class_under(
                paludis_module(), "ChoiceRequirement", c_package_dep_spec_requirement);
        rb_funcall(c_choice_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
    }
}

VALUE
paludis::ruby::package_dep_spec_requirement_to_value(const std::shared_ptr<const PackageDepSpecRequirement> & m)
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

RegisterRubyClass::Register paludis_ruby_register_package_dep_spec_requirement PALUDIS_ATTRIBUTE((used))
    (&do_register_package_dep_spec_requirement);

