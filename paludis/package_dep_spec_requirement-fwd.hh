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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_REQUIREMENT_FWD_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_REQUIREMENT_FWD_HH 1

#include <paludis/util/pool-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/sequence-fwd.hh>
#include <iosfwd>
#include <memory>

namespace paludis
{
    class PackageDepSpecRequirement;

    class NameRequirement;
    typedef Pool<NameRequirement> NameRequirementPool;

    class PackageNamePartRequirement;
    typedef Pool<PackageNamePartRequirement> PackageNamePartRequirementPool;

    class CategoryNamePartRequirement;
    typedef Pool<CategoryNamePartRequirement> CategoryNamePartRequirementPool;

    class VersionRequirement;

    typedef Sequence<std::shared_ptr<const VersionRequirement> > VersionRequirementSequence;

    class InRepositoryRequirement;
    typedef Pool<InRepositoryRequirement> InRepositoryRequirementPool;

    class FromRepositoryRequirement;
    typedef Pool<FromRepositoryRequirement> FromRepositoryRequirementPool;

    class InstalledAtPathRequirement;
    typedef Pool<InstalledAtPathRequirement> InstalledAtPathRequirementPool;

    class InstallableToPathRequirement;
    typedef Pool<InstallableToPathRequirement> InstallableToPathRequirementPool;

    class InstallableToRepositoryRequirement;
    typedef Pool<InstallableToRepositoryRequirement> InstallableToRepositoryRequirementPool;

    class ExactSlotRequirement;
    typedef Pool<ExactSlotRequirement> ExactSlotRequirementPool;

    class AnySlotRequirement;
    typedef Pool<AnySlotRequirement> AnySlotRequirementPool;

    class KeyRequirement;
    typedef Pool<KeyRequirement> KeyRequirementPool;

    typedef Sequence<std::shared_ptr<const KeyRequirement> > KeyRequirementSequence;

    class ChoiceRequirement;

    typedef Sequence<std::shared_ptr<const ChoiceRequirement> > ChoiceRequirementSequence;

#include <paludis/package_dep_spec_requirement-se.hh>

}

#endif
