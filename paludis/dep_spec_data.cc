/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/dep_spec_data.hh>
#include <paludis/package_dep_spec_requirement.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/accept_visitor.hh>

#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/dep_spec_data-se.cc>

ConditionalDepSpecData::~ConditionalDepSpecData() = default;

namespace paludis
{
    template <>
    struct Imp<PackageDepSpecData>
    {
        std::shared_ptr<PackageDepSpecRequirementSequence> requirements;
        PackageDepSpecDataOptions options;

        Imp(const PackageDepSpecDataOptions & o) :
            requirements(std::make_shared<PackageDepSpecRequirementSequence>()),
            options(o)
        {
        }

        Imp(const PackageDepSpecData & other) :
            requirements(std::make_shared<PackageDepSpecRequirementSequence>()),
            options(other.options())
        {
            std::copy(other.requirements()->begin(), other.requirements()->end(), requirements->back_inserter());
        }
    };
}

PackageDepSpecData::PackageDepSpecData(const PackageDepSpecDataOptions & o) :
    _imp(o)
{
}

PackageDepSpecData::PackageDepSpecData(const PackageDepSpecData & o) :
    _imp(o)
{
}

PackageDepSpecData::~PackageDepSpecData() = default;

const std::shared_ptr<const PackageDepSpecRequirementSequence>
PackageDepSpecData::requirements() const
{
    return _imp->requirements;
}

const PackageDepSpecDataOptions
PackageDepSpecData::options() const
{
    return _imp->options;
}

namespace
{
    template <typename T_>
    struct Detect
    {
        bool visit(const T_ &) const
        {
            return true;
        }

        bool visit(const PackageDepSpecRequirement &) const
        {
            return false;
        }
    };
}

MutablePackageDepSpecData::MutablePackageDepSpecData(const PackageDepSpecDataOptions & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::MutablePackageDepSpecData(const PackageDepSpecData & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::MutablePackageDepSpecData(const MutablePackageDepSpecData & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::~MutablePackageDepSpecData() = default;

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_package(const QualifiedPackageName & name)
{
    _imp->requirements->push_front(NameRequirementPool::get_instance()->create(name));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_package()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<NameRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_package_name_part(const PackageNamePart & part)
{
    _imp->requirements->push_front(PackageNamePartRequirementPool::get_instance()->create(part));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_package_name_part()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<PackageNamePartRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_category_name_part(const CategoryNamePart & part)
{
    _imp->requirements->push_front(CategoryNamePartRequirementPool::get_instance()->create(part));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_category_name_part()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<CategoryNamePartRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_version(const VersionRequirementCombiner vc, const VersionOperator & vo, const VersionSpec & vs)
{
    auto r(std::make_shared<VersionRequirement>(vs, vo, vc));
    _imp->requirements->push_back(r);

    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_versions()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<VersionRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_exact_slot(const SlotName & n, const bool s)
{
    _imp->requirements->push_back(ExactSlotRequirementPool::get_instance()->create(n, s));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_exact_slot()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<ExactSlotRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_in_repository(const RepositoryName & s)
{
    _imp->requirements->push_back(InRepositoryRequirementPool::get_instance()->create(s));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_in_repository()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<InRepositoryRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_installable_to_path(const FSPath & s, const bool i)
{
    _imp->requirements->push_back(InstallableToPathRequirementPool::get_instance()->create(s, i));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_installable_to_path()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<InstallableToPathRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_installable_to_repository(const RepositoryName & n, const bool i)
{
    _imp->requirements->push_back(InstallableToRepositoryRequirementPool::get_instance()->create(n, i));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_installable_to_repository()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<InstallableToRepositoryRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_from_repository(const RepositoryName & n)
{
    _imp->requirements->push_back(FromRepositoryRequirementPool::get_instance()->create(n));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_from_repository()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<FromRepositoryRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_installed_at_path(const FSPath & s)
{
    _imp->requirements->push_back(InstalledAtPathRequirementPool::get_instance()->create(s));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_installed_at_path()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<InstalledAtPathRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_any_slot(const bool s)
{
    _imp->requirements->push_back(AnySlotRequirementPool::get_instance()->create(s));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_any_slot()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<AnySlotRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_choice(const std::shared_ptr<const ChoiceRequirement> & c)
{
    _imp->requirements->push_back(c);

    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_choices()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<ChoiceRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::require_key(const KeyRequirementKeyType t, const std::string & k, const KeyRequirementOperation o, const std::string & p)
{
    auto r(KeyRequirementPool::get_instance()->create(t, k, o, p));

    _imp->requirements->push_back(r);

    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unrequire_keys()
{
    auto r(std::make_shared<PackageDepSpecRequirementSequence>());
    for (auto u(_imp->requirements->begin()), u_end(_imp->requirements->end()) ;
            u != u_end ; ++u)
        if (! (*u)->accept_returning<bool>(Detect<KeyRequirement>()))
            r->push_back(*u);

    _imp->requirements = r;
    return *this;
}

MutablePackageDepSpecData::operator PackageDepSpec() const
{
    /* convoluted because it's private... */
    PackageDepSpecData * data(new MutablePackageDepSpecData(*this));
    return PackageDepSpec(std::shared_ptr<PackageDepSpecData>(data));
}

