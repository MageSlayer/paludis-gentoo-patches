/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/package_dep_spec_properties.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_dep_spec_requirement.hh>

#include <paludis/util/sequence.hh>

using namespace paludis;

namespace
{
    struct Finder
    {
        bool has_any_slot_requirement;
        bool has_category_name_part;
        bool has_choice_requirements;
        bool has_exact_slot_requirement;
        bool has_from_repository;
        bool has_in_repository;
        bool has_installable_to_path;
        bool has_installable_to_repository;
        bool has_installed_at_path;
        bool has_key_requirements;
        bool has_package;
        bool has_package_name_part;
        bool has_tag;
        bool has_version_requirements;

        Finder() :
            has_any_slot_requirement(false),
            has_category_name_part(false),
            has_choice_requirements(false),
            has_exact_slot_requirement(false),
            has_from_repository(false),
            has_in_repository(false),
            has_installable_to_path(false),
            has_installable_to_repository(false),
            has_installed_at_path(false),
            has_key_requirements(false),
            has_package(false),
            has_package_name_part(false),
            has_tag(false),
            has_version_requirements(false)
        {
        }

        void visit(const NameRequirement &)
        {
            has_package = true;
        }

        void visit(const PackageNamePartRequirement &)
        {
            has_package_name_part = true;
        }

        void visit(const CategoryNamePartRequirement &)
        {
            has_category_name_part = true;
        }

        void visit(const VersionRequirement &)
        {
            has_version_requirements = true;
        }

        void visit(const InRepositoryRequirement &)
        {
            has_in_repository = true;
        }

        void visit(const FromRepositoryRequirement &)
        {
            has_from_repository = true;
        }

        void visit(const InstalledAtPathRequirement &)
        {
            has_installed_at_path = true;
        }

        void visit(const InstallableToRepositoryRequirement &)
        {
            has_installable_to_repository = true;
        }

        void visit(const InstallableToPathRequirement &)
        {
            has_installable_to_path = true;
        }

        void visit(const ExactSlotRequirement &)
        {
            has_exact_slot_requirement = true;
        }

        void visit(const AnySlotRequirement &)
        {
            has_any_slot_requirement = true;
        }

        void visit(const ChoiceRequirement &)
        {
            has_choice_requirements = true;
        }

        void visit(const KeyRequirement &)
        {
            has_key_requirements = true;
        }
    };
}

bool
paludis::package_dep_spec_has_properties(const PackageDepSpec & spec, const PackageDepSpecProperties & properties)
{
    Finder f;
    for (auto r(spec.requirements()->begin()), r_end(spec.requirements()->end()) ;
            r != r_end ; ++r)
        (*r)->accept(f);

    if (! properties.has_any_slot_requirement().is_indeterminate())
        if (properties.has_any_slot_requirement().is_true() != f.has_any_slot_requirement)
            return false;

    if (! properties.has_category_name_part().is_indeterminate())
        if (properties.has_category_name_part().is_true() != f.has_category_name_part)
            return false;

    if (! properties.has_choice_requirements().is_indeterminate())
        if (properties.has_choice_requirements().is_true() != f.has_choice_requirements)
            return false;

    if (! properties.has_exact_slot_requirement().is_indeterminate())
        if (properties.has_exact_slot_requirement().is_true() != f.has_exact_slot_requirement)
            return false;

    if (! properties.has_from_repository().is_indeterminate())
        if (properties.has_from_repository().is_true() != f.has_from_repository)
            return false;

    if (! properties.has_in_repository().is_indeterminate())
        if (properties.has_in_repository().is_true() != f.has_in_repository)
            return false;

    if (! properties.has_installable_to_path().is_indeterminate())
        if (properties.has_installable_to_path().is_true() != f.has_installable_to_path)
            return false;

    if (! properties.has_installable_to_repository().is_indeterminate())
        if (properties.has_installable_to_repository().is_true() != f.has_installable_to_repository)
            return false;

    if (! properties.has_installed_at_path().is_indeterminate())
        if (properties.has_installed_at_path().is_true() != f.has_installed_at_path)
            return false;

    if (! properties.has_key_requirements().is_indeterminate())
        if (properties.has_key_requirements().is_true() != f.has_key_requirements)
            return false;

    if (! properties.has_package().is_indeterminate())
        if (properties.has_package().is_true() != f.has_package)
            return false;

    if (! properties.has_package_name_part().is_indeterminate())
        if (properties.has_package_name_part().is_true() != f.has_package_name_part)
            return false;

    if (! properties.has_tag().is_indeterminate())
        if (properties.has_tag().is_true() != f.has_tag)
            return false;

    if (! properties.has_version_requirements().is_indeterminate())
        if (properties.has_version_requirements().is_true() != f.has_version_requirements)
            return false;

    return true;
}

